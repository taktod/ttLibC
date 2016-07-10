/*
 * @file   speexdspResampler.c
 * @brief  resampler by speexdsp.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/26
<<<<<<< HEAD
=======
 *
 * @note Is speexdsp thread safe?
>>>>>>> 9bbbf00f57e1bb3b2a36a3faa606dbefb135e8a0
 */

#ifdef __ENABLE_SPEEXDSP__

#include "speexdspResampler.h"
#include "../log.h"
#include "../allocator.h"
<<<<<<< HEAD
=======
#include "../ttLibC_common.h"
>>>>>>> 9bbbf00f57e1bb3b2a36a3faa606dbefb135e8a0
#include <speex/speex_resampler.h>
#include <stdlib.h>
#include <string.h>

/**
 * speexdsp resampler detail definition.
 */
typedef struct {
	/** inherit data from ttLibC_SpeexdspResampler */
	ttLibC_SpeexdspResampler inherit_super;
	/** resampler object (speexdsp) */
	SpeexResamplerState *resampler;
} ttLibC_Resampler_SpeexdspResampler_;

typedef ttLibC_Resampler_SpeexdspResampler_ ttLibC_SpeexdspResampler_;

/*
 * make speexdsp resampler.
 * @param channel_num        target channel num
 * @param input_sample_rate  input sample rate
 * @param output_sample_rate output sample rate
 * @param quality            quality for speexdsp resampler. 0:low quality - 10:max quality.
 * @return resampler object.
 */
ttLibC_SpeexdspResampler *ttLibC_SpeexdspResampler_make(uint32_t channel_num, uint32_t input_sample_rate, uint32_t output_sample_rate, uint32_t quality) {
	ttLibC_SpeexdspResampler_ *resampler = (ttLibC_SpeexdspResampler_ *)ttLibC_malloc(sizeof(ttLibC_SpeexdspResampler_));
	if(resampler == NULL) {
		ERR_PRINT("failed to allocate resampler object.");
		return NULL;
	}
	int error_num;
	resampler->resampler = speex_resampler_init(channel_num, input_sample_rate, output_sample_rate, quality, &error_num);
	if(error_num != 0 || resampler->resampler == NULL) {
		ERR_PRINT("failed to init speex resampler.");
		ttLibC_free(resampler);
		return NULL;
	}
	resampler->inherit_super.channel_num        = channel_num;
	resampler->inherit_super.input_sample_rate  = input_sample_rate;
	resampler->inherit_super.output_sample_rate = output_sample_rate;
	resampler->inherit_super.quality            = quality;
	return (ttLibC_SpeexdspResampler *)resampler;
}

/*
 * sample_rate resample.
 * @param resampler  resampler object.
 * @param prev_frame reuse frame.
 * @param src_pcms16 source pcms16 data.
 * @return resampled pcms16 data.
 */
