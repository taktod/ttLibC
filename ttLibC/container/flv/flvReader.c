/*
 * @file   flvReader.c
 * @brief  
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/12
 */

#include "flvReader.h"

#include "../../log.h"
#include "../../allocator.h"
#include "../../util/hexUtil.h"
#include "../../util/ioUtil.h"

#include <stdlib.h>
#include <string.h>

ttLibC_FlvReader *ttLibC_FlvReader_make() {
	ttLibC_FlvReader_ *reader = (ttLibC_FlvReader_ *)ttLibC_ContainerReader_make(
			containerType_flv,
			sizeof(ttLibC_FlvReader_));

	reader->flv_tag   = NULL;
	reader->audio_tag = NULL;
	reader->video_tag = NULL;

	reader->type        = FlvType_header;
	reader->status      = body;
	reader->target_size = 13;

	reader->tmp_buffer_size     = 65536;
	reader->tmp_buffer          = ttLibC_malloc(reader->tmp_buffer_size);
	reader->tmp_buffer_next_pos = 0;
	return (ttLibC_FlvReader *)reader;
}

static bool FlvReader_read(
		ttLibC_FlvReader_ *reader,
		uint8_t *buffer,
		size_t buffer_size,
		ttLibC_FlvReaderFunc callback,
		void *ptr) {
	ttLibC_FlvTag *tag = NULL;
	switch(reader->type) {
	case FlvType_audio:
		tag = (ttLibC_FlvTag *)ttLibC_FlvAudioTag_getTag(
				(ttLibC_FlvTag *)reader->audio_tag,
				buffer,
				buffer_size);
		if(tag == NULL) {
			ERR_PRINT("failed to get flv audio tag.");
			return false;
		}
		reader->audio_tag = (ttLibC_FlvAudioTag *)tag;
		return callback(ptr, (ttLibC_Flv *)reader->audio_tag);
	default:
	case FlvType_header:
		tag = (ttLibC_FlvTag *)ttLibC_FlvHeaderTag_getTag(
				reader->flv_tag,
				buffer,
				buffer_size);
		if(tag == NULL) {
			ERR_PRINT("failed to get flv header tag.");
			return false;
		}
		reader->flv_tag = tag;
		reader->inherit_super.has_audio = ((ttLibC_FlvHeaderTag *)tag)->has_audio;
		reader->inherit_super.has_video = ((ttLibC_FlvHeaderTag *)tag)->has_video;
		return callback(ptr, (ttLibC_Flv *)reader->flv_tag);
	case FlvType_meta:
		tag = (ttLibC_FlvTag *)ttLibC_FlvMetaTag_getTag(
				reader->flv_tag,
				buffer,
				buffer_size);
		if(tag == NULL) {
			ERR_PRINT("failed to get flv meta tag.");
			return false;
		}
		reader->flv_tag = tag;
		return callback(ptr, (ttLibC_Flv *)reader->flv_tag);
	case FlvType_video:
		tag = (ttLibC_FlvTag *)ttLibC_FlvVideoTag_getTag(
				(ttLibC_FlvTag *)reader->video_tag,
				buffer,
				buffer_size);
		if(tag == NULL) {
			ERR_PRINT("failed to get flv video tag.");
			return false;
		}
		reader->video_tag = (ttLibC_FlvVideoTag *)tag;
		return callback(ptr, (ttLibC_Flv *)reader->video_tag);
	}
}

bool ttLibC_FlvReader_read(
		ttLibC_FlvReader *reader,
		void *data,
		size_t data_size,
		ttLibC_FlvReaderFunc callback,
		void *ptr) {
	ttLibC_FlvReader_ *reader_ = (ttLibC_FlvReader_ *)reader;
	if(reader == NULL) {
		return false;
	}
	uint8_t *buffer = data;
	size_t left_size = data_size;
	if(reader_->tmp_buffer_next_pos != 0) {
		// 必要分コピーしておきます。
		size_t copy_size = reader_->target_size - reader_->tmp_buffer_next_pos;
		memcpy(reader_->tmp_buffer + reader_->tmp_buffer_next_pos, buffer, copy_size);
		buffer += copy_size;
		left_size -= copy_size;
		if(reader_->status == size) {
			// サイズ状態なら、サイズ分のデータを読み込んでなんとかする。
			reader_->type = *reader_->tmp_buffer;
			uint32_t tag_size = be_uint32_t(*((uint32_t *)reader_->tmp_buffer)) & 0x00FFFFFF;

			reader_->status = body;
			reader_->target_size = tag_size + 11 + 4;

			// body用のデータを読み込む必要あり。
			if(reader_->tmp_buffer_size < reader_->target_size) {
				LOG_PRINT("tmp buffer is too small.(rarely occured I think.):%zx", reader_->target_size);
				return false;
			}
			// データが足りるのでデータをコピーしないといけない。
			copy_size = reader_->target_size - 4;
			memcpy(reader_->tmp_buffer + 4, buffer, copy_size);
			buffer += copy_size;
			left_size -= copy_size;
		}
		if(!FlvReader_read(reader_, reader_->tmp_buffer, reader_->target_size, callback, ptr)) {
			return false;
		}
		reader_->status = size;
		reader_->target_size = 4;
	}
	do {
		if(reader_->target_size > left_size) {
			if(reader_->tmp_buffer_size < reader_->target_size) {
				// tmp_bufferの内容をallocしなおしてサイズを大きくしないと足りなくなります。
				LOG_PRINT("tmp buffer is too small.(rarely occured I think.):%zx", reader_->target_size);
				return false;
			}
			memcpy(reader_->tmp_buffer, buffer, left_size);
			reader_->tmp_buffer_next_pos = left_size;
			// コピーしておいとく。
			return true;
		}
		switch(reader_->status) {
		case size:
			// こちら側でデータが足りなくなった場合は・・・こまったことになる。
			reader_->type = *buffer;
			// タグサイズ取得
			uint32_t tag_size = be_uint32_t(*((uint32_t *)buffer)) & 0x00FFFFFF;
			reader_->status = body;
			reader_->target_size = tag_size + 11 + 4;
			break;
		default:
		case body:
			if(!FlvReader_read(reader_, buffer, reader_->target_size, callback, ptr)) {
				return false;
			}
			buffer += reader_->target_size;
			left_size -= reader_->target_size;

			reader_->status = size;
			reader_->target_size = 4;
			break;
		}
	} while(true);
}

void ttLibC_FlvReader_close(ttLibC_FlvReader **reader) {
	ttLibC_FlvReader_ *target = (ttLibC_FlvReader_ *)*reader;
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.inherit_super.type != containerType_flv) {
		ERR_PRINT("this reader is not flv reader.");
		return;
	}
	ttLibC_FlvTag_close(&target->flv_tag);
	ttLibC_FlvTag_close((ttLibC_FlvTag **)&target->audio_tag);
	ttLibC_FlvTag_close((ttLibC_FlvTag **)&target->video_tag);
	if(target->tmp_buffer != NULL) {
		ttLibC_free(target->tmp_buffer);
	}
	ttLibC_free(target);
	*reader = NULL;
}
