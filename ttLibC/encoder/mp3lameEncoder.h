/**
 * @file   mp3lameEncoder.h
 * @brief  encode pcms16 with mp3lame.
 *
 * this code is under LGPLv3 license.
 *
 * @author taktod
 * @date   2015/07/21
 */

#ifndef TTLIBC_ENCODER_MP3LAMEENCODER_H_
#define TTLIBC_ENCODER_MP3LAMEENCODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../ttLibC_predef.h"
#include "../frame/audio/mp3.h"
#include "../frame/audio/pcms16.h"

/**
 * mp3lame encoder type
 */
typedef enum ttLibC_Mp3lameEncoder_Type {
	Mp3lameEncoderType_CBR,
	Mp3lameEncoderType_ABR,
	Mp3lameEncoderType_VBR,
} ttLibC_Mp3lameEncoder_Type;

/**
 * mp3lame encoder definition
 */
typedef struct ttLibC_Encoder_Mp3lameEncoder {
	/** mp3lame encoder type */
	ttLibC_Mp3lameEncoder_Type type;
	/** target sample_rate */
	uint32_t sample_rate;
	/** target channel num 1:monoral 2:stereo */
	uint32_t channel_num;
	/** mp3lame convert quality 0 - 10 2:near best, 5 good fast, 7 ok, very fast. */
	uint32_t quality;
} ttLibC_Encoder_Mp3lameEncoder;

typedef ttLibC_Encoder_Mp3lameEncoder ttLibC_Mp3lameEncoder;

/**
 * callback function for mp3lame encoder.
 * @param ptr user def value pointer.
 * @param mp3 encoded mp3 frame.
 */
typedef bool (* ttLibC_Mp3lameEncodeFunc)(void *ptr, ttLibC_Mp3 *mp3);

/**
 * make mp3lame encoder (cbr)
 * @param sample_rate desire sample_rate
 * @param channel_num desire channel_num
 * @param quality     target quality from 0 to 10. 2:near best 5:good fast 7:ok,very fast.
 * @return mp3lame encoder object.
 */
ttLibC_Mp3lameEncoder TT_ATTRIBUTE_API *ttLibC_Mp3lameEncoder_make(
		uint32_t sample_rate,
		uint32_t channel_num,
		uint32_t quality);

/**
 * function to make mp3lame encoder for vbr or abr, I'm planning to make later.
 */
//ttLibC_Mp3lameEncoder *ttLibC_Mp3lameEncoder_make_ex();

/**
 * encode frame.
 * @param encoder  mp3lame encoder object.
 * @param pcm      source pcm data.
 * @param callback callback func for mp3 creation.
 * @param ptr      pointer for user def value, which will call in callback.
 * @return true / false
 */
bool TT_ATTRIBUTE_API ttLibC_Mp3lameEncoder_encode(
		ttLibC_Mp3lameEncoder *encoder,
		ttLibC_PcmS16 *pcm,
		ttLibC_Mp3lameEncodeFunc callback,
		void *ptr);

/**
 * close mp3lame encoder.
 * @param encoder
 */
void TT_ATTRIBUTE_API ttLibC_Mp3lameEncoder_close(ttLibC_Mp3lameEncoder **encoder);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_ENCODER_MP3LAMEENCODER_H_ */
