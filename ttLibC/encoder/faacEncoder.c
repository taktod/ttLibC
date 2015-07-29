/*
 * @file   faacEncoder.c
 * @brief  encode pcms16 with faac.
 *
 * this code is under LGPLv3 license.
 *
 * @author taktod
 * @date   2015/07/23
 */

#ifdef __ENABLE_FAAC_ENCODE__

#include "faacEncoder.h"
#include "../frame/audio/aac.h"
#include "../log.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <faac.h>
#include <faaccfg.h>

/*
 * faac encoder detail definition.
 */
typedef struct {
	ttLibC_FaacEncoder inherit_super;
	/** handle of faac */
	faacEncHandle handle;
	/** config information of faac */
	faacEncConfiguration *config;
	/** reuse aac object. */
	ttLibC_Aac *aac;
	/** length of input byte size */
	uint32_t samples_length;
	/** samples_input information from faac. */
	unsigned long int samples_input;
	/** buffer for remain data of encode. */
	uint8_t *pcm_buffer;
	/** position of next addition of pcm_buffer */
	size_t pcm_buffer_next_pos;
	/** data for output aac. */
	uint8_t *data;
	/** allocate size of data */
	size_t data_size;
	/** pts data for next aac object. */
	uint64_t pts;
} ttLibC_Encoder_FaacEncoder_;

typedef ttLibC_Encoder_FaacEncoder_ ttLibC_FaacEncoder_;

/*
 * make faac encoder(Main)
 * @param sample_rate,
 * @param channel_num
 * @param bitrate
 */
ttLibC_FaacEncoder *ttLibC_FaacEncoder_make(
		ttLibC_FaacEncoder_Type type,
		uint32_t sample_rate,
		uint32_t channel_num,
		uint32_t bitrate) {
	ttLibC_FaacEncoder_ *encoder = malloc(sizeof(ttLibC_FaacEncoder_));
	if(encoder == NULL) {
		ERR_PRINT("failed to allocate memory for encoder.");
		return NULL;
	}
	unsigned long int samples_input, max_bytes_output;
	encoder->handle = faacEncOpen(sample_rate, channel_num, &samples_input, &max_bytes_output);
	if(!encoder->handle) {
		ERR_PRINT("failed to open faac.");
		free(encoder);
		return NULL;
	}
	encoder->config = faacEncGetCurrentConfiguration(encoder->handle);
	if(encoder->config == NULL) {
		ERR_PRINT("failed to ref current configuration.");
		faacEncClose(encoder->handle);
		free(encoder);
		return NULL;
	}
	if(encoder->config->version != FAAC_CFG_VERSION) {
		ERR_PRINT("version is mismatch for faac.");
		faacEncClose(encoder->handle);
		free(encoder);
		return NULL;
	}
	switch(type) {
	case FaacEncoderType_Main:
		encoder->config->aacObjectType = MAIN;
		break;
	case FaacEncoderType_Low:
		encoder->config->aacObjectType = LOW;
		break;
	case FaacEncoderType_SSR:
		encoder->config->aacObjectType = SSR;
		break;
	case FaacEncoderType_LTP:
		encoder->config->aacObjectType = LTP;
		break;
	default:
		ERR_PRINT("unknown faac encoder type:%d", type);
		encoder->config->aacObjectType = LOW;
		break;
	}
	encoder->config->mpegVersion   = MPEG4;
	encoder->config->useTns        = 0; // noise sharping
	encoder->config->allowMidside  = 1; // 5.1ch's .1ch is allowed.
	encoder->config->bitRate       = bitrate; // target bitrate
	encoder->config->outputFormat  = 1; // use adts.
	encoder->config->inputFormat   = FAAC_INPUT_16BIT; // accept pcms16
	if(!faacEncSetConfiguration(encoder->handle, encoder->config)) {
		ERR_PRINT("failed to set configuration.");
		faacEncClose(encoder->handle);
		free(encoder);
		return NULL;
	}
	encoder->aac                       = NULL;
	encoder->data_size                 = max_bytes_output;
	encoder->data                      = malloc(encoder->data_size);
	encoder->samples_input             = samples_input;
	encoder->samples_length            = samples_input * 2; // pcmS16 is 2byte for each sample.
	encoder->pcm_buffer_next_pos       = 0;
	encoder->pcm_buffer                = malloc(samples_input * sizeof(int16_t));
	encoder->pts                       = 0;
	encoder->inherit_super.channel_num = channel_num;
	encoder->inherit_super.sample_rate = sample_rate;
	encoder->inherit_super.bitrate     = bitrate;
	// now ready to go.
	return (ttLibC_FaacEncoder *)encoder;
}

