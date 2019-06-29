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
#include "../ttLibC_predef.h"
#include "../frame/audio/aac.h"
#include "../_log.h"
#include "../allocator.h"
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
	bool is_pts_initialized;
	uint32_t id;
} ttLibC_Encoder_FaacEncoder_;

typedef ttLibC_Encoder_FaacEncoder_ ttLibC_FaacEncoder_;

/*
 * make faac encoder(Main)
 * @param sample_rate,
 * @param channel_num
 * @param bitrate
 */
ttLibC_FaacEncoder TT_VISIBILITY_DEFAULT *ttLibC_FaacEncoder_make(
		ttLibC_FaacEncoder_Type type,
		uint32_t sample_rate,
		uint32_t channel_num,
		uint32_t bitrate) {
	faacEncConfiguration config;
	ttLibC_FaacEncoder_getDefaultFaacEncConfiguration(&config);
	switch(type) {
	case FaacEncoderType_Main:
		config.aacObjectType = MAIN;
		break;
	default:
	case FaacEncoderType_Low:
		config.aacObjectType = LOW;
		break;
	case FaacEncoderType_SSR:
		config.aacObjectType = SSR;
		break;
	case FaacEncoderType_LTP:
		config.aacObjectType = LTP;
		break;
	}
	config.bitRate = bitrate / channel_num;
	uint32_t band_width = 4000 + bitrate / 8;
	if(band_width > 12000 + bitrate / 32) {
		band_width = 12000 + bitrate / 32;
	}
	if(band_width > sample_rate / 2) {
		band_width = sample_rate / 2;
	}
	config.bandWidth = band_width;
	return ttLibC_FaacEncoder_makeWithFaacEncConfiguration(&config, sample_rate, channel_num);
}

void TT_VISIBILITY_DEFAULT ttLibC_FaacEncoder_getDefaultFaacEncConfiguration(void *encConfig) {
	faacEncConfigurationPtr config = (faacEncConfigurationPtr)encConfig;
	if(config == NULL) {
		return;
	}
	config->aacObjectType = LOW;
	config->allowMidside = 0;
	config->bandWidth = 0;
	config->bitRate = 0;
//	for(int i = 0;i < 64;++ i) {
//		config->channel_map[i] = i;
//	}
	config->inputFormat = FAAC_INPUT_16BIT;
	config->mpegVersion = MPEG4;
//	config->name
	config->outputFormat = 1; // use adts
//	config->psymodelidx
//	config->psymodellist
	config->quantqual = 0;
	config->shortctl = SHORTCTL_NORMAL;
	config->useLfe = 0;
	config->useTns = 0;
//	config->version
}

ttLibC_FaacEncoder TT_VISIBILITY_DEFAULT *ttLibC_FaacEncoder_makeWithFaacEncConfiguration(
		void *encConfig,
		uint32_t sample_rate,
		uint32_t channel_num) {
	ttLibC_FaacEncoder_ *encoder = ttLibC_malloc(sizeof(ttLibC_FaacEncoder_));
	if(encoder == NULL) {
		ERR_PRINT("failed to allocate memory for encoder.");
		return NULL;
	}
	faacEncConfigurationPtr encConfig_ = (faacEncConfigurationPtr)encConfig;
	unsigned long int samples_input, max_bytes_output;
	encoder->handle = faacEncOpen(
			sample_rate,
			channel_num,
			&samples_input,
			&max_bytes_output);
	if(!encoder->handle) {
		ERR_PRINT("failed to open faac.");
		ttLibC_free(encoder);
		return NULL;
	}
	faacEncConfigurationPtr config = faacEncGetCurrentConfiguration(encoder->handle);
	if(config == NULL) {
		ERR_PRINT("failed to ref current configuration.");
		faacEncClose(encoder->handle);
		ttLibC_free(encoder);
		return NULL;
	}
	if(config->version != FAAC_CFG_VERSION) {
		ERR_PRINT("version is mismatch for faac.");
		faacEncClose(encoder->handle);
		ttLibC_free(encoder);
		return NULL;
	}
	config->aacObjectType = encConfig_->aacObjectType;
	config->allowMidside = encConfig_->allowMidside;
	if(encConfig_->bandWidth != 0) {
		config->bandWidth = encConfig_->bandWidth;
	}
	if(encConfig_->bitRate != 0) {
		config->bitRate = encConfig_->bitRate;
	}
//	config->inputFormat = encConfig_->inputFormat;
	config->inputFormat = FAAC_INPUT_16BIT;
	config->mpegVersion = encConfig_->mpegVersion;
	config->outputFormat = 1; // adts fixed.
//	config->psymodelidx
//	config->psymodellist
	if(encConfig_->quantqual != 0) {
		config->quantqual = encConfig_->quantqual;
	}
	config->shortctl = encConfig_->shortctl;
	config->useLfe = encConfig_->useLfe;
	config->useTns = encConfig_->useTns;
	if(!faacEncSetConfiguration(encoder->handle, config)) {
		ERR_PRINT("failed to update configuration.");
		faacEncClose(encoder->handle);
		ttLibC_free(encoder);
		return NULL;
	}
	encoder->aac                       = NULL;
	encoder->data_size                 = max_bytes_output;
	encoder->data                      = ttLibC_malloc(encoder->data_size);
	encoder->samples_input             = samples_input;
	encoder->samples_length            = samples_input * 2; // pcmS16 is 2byte for each sample.
	encoder->pcm_buffer_next_pos       = 0;
	encoder->pcm_buffer                = ttLibC_malloc(samples_input * sizeof(int16_t));
	encoder->pts                       = 0;
	encoder->inherit_super.channel_num = channel_num;
	encoder->inherit_super.sample_rate = sample_rate;
	encoder->inherit_super.bitrate     = config->bitRate * channel_num;
	encoder->is_pts_initialized = false;
	return (ttLibC_FaacEncoder *)encoder;
}

