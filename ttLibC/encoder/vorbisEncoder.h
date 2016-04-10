/**
 * @file   vorbisEncoder.h
 * @brief  encode vorbis with libvorbis
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/04/08
 */

#ifndef TTLIBC_ENCODER_VORBISENCODER_H_
#define TTLIBC_ENCODER_VORBISENCODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../frame/audio/audio.h"
#include "../frame/audio/vorbis.h"

/**
 * vorbis encoder definition
 */
typedef struct ttLibC_Encoder_VorbisEncoder {
	uint32_t sample_rate;
	uint32_t channel_num;
} ttLibC_Encoder_VorbisEncoder;

typedef ttLibC_Encoder_VorbisEncoder ttLibC_VorbisEncoder;

/**
 * callback function for vorbis encoder.
 * @param ptr    user def value pointer.
 * @param vorbis encoded vorbis frame.
 */
typedef bool (* ttLibC_VorbisEncodeFunc)(void *ptr, ttLibC_Vorbis *vorbis);

/**
 * make vorbisEncoder
 * @param sample_rate
 * @param channel_num
 * @return vorbisEncoder object.
 */
ttLibC_VorbisEncoder *ttLibC_VorbisEncoder_make(
		uint32_t sample_rate,
		uint32_t channel_num);

/**
 * make vorbisEncoder
 * @param vi
 * @return vorbisEncoder object.
 */
//ttLibC_VorbisEncoder *ttLibC_VorbisEncoder_makeWithInfo(void *vi);

/**
 * ref vorbis context
 * @param encoder
 * @return NULL
 */
//void *ttLibC_VorbisEncoder_refNativeEncodeContext(ttLibC_VorbisEncoder *encoder);

/**
 * encode frame
 * @param encoder
 * @param pcm (pcmS16 pcmF32 is supported.)
 * @param callback
 * @param ptr
 * @return true:success / false:error
 */
bool ttLibC_VorbisEncoder_encode(
		ttLibC_VorbisEncoder *encoder,
		ttLibC_Audio *pcm,
		ttLibC_VorbisEncodeFunc callback,
		void *ptr);

/**
 * close encoder
 * @param encoder
 */
void ttLibC_VorbisEncoder_close(ttLibC_VorbisEncoder **encoder);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_ENCODER_VORBISENCODER_H_ */
