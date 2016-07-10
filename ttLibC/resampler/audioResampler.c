/*
 * @file   audioResampler.c
 * @brief  resample pcm data.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/21
 */

#include "audioResampler.h"
#include "../log.h"
#include "../allocator.h"
#include "../util/ioUtil.h"
#include <stdlib.h>

/**
 * support func for data convert. source is pcms16
 * @param l_data     result data left channel.
 * @param r_data     result data right channel.
 * @param step       step for each sample.
 * @param frame_type target frame type.
 * @param type       target detail type.(ttLibC_PcmS16_Type, ttLibC_PcmF32_type...)
 * @param src_frame  source frame.
 */
static void AudioResampler_convertFormatSrcPcmS16(
		uint8_t *l_data,
		uint8_t *r_data,
		uint32_t step,
		ttLibC_Frame_Type frame_type,
		uint32_t type,
		ttLibC_PcmS16 *src_frame) {
	int16_t *src_l_data = NULL;
	int16_t *src_r_data = NULL;
	uint32_t src_step = 0;
	switch(src_frame->type) {
	case PcmS16Type_bigEndian:
	default:
	case PcmS16Type_littleEndian:
		src_l_data = (int16_t *)src_frame->l_data;
		if(src_frame->inherit_super.channel_num == 2) {
			src_r_data = src_l_data + 1;
			src_step = 2;
		}
		else {
			src_step = 1;
		}
		break;
	case PcmS16Type_bigEndian_planar:
	case PcmS16Type_littleEndian_planar:
		src_l_data = (int16_t *)src_frame->l_data;
		if(src_frame->inherit_super.channel_num == 2) {
			src_r_data = (int16_t *)src_frame->r_data;
		}
		src_step = 1;
		break;
	}
	for(uint32_t i = 0;i < src_frame->inherit_super.sample_num;++ i) {
		int16_t l_val = 0;
		int16_t r_val = 0;
		switch(src_frame->type) {
		case PcmS16Type_bigEndian:
		case PcmS16Type_bigEndian_planar:
			l_val = be_int16_t(*src_l_data);
			if(src_r_data != NULL) {
				r_val = be_int16_t(*src_r_data);
			}
			break;
		case PcmS16Type_littleEndian:
		case PcmS16Type_littleEndian_planar:
			l_val = le_int16_t(*src_l_data);
			if(src_r_data != NULL) {
				r_val = le_int16_t(*src_r_data);
			}
			break;
		}
		src_l_data += src_step;
		if(src_r_data != NULL) {
			src_r_data += src_step;
		}
		if(r_data == NULL) {
			if(src_r_data != NULL) {
				// source:stereo output:monoral (get average.)
				l_val = (l_val + r_val) / 2;
			}
		}
		else {
			if(src_r_data == NULL) {
				// source:monoral output:stereo (apply same data for both channel.)
				r_val = l_val;
			}
		}
		switch(frame_type) {
		case frameType_pcmS16:
			{
				switch(type) {
				case PcmS16Type_bigEndian:
				case PcmS16Type_bigEndian_planar:
					*((int16_t *)l_data) = be_int16_t(l_val);
					l_data += step;
					if(r_data != NULL) {
						*((int16_t *)r_data) = be_int16_t(r_val);
						r_data += step;
					}
					break;
				case PcmS16Type_littleEndian:
				case PcmS16Type_littleEndian_planar:
					*((int16_t *)l_data) = le_int16_t(l_val);
					l_data += step;
					if(r_data != NULL) {
						*((int16_t *)r_data) = le_int16_t(r_val);
						r_data += step;
					}
					break;
				}
			}
			break;
		case frameType_pcmF32:
			{
				*((float *)l_data) = l_val / 32768.0f;
				l_data += step;
				if(r_data != NULL) {
					*((float *)r_data) = r_val / 32768.0f;
					r_data += step;
				}
			}
			break;
		default:
			break;
		}
	}
}

/**
 * support func for data convert. source is pcmf32
 * @param l_data     result data left channel.
 * @param r_data     result data right channel.
 * @param step       step for each sample.
 * @param frame_type target frame type.
 * @param type       target detail type.(ttLibC_PcmS16_Type, ttLibC_PcmF32_type...)
 * @param src_frame  source frame.
 */
