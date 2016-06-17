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

uint32_t ttLibC_Opus_getChannelNum(void *data, size_t data_size) {
	if(data_size < 1) {
		ERR_PRINT("invalid opus data.");
		return 0;
	}
	uint8_t *u = (uint8_t *)data;
	return (u[0] & 0x04) ? 2 : 1;
}

uint32_t ttLibC_Opus_getSampleRate(void *data, size_t data_size) {
	return 48000;
}

uint32_t ttLibC_Opus_getNbFrameCount(void *data, size_t data_size) {
	if(data_size < 1) {
		ERR_PRINT("invalid opus data.");
		return 0;
	}
	uint8_t *u = (uint8_t *)data;
	int count = (u[0] & 0x03);
	if(count == 0) {
		return 1;
	}
	else if(count != 3) {
		return 2;
	}
	if(data_size < 2) {
		ERR_PRINT("invalid opus data.");
		return 0;
	}
	return u[1] & 0x3F;
}

uint32_t ttLibC_Opus_getSampleNum(void *data, size_t data_size) {
	uint32_t nbFrameCount = ttLibC_Opus_getNbFrameCount(data, data_size);
	if(nbFrameCount == 0) {
		return 0;
	}
	uint8_t *u = (uint8_t *)data;
	uint8_t TOCConfig = (u[0] >> 3) & 0x1F;
	switch(TOCConfig) {
	case 0x00:case 0x04:case 0x08:case 0x0C:case 0x0E:
		return 0.01 * ttLibC_Opus_getSampleRate(data, data_size) * nbFrameCount;
	case 0x01:case 0x05:case 0x09:case 0x0D:case 0x0F:
		return 0.02 * ttLibC_Opus_getSampleRate(data, data_size) * nbFrameCount;
	case 0x02:case 0x06:case 0x0A:
		return 0.04 * ttLibC_Opus_getSampleRate(data, data_size) * nbFrameCount;
	case 0x03:case 0x07:case 0x0B:
		return 0.06 * ttLibC_Opus_getSampleRate(data, data_size) * nbFrameCount;

	case 0x10:case 0x14:case 0x18:case 0x1C:
		return 0.0025 * ttLibC_Opus_getSampleRate(data, data_size) * nbFrameCount;
	case 0x11:case 0x15:case 0x19:case 0x1D:
		return 0.005 * ttLibC_Opus_getSampleRate(data, data_size) * nbFrameCount;
	case 0x12:case 0x16:case 0x1A:case 0x1E:
		return 0.01 * ttLibC_Opus_getSampleRate(data, data_size) * nbFrameCount;
	case 0x13:case 0x17:case 0x1B:case 0x1F:
		return 0.02 * ttLibC_Opus_getSampleRate(data, data_size) * nbFrameCount;
	default: /* no way to be here. */
		return 0;
	}
}

/**
 * make opus frame from byte data.
 * @param prev_frame reuse opus frame.
 * @param data       opus binary data.
 * @param data_size  data size
 * @param pts        pts for opus frame.
 * @param timebase   timebase for opus frame.
 */
ttLibC_Opus *ttLibC_Opus_makeFrame(
		ttLibC_Opus *prev_frame,
		void *data,
		size_t data_size,
		uint64_t pts,
		uint32_t timebase) {
	// header is not supported.
	uint32_t channel_num = ttLibC_Opus_getChannelNum(data, data_size);
	uint32_t sample_rate = ttLibC_Opus_getSampleRate(data, data_size);
	uint32_t sample_num = ttLibC_Opus_getSampleNum(data, data_size);
	if(channel_num == 0 || sample_rate == 0 || sample_num == 0) {
		ERR_PRINT("failed to get opus frame information. corrupt.");
		return NULL;
	}
	return ttLibC_Opus_make(prev_frame,
			OpusType_frame,
			sample_rate,
			sample_num,
			channel_num,
			data,
			data_size,
			true,
			pts,
			timebase);
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