ttLibC_PcmS16 *ttLibC_SpeexdspResampler_resample(ttLibC_SpeexdspResampler *resampler, ttLibC_PcmS16 *prev_frame, ttLibC_PcmS16 *src_pcms16) {
	ttLibC_SpeexdspResampler_ *resampler_ = (ttLibC_SpeexdspResampler_ *)resampler;
	if(resampler_ == NULL) {
		return NULL;
	}
	if(src_pcms16 == NULL) {
		return NULL;
	}
	switch(src_pcms16->type) {
	case PcmS16Type_bigEndian:
	case PcmS16Type_bigEndian_planar:
<<<<<<< HEAD
=======
		resampler_->inherit_super.error = ttLibC_updateError(Target_On_Resampler, Error_InvalidOperation);
>>>>>>> 9bbbf00f57e1bb3b2a36a3faa606dbefb135e8a0
		return NULL;
	default:
		return NULL;
	case PcmS16Type_littleEndian:
	case PcmS16Type_littleEndian_planar:
		break;
	}
	ttLibC_PcmS16 *pcms16 = prev_frame;
	uint32_t out_sample_num = (src_pcms16->inherit_super.sample_num
			* resampler_->inherit_super.output_sample_rate
			/ resampler_->inherit_super.input_sample_rate + 1);
	uint32_t in_sample_num  = src_pcms16->inherit_super.sample_num;
	// estimate result data size.
	size_t data_size = out_sample_num * sizeof(int16_t) * resampler_->inherit_super.channel_num;
	uint8_t *data = NULL;
	bool alloc_flag = false;
	if(pcms16 != NULL) {
		if(!pcms16->inherit_super.inherit_super.is_non_copy) {
			if(pcms16->inherit_super.inherit_super.data_size >= data_size) {
				// reuse frame have enough buffer.
				data = pcms16->inherit_super.inherit_super.data;
				data_size = pcms16->inherit_super.inherit_super.data_size;
			}
			else {
				ttLibC_free(pcms16->inherit_super.inherit_super.data);
			}
		}
		pcms16->inherit_super.inherit_super.is_non_copy = true;
	}
	if(data == NULL) {
		data = ttLibC_malloc(data_size);
<<<<<<< HEAD
=======
		if(data == NULL) {
			resampler_->inherit_super.error = ttLibC_updateError(Target_On_Resampler, Error_MemoryAllocate);
			return NULL;
		}
>>>>>>> 9bbbf00f57e1bb3b2a36a3faa606dbefb135e8a0
		alloc_flag = true;
	}
	int res;
	switch(src_pcms16->type) {
	default:
		ERR_PRINT("no way to be here..");
		if(alloc_flag) {
			ttLibC_free(data);
		}
		return NULL;
	case PcmS16Type_littleEndian:
		res = speex_resampler_process_interleaved_int(resampler_->resampler, (const int16_t *)src_pcms16->l_data, &in_sample_num, (int16_t *)data, &out_sample_num);
		break;
	case PcmS16Type_littleEndian_planar:
		res = speex_resampler_process_int(resampler_->resampler, resampler_->inherit_super.channel_num, (const int16_t *)src_pcms16->l_data, &in_sample_num, (int16_t *)data, &out_sample_num);
		break;
	}
	if(res != 0) {
		ERR_PRINT("failed to resampler: %s", speex_resampler_strerror(res));
		if(alloc_flag) {
			ttLibC_free(data);
		}
<<<<<<< HEAD
=======
		resampler_->inherit_super.error = ttLibC_updateError(Target_On_Resampler, Error_LibraryError);
>>>>>>> 9bbbf00f57e1bb3b2a36a3faa606dbefb135e8a0
		return NULL;
	}
	uint64_t pts = src_pcms16->inherit_super.inherit_super.pts * resampler_->inherit_super.output_sample_rate / resampler_->inherit_super.input_sample_rate;
	uint8_t *l_data = NULL;
	uint32_t l_stride = 0;
	uint8_t *r_data = NULL;
	uint32_t r_stride = 0;
	switch(src_pcms16->type) {
	case PcmS16Type_bigEndian:
	case PcmS16Type_littleEndian:
		l_data = data;
		l_stride = out_sample_num * 2 * resampler_->inherit_super.channel_num;
		break;
	case PcmS16Type_bigEndian_planar:
	case PcmS16Type_littleEndian_planar:
		l_data = data;
		l_stride = out_sample_num * 2;
		if(resampler_->inherit_super.channel_num == 2) {
			r_data = data + l_stride;
			r_stride = l_stride;
		}
		break;
	}
	pcms16 = ttLibC_PcmS16_make(
			pcms16,
			src_pcms16->type,
			resampler_->inherit_super.output_sample_rate,
			out_sample_num,
			resampler_->inherit_super.channel_num,
			data,
			data_size,
			l_data,
			l_stride,
			r_data,
			r_stride,
			true,
			pts,
			resampler_->inherit_super.output_sample_rate);
	if(pcms16 == NULL) {
		if(alloc_flag) {
			ttLibC_free(data);
		}
<<<<<<< HEAD
=======
		resampler_->inherit_super.error = ttLibC_updateError(Target_On_Resampler, Error_MemoryAllocate);
>>>>>>> 9bbbf00f57e1bb3b2a36a3faa606dbefb135e8a0
		return NULL;
	}
	pcms16->inherit_super.inherit_super.is_non_copy = false;
	pcms16->inherit_super.inherit_super.buffer_size = pcms16->inherit_super.sample_num * pcms16->inherit_super.channel_num * sizeof(int16_t);
	return pcms16;
}

/*
 * close resampler.
 * @param resampler
 */
void ttLibC_SpeexdspResampler_close(ttLibC_SpeexdspResampler **resampler) {
	ttLibC_SpeexdspResampler_ *target = (ttLibC_SpeexdspResampler_ *)*resampler;
	if(target == NULL) {
		return;
	}
	speex_resampler_destroy(target->resampler);
	ttLibC_free(target);
	*resampler = NULL;
}

#endif

