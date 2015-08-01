/*
 * @file   speex.c
 * @brief  speex frame information
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/28
 */

#include "speex.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../log.h"

typedef ttLibC_Frame_Audio_Speex ttLibC_Speex_;

/*
 * make speex frame
 * @param prev_frame    reuse frame.
 * @param type          type of speex
 * @param sample_rate   sample rate of data.
 * @param sample_num    sample number of data.
 * @param channel_num   channel number of data. 1:monoral 2:stereo
 * @param data          speex data
 * @param data_size     speex data size
 * @param non_copy_mode true:hold the data pointer. false:data will copy.
 * @param pts           pts for speex data.
 * @param timebase      timebase number for pts.
 * @return speex object.
 */
ttLibC_Speex *ttLibC_Speex_make(
		ttLibC_Speex *prev_frame,
		ttLibC_Speex_Type type,
		uint32_t sample_rate,
		uint32_t sample_num,
		uint32_t channel_num,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	ttLibC_Speex_ *speex = (ttLibC_Speex_ *)prev_frame;
	size_t data_size_ = data_size;
	size_t buffer_size_ = data_size;
	switch(type) {
	case SpeexType_comment:
	case SpeexType_header:
	case SpeexType_frame:
		break;
	default:
		ERR_PRINT("unknown speex type:%d", type);
		return NULL;
	}
	if(speex == NULL) {
		speex = malloc(sizeof(ttLibC_Speex_));
		if(speex == NULL) {
			ERR_PRINT("failed to allocate memory for speex frame.");
			return NULL;
		}
		speex->inherit_super.inherit_super.data = NULL;
	}
	else {
		if(!speex->inherit_super.inherit_super.is_non_copy) {
			if(non_copy_mode || speex->inherit_super.inherit_super.data_size < data_size) {
				free(speex->inherit_super.inherit_super.data);
				speex->inherit_super.inherit_super.data = NULL;
			}
			else {
				data_size_ = speex->inherit_super.inherit_super.data_size;
			}
		}
	}
	speex->type                                    = type;
	speex->inherit_super.channel_num               = channel_num;
	speex->inherit_super.sample_rate               = sample_rate;
	speex->inherit_super.sample_num                = sample_num;
	speex->inherit_super.inherit_super.buffer_size = buffer_size_;
	speex->inherit_super.inherit_super.data_size   = data_size_;
	speex->inherit_super.inherit_super.is_non_copy = non_copy_mode;
	speex->inherit_super.inherit_super.pts         = pts;
	speex->inherit_super.inherit_super.timebase    = timebase;
	speex->inherit_super.inherit_super.type        = frameType_speex;
	if(non_copy_mode) {
		speex->inherit_super.inherit_super.data = data;
	}
	else {
		if(speex->inherit_super.inherit_super.data == NULL) {
			speex->inherit_super.inherit_super.data = malloc(data_size);
			if(speex->inherit_super.inherit_super.data == NULL) {
				ERR_PRINT("failed to allocate memory for data.");
				if(prev_frame == NULL) {
					free(speex);
				}
				return NULL;
			}
		}
		memcpy(speex->inherit_super.inherit_super.data, data, data_size);
	}
	return (ttLibC_Speex *)speex;
}

/*
 * make speex frame from byte data.
 * @param prev_frame reuse speex frame.
 * @param data       speex binary data.
 * @param data_size  data size
 * @param pts        pts for speex frame.
 * @param timebase   timebase for speex frame.
 * @return speex object.
 */
ttLibC_Speex *ttLibC_Speex_makeFrame(
		ttLibC_Speex *prev_frame,
		void *data,
		size_t data_size,
		uint64_t pts,
		uint32_t timebase) {
	// TODO make this.
	return NULL;
}

/*
 * close frame
 * @param frame
 */
void ttLibC_Speex_close(ttLibC_Speex **frame) {
	ttLibC_Speex_ *target = (ttLibC_Speex_ *)*frame;
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.inherit_super.type != frameType_speex) {
		ERR_PRINT("found non speex frame in speex_close.");
		return;
	}
	if(!target->inherit_super.inherit_super.is_non_copy) {
		free(target->inherit_super.inherit_super.data);
	}
	free(target);
	*frame = NULL;

}
