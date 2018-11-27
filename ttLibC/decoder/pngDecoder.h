/**
 * @file   pngDecoder.h
 * @brief  decode png with libpng.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2018/11/25
 */

#ifndef TTLIBC_DECODER_PNGDECODER_H_
#define TTLIBC_DECODER_PNGDECODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../frame/video/png.h"
#include "../frame/video/bgr.h"

/**
 * png decoder definition
 */
/*typedef struct ttLibC_Decoder_PngDecoder {
	;
} ttLibC_Decoder_PngDecoder;
*/
typedef void* ttLibC_Decoder_PngDecoder;
typedef ttLibC_Decoder_PngDecoder ttLibC_PngDecoder;
/**
 * callback function for png decoder.
 */
typedef bool (* ttLibC_PngDecodeFunc)(void *ptr, ttLibC_Bgr *bgr);

/**
 * make png decoder
 * @return pngDecoder object.
 */
ttLibC_PngDecoder *ttLibC_PngDecoder_make();

/**
 * decode frame
 * @param decoder  png decoder object.
 * @param png      source png data
 * @param callback callback func for png decoder
 * @param ptr      pointer for use def value, which will call in callback.
 * @return true / false
 */
bool ttLibC_PngDecoder_decode(
		ttLibC_PngDecoder *decoder,
		ttLibC_Png *png,
		ttLibC_PngDecodeFunc callback,
		void *ptr);

/**
 * close png decoder
 * @param decoder
 */
void ttLibC_PngDecoder_close(ttLibC_PngDecoder **decoder);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_DECODER_PNGDECODER_H_ */