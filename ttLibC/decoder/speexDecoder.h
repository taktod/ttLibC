/**
 * @file   speexDecoder.h
 * @brief  decoder speex with speex.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/01
 */

#ifndef TTLIBC_DECODER_SPEEXDECODER_H_
#define TTLIBC_DECODER_SPEEXDECODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../frame/audio/speex.h"
#include "../frame/audio/pcms16.h"

/**
 * speex decoder definition
 */
typedef struct ttLibC_Decoder_SpeexDecoder {
	uint32_t sample_rate;
	uint32_t channel_num;
} ttLibC_Decoder_SpeexDecoder;

typedef ttLibC_Decoder_SpeexDecoder ttLibC_SpeexDecoder;

/**
 * callback function for speex decoder.
 * @param ptr    user def value pointer.
 * @param pcms16 decoded frame.
 */
typedef bool (* ttLibC_SpeexDecodeFunc)(void *ptr, ttLibC_PcmS16 *pcms16);

/**
 * make speex decoder
 * @param sample_rate
 * @param channel_num
 * @return speex decoder object.
 */
ttLibC_SpeexDecoder *ttLibC_SpeexDecoder_make(
		uint32_t sample_rate,
		uint32_t channel_num);

/**
 * decode frame.
 * @param decoder  speex decoder object
 * @param speex    source speex data.
 * @param callback callback func for speex decode.
 * @param ptr      pointer for user def value.
 * @return true / false
 */
bool ttLibC_SpeexDecoder_decode(
		ttLibC_SpeexDecoder *decoder,
		ttLibC_Speex *speex,
		ttLibC_SpeexDecodeFunc callback,
		void *ptr);

/**
 * close speex decoder.
 * @param decoder
 */
void ttLibC_SpeexDecoder_close(ttLibC_SpeexDecoder **decoder);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_DECODER_SPEEXDECODER_H_ */