/*
 * handle encoded data after encode.
 * @param encoder     encoder object
 * @param encode_size size of encode data.
 * @param callback    callback func
 * @param ptr         user def data pointer.
 */
static bool checkEncodedData(ttLibC_FaacEncoder_ *encoder, uint32_t encode_size, ttLibC_FaacEncodeFunc callback, void *ptr) {
	// detail data from encoder.
	// this func is called for each aac frame.
	// calcurate sample_num from encoder->samples_put. 1024 fixed?
	uint32_t sample_num = encoder->samples_input / encoder->inherit_super.channel_num;
	ttLibC_Aac *aac = encoder->aac;
	aac = ttLibC_Aac_make(
			aac,
			AacType_adts,
			encoder->inherit_super.sample_rate,
			sample_num,
			encoder->inherit_super.channel_num,
			encoder->data,
			encode_size,
			true,
			encoder->pts,
			encoder->inherit_super.sample_rate,
			0);
	if(aac != NULL) {
		encoder->aac = aac;
		encoder->aac->inherit_super.inherit_super.id = encoder->id;
		encoder->pts += sample_num;
		if(!callback(ptr, aac)) {
			return false;
		}
	}
	else {
		return false;
	}
	return true;
}

/*
 * encode frame.
 * @param encoder  faac encoder object.
 * @param pcm      source pcm data. support little endian interleave only.
 * @param callback callback func for aac creation.
 * @param ptr      pointer for user def value, which will call in callback.
 */
bool TT_VISIBILITY_DEFAULT ttLibC_FaacEncoder_encode(
		ttLibC_FaacEncoder *encoder,
		ttLibC_PcmS16 *pcm,
		ttLibC_FaacEncodeFunc callback,
		void *ptr) {
	if(encoder == NULL) {
		return false;
	}
	if(pcm == NULL) {
		return true;
	}
	ttLibC_FaacEncoder_ *encoder_ = (ttLibC_FaacEncoder_ *)encoder;
	encoder_->id = pcm->inherit_super.inherit_super.id;
	if(!encoder_->is_pts_initialized) {
		encoder_->is_pts_initialized = true;
		encoder_->pts = pcm->inherit_super.inherit_super.pts * encoder_->inherit_super.sample_rate / pcm->inherit_super.inherit_super.timebase;
	}
	// support only little endian & interleave.
	switch(pcm->type) {
	case PcmS16Type_bigEndian:
	case PcmS16Type_bigEndian_planar:
	case PcmS16Type_littleEndian_planar:
	default:
		ERR_PRINT("non supported type of pcm:%d", pcm->type);
		return false;
	case PcmS16Type_littleEndian:
		break;
	}
	size_t left_size = pcm->l_stride;
	uint8_t *data = pcm->l_data;
	if(encoder_->pcm_buffer_next_pos != 0) {
		if(left_size < encoder_->samples_length - encoder_->pcm_buffer_next_pos) {
			// append data is not enough for encode. just add.
			memcpy(encoder_->pcm_buffer + encoder_->pcm_buffer_next_pos, data, left_size);
			encoder_->pcm_buffer_next_pos += left_size;
			return true;
		}
		memcpy(encoder_->pcm_buffer + encoder_->pcm_buffer_next_pos, data, encoder_->samples_length - encoder_->pcm_buffer_next_pos);
		data += encoder_->samples_length - encoder_->pcm_buffer_next_pos;
		left_size -= encoder_->samples_length - encoder_->pcm_buffer_next_pos;
		size_t encode_size = faacEncEncode(encoder_->handle, (int32_t *)encoder_->pcm_buffer, encoder_->samples_input, encoder_->data, encoder_->data_size);
		if(encode_size > 0) {
			if(!checkEncodedData(encoder_, encode_size, callback, ptr)) {
				return false;
			}
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
			return true;
		}
		// data have enough size.
		size_t encode_size = faacEncEncode(encoder_->handle, (int32_t *)data, encoder_->samples_input, encoder_->data, encoder_->data_size);
		if(encode_size > 0) {
			if(!checkEncodedData(encoder_, encode_size, callback, ptr)) {
				return false;
			}
		}
		data += encoder_->samples_length;
		left_size -= encoder_->samples_length;
	}while(true);
}

/*
 * close faac encoder.
 * @param encoder
 */
void TT_VISIBILITY_DEFAULT ttLibC_FaacEncoder_close(ttLibC_FaacEncoder **encoder) {
	if(*encoder == NULL) {
		return;
	}
	ttLibC_FaacEncoder_ *target = (ttLibC_FaacEncoder_ *)*encoder;
	if(target->pcm_buffer) {
		ttLibC_free(target->pcm_buffer);
	}
	if(target->handle) {
		faacEncClose(target->handle);
	}
	if(target->data) {
		ttLibC_free(target->data);
		target->data = NULL;
	}
	ttLibC_Aac_close(&target->aac);
	ttLibC_free(target);
	*encoder = NULL;
}

#endif
