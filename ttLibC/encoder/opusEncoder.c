/*
 * @file   opusEncoder.c
 * @brief  encode opus with libopus.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/01
 */

#ifdef __ENABLE_OPUS__

#include "opusEncoder.h"
#include "../log.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <opus/opus.h>

/**
 * opus encoder detail definition
 */
typedef struct {
	/** inherit data from ttLibC_OpusEncoder */
	ttLibC_OpusEncoder inherit_super;
	/** opus encoder object */
	OpusEncoder *encoder;
	/** reuse opus frame */
	ttLibC_Opus *opus;
	/** to handle remain data. */
	uint8_t *pcm_buffer;
	/** addtion position for pcm_buffer */
	size_t pcm_buffer_next_pos;
	/** buffer size */
	size_t pcm_buffer_size;
	/** output opus data buffer. */
	uint8_t *data;
	/** data size */
	size_t data_size;
	/** pts counter */
	uint64_t pts;
} ttLibC_Encoder_OpusEncoder_;

typedef ttLibC_Encoder_OpusEncoder_ ttLibC_OpusEncoder_;

/*
 * make opus encoder.
 * @param sample_rate     target sample_rate
 * @param channel_num     target channel_num
 * @param unit_sample_num sample_num for each opus frame.
 * @return opus encoder object
 */
ttLibC_OpusEncoder *ttLibC_OpusEncoder_make(
		uint32_t sample_rate,
		uint32_t channel_num,
		uint32_t unit_sample_num) {
	ttLibC_OpusEncoder_ *encoder = malloc(sizeof(ttLibC_OpusEncoder_));
	if(encoder == NULL) {
		ERR_PRINT("failed to alloc encoder object.");
		return NULL;
	}
	int error;
	encoder->encoder = opus_encoder_create(sample_rate, channel_num, OPUS_APPLICATION_RESTRICTED_LOWDELAY, &error);
	if(error != OPUS_OK) {
		ERR_PRINT("error to create opus encoder:%s", opus_strerror(error));
		free(encoder);
		return NULL;
	}
	if(encoder->encoder == NULL) {
		ERR_PRINT("failed to make opus encoder.");
		free(encoder);
		return NULL;
	}
	encoder->opus                          = NULL;
	int bitrate;
	opus_encoder_ctl(encoder->encoder, OPUS_GET_BITRATE(&bitrate));
	encoder->inherit_super.bitrate         = bitrate;
	encoder->inherit_super.channel_num     = channel_num;
	int complexity;
	opus_encoder_ctl(encoder->encoder, OPUS_GET_COMPLEXITY(&complexity));
	encoder->inherit_super.complexity      = complexity;
	encoder->inherit_super.sample_rate     = sample_rate;
	encoder->inherit_super.unit_sample_num = unit_sample_num;
	encoder->data_size = 512; // 512 for instance.
	encoder->data = malloc(encoder->data_size);
	encoder->pcm_buffer_size = unit_sample_num * channel_num * sizeof(int16_t);
	encoder->pcm_buffer = malloc(encoder->pcm_buffer_size);
	encoder->pcm_buffer_next_pos = 0;
	if(encoder->data == NULL || encoder->pcm_buffer == NULL) {
		ttLibC_OpusEncoder_close((ttLibC_OpusEncoder **)&encoder);
		return NULL;
	}
	return (ttLibC_OpusEncoder *)encoder;
}

/*
 * handle encode task
 * @param encoder  opus encoder object
 * @param data     target pcms16 buffer data
 * @param callback callback func
 * @param ptr      user def data pointer.
 */
static void doEncode(ttLibC_OpusEncoder_ *encoder, void *data, ttLibC_OpusEncodeFunc callback, void *ptr) {
	int size = opus_encode(encoder->encoder, (int16_t *)data, encoder->inherit_super.unit_sample_num, encoder->data, encoder->data_size);
	if(size == 0) {
		ERR_PRINT("failed to make opus.");
		return;
	}
	ttLibC_Opus *opus = ttLibC_Opus_make(encoder->opus, OpusType_frame, encoder->inherit_super.sample_rate, encoder->inherit_super.unit_sample_num, encoder->inherit_super.channel_num, encoder->data, size, true, encoder->pts, encoder->inherit_super.sample_rate);
	if(opus != NULL) {
		callback(ptr, opus);
		encoder->opus = opus;
	}
	encoder->pts += encoder->inherit_super.unit_sample_num;
}

