/*
 * @file   opusDecoder.c
 * @brief  
 * @author taktod
 * @date   2015/08/01
 */

#ifdef __ENABLE_OPUS__

#include "opusDecoder.h"
#include "../log.h"
#include "../allocator.h"
#include <stdlib.h>
#include <opus/opus.h>

/*
 * opus decoder detail definition.
 */
typedef struct {
	ttLibC_OpusDecoder inherit_super;
	OpusDecoder *decoder;
	ttLibC_PcmS16 *pcms16;
	int16_t *pcm_buffer;
	size_t pcm_buffer_size;
} ttLibC_Decoder_OpusDecoder_;

typedef ttLibC_Decoder_OpusDecoder_ ttLibC_OpusDecoder_;

/*
 * make opus decoder
 * @param sample_rate target sample_rate
 * @param channel_num target channel_num
 * @return opus decoder object
 */
ttLibC_OpusDecoder *ttLibC_OpusDecoder_make(
		uint32_t sample_rate,
		uint32_t channel_num) {
	ttLibC_OpusDecoder_ *decoder = ttLibC_malloc(sizeof(ttLibC_OpusDecoder_));
	if(decoder == NULL) {
		ERR_PRINT("failed to allocate memory for decoder.");
		return NULL;
	}
	int error;
	decoder->decoder = opus_decoder_create(sample_rate, channel_num, &error);
	if(error != OPUS_OK) {
		ERR_PRINT("error to create opus decoder:%s", opus_strerror(error));
		ttLibC_free(decoder);
		return NULL;
	}
	if(decoder == NULL) {
		ERR_PRINT("failed to make opus decoder.");
		ttLibC_free(decoder);
		return NULL;
	}
	decoder->pcms16 = NULL;
	decoder->inherit_super.channel_num = channel_num;
	decoder->inherit_super.sample_rate = sample_rate;
	decoder->pcm_buffer_size = 2880 * sizeof(int16_t) * channel_num;
	decoder->pcm_buffer = ttLibC_malloc(decoder->pcm_buffer_size);
	if(decoder->pcm_buffer == NULL) {
		opus_decoder_destroy(decoder->decoder);
		ttLibC_free(decoder);
	}
	return (ttLibC_OpusDecoder *)decoder;
}

/*
 * decode frame.
 * @param decoder  opus decoder object
 * @param opus     source opus data.
 * @param callback callback func for opus decode.
 * @param ptr      pointer for user def value.
 */
void ttLibC_OpusDecoder_decode(
		ttLibC_OpusDecoder *decoder,
		ttLibC_Opus *opus,
		ttLibC_OpusDecodeFunc callback,
		void *ptr) {
	if(decoder == NULL) {
		return;
	}
	if(opus == NULL) {
		return;
	}
	ttLibC_OpusDecoder_ *decoder_ = (ttLibC_OpusDecoder_ *)decoder;
	int size = opus_decode(decoder_->decoder, opus->inherit_super.inherit_super.data, opus->inherit_super.inherit_super.buffer_size, decoder_->pcm_buffer, decoder_->pcm_buffer_size, 0);
	if(size == 0) {
		return;
	}
	// TODO put the pts and timebase from opus, however, this value should correspond with sample_rate for decoded data?
	ttLibC_PcmS16 *pcm = ttLibC_PcmS16_make(
			decoder_->pcms16,
			PcmS16Type_littleEndian,
			decoder_->inherit_super.sample_rate,
			size,
			decoder_->inherit_super.channel_num,
			decoder_->pcm_buffer,
			size * sizeof(int16_t) * decoder_->inherit_super.channel_num,
			decoder_->pcm_buffer,
			size * sizeof(int16_t) * decoder_->inherit_super.channel_num,
			NULL,
			0,
			true, opus->inherit_super.inherit_super.pts,
			opus->inherit_super.inherit_super.timebase);
	if(pcm == NULL) {
		return;
	}
	decoder_->pcms16 = pcm;
	callback(ptr, decoder_->pcms16);
}

/*
 * close opus decoder.
 * @param decoder
 */
void ttLibC_OpusDecoder_close(ttLibC_OpusDecoder **decoder) {
	ttLibC_OpusDecoder_ *target = (ttLibC_OpusDecoder_ *)*decoder;
	if(target == NULL) {
		return;
	}
	if(target->decoder != NULL) {
		opus_decoder_destroy(target->decoder);
	}
	ttLibC_PcmS16_close(&target->pcms16);
	if(target->pcm_buffer != NULL) {
		ttLibC_free(target->pcm_buffer);
	}
	ttLibC_free(target);
	*decoder = NULL;
}

#endif
