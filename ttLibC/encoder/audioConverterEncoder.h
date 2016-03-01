/**
 * @file   audioConverterEncoder.h
 * @brief  osx or iso native audio encode.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/01
 */

#ifndef TTLIBC_ENCODER_AUDIOCONVERTERENCODER_H_
#define TTLIBC_ENCODER_AUDIOCONVERTERENCODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../frame/audio/audio.h"
#include "../frame/audio/pcms16.h"

/**
 * audioConverter encoder definition.
 */
typedef struct ttLibC_Encoder_AudioConverter_AcEncoder {
	uint32_t sample_rate;
	uint32_t channel_num;
	uint32_t bitrate;
	ttLibC_Frame_Type frame_type;
} ttLibC_Encoder_AudioConverter_AcEncoder;

typedef ttLibC_Encoder_AudioConverter_AcEncoder ttLibC_AcEncoder;

/**
 * callback function for audioConverter encoder.
 * @param ptr   user def value pointer.
 * @param audio encode audio frame.
 * @return true:success false:error
 */
typedef bool (*ttLibC_AcEncodeFunc)(void *ptr, ttLibC_Audio *audio);

/**
 * make audioConverter encoder.
 * @param sample_rate       target sample rate
 * @param channel_num       target channel num
 * @param target_bitrate    target bitrate in bits/sec
 * @param target_frame_type target frame type(currently support frameType_aac only(low profile.))
 * @return ttLibC_AcEncoder object.
 */
ttLibC_AcEncoder *ttLibC_AcEncoder_make(
		uint32_t sample_rate,
		uint32_t channel_num,
		uint32_t target_bitrate,
		ttLibC_Frame_Type target_frame_type);

/**
 * encode frame.
 * @param encoder  acEncoder object.
 * @param pcm      source pcm
 * @param callback callback func for audio encode.
 * @param ptr      pointer for user def value, which will call in callback.
 * @return true / false
 */
bool ttLibC_AcEncoder_encode(
		ttLibC_AcEncoder *encoder,
		ttLibC_PcmS16 *pcm,
		ttLibC_AcEncodeFunc callback,
		void *ptr);

/**
 * close acEncoder object.
 * @param encoder
 */
void ttLibC_AcEncoder_close(ttLibC_AcEncoder **encoder);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_ENCODER_AUDIOCONVERTERENCODER_H_ */
