/*
 * @file   opusDecoder.c
 * @brief  
 * @author taktod
 * @date   2015/08/01
 */

#ifdef __ENABLE_OPUS__

#include "opusDecoder.h"
#include "../ttLibC_predef.h"
#include "../_log.h"
#include "../allocator.h"
#include <stdlib.h>
#include <string.h>
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
ttLibC_OpusDecoder TT_VISIBILITY_DEFAULT *ttLibC_OpusDecoder_make(
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
 * @return true / false
 */
bool TT_VISIBILITY_DEFAULT ttLibC_OpusDecoder_decode(
		ttLibC_OpusDecoder *decoder,
		ttLibC_Opus *opus,
		ttLibC_OpusDecodeFunc callback,
		void *ptr) {
	if(decoder == NULL) {
		return false;
	}
	if(opus == NULL) {
		return true;
	}
	ttLibC_OpusDecoder_ *decoder_ = (ttLibC_OpusDecoder_ *)decoder;
	int size = opus_decode(decoder_->decoder, opus->inherit_super.inherit_super.data, opus->inherit_super.inherit_super.buffer_size, decoder_->pcm_buffer, decoder_->pcm_buffer_size, 0);
	if(size == 0) {
		return true;
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
			true,
			opus->inherit_super.inherit_super.pts,
			opus->inherit_super.inherit_super.timebase);
	if(pcm == NULL) {
		return false;
	}
	decoder_->pcms16 = pcm;
	decoder_->pcms16->inherit_super.inherit_super.id = opus->inherit_super.inherit_super.id;
	if(!callback(ptr, decoder_->pcms16)) {
		return false;
	}
	return true;
}

/*
 * ref libopus native decoder object (defined in opus/opus.h).
 * @param decoder opus decoder object.
 * @return OpusDecoder pointer.
 */
void TT_VISIBILITY_DEFAULT *ttLibC_OpusDecoder_refNativeDecoder(ttLibC_OpusDecoder *decoder) {
	if(decoder == NULL) {
		return NULL;
	}
	return (void *)((ttLibC_OpusDecoder_ *)decoder)->decoder;
}

/*
 * close opus decoder.
 * @param decoder
 */
void TT_VISIBILITY_DEFAULT ttLibC_OpusDecoder_close(ttLibC_OpusDecoder **decoder) {
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

/*
 * call codecControl
 * @param control control name
 * @param value   target value
 * @return api result.
 */
int TT_VISIBILITY_DEFAULT ttLibC_OpusDecoder_codecControl(ttLibC_OpusDecoder *decoder, const char *control, int value) {
	if(decoder == NULL) {
		return -1;
	}
	OpusDecoder *nativeDecoder = ((ttLibC_OpusDecoder_ *)decoder)->decoder;
#define OpusSetCtl(a, b) if(strcmp(control, #a) == 0) { \
    return opus_decoder_ctl(nativeDecoder, a(b));\
  }
#define OpusCtl(a) if(strcmp(control, #a) == 0) { \
    return opus_decoder_ctl(nativeDecoder, a);\
  }
#define OpusGetCtl(a) if(strcmp(control, #a) == 0) { \
    int result, res; \
    res = opus_decoder_ctl(nativeDecoder, a(&result)); \
    if(res != OPUS_OK) {return res;} \
    return result; \
  }
#define OpusGetUCtl(a) if(strcmp(control, #a) == 0) { \
    uint32_t result; \
    int res; \
    res = opus_decoder_ctl(nativeDecoder, a(&result)); \
    if(res != OPUS_OK) {return res;} \
    return result; \
  }
  OpusCtl(OPUS_RESET_STATE)
  OpusGetUCtl(OPUS_GET_FINAL_RANGE)
  OpusGetCtl(OPUS_GET_BANDWIDTH)
  OpusGetCtl(OPUS_GET_SAMPLE_RATE)
  OpusSetCtl(OPUS_SET_PHASE_INVERSION_DISABLED, value)
  OpusGetCtl(OPUS_GET_PHASE_INVERSION_DISABLED)
  OpusSetCtl(OPUS_SET_GAIN, value)
  OpusGetCtl(OPUS_GET_GAIN)
  OpusGetCtl(OPUS_GET_LAST_PACKET_DURATION)
  OpusGetCtl(OPUS_GET_PITCH)
#undef OpusSetCtl
#undef OpusCtl
#undef OpusGetCtl
#undef OpusGetUCtl
  return OPUS_OK;
}

#endif
