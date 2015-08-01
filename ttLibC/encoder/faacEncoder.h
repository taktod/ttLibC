/**
 * @file   faacEncoder.h
 * @brief  encode pcms16 with faac.
 *
 * this code is under LGPLv3 license.
 *
 * @author taktod
 * @date   2015/07/23
 */

#ifndef TTLIBC_ENCODER_FAACENCODER_H_
#define TTLIBC_ENCODER_FAACENCODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../frame/audio/aac.h"
#include "../frame/audio/pcms16.h"

/**
 * faac encoder type
 */
typedef enum {
	FaacEncoderType_Main,
	FaacEncoderType_Low,
	FaacEncoderType_SSR,
	FaacEncoderType_LTP
} ttLibC_FaacEncoder_Type;

/**
 * faac encoder definition
 */
typedef struct {
	uint32_t sample_rate;
	uint32_t channel_num;
	uint32_t bitrate;
} ttLibC_Encoder_FaacEncoder;

typedef ttLibC_Encoder_FaacEncoder ttLibC_FaacEncoder;

/**
 * callback function for faac encoder.
 * @param ptr user def value pointer.
 * @param aac encoded aac frame.
 */
typedef void (* ttLibC_FaacEncodeFunc)(void *ptr, ttLibC_Aac *aac);

/**
 * make faac encoder
 * @param type        target type of aac
 * @param sample_rate target sample_rate
 * @param channel_num target channel_num
 * @param bitrate     target bitrate
 * @return faac encoder object.
 */
ttLibC_FaacEncoder *ttLibC_FaacEncoder_make(
		ttLibC_FaacEncoder_Type type,
		uint32_t sample_rate,
		uint32_t channel_num,
		uint32_t bitrate);

/**
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
		void *ptr);

/**
 * close faac encoder.
 * @param encoder
 */
void ttLibC_FaacEncoder_close(ttLibC_FaacEncoder **encoder);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_ENCODER_FAACENCODER_H_ */