/*
 * encode frame.
 * @param encoder  opus encoder object
 * @param pcm      source pcm data. support little endian interleave only.
 * @param callback callback func for opus creation.
 * @param ptr      pointer for user def value, which will call in callback
 */
void ttLibC_OpusEncoder_encode(
		ttLibC_OpusEncoder *encoder,
		ttLibC_PcmS16 *pcm,
		ttLibC_OpusEncodeFunc callback,
		void *ptr) {
	if(encoder == NULL) {
		return;
	}
	if(pcm == NULL) {
		return;
	}
	ttLibC_OpusEncoder_ *encoder_ = (ttLibC_OpusEncoder_ *)encoder;
	switch(pcm->type) {
	default:
	case PcmS16Type_bigEndian:
	case PcmS16Type_bigEndian_planar:
	case PcmS16Type_littleEndian_planar:
		ERR_PRINT("non supported type of pcm:%d", pcm->type);
		return;
	case PcmS16Type_littleEndian:
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
 * update bitrate for opus encoder.
 * @param encoder opus encoder object
 * @param bitrate target bitrate.
 * @return true:success false:error
 * @note you can check real value for bitrate, encoder->bitrate;
 */
bool ttLibC_OpusEncoder_setBitrate(
		ttLibC_OpusEncoder *encoder,
		uint32_t bitrate) {
	if(encoder == NULL) {
		return false;
	}
	ttLibC_OpusEncoder_ *encoder_ = (ttLibC_OpusEncoder_ *)encoder;
	if(encoder_->encoder == NULL) {
		return false;
	}
	int result = opus_encoder_ctl(encoder_->encoder, OPUS_SET_BITRATE(bitrate));
	if(result != OPUS_OK) {
		return false;
	}
	opus_encoder_ctl(encoder_->encoder, OPUS_GET_BITRATE(&result));
	encoder_->inherit_super.bitrate = result;
	return true;
}

/*
 * update complexity for opus encoder.
 * @param encoder    opus encoder object
 * @param complexity complexity value(0 - 10) 0:less complexity bad quality, 10:full complexity good quality.
 * @return true:success false:error
 * @note you can check real value for complexity, encoder->complexity;
 */
bool ttLibC_OpusEncoder_setComplexity(
		ttLibC_OpusEncoder *encoder,
		uint32_t complexity) {
	if(encoder == NULL) {
		return false;
	}
	ttLibC_OpusEncoder_ *encoder_ = (ttLibC_OpusEncoder_ *)encoder;
	if(encoder_->encoder == NULL) {
		return false;
	}
	int result = opus_encoder_ctl(encoder_->encoder, OPUS_SET_COMPLEXITY(complexity));
	if(result != OPUS_OK) {
		return false;
	}
	opus_encoder_ctl(encoder_->encoder, OPUS_GET_COMPLEXITY(&result));
	encoder_->inherit_super.complexity = result;
	return true;
}

/*
 * close opus encoder
 * @param encoder
 */
void ttLibC_OpusEncoder_close(ttLibC_OpusEncoder **encoder) {
	ttLibC_OpusEncoder_ *target = (ttLibC_OpusEncoder_ *)*encoder;
	if(target == NULL) {
		return;
	}
	if(target->encoder != NULL) {
		opus_encoder_destroy(target->encoder);
	}
	if(target->data != NULL) {
		free(target->data);
		target->data = NULL;
	}
	if(target->pcm_buffer != NULL) {
		free(target->pcm_buffer);
		target->pcm_buffer = NULL;
	}
	ttLibC_Opus_close(&target->opus);
	free(target);
	*encoder = NULL;
}

#endif
