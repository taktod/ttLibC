/*
 * @file   speexDecoder.c
 * @brief  decoder speex with speex.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/01
 */

#ifdef __ENABLE_SPEEX__

#include "speexDecoder.h"
#include "../log.h"
#include "../allocator.h"
#include <stdlib.h>
#include <speex/speex.h>
#include <speex/speex_header.h>
#include <speex/speex_stereo.h>

/*
 * speex decoder detail definition.
 */
typedef struct {
	/** inherit data from ttLibC_SpeexDecoder */
	ttLibC_SpeexDecoder inherit_super;
	/** speex decode object */
	void *dec_state;
	/** speex bits handler */
	SpeexBits bits;
	/** for stereo */
//	SpeexStereoState stereo;
	/** pcms16 data for reuse */
	ttLibC_PcmS16 *pcms16;
	/** pcm buffer for decode */
	int16_t *pcm_buffer;
	/** size of pcm buffer */
	size_t pcm_buffer_size;
} ttLibC_Decoder_SpeexDecoder_;

typedef ttLibC_Decoder_SpeexDecoder_ ttLibC_SpeexDecoder_;

/*
 * make speex decoder
 * @param sample_rate
 * @param channel_num
 * @return speex decoder object.
 */
ttLibC_SpeexDecoder *ttLibC_SpeexDecoder_make(
		uint32_t sample_rate,
		uint32_t channel_num) {
	const SpeexMode *mode;
	switch(sample_rate) {
	case 8000:
		mode = &speex_nb_mode;
		break;
	case 16000:
		mode = &speex_wb_mode;
		break;
	case 32000:
		mode = &speex_uwb_mode;
		break;
	default:
		LOG_PRINT("support only 8k 16k 32kHz.");
		return NULL;
	}

	ttLibC_SpeexDecoder_ *decoder = ttLibC_malloc(sizeof(ttLibC_SpeexDecoder_));
	if(decoder == NULL) {
		ERR_PRINT("failed to allocate memory for decoder.");
		return NULL;
	}
	decoder->pcms16 = NULL;
	decoder->inherit_super.channel_num = channel_num;
	decoder->inherit_super.sample_rate = sample_rate;

	speex_bits_init(&decoder->bits);
	decoder->dec_state = speex_decoder_init(mode);
	decoder->pcm_buffer_size = sample_rate / 50 * channel_num * sizeof(int16_t);
	decoder->pcm_buffer = ttLibC_malloc(decoder->pcm_buffer_size);
	if(decoder->pcm_buffer == NULL) {
		ERR_PRINT("failed to allocate memory for buffer.");
		speex_bits_destroy(&decoder->bits);
		speex_decoder_destroy(decoder->dec_state);
		return NULL;
	}
	return (ttLibC_SpeexDecoder *)decoder;
}

/*
 * decode frame.
 * @param decoder  speex decoder object
 * @param speex    source speex data.
 * @param callback callback func for speex decode.
 * @param ptr      pointer for user def value.
 */
void ttLibC_SpeexDecoder_decode(
		ttLibC_SpeexDecoder *decoder,
		ttLibC_Speex *speex,
		ttLibC_SpeexDecodeFunc callback,
		void *ptr) {
	if(decoder == NULL) {
		return;
	}
	if(speex == NULL) {
		return;
	}
	ttLibC_SpeexDecoder_ *decoder_ = (ttLibC_SpeexDecoder_ *)decoder;
	speex_bits_read_from(&decoder_->bits, (char *)speex->inherit_super.inherit_super.data, speex->inherit_super.inherit_super.buffer_size);
	int result = speex_decode_int(decoder_->dec_state, &decoder_->bits, decoder_->pcm_buffer);
	if(result != 0) {
		ERR_PRINT("failed to decode.%d", result);
		return;
	}
	uint32_t sample_num = decoder_->inherit_super.sample_rate / 50;
	ttLibC_PcmS16 *p = ttLibC_PcmS16_make(
			decoder_->pcms16,
			PcmS16Type_littleEndian,
			decoder_->inherit_super.sample_rate,
			sample_num,
			decoder_->inherit_super.channel_num,
			decoder_->pcm_buffer,
			decoder_->pcm_buffer_size,
			decoder_->pcm_buffer,
			sample_num * 2 * decoder_->inherit_super.channel_num,
			NULL,
			0,
			true,
			speex->inherit_super.inherit_super.pts,
			speex->inherit_super.inherit_super.timebase);
	if(p == NULL) {
		return;
	}
	decoder_->pcms16 = p;
	callback(ptr, decoder_->pcms16);
}

/*
 * close speex decoder.
 * @param decoder
 */
void ttLibC_SpeexDecoder_close(ttLibC_SpeexDecoder **decoder) {
	ttLibC_SpeexDecoder_ *target = (ttLibC_SpeexDecoder_ *)*decoder;
	if(target == NULL) {
		return;
	}
	speex_bits_destroy(&target->bits);
	speex_decoder_destroy(target->dec_state);
	if(target->pcm_buffer != NULL) {
		ttLibC_free(target->pcm_buffer);
	}
	ttLibC_PcmS16_close(&target->pcms16);
	ttLibC_free(target);
	*decoder = NULL;
}

#endif
