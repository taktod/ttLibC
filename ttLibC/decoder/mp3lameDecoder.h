/**
 * @file   mp3lameDecoder.h
 * @brief  decode mp3 with mp3lame.
 *
 * this code is under GPLv3 license.
 *
 * @author taktod
 * @date   2015/07/28
 */

#ifndef TTLIBC_DECODER_MP3LAMEDECODER_H_
#define TTLIBC_DECODER_MP3LAMEDECODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../frame/audio/mp3.h"
#include "../frame/audio/pcms16.h"

/**
 * mp3lame decoder type
 * /
typedef enum ttLibC_Mp3lameDecoder_Type {
	;
} ttLibC_Mp3lameDecoder_Type;
*/

/**
 * mp3lame decoder definition
 */
typedef struct ttLibC_Decoder_Mp3lameDecoder {
//	ttLibC_Mp3lameDecoder_Type type;
	uint32_t sample_rate;
	uint32_t channel_num;
} ttLibC_Decoder_Mp3lameDecoder;

typedef ttLibC_Decoder_Mp3lameDecoder ttLibC_Mp3lameDecoder;

/**
 * callback function for mp3lame decoder.
 * @param ptr    user def value pointer.
 * @param pcms16 decoded frame.
 */
typedef bool (* ttLibC_Mp3lameDecodeFunc)(void *ptr, ttLibC_PcmS16 *pcms16);

/**
 * make mp3lame decoder
 */
ttLibC_Mp3lameDecoder *ttLibC_Mp3lameDecoder_make();

/**
 * decode frame.
 * @param decoder  mp3lame decoder object
 * @param mp3      source mp3 data.
 * @param callback callback func for mp3 decode.
 * @param ptr      pointer for user def value, which willl call in callback.
 * @return true / false
 */
bool ttLibC_Mp3lameDecoder_decode(
		ttLibC_Mp3lameDecoder *decoder,
		ttLibC_Mp3 *mp3,
		ttLibC_Mp3lameDecodeFunc callback,
		void *ptr);

/**
 * close mp3lame decoder.
 * @param decoder.
 */
void ttLibC_Mp3lameDecoder_close(ttLibC_Mp3lameDecoder **decoder);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_DECODER_MP3LAMEDECODER_H_ */
