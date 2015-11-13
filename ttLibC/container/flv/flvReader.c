/*
 * @file   flvReader.c
 * @brief  flvTag reader from binary data.
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

	// first header has not size but data.
	reader->type        = FlvType_header;
	reader->status      = body;
	reader->target_size = 13;

	reader->tmp_buffer = ttLibC_DynamicBuffer_make();
	return (ttLibC_FlvReader *)reader;
}

static bool FlvReader_read(
		ttLibC_FlvReader_ *reader,
		uint8_t *buffer,
		size_t buffer_size,
		ttLibC_FlvReadFunc callback,
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
		ttLibC_FlvReadFunc callback,
		void *ptr) {
	if(reader == NULL) {
		return false;
	}
	ttLibC_FlvReader_ *reader_ = (ttLibC_FlvReader_ *)reader;
	ttLibC_DynamicBuffer_append(reader_->tmp_buffer, (uint8_t *)data, data_size);
	do {
		uint8_t *buffer = ttLibC_DynamicBuffer_refData(reader_->tmp_buffer);
		size_t left_size = ttLibC_DynamicBuffer_refSize(reader_->tmp_buffer);
		if(reader_->target_size > ttLibC_DynamicBuffer_refSize(reader_->tmp_buffer)) {
			ttLibC_DynamicBuffer_clear(reader_->tmp_buffer);
			return true; // continue
		}
		switch(reader_->status) {
		case size:
			reader_->type = *buffer;
			uint32_t tag_size = be_uint32_t(*((uint32_t *)buffer)) & 0x00FFFFFF;
			reader_->status = body;
			reader_->target_size = tag_size + 11 + 4;
			break;
		default:
		case body:
			if(!FlvReader_read(reader_, buffer, reader_->target_size, callback, ptr)) {
				ttLibC_DynamicBuffer_clear(reader_->tmp_buffer);
				return false;
			}
			ttLibC_DynamicBuffer_markAsRead(reader_->tmp_buffer, reader_->target_size);
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
	ttLibC_DynamicBuffer_close(&target->tmp_buffer);
	ttLibC_free(target);
	*reader = NULL;
}
