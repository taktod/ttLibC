/**
 * @file   jpegDecoder.h
 * @brief  decode jpeg with libjpeg
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/11/05
 */

#ifndef TTLIBC_DECODER_JPEGDECODER_H_
#define TTLIBC_DECODER_JPEGDECODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../frame/video/yuv420.h"
#include "../frame/video/jpeg.h"

/**
 * jpeg decoder definition
 */
typedef struct ttLibC_Decoder_JpegDecoder {
	uint32_t width;
	uint32_t height;
} ttLibC_Decoder_JpegDecoder;

typedef ttLibC_Decoder_JpegDecoder ttLibC_JpegDecoder;

/**
 * callback function for jpeg decoder.
 */
typedef bool (* ttLibC_JpegDecodeFunc)(void *ptr, ttLibC_Yuv420 *yuv);

/**
 * make jpeg decoder
 * @return jpegDecoder object.
 */
ttLibC_JpegDecoder *ttLibC_JpegDecoder_make();

/**
 * decode frame
 * @param decoder  jpeg decoder object.
 * @param jpeg     source jpeg data
 * @param callback callback func for jpeg decode
 * @param ptr      pointer for use def value, which will call in callback.
 */
void ttLibC_JpegDecoder_decode(
		ttLibC_JpegDecoder *decoder,
		ttLibC_Jpeg *jpeg,
		ttLibC_JpegDecodeFunc callback,
		void *ptr);

/**
 * close jpeg decoder
 * @param decoder
 */
void ttLibC_JpegDecoder_close(ttLibC_JpegDecoder **decoder);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_DECODER_JPEGDECODER_H_ */
