/**
 * @file   opusDecoder.h
 * @brief  
 * @author taktod
 * @date   2015/08/01
 */

#ifndef TTLIBC_DECODER_OPUSDECODER_H_
#define TTLIBC_DECODER_OPUSDECODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../frame/audio/opus.h"
#include "../frame/audio/pcms16.h"

/**
 * opus decoder definition
 */
typedef struct ttLibC_Decoder_OpusDecoder {
	uint32_t sample_rate;
	uint32_t channel_num;
} ttLibC_Decoder_OpusDecoder;

typedef ttLibC_Decoder_OpusDecoder ttLibC_OpusDecoder;

/**
 * callback function for opus decoder.
 * @param ptr    user def value pointer.
 * @param pcms16 decoded fra.e
 */
typedef bool (* ttLibC_OpusDecodeFunc)(void *ptr, ttLibC_PcmS16 *pcms16);

/**
 * make opus decoder
 * @param sample_rate target sample_rate
 * @param channel_num target channel_num
 * @return opus decoder object
 */
ttLibC_OpusDecoder *ttLibC_OpusDecoder_make(
		uint32_t sample_rate,
		uint32_t channel_num);

/**
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
		void *ptr);

/**
 * ref libopus native decoder object (defined in opus/opus.h).
 * @param decoder opus decoder object.
 * @return OpusDecoder pointer.
 */
void *ttLibC_OpusDecoder_refNativeDecoder(ttLibC_OpusDecoder *decoder);

/**
 * close opus decoder.
 * @param decoder
 */
void ttLibC_OpusDecoder_close(ttLibC_OpusDecoder **decoder);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_DECODER_OPUSDECODER_H_ */
