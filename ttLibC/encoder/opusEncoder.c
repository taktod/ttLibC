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
#include "../_log.h"
#include "../allocator.h"
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
	bool is_pts_initialized;
	uint32_t id;
} ttLibC_Encoder_OpusEncoder_;

typedef ttLibC_Encoder_OpusEncoder_ ttLibC_OpusEncoder_;

/*
 * make opus encoder.
 * @param sample_rate     target sample_rate
 * @param channel_num     target channel_num
 * @param unit_sample_num sample_num for each opus frame.
 * @return opus encoder object
 */
ttLibC_OpusEncoder TT_ATTRIBUTE_API *ttLibC_OpusEncoder_make(
		uint32_t sample_rate,
		uint32_t channel_num,
		uint32_t unit_sample_num) {
	ttLibC_OpusEncoder_ *encoder = ttLibC_malloc(sizeof(ttLibC_OpusEncoder_));
	if(encoder == NULL) {
		ERR_PRINT("failed to alloc encoder object.");
		return NULL;
	}
	int error;
	encoder->encoder = opus_encoder_create(
			sample_rate,
			channel_num,
			OPUS_APPLICATION_RESTRICTED_LOWDELAY,
			&error);
	if(error != OPUS_OK) {
		ERR_PRINT("error to create opus encoder:%s", opus_strerror(error));
		ttLibC_free(encoder);
		return NULL;
	}
	if(encoder->encoder == NULL) {
		ERR_PRINT("failed to make opus encoder.");
		ttLibC_free(encoder);
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
	encoder->data = ttLibC_malloc(encoder->data_size);
	encoder->pcm_buffer_size = unit_sample_num * channel_num * sizeof(int16_t);
	encoder->pcm_buffer = ttLibC_malloc(encoder->pcm_buffer_size);
	encoder->pcm_buffer_next_pos = 0;
	encoder->is_pts_initialized = false;
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
static bool OpusEncoder_doEncode(ttLibC_OpusEncoder_ *encoder, void *data, ttLibC_OpusEncodeFunc callback, void *ptr) {
	int size = opus_encode(encoder->encoder, (int16_t *)data, encoder->inherit_super.unit_sample_num, encoder->data, encoder->data_size);
	if(size == 0) {
		ERR_PRINT("failed to make opus.");
		return false;
	}
	ttLibC_Opus *opus = ttLibC_Opus_make(
			encoder->opus,
			OpusType_frame,
			encoder->inherit_super.sample_rate,
			encoder->inherit_super.unit_sample_num,
			encoder->inherit_super.channel_num,
			encoder->data,
			size,
			true,
			encoder->pts,
			encoder->inherit_super.sample_rate);
	if(opus != NULL) {
		encoder->opus = opus;
		encoder->opus->inherit_super.inherit_super.id = encoder->id;
		if(callback != NULL) {
			if(!callback(ptr, opus)) {
				return false;
			}
		}
	}
	encoder->pts += encoder->inherit_super.unit_sample_num;
	return true;
}

/*
 * encode frame.
 * @param encoder  opus encoder object
 * @param pcm      source pcm data. support little endian interleave only.
 * @param callback callback func for opus creation.
 * @param ptr      pointer for user def value, which will call in callback
 * @return true / false
 */
bool TT_ATTRIBUTE_API ttLibC_OpusEncoder_encode(
		ttLibC_OpusEncoder *encoder,
		ttLibC_PcmS16 *pcm,
		ttLibC_OpusEncodeFunc callback,
		void *ptr) {
	if(encoder == NULL) {
		return false;
	}
	if(pcm == NULL) {
		return true;
	}
	ttLibC_OpusEncoder_ *encoder_ = (ttLibC_OpusEncoder_ *)encoder;
	encoder_->id = pcm->inherit_super.inherit_super.id;
	if(!encoder_->is_pts_initialized) {
		encoder_->is_pts_initialized = true;
		encoder_->pts = pcm->inherit_super.inherit_super.pts * encoder_->inherit_super.sample_rate / pcm->inherit_super.inherit_super.timebase;
	}
	switch(pcm->type) {
	default:
	case PcmS16Type_bigEndian:
	case PcmS16Type_bigEndian_planar:
	case PcmS16Type_littleEndian_planar:
		ERR_PRINT("non supported type of pcm:%d", pcm->type);
		return false;
	case PcmS16Type_littleEndian:
		break;
	}
	size_t left_size = pcm->l_stride;
	uint8_t *data = pcm->l_data;
	if(encoder_->pcm_buffer_next_pos != 0) {
		if(left_size < encoder_->pcm_buffer_size - encoder_->pcm_buffer_next_pos) {
			// need more buffer.
			memcpy(encoder_->pcm_buffer + encoder_->pcm_buffer_next_pos, data, left_size);
			encoder_->pcm_buffer_next_pos += left_size;
			return true;
		}
		// enough.
		memcpy(encoder_->pcm_buffer + encoder_->pcm_buffer_next_pos, data, encoder_->pcm_buffer_size - encoder_->pcm_buffer_next_pos);
		data += encoder_->pcm_buffer_size - encoder_->pcm_buffer_next_pos;
		left_size -= encoder_->pcm_buffer_size - encoder_->pcm_buffer_next_pos;
		if(!OpusEncoder_doEncode(encoder_, encoder_->pcm_buffer, callback, ptr)) {
			return false;
		}
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
		if(!OpusEncoder_doEncode(encoder_, data, callback, ptr)) {
			return false;
		}
		data += encoder_->pcm_buffer_size;
		left_size -= encoder_->pcm_buffer_size;
	}while(true);
	return true;
}

/*
 * ref libopus native encoder object (defined in opus/opus.h).
 * @param decoder opus encoder object.
 * @return OpusEncoder pointer.
 */
void TT_ATTRIBUTE_API *ttLibC_OpusEncoder_refNativeEncoder(ttLibC_OpusEncoder *encoder) {
	if(encoder == NULL) {
		return NULL;
	}
	return (void *)((ttLibC_OpusEncoder_ *)encoder)->encoder;
}

/*
 * update bitrate for opus encoder.
 * @param encoder opus encoder object
 * @param bitrate target bitrate.
 * @return true:success false:error
 * @note you can check real value for bitrate, encoder->bitrate;
 */
bool TT_ATTRIBUTE_API ttLibC_OpusEncoder_setBitrate(
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
bool TT_ATTRIBUTE_API ttLibC_OpusEncoder_setComplexity(
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
void TT_ATTRIBUTE_API ttLibC_OpusEncoder_close(ttLibC_OpusEncoder **encoder) {
	ttLibC_OpusEncoder_ *target = (ttLibC_OpusEncoder_ *)*encoder;
	if(target == NULL) {
		return;
	}
	if(target->encoder != NULL) {
		opus_encoder_destroy(target->encoder);
	}
	if(target->data != NULL) {
		ttLibC_free(target->data);
		target->data = NULL;
	}
	if(target->pcm_buffer != NULL) {
		ttLibC_free(target->pcm_buffer);
		target->pcm_buffer = NULL;
	}
	ttLibC_Opus_close(&target->opus);
	ttLibC_free(target);
	*encoder = NULL;
}

/**
 * call codecControl
 * @param encoder opus encoder object
 * @param control control name
 * @param value   target value
 * @return api result.
 */
int TT_ATTRIBUTE_API ttLibC_OpusEncoder_codecControl(ttLibC_OpusEncoder *encoder, const char *control, int value) {
	if(encoder == NULL) {
		return -1;
	}	
	OpusEncoder *nativeEncoder = ((ttLibC_OpusEncoder_ *)encoder)->encoder;
#define OpusSetCtl(a, b) if(strcmp(control, #a) == 0) { \
    return opus_encoder_ctl(nativeEncoder, a(b));\
  }
#define OpusCtl(a) if(strcmp(control, #a) == 0) { \
    return opus_encoder_ctl(nativeEncoder, a);\
  }
#define OpusGetCtl(a) if(strcmp(control, #a) == 0) { \
    int result, res; \
    res = opus_encoder_ctl(nativeEncoder, a(&result)); \
    if(res != OPUS_OK) {return res;} \
    return result; \
  }
#define OpusGetUCtl(a) if(strcmp(control, #a) == 0) { \
    uint32_t result; \
    int res; \
    res = opus_encoder_ctl(nativeEncoder, a(&result)); \
    if(res != OPUS_OK) {return res;} \
    return result; \
  }
  OpusSetCtl(OPUS_SET_COMPLEXITY, value)
  OpusGetCtl(OPUS_GET_COMPLEXITY)
  OpusSetCtl(OPUS_SET_BITRATE, value)
  OpusGetCtl(OPUS_GET_BITRATE)
  OpusSetCtl(OPUS_SET_VBR, value)
  OpusGetCtl(OPUS_GET_VBR)
  OpusSetCtl(OPUS_SET_VBR_CONSTRAINT, value)
  OpusGetCtl(OPUS_GET_VBR_CONSTRAINT)
  OpusSetCtl(OPUS_SET_FORCE_CHANNELS, value)
  OpusGetCtl(OPUS_GET_FORCE_CHANNELS)
  OpusSetCtl(OPUS_SET_MAX_BANDWIDTH, value)
  OpusGetCtl(OPUS_GET_MAX_BANDWIDTH)
  OpusSetCtl(OPUS_SET_BANDWIDTH, value)
  OpusSetCtl(OPUS_SET_SIGNAL, value)
  OpusGetCtl(OPUS_GET_SIGNAL)
  OpusSetCtl(OPUS_SET_APPLICATION, value)
  OpusGetCtl(OPUS_GET_APPLICATION)
  OpusGetCtl(OPUS_GET_LOOKAHEAD)
  OpusSetCtl(OPUS_SET_INBAND_FEC, value)
  OpusGetCtl(OPUS_GET_INBAND_FEC)
  OpusSetCtl(OPUS_SET_PACKET_LOSS_PERC, value)
  OpusGetCtl(OPUS_GET_PACKET_LOSS_PERC)
  OpusSetCtl(OPUS_SET_DTX, value)
  OpusGetCtl(OPUS_GET_DTX)
  OpusSetCtl(OPUS_SET_LSB_DEPTH, value)
  OpusGetCtl(OPUS_GET_LSB_DEPTH)
  OpusSetCtl(OPUS_SET_EXPERT_FRAME_DURATION, value)
  OpusGetCtl(OPUS_GET_EXPERT_FRAME_DURATION)
  OpusSetCtl(OPUS_SET_PREDICTION_DISABLED, value)
  OpusGetCtl(OPUS_GET_PREDICTION_DISABLED)
  OpusCtl(OPUS_RESET_STATE)
  OpusGetUCtl(OPUS_GET_FINAL_RANGE)
  OpusGetCtl(OPUS_GET_BANDWIDTH)
  OpusGetCtl(OPUS_GET_SAMPLE_RATE)
  OpusSetCtl(OPUS_SET_PHASE_INVERSION_DISABLED, value)
  OpusGetCtl(OPUS_GET_PHASE_INVERSION_DISABLED)
#undef OpusSetCtl
#undef OpusCtl
#undef OpusGetCtl
#undef OpusGetUCtl
  return OPUS_OK;
}

#endif
