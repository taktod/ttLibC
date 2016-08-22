/*
 * @file   mkvTag.c
 * @brief  mkv container support.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/09/04
 */

#include "mkvTag.h"
#include "type/simpleBlock.h"
#include "mkvReader.h"
#include "../../log.h"
#include "../../allocator.h"
#include "../../frame/video/h264.h"
#include "../../frame/audio/aac.h"
#include "../../frame/audio/vorbis.h"
#include <string.h>

ttLibC_MkvTag *ttLibC_MkvTag_make(
		ttLibC_MkvTag *prev_tag,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		ttLibC_Mkv_Type type) {
	ttLibC_MkvTag *tag = (ttLibC_MkvTag *)ttLibC_Container_make(
			(ttLibC_Container *)prev_tag,
			sizeof(union{
				ttLibC_MkvTag tag;
			}),
			containerType_mkv,
			data,
			data_size,
			non_copy_mode,
			pts,
			timebase);
	if(tag != NULL) {
		tag->inherit_super.type = type;
	}
	return tag;
}

bool ttLibC_Mkv_getFrame(ttLibC_Mkv *mkv, ttLibC_getFrameFunc callback, void *ptr) {
	switch(mkv->type) {
	case MkvType_SimpleBlock:
		// just now, only simpleBlock support to get frame.
		return ttLibC_SimpleBlock_getFrame((ttLibC_MkvTag *)mkv, callback, ptr);
	case MkvType_Block:
//		return ttLibC_Block_getFrame()
		return ttLibC_SimpleBlock_getFrame((ttLibC_MkvTag *)mkv, callback, ptr);
	default:
		return true;
	}
	return true;
}

/**
 * analyze frames in private data.
 * @param reader
 * @param track
 * @param callback
 * @param ptr
 * @note in the case of first reply of simple block, we will return private data information.
 * this code will be move to mkvTag.c
 */
void ttLibC_MkvTag_getPrivateDataFrame(
		ttLibC_MkvReader *reader,
		ttLibC_MkvTrack *track,
		ttLibC_getFrameFunc callback,
		void *ptr) {
	ttLibC_MkvReader_ *reader_ = (ttLibC_MkvReader_ *)reader;
	uint8_t *private_data = track->private_data;
	size_t private_data_size = track->private_data_size;
	switch(track->type) {
	case frameType_h264:
		{
			uint32_t size_length = 0;
			ttLibC_H264 *h264 = ttLibC_H264_analyzeAvccTag(NULL, private_data, private_data_size, &size_length);
			if(h264 == NULL) {
				ERR_PRINT("failed to analyze avccTag");
				reader_->error_number = 3;
				return;
			}
			track->frame = (ttLibC_Frame *)h264;
			track->frame->id = track->track_number;
			track->size_length = size_length;
			if(size_length < 3) {
				ERR_PRINT("avcc size is too small.");
				reader_->error_number = 1;
				return;
			}
			if(callback != NULL) {
				if(!callback(ptr, track->frame)) {
					reader_->error_number = 5;
				}
			}
		}
		break;
	case frameType_aac:
		{
			memcpy(&track->dsi_info, private_data, private_data_size);
			// try to callback data.
			ttLibC_Aac *aac = ttLibC_Aac_make(
					NULL,
					AacType_dsi,
					track->sample_rate,
					0,
					track->channel_num,
					private_data,
					private_data_size,
					true,
					0,
					reader_->timebase,
					track->dsi_info);
			if(aac != NULL) {
				aac->inherit_super.inherit_super.id = track->track_number;
				track->frame = (ttLibC_Frame *)aac;
				if(callback != NULL) {
					if(!callback(ptr, track->frame)) {
						reader_->error_number = 5;
					}
				}
			}
		}
		break;
	case frameType_vorbis:
		{
			uint32_t diff[4];
			diff[0] = private_data[0];
			diff[1] = private_data[1];
			diff[2] = private_data[2];
			diff[3] = private_data_size - 3 - diff[1] - diff[2];

			private_data += 3;
			uint32_t block0 = private_data[28] & 0x0F;
			uint32_t block1 = (private_data[28] >> 4) & 0x0F;
			track->block0 = (1 << block0);
			track->block1 = (1 << block1);
			ttLibC_Vorbis *vorbis = NULL;
			// identification frame.
			ttLibC_Vorbis *v = ttLibC_Vorbis_make(
					NULL,
					VorbisType_identification,
					track->sample_rate,
					0,
					track->channel_num,
					private_data,
					diff[1],
					true,
					reader_->pts,
					reader_->timebase);
			if(v == NULL) {
				ERR_PRINT("failed to get vorbis identification frame.");
				reader_->error_number = 5;
				return;
			}
			vorbis = v;
			vorbis->inherit_super.inherit_super.id = track->track_number;
			if(callback != NULL) {
				if(!callback(ptr, (ttLibC_Frame *)vorbis)) {
					reader_->error_number = 5;
				}
			}
			private_data += diff[1];
			// next, comment frame.
			v = ttLibC_Vorbis_make(
					vorbis,
					VorbisType_comment,
					track->sample_rate,
					0,
					track->channel_num,
					private_data,
					diff[2],
					true,
					reader_->pts,
					reader_->timebase);
			if(v == NULL) {
				ERR_PRINT("failed to get vorbis comment frame.");
				reader_->error_number = 5;
				ttLibC_Vorbis_close(&vorbis);
				return;
			}
			vorbis = v;
			vorbis->inherit_super.inherit_super.id = track->track_number;
			if(callback != NULL) {
				if(!callback(ptr, (ttLibC_Frame *)vorbis)) {
					reader_->error_number = 5;
				}
			}
			private_data += diff[2];
			// the last, setup frame.
			v = ttLibC_Vorbis_make(
					vorbis,
					VorbisType_setup,
					track->sample_rate,
					0,
					track->channel_num,
					private_data,
					diff[3],
					true,
					reader_->pts,
					reader_->timebase);
			if(v == NULL) {
				ERR_PRINT("failed to get vorbis comment frame.");
				reader_->error_number = 5;
				ttLibC_Vorbis_close(&vorbis);
				return;
			}
			vorbis = v;
			vorbis->inherit_super.inherit_super.id = track->track_number;
			if(callback != NULL) {
				if(!callback(ptr, (ttLibC_Frame *)vorbis)) {
					reader_->error_number = 5;
				}
			}
			ttLibC_Vorbis_close(&vorbis);
		}
		break;
	default:
		// if we need to do something, add here...
		break;
	}
}

void ttLibC_MkvTag_close(ttLibC_MkvTag **tag) {
	ttLibC_MkvTag *target = *tag;
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.inherit_super.type != containerType_mkv) {
		ERR_PRINT("container type is not mkv");
		return;
	}
	if(!target->inherit_super.inherit_super.is_non_copy) {
		ttLibC_free(target->inherit_super.inherit_super.data);
	}
	ttLibC_free(target);
	*tag = NULL;
}
