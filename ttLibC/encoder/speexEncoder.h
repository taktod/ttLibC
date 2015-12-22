/**
 * @file   speexEncoder.h
 * @brief  encoder speex with speex.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/28
 */

#ifndef TTLIBC_ENCODER_SPEEXENCODER_H_
#define TTLIBC_ENCODER_SPEEXENCODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../frame/audio/speex.h"
#include "../frame/audio/pcms16.h"

/**
 * speex encoder type
 */
typedef enum ttLibC_SpeexEncoder_Type {
	SpeexEncoderType_CBR,
	SpeexEncoderType_ABR,
	SpeexEncoderType_VBR,
} ttLibC_SpeexEncoder_Type;

/**
 * speex encoder definition
 */
typedef struct ttLibC_Encoder_SpeexEncoder {
	/** type information */
	ttLibC_SpeexEncoder_Type type;
	/** target sample_rate */
	uint32_t sample_rate;
	/** target channel_num */
	uint32_t channel_num;
	/** target quality */
	uint32_t quality;
} ttLibC_Encoder_SpeexEncoder;

typedef ttLibC_Encoder_SpeexEncoder ttLibC_SpeexEncoder;

/**
 * callback function for speex encoder.
 * @param ptr   user def value pointer.
 * @param speex encoded speex frame.
 */
typedef bool (* ttLibC_SpeexEncodeFunc)(void *ptr, ttLibC_Speex *speex);

/**
 * make speex encoder(CBR).
 * @param sample_rate target sample_rate
 * @param channel_num target channel_num
 * @param quality     target quality
 * @return speex encoder object
 */
ttLibC_SpeexEncoder *ttLibC_SpeexEncoder_make(
		uint32_t sample_rate,
		uint32_t channel_num,
		uint32_t quality);

/**
 * encode frame.
 * @param encoder  speex encoder object.
 * @param pcm      source pcm data. support little endian interleave only.
 * @param callback callback func for speex creation.
 * @param ptr      pointer for user def value, which will call in callback.
 * @return true / false
 */
bool ttLibC_SpeexEncoder_encode(
		ttLibC_SpeexEncoder *encoder,
		ttLibC_PcmS16 *pcm,
		ttLibC_SpeexEncodeFunc callback,
		void *ptr);

/**
 * close speex encoder.
 * @param encoder
 */
void ttLibC_SpeexEncoder_close(ttLibC_SpeexEncoder **encoder);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_ENCODER_SPEEXENCODER_H_ */