static void AudioResampler_convertFormatSrcPcmF32(
		uint8_t *l_data,
		uint8_t *r_data,
		uint32_t step,
		ttLibC_Frame_Type frame_type,
		uint32_t type,
		ttLibC_PcmF32 *src_frame) {
	float *src_l_data = NULL;
	float *src_r_data = NULL;
	uint32_t src_step = 0;
	switch(src_frame->type) {
	default:
	case PcmF32Type_interleave:
		src_l_data = (float *)src_frame->l_data;
		if(src_frame->inherit_super.channel_num == 2) {
			src_r_data = src_l_data + 1;
			src_step = 2;
		}
		else {
			src_step = 1;
		}
		break;
	case PcmF32Type_planar:
		src_l_data = (float *)src_frame->l_data;
		if(src_frame->inherit_super.channel_num == 2) {
			src_r_data = (float *)src_frame->r_data;
		}
		src_step = 1;
		break;
	}
	for(uint32_t i = 0;i < src_frame->inherit_super.sample_num; ++ i) {
		float l_val = *src_l_data;
		float r_val = 0;
		src_l_data += src_step;
		if(src_r_data != NULL) {
			r_val = *src_r_data;
			src_r_data += src_step;
		}
		if(r_data == NULL) {
			if(src_r_data != NULL) {
				l_val = (l_val + r_val) / 2;
			}
		}
		else {
			if(src_r_data == NULL) {
				r_val = l_val;
			}
		}
		switch(frame_type) {
		case frameType_pcmS16:
			{
				int16_t int_l_val = (int16_t)(l_val * 32767);
				int16_t int_r_val = (int16_t)(r_val * 32767);
				switch(type) {
				case PcmS16Type_bigEndian:
				case PcmS16Type_bigEndian_planar:
					*((int16_t *)l_data) = be_int16_t(int_l_val);
					l_data += step;
					if(r_data != NULL) {
						*((int16_t *)r_data) = be_int16_t(int_r_val);
						r_data += step;
					}
					break;
				case PcmS16Type_littleEndian:
				case PcmS16Type_littleEndian_planar:
					*((int16_t *)l_data) = le_int16_t(int_l_val);
					l_data += step;
					if(r_data != NULL) {
						*((int16_t *)r_data) = le_int16_t(int_r_val);
						r_data += step;
					}
					break;
				}
			}
			break;
		case frameType_pcmF32:
			{
				*((float *)l_data) = l_val;
				l_data += step;
				if(r_data != NULL) {
					*((float *)r_data) = r_val;
					r_data += step;
				}
			}
			break;
		default:
			break;
		}
	}
}

/**
 * convert ttLibC_Audio frame
 * @param prev_frame  reuse frame
 * @param frame_type  target frame
 * @param type        target frame type
 * @param channel_num target frame channel 1:monoral 2:stereo
 * @param src_frame   source frame
 */
