/**
 * @file   opusEncoder.h
 * @brief  encode opus with libopus.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/01
 */

#ifndef TTLIBC_ENCODER_OPUSENCODER_H_
#define TTLIBC_ENCODER_OPUSENCODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../frame/audio/opus.h"
#include "../frame/audio/pcms16.h"

/**
 * opus encoder definition
 */
typedef struct ttLibC_Encoder_OpusEncoder {
	/** target sample_rate */
	uint32_t sample_rate;
	/** target channel_num */
	uint32_t channel_num;
	/** target unit sample num */
	uint32_t unit_sample_num;
	/** target bitrate */
	uint32_t bitrate;
	/** target complexity */
	uint32_t complexity;
} ttLibC_Encoder_OpusEncoder;

typedef ttLibC_Encoder_OpusEncoder ttLibC_OpusEncoder;

/**
 * callback function for opus encoder.
 * @param ptr  user def value pointer.
 * @param opus encoded opus frame.
 */
typedef bool (* ttLibC_OpusEncodeFunc)(void *ptr, ttLibC_Opus *opus);

/**
 * make opus encoder.
 * @param sample_rate     target sample_rate
 * @param channel_num     target channel_num
 * @param unit_sample_num sample_num for each opus frame.
 * @return opus encoder object
 */
ttLibC_OpusEncoder *ttLibC_OpusEncoder_make(
		uint32_t sample_rate,
		uint32_t channel_num,
		uint32_t unit_sample_num);

/**
 * encode frame.
 * @param encoder  opus encoder object
 * @param pcm      source pcm data. support little endian interleave only.
 * @param callback callback func for opus creation.
 * @param ptr      pointer for user def value, which will call in callback
 * @return true / false
 */
bool ttLibC_OpusEncoder_encode(
		ttLibC_OpusEncoder *encoder,
		ttLibC_PcmS16 *pcm,
		ttLibC_OpusEncodeFunc callback,
		void *ptr);

/**
 * update bitrate for opus encoder.
 * @param encoder opus encoder object
 * @param bitrate target bitrate.
 * @return true:success false:error
 * @note you can check real value for bitrate, encoder->bitrate;
 */
bool ttLibC_OpusEncoder_setBitrate(
		ttLibC_OpusEncoder *encoder,
		uint32_t bitrate);

/**
 * update complexity for opus encoder.
 * @param encoder    opus encoder object
 * @param complexity complexity value(0 - 10) 0:less complexity bad quality, 10:full complexity good quality.
 * @return true:success false:error
 * @note you can check real value for complexity, encoder->complexity;
 */
bool ttLibC_OpusEncoder_setComplexity(
		ttLibC_OpusEncoder *encoder,
		uint32_t complexity);

/**
 * close opus encoder
 * @param encoder
 */
void ttLibC_OpusEncoder_close(ttLibC_OpusEncoder **encoder);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_ENCODER_OPUSENCODER_H_ */
