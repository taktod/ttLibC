/**
 * @file   audioConverterDecoder.h
 * @brief  osx or iso native audio decode.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/01
 */

#ifndef TTLIBC_DECODER_AUDIOCONVERTERDECODER_H_
#define TTLIBC_DECODER_AUDIOCONVERTERDECODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../frame/audio/audio.h"
#include "../frame/audio/pcms16.h"

/**
 * audioConverter decoder definition.
 */
typedef struct ttLibC_Decoder_AudioConverter_AcDecoder {
	uint32_t sample_rate;
	uint32_t channel_num;
	ttLibC_Frame_Type frame_type;
} ttLibC_Decoder_AudioConverter_AcDecoder;

typedef ttLibC_Decoder_AudioConverter_AcDecoder ttLibC_AcDecoder;

/**
 * callback function for audioConverter decoder.
 * @param ptr user def value pointer.
 * @param pcm decoded pcms16 frame.
 * @return true:success false:error.
 */
typedef bool (*ttLibC_AcDecodeFunc)(void *ptr, ttLibC_PcmS16 *pcm);

/**
 * make audioConverter decoder.
 * @param sample_rate       target sample rate
 * @param channel_num       target channel num
 * @param target_frame_type target frame type(currently support frameType_aac or frameType_mp3)
 * @return ttLibC_AcDecoder object.
 */
ttLibC_AcDecoder *ttLibC_AcDecoder_make(
		uint32_t sample_rate,
		uint32_t channel_num,
		ttLibC_Frame_Type target_frame_type);

/**
 * decode frame.
 * @param decoder  acDecoder object.
 * @param audio    source audio frame.
 * @param callback callback func for audio decode.
 * @param ptr      pointer for user def value, which will call in callback.
 * @return true / false
 */
bool ttLibC_AcDecoder_decode(
		ttLibC_AcDecoder *decoder,
		ttLibC_Audio *audio,
		ttLibC_AcDecodeFunc callback,
		void *ptr);

/**
 * close acDecoder object.
 * @param decoder
 */
void ttLibC_AcDecoder_close(ttLibC_AcDecoder **decoder);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_DECODER_AUDIOCONVERTERDECODER_H_ */
