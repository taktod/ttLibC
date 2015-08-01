/*
 * @file   speexEncoder.c
 * @brief  encoder speex with speex.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/28
 */

#ifdef __ENABLE_SPEEX__

#include "speexEncoder.h"
#include "../log.h"
#include "../util/hexUtil.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <speex/speex.h>
#include <speex/speex_header.h>
#include <speex/speex_stereo.h>

/*
 * speex encoder detail definition.
 */
typedef struct {
	/** inherit data from ttLibC_SpeexEncoder */
	ttLibC_SpeexEncoder inherit_super;
	/** speex bits object */
	SpeexBits bits;
	/** speex header information. */
	SpeexHeader header;
	/** speex encode object. */
	void *enc_state;
	/** speex frame object for reuse */
	ttLibC_Speex *speex;
	/** buffer for remain data. */
	uint8_t *pcm_buffer;
	/** position of next addition of pcm_buffer */
	size_t pcm_buffer_next_pos;
	/** buffer size */
	size_t pcm_buffer_size;
	/** output speex data buffer. */
	uint8_t *data;
	/** data size */
	size_t data_size;
	/** pts counter */
	uint64_t pts;
} ttLibC_Encoder_SpeexEncoder_;

typedef ttLibC_Encoder_SpeexEncoder_ ttLibC_SpeexEncoder_;

/*
 * make speex encoder(CBR).
 * @param sample_rate target sample_rate
 * @param channel_num target channel_num
 * @param quality     target quality
 * @return speex encoder object
 */
ttLibC_SpeexEncoder *ttLibC_SpeexEncoder_make(
		uint32_t sample_rate,
		uint32_t channel_num,
		uint32_t quality) {
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
		// XXX actually, that's possible to handle 44.1k or others. but deprecated
		ERR_PRINT("support only 8k 16k 32kHz");
		return NULL;
	}
	ttLibC_SpeexEncoder_ *encoder = malloc(sizeof(ttLibC_SpeexEncoder_));
	if(encoder == NULL) {
		ERR_PRINT("failed to alloc encoder object.");
		return NULL;
	}
	encoder->enc_state = speex_encoder_init(mode);
	if(!encoder->enc_state) {
		ERR_PRINT("failed to init encoder.");
		free(encoder);
		return NULL;
	}
	speex_init_header(&encoder->header, sample_rate, channel_num, mode);
	speex_encoder_ctl(encoder->enc_state, SPEEX_SET_QUALITY, &quality);

	speex_bits_init(&encoder->bits);
	encoder->speex = NULL;
	encoder->inherit_super.channel_num = channel_num;
	encoder->inherit_super.quality = quality;
	encoder->inherit_super.sample_rate = sample_rate;
	encoder->inherit_super.type = SpeexEncoderType_CBR;
	encoder->data_size = 512; // 512 for instance.
	encoder->data = malloc(encoder->data_size);
 	encoder->pcm_buffer_size = sample_rate / 50 * channel_num * sizeof(int16_t); // 20milisec分保持しなければいけない。
	encoder->pcm_buffer = malloc(encoder->pcm_buffer_size);
	encoder->pcm_buffer_next_pos = 0;
	if(encoder->data == NULL || encoder->pcm_buffer == NULL) {
		ttLibC_SpeexEncoder_close((ttLibC_SpeexEncoder **)&encoder);
		return NULL;
	}
	return (ttLibC_SpeexEncoder *)encoder;
}

/*
 * handle encode task.
 * @param encoder  encoder object
 * @param data     target pcms16 buffer data
 * @param callback callback func
 * @param ptr      user def data pointer.
 */