ttLibC_Audio *ttLibC_AudioResampler_convertFormat(
		ttLibC_Audio *prev_frame,
		ttLibC_Frame_Type frame_type,
		uint32_t type,
		uint32_t channel_num,
		ttLibC_Audio *src_frame) {
	ttLibC_Audio *target_frame = prev_frame;
	if(src_frame == NULL) {
		// input data is null, nothing to do.
		return NULL;
	}
	uint32_t sample_rate = src_frame->sample_rate;
	uint32_t sample_num  = src_frame->sample_num;
	// channel_num is settled by param.
//	uint32_t channel_num = src_frame->channel_num;
	if(sample_num == 0) {
		ERR_PRINT("source data doesn't have any sample.");
		return NULL;
	}
	size_t data_size = 0;
	size_t buffer_size = 0;
	uint8_t *data = NULL;
	bool alloc_flag = false;
	switch(frame_type) {
	case frameType_pcmS16:
		data_size = 2 * sample_num * channel_num;
		break;
	case frameType_pcmF32:
		data_size = 4 * sample_num * channel_num;
		break;
	default:
		ERR_PRINT("unknown pcm type.%d", frame_type);
		return NULL;
	}
	buffer_size = data_size;
	// check input frame type.
	switch(src_frame->inherit_super.type) {
	case frameType_pcmF32:
	case frameType_pcmS16:
		break;
	default:
		ERR_PRINT("unknown input pcm type.%d", frame_type);
		return NULL;
	}
	if(target_frame != NULL) {
		if(target_frame->inherit_super.type != frame_type) {
			// frame type is different, clear them.
			ttLibC_Audio_close(&target_frame);
		}
		else {
			// possible to reuse prev data?
			if(!target_frame->inherit_super.is_non_copy) {
				// check the data size.
				if(target_frame->inherit_super.data_size >= data_size) {
					data = target_frame->inherit_super.data;
					data_size = target_frame->inherit_super.data_size;
				}
				else {
					ttLibC_free(target_frame->inherit_super.data);
					target_frame->inherit_super.data = NULL;
				}
			}
			// force to put non copy.
			target_frame->inherit_super.is_non_copy = true;
		}
	}
	// if data is null, we need to allocate.
	if(data == NULL) {
		data = ttLibC_malloc(data_size);
		if(data == NULL) {
			return NULL;
		}
		alloc_flag = true;
	}
	uint8_t *l_data = NULL;
	uint8_t *r_data = NULL;
	uint32_t step = 0;
	switch(frame_type) {
	default:
		if(alloc_flag) {
			ttLibC_free(data);
		}
		return NULL;
	case frameType_pcmS16:
		switch(type) {
		case PcmS16Type_bigEndian:
		default:
		case PcmS16Type_littleEndian:
			l_data = data;
			if(channel_num == 2) {
				r_data = l_data + 2;
				step = 4;
			}
			else {
				step = 2;
			}
			break;
		case PcmS16Type_bigEndian_planar:
		case PcmS16Type_littleEndian_planar:
			l_data = data;
			if(channel_num == 2) {
				r_data = l_data + (buffer_size >> 1);
			}
			step = 2;
		}
		break;
	case frameType_pcmF32:
		switch(type) {
		default:
		case PcmF32Type_interleave:
			l_data = data;
			if(channel_num == 2) {
				r_data = l_data + 4;
				step = 8;
			}
			else {
				step = 4;
			}
			break;
		case PcmF32Type_planar:
			l_data = data;
			if(channel_num == 2) {
				r_data = l_data + (buffer_size >> 1);
			}
			step = 4;
			break;
		}
		break;
	}
	switch(src_frame->inherit_super.type) {
	case frameType_pcmF32:
		AudioResampler_convertFormatSrcPcmF32(
				l_data, r_data, step, frame_type, type, (ttLibC_PcmF32 *)src_frame);
		break;
	default:
	case frameType_pcmS16:
		AudioResampler_convertFormatSrcPcmS16(
				l_data, r_data, step, frame_type, type, (ttLibC_PcmS16 *)src_frame);
		break;
	}
	// now data should be ready, just make frame and reply.
	switch(frame_type) {
	default:
		break;
	case frameType_pcmS16:
		{
			ttLibC_PcmS16 *pcms16 = NULL;
			uint32_t l_stride = 0;
			uint32_t r_stride = 0;
			switch(type) {
			case PcmS16Type_bigEndian:
			case PcmS16Type_littleEndian:
//				l_data = data;
				l_stride = sample_num * 2 * channel_num;
				break;
			case PcmS16Type_bigEndian_planar:
			case PcmS16Type_littleEndian_planar:
//				l_data = data;
				l_stride = sample_num * 2;
				if(channel_num == 2) {
//					r_data = data + l_stride;
					r_stride = l_stride;
				}
				break;
			}
			pcms16 = ttLibC_PcmS16_make(
					(ttLibC_PcmS16 *)target_frame,
					type,
					sample_rate,
					sample_num,
					channel_num,
					data,
					data_size,
					l_data,
					l_stride,
					r_data,
					r_stride,
					true,
					src_frame->inherit_super.pts,
					src_frame->inherit_super.timebase);
			if(pcms16 == NULL) {
				break;
			}
			pcms16->inherit_super.inherit_super.is_non_copy = false;
			return (ttLibC_Audio *)pcms16;
		}
	case frameType_pcmF32:
		{
			ttLibC_PcmF32 *pcmf32 = NULL;
			uint32_t l_stride = 0;
			uint32_t r_stride = 0;
			switch(type) {
			case PcmF32Type_interleave:
				l_stride = sample_num * 4 * channel_num;
				break;
			case PcmF32Type_planar:
				l_stride = sample_num * 4;
				if(channel_num == 2) {
					r_stride = l_stride;
				}
				break;
			}
			pcmf32 = ttLibC_PcmF32_make(
					(ttLibC_PcmF32 *)target_frame,
					type,
					sample_rate,
					sample_num,
					channel_num,
					data,
					data_size,
					l_data,
					l_stride,
					r_data,
					r_stride,
					true,
					src_frame->inherit_super.pts,
					src_frame->inherit_super.timebase);
			if(pcmf32 == NULL) {
				break;
			}
			pcmf32->inherit_super.inherit_super.is_non_copy = false;
			return (ttLibC_Audio *)pcmf32;
		}
	}
	if(alloc_flag) {
		ttLibC_free(data);
	}
	return NULL;
}

/**
 * make ttLibC_PcmF32 from ttLibC_PcmS16.
 * @param prev_frame reuse frame.
 * @param type       pcmf32 type.
 * @param src_frame  src pcms16 frame.
 */
ttLibC_PcmF32 *ttLibC_AudioResampler_makePcmF32FromPcmS16(
		ttLibC_PcmF32 *prev_frame,
		ttLibC_PcmF32_Type type,
		ttLibC_PcmS16 *src_frame) {
	return (ttLibC_PcmF32 *)ttLibC_AudioResampler_convertFormat(
			(ttLibC_Audio *)prev_frame,
			frameType_pcmF32,
			type,
			src_frame->inherit_super.channel_num,
			(ttLibC_Audio *)src_frame);
}

/**
 * make ttLibC_PcmS16 from ttLibC_PcmF32.
 * @param prev_frame reuse frame.
 * @param type       pcms16 type.
 * @param src_frame  src pcmf32 frame.
 */
ttLibC_PcmS16 *ttLibC_AudioResampler_makePcmS16FromPcmF32(
		ttLibC_PcmS16 *prev_frame,
		ttLibC_PcmS16_Type type,
		ttLibC_PcmF32 *src_frame) {
	return (ttLibC_PcmS16 *)ttLibC_AudioResampler_convertFormat(
			(ttLibC_Audio *)prev_frame,
			frameType_pcmS16,
			type,
			src_frame->inherit_super.channel_num,
			(ttLibC_Audio *)src_frame);
}

