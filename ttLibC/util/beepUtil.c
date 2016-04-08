/*
 * @file   beepUtil.c
 * @brief  library for beep sound.(pcms16 only.)
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/20
 */

#include <stdlib.h>
#include <math.h>
#include "beepUtil.h"
#include "../log.h"
#include "../allocator.h"

/*
 * make beep generator
 * @param type        pcms16 type information.
 * @param target_Hz   target Hz 440 = "A" sound.
 * @param sample_rate sample_rate for generated beep.
 * @param channel_num channel_num for generated beep. 1:monoral 2:stereo
 */
ttLibC_BeepGenerator *ttLibC_BeepGenerator_make(
		ttLibC_PcmS16_Type type,
		uint32_t target_Hz,
		uint32_t sample_rate,
		uint32_t channel_num) {
	ttLibC_BeepGenerator *generator = (ttLibC_BeepGenerator *)ttLibC_malloc(sizeof(ttLibC_BeepGenerator));
	if(generator == NULL) {
		ERR_PRINT("failed to allocate memory for beepGenerator.");
		return NULL;
	}
	generator->type        = type;
	generator->sample_rate = sample_rate;
	generator->channel_num = channel_num;
	generator->target_Hz   = target_Hz;
	generator->amplitude   = 32767;
	generator->pos = 0;
	return generator;
}

/**
 * make beep sound.
 * @param generator  generator object.
 * @param prev_frame reuse frame
 * @param mili_sec   length of pcmframe in mili sec.
 */
ttLibC_PcmS16 *ttLibC_BeepGenerator_makeBeepByMiliSec(
		ttLibC_BeepGenerator *generator,
		ttLibC_PcmS16 *prev_frame,
		uint32_t mili_sec) {
	if(generator == NULL) {
		ERR_PRINT("generator is NULL");
		return NULL;
	}
	uint32_t target_sample_num = (uint32_t)(1L * mili_sec * generator->sample_rate / 1000);
	return ttLibC_BeepGenerator_makeBeepBySampleNum(generator, prev_frame, target_sample_num);
}

/*
 * make beep sound
 * @param generator  generator object.
 * @param prev_frame reuse frame.
 * @param sample_num target sample num.
 */
