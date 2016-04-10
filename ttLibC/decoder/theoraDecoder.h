/**
 * @file   theoraDecoder.h
 * @brief  decode theora with libtheora.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/04/08
 */

#ifndef TTLIBC_DECODER_THEORADECODER_H_
#define TTLIBC_DECODER_THEORADECODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../frame/video/theora.h"
#include "../frame/video/yuv420.h"

/**
 * theora decoder definition
 */
typedef struct ttLibC_Decoder_TheoraDecoder {
	uint32_t width;
	uint32_t height;
} ttLibC_Decoder_TheoraDecoder;

typedef ttLibC_Decoder_TheoraDecoder ttLibC_TheoraDecoder;

/**
 * callback function for theora decoder
 * @param ptr    user def value pointer.
 * @param yuv420 decoded yuv420 frame.
 * @return true:success to continue. / false:error to stop.
 */
typedef bool (* ttLibC_TheoraDecodeFunc)(void *ptr, ttLibC_Yuv420 *yuv420);

/**
 * make theoraDecoder
 * @return theoraDecoder object.
 */
ttLibC_TheoraDecoder *ttLibC_TheoraDecoder_make();

/**
 * make theoraDecoder with th_info
 * @param ti th_info object.
 * @return theoraDecoder object.
 */
ttLibC_TheoraDecoder *ttLibC_TheoraDecoder_makeWithInfo(void *ti);

/**
 * ref th_dec_ctx object.
 * @param decoder
 * @return th_dec_ctx object.
 */
void *ttLibC_TheoraDecoder_refNativeDecodeContext(ttLibC_TheoraDecoder *decoder);

/**
 * decode frame
 * @param decoder
 * @param theora
 * @param callback
 * @param ptr
 * @return true:sucess / false:error
 */
bool ttLibC_TheoraDecoder_decode(
		ttLibC_TheoraDecoder *decoder,
		ttLibC_Theora *theora,
		ttLibC_TheoraDecodeFunc callback,
		void *ptr);

/**
 * close theora decoder.
 * @param decoder
 */
void ttLibC_TheoraDecoder_close(ttLibC_TheoraDecoder **decoder);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_DECODER_THEORADECODER_H_ */