/*
 * handle encoded data after encode.
 * @param encoder     encoder object
 * @param encode_size size of encode data.
 * @param callback    callback func
 * @param ptr         user def data pointer.
 */
static void checkEncodedData(ttLibC_FaacEncoder_ *encoder, uint32_t encode_size, ttLibC_FaacEncodeFunc callback, void *ptr) {
	// detail data from encoder.
	// this func is called for each aac frame.
	// calcurate sample_num from encoder->samples_put. 1024 fixed?
	uint32_t sample_num = encoder->samples_input / encoder->inherit_super.channel_num;
	ttLibC_Aac *aac = encoder->aac;
	aac = ttLibC_Aac_make(aac, AacType_adts, encoder->inherit_super.sample_rate, sample_num, encoder->inherit_super.channel_num, encoder->data, encode_size, true, encoder->pts, encoder->inherit_super.sample_rate);
	if(aac != NULL) {
		callback(ptr, aac);
		encoder->aac = aac;
	}
	encoder->pts += sample_num;
}

/*
 * encode frame.
 * @param encoder  faac encoder object.
 * @param pcm      source pcm data. support little endian interleave only.
 * @param callback callback func for aac creation.
 * @param ptr      pointer for user def value, which will call in callback.
 */
void ttLibC_FaacEncoder_encode(
		ttLibC_FaacEncoder *encoder,
		ttLibC_PcmS16 *pcm,
		ttLibC_FaacEncodeFunc callback,
		void *ptr) {
	if(encoder == NULL) {
		return;
	}
	if(pcm == NULL) {
		return;
	}
	ttLibC_FaacEncoder_ *encoder_ = (ttLibC_FaacEncoder_ *)encoder;
	// support only little endian & interleave.
	switch(pcm->type) {
	case PcmS16Type_bigEndian:
	case PcmS16Type_bigEndian_planar:
	case PcmS16Type_littleEndian_planar:
	default:
		ERR_PRINT("non supported type of pcm:%d", pcm->type);
		return;
	case PcmS16Type_littleEndian:
		break;
	}
	size_t left_size = pcm->inherit_super.inherit_super.buffer_size;
	uint8_t *data = pcm->inherit_super.inherit_super.data;
	if(encoder_->pcm_buffer_next_pos != 0) {
		if(left_size < encoder_->samples_length - encoder_->pcm_buffer_next_pos) {
			// append data is not enough for encode. just add.
			memcpy(encoder_->pcm_buffer + encoder_->pcm_buffer_next_pos, data, left_size);
			encoder_->pcm_buffer_next_pos += left_size;
			return;
		}
		memcpy(encoder_->pcm_buffer + encoder_->pcm_buffer_next_pos, data, encoder_->samples_length - encoder_->pcm_buffer_next_pos);
		data += encoder_->samples_length - encoder_->pcm_buffer_next_pos;
		left_size -= encoder_->samples_length - encoder_->pcm_buffer_next_pos;
		size_t encode_size = faacEncEncode(encoder_->handle, (int32_t *)encoder_->pcm_buffer, encoder_->samples_input, encoder_->data, encoder_->data_size);
		if(encode_size > 0) {
			checkEncodedData(encoder_, encode_size, callback, ptr);
		}
		encoder_->pcm_buffer_next_pos = 0;
	}
	do {
		// get the data from pcm buffer.
		if(left_size < encoder_->samples_length) {
			// need more data.
			if(left_size != 0) {
				memcpy(encoder_->pcm_buffer, data, left_size);
				encoder_->pcm_buffer_next_pos = left_size;
			}
			break;
		}
		// data have enough size.
		size_t encode_size = faacEncEncode(encoder_->handle, (int32_t *)data, encoder_->samples_input, encoder_->data, encoder_->data_size);
		if(encode_size > 0) {
			checkEncodedData(encoder_, encode_size, callback, ptr);
		}
		data += encoder_->samples_length;
		left_size -= encoder_->samples_length;
	}while(true);
}

/*
 * close faac encoder.
 * @param encoder
 */
void ttLibC_FaacEncoder_close(ttLibC_FaacEncoder **encoder) {
	if(*encoder == NULL) {
		return;
	}
	ttLibC_FaacEncoder_ *target = (ttLibC_FaacEncoder_ *)*encoder;
	if(target->handle) {
		faacEncClose(target->handle);
	}
	if(target->data) {
		free(target->data);
		target->data = NULL;
	}
	ttLibC_Aac_close(&target->aac);
	free(target);
	*encoder = NULL;
}

#endif