ttLibC_PcmS16 *ttLibC_BeepGenerator_makeBeepBySampleNum(
		ttLibC_BeepGenerator *generator,
		ttLibC_PcmS16 *prev_frame,
		uint32_t sample_num) {
	if(generator == NULL) {
		ERR_PRINT("generator is NULL");
		return NULL;
	}
	ttLibC_PcmS16 *pcms16 = prev_frame;
	size_t data_size = sample_num * generator->channel_num * 2;
	void *data = NULL;
	bool alloc_flag = false;
	if(pcms16 != NULL) {
		if(!pcms16->inherit_super.inherit_super.is_non_copy) {
			if(pcms16->inherit_super.inherit_super.data_size >= data_size) {
				data = pcms16->inherit_super.inherit_super.data;
				data_size = pcms16->inherit_super.inherit_super.data_size;
			}
			else {
				ttLibC_free(pcms16->inherit_super.inherit_super.data);
			}
		}
		if(data == NULL) {
			pcms16->inherit_super.inherit_super.data = NULL;
			pcms16->inherit_super.inherit_super.data_size = 0;
		}
		pcms16->inherit_super.inherit_super.is_non_copy = true;
	}
	if(data == NULL) {
		data = ttLibC_malloc(data_size);
		alloc_flag = true;
	}
	// generator beep sound.
	uint8_t *left_u_data = NULL;
	uint8_t *left_l_data = NULL;
	uint8_t *right_u_data = NULL;
	uint8_t *right_l_data = NULL;
	uint32_t step = 2;
	switch(generator->type) {
	case PcmS16Type_bigEndian:
		left_u_data = data;
		left_l_data = data + 1;
		if(generator->channel_num == 2) {
			right_u_data = data + 2;
			right_l_data = data + 3;
			step = 4;
		}
		else {
			step = 2;
		}
		break;
	case PcmS16Type_bigEndian_planar:
		left_u_data = data;
		left_l_data = data + 1;
		if(generator->channel_num == 2) {
			right_u_data = data + (data_size >> 1);
			right_l_data = data + (data_size >> 1) + 1;
		}
		step = 2;
		break;
	case PcmS16Type_littleEndian:
		left_u_data = data + 1;
		left_l_data = data;
		if(generator->channel_num == 2) {
			right_u_data = data + 3;
			right_l_data = data + 2;
			step = 4;
		}
		else {
			step = 2;
		}
		break;
	case PcmS16Type_littleEndian_planar:
		left_u_data = data + 1;
		left_l_data = data;
		if(generator->channel_num == 2) {
			right_u_data = data + (data_size >> 1) + 1;
			right_l_data = data + (data_size >> 1);
		}
		step = 2;
		break;
 	default:
		ERR_PRINT("found unknown pcms16Type.%d", generator->type);
		if(alloc_flag) {
			ttLibC_free(data);
		}
		return NULL;
	}
	short beep = 0;
	uint64_t pts = generator->pos;
	switch(generator->channel_num) {
	case 1:
		for(uint32_t i = 0;i < sample_num;++ i) {
			beep = sin(generator->pos * 3.14159 * 2 * generator->target_Hz / generator->sample_rate) * generator->amplitude;
			++ generator->pos;
			(*left_u_data) = ((beep >> 8) & 0xFF);
			(*left_l_data) = (beep & 0xFF);
			left_u_data += step;
			left_l_data += step;
		}
		break;
	case 2:
		for(uint32_t i = 0;i < sample_num;++ i) {
			beep = sin(generator->pos * 3.14159 * 2 * generator->target_Hz / generator->sample_rate) * generator->amplitude;
			++ generator->pos;
			(*left_u_data)  = ((beep >> 8) & 0xFF);
			(*left_l_data)  = (beep & 0xFF);
			(*right_u_data) = ((beep >> 8) & 0xFF);
			(*right_l_data) = (beep & 0xFF);
			left_u_data  += step;
			left_l_data  += step;
			right_u_data += step;
			right_l_data += step;
		}
		break;
	default:
		ERR_PRINT("channel = %d is undefined.", generator->channel_num);
		if(alloc_flag) {
			ttLibC_free(data);
		}
		return NULL;
	}
	uint8_t *l_data = NULL;
	uint32_t l_stride = 0;
	uint8_t *r_data = NULL;
	uint32_t r_stride = 0;
	switch(generator->type) {
	case PcmS16Type_bigEndian:
	case PcmS16Type_littleEndian:
		l_data = data;
		l_stride = sample_num * 2 * generator->channel_num;
		break;
	case PcmS16Type_bigEndian_planar:
	case PcmS16Type_littleEndian_planar:
		l_data = data;
		l_stride = sample_num * 2;
		if(generator->channel_num == 2) {
			r_data = data + l_stride;
			r_stride = l_stride;
		}
		break;
	}
	pcms16 = ttLibC_PcmS16_make(
			pcms16,
			generator->type,
			generator->sample_rate,
			sample_num,
			generator->channel_num,
			data,
			data_size,
			l_data,
			l_stride,
			r_data,
			r_stride,
			true,
			pts,
			generator->sample_rate);
	if(pcms16 == NULL) {
		if(alloc_flag) {
			ttLibC_free(data);
		}
		return NULL;
	}
	pcms16->inherit_super.inherit_super.is_non_copy = false;
	return pcms16;
}

/*
 * close the generator object.
 */
void ttLibC_BeepGenerator_close(ttLibC_BeepGenerator **generator) {
	if(*generator == NULL) {
		return;
	}
	ttLibC_free(*generator);
	*generator = NULL;
}

