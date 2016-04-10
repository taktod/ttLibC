/**
 * @file   vorbisDecoder.h
 * @brief  decode vorbis with libvorbis
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/04/08
 */

#ifndef TTLIBC_DECODER_VORBISDECODER_H_
#define TTLIBC_DECODER_VORBISDECODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../frame/audio/vorbis.h"
#include "../frame/audio/pcmf32.h"

/**
 * vorbis decoder definition
 */
typedef struct ttLibC_Decoder_VorbisDecoder {
	uint32_t sample_rate;
	uint32_t channel_num;
} ttLibC_Decoder_VorbisDecoder;

typedef ttLibC_Decoder_VorbisDecoder ttLibC_VorbisDecoder;

/**
 * callback
 * @param ptr user def value pointer.
 * @param pcm generated frame.
 * @return true:continue / false:stop
 */
typedef bool (* ttLibC_VorbisDecodeFunc)(void *ptr, ttLibC_PcmF32 *pcm);

/**
 * make vorbisDecoder
 * @return vorbisDecoder object.
 */
ttLibC_VorbisDecoder *ttLibC_VorbisDecoder_make();

/**
 * make vorbisDecoder with vorbis_info
 * @param vi
 * @return vorbisDecoder object.
 */
//ttLibC_VorbisDecoder *ttLibC_VorbisDecoder_makeWithInfo(void *vi);

/**
 * decode frame.
 * @param decoder
 * @param vorbis
 * @param callback
 * @param ptr
 * @return true:success / false:error
 */
bool ttLibC_VorbisDecoder_decode(
		ttLibC_VorbisDecoder *decoder,
		ttLibC_Vorbis *vorbis,
		ttLibC_VorbisDecodeFunc callback,
		void *ptr);

/**
 * close decoder
 * @param decoder
 */
void ttLibC_VorbisDecoder_close(ttLibC_VorbisDecoder **decoder);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_DECODER_VORBISDECODER_H_ */