static void doEncode(ttLibC_SpeexEncoder_ *encoder, void *data, ttLibC_SpeexEncodeFunc callback, void *ptr) {
	speex_bits_reset(&encoder->bits);
	// encode
	speex_encode_int(encoder->enc_state, (int16_t *)data, &encoder->bits);
	// write for bits.
	int write_size = speex_bits_write(&encoder->bits, (char *)encoder->data, encoder->data_size);
	uint32_t sample_num = encoder->inherit_super.sample_rate / 50;
	ttLibC_Speex *speex = ttLibC_Speex_make(encoder->speex, SpeexType_frame, encoder->inherit_super.sample_rate, sample_num, encoder->inherit_super.channel_num, encoder->data, write_size, true, encoder->pts, encoder->inherit_super.sample_rate);
	if(speex != NULL) {
		callback(ptr, speex);
		encoder->speex = speex;
	}
	encoder->pts += sample_num;
}

/*
 * encode frame.
 * @param encoder  speex encoder object.
 * @param pcm      source pcm data. support little endian interleave only.
 * @param callback callback func for speex creation.
 * @param ptr      pointer for user def value, which will call in callback.
 */
void ttLibC_SpeexEncoder_encode(
		ttLibC_SpeexEncoder *encoder,
		ttLibC_PcmS16 *pcm,
		ttLibC_SpeexEncodeFunc callback,
		void *ptr) {
	if(encoder == NULL) {
		return;
	}
	if(pcm == NULL) {
		return;
	}
	ttLibC_SpeexEncoder_ *encoder_ = (ttLibC_SpeexEncoder_ *)encoder;
	switch(pcm->type) {
	default:
	case PcmS16Type_bigEndian:
	case PcmS16Type_bigEndian_planar:
		ERR_PRINT("non supported type of pcm:%d", pcm->type);
		return;
	case PcmS16Type_littleEndian:
	case PcmS16Type_littleEndian_planar:
		break;
	}
	size_t left_size = pcm->inherit_super.inherit_super.buffer_size;
	uint8_t *data = pcm->inherit_super.inherit_super.data;
	if(encoder_->pcm_buffer_next_pos != 0) {
		if(left_size < encoder_->pcm_buffer_size - encoder_->pcm_buffer_next_pos) {
			// need more buffer.
			memcpy(encoder_->pcm_buffer + encoder_->pcm_buffer_next_pos, data, left_size);
			encoder_->pcm_buffer_next_pos += left_size;
			return;
		}
		// enough.
		memcpy(encoder_->pcm_buffer + encoder_->pcm_buffer_next_pos, data, encoder_->pcm_buffer_size - encoder_->pcm_buffer_next_pos);
		data += encoder_->pcm_buffer_size - encoder_->pcm_buffer_next_pos;
		left_size -= encoder_->pcm_buffer_size - encoder_->pcm_buffer_next_pos;
		doEncode(encoder_, encoder_->pcm_buffer, callback, ptr);
		encoder_->pcm_buffer_next_pos = 0;
	}
	do {
		if(left_size < encoder_->pcm_buffer_size) {
			if(left_size != 0) {
				memcpy(encoder_->pcm_buffer, data, left_size);
				encoder_->pcm_buffer_next_pos = left_size;
			}
			break;
		}
		doEncode(encoder_, data, callback, ptr);
		data += encoder_->pcm_buffer_size;
		left_size -= encoder_->pcm_buffer_size;
	}while(true);
}

/*
 * close speex encoder.
 * @param encoder
 */
void ttLibC_SpeexEncoder_close(ttLibC_SpeexEncoder **encoder) {
	ttLibC_SpeexEncoder_ *target = (ttLibC_SpeexEncoder_ *)*encoder;
	if(target == NULL) {
		return;
	}
	if(target->pcm_buffer != NULL) {
		free(target->pcm_buffer);
		target->pcm_buffer = NULL;
	}
	if(target->data != NULL) {
		free(target->data);
		target->data = NULL;
	}
	speex_bits_destroy(&target->bits);
	speex_encoder_destroy(target->enc_state);
	ttLibC_Speex_close(&target->speex);
	free(target);
	*encoder = NULL;
}

#endif
