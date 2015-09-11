/*
 * @file   opus.c
 * @brief  opus frame information
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/01
 */

#include "opus.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../log.h"
#include "../../allocator.h"

typedef ttLibC_Frame_Audio_Opus ttLibC_Opus_;

/*
 * make opus frame
 * @param prev_frame    reuse frame.
 * @param type          type of opus
 * @param sample_rate   sample rate of data.
 * @param sample_num    sample number of data.
 * @param channel_num   channel number of data. 1:monoral 2:stereo
 * @param data          opus data
 * @param data_size     opus data size
 * @param non_copy_mode true:hold the data pointer. false:data will copy.
 * @param pts           pts for opus data.
 * @param timebase      timebase number for pts.
 * @return opus object.
 */
ttLibC_Opus *ttLibC_Opus_make(
		ttLibC_Opus *prev_frame,
		ttLibC_Opus_Type type,
		uint32_t sample_rate,
		uint32_t sample_num,
		uint32_t channel_num,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	ttLibC_Opus_ *opus = (ttLibC_Opus_ *)prev_frame;
	size_t data_size_ = data_size;
	size_t buffer_size_ = data_size;
	switch(type) {
	case OpusType_comment:
	case OpusType_header:
	case OpusType_frame:
		break;
	default:
		ERR_PRINT("unknown opus type:%d", type);
		return NULL;
	}
	if(opus == NULL) {
		opus = ttLibC_malloc(sizeof(ttLibC_Opus_));
		if(opus == NULL) {
			ERR_PRINT("failed to allocate memory for opus frame.");
			return NULL;
		}
		opus->inherit_super.inherit_super.data = NULL;
	}
	else {
		if(!opus->inherit_super.inherit_super.is_non_copy) {
			if(non_copy_mode || opus->inherit_super.inherit_super.data_size < data_size) {
				ttLibC_free(opus->inherit_super.inherit_super.data);
				opus->inherit_super.inherit_super.data = NULL;
			}
			else {
				data_size_ = opus->inherit_super.inherit_super.data_size;
			}
		}
	}
	opus->type                                    = type;
	opus->inherit_super.channel_num               = channel_num;
	opus->inherit_super.sample_rate               = sample_rate;
	opus->inherit_super.sample_num                = sample_num;
	opus->inherit_super.inherit_super.buffer_size = buffer_size_;
	opus->inherit_super.inherit_super.data_size   = data_size_;
	opus->inherit_super.inherit_super.is_non_copy = non_copy_mode;
	opus->inherit_super.inherit_super.pts         = pts;
	opus->inherit_super.inherit_super.timebase    = timebase;
	opus->inherit_super.inherit_super.type        = frameType_opus;
	if(non_copy_mode) {
		opus->inherit_super.inherit_super.data = data;
	}
	else {
		if(opus->inherit_super.inherit_super.data == NULL) {
			opus->inherit_super.inherit_super.data = ttLibC_malloc(data_size);
			if(opus->inherit_super.inherit_super.data == NULL) {
				ERR_PRINT("failed to allocate memory for data.");
				if(prev_frame == NULL) {
					ttLibC_free(opus);
				}
				return NULL;
			}
		}
		memcpy(opus->inherit_super.inherit_super.data, data, data_size);
	}
	return (ttLibC_Opus *)opus;
}

ttLibC_Opus *ttLibC_Opus_makeFrame(
		) {
	return NULL;
}

/*
 * close frame
 * @param frame
 */
void ttLibC_Opus_close(ttLibC_Opus **frame) {
	ttLibC_Opus_ *target = (ttLibC_Opus_ *)*frame;
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.inherit_super.type != frameType_opus) {
		ERR_PRINT("found non opus frame in opus_close.");
		return;
	}
	if(!target->inherit_super.inherit_super.is_non_copy) {
		ttLibC_free(target->inherit_super.inherit_super.data);
	}
	ttLibC_free(target);
	*frame = NULL;
}

