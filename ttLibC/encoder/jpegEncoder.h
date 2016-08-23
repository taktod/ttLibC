/**
 * @file   jpegEncoder.h
 * @brief  encode yuv420 with libjpeg.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/11/05
 */

#ifndef TTLIBC_ENCODER_JPEGENCODER_H_
#define TTLIBC_ENCODER_JPEGENCODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../frame/video/jpeg.h"
#include "../frame/video/yuv420.h"

/**
 * jpeg encoder definition
 */
typedef struct ttLibC_Encoder_JpegEncoder {
	uint32_t width;
	uint32_t height;
	uint32_t quality;
} ttLibC_Encoder_JpegEncoder;

typedef ttLibC_Encoder_JpegEncoder ttLibC_JpegEncoder;

/**
 * callback function for jpeg encoder.
 * @param ptr  user def value pointer.
 * @param jpeg encoded jpeg frame.
 */
typedef bool (* ttLibC_JpegEncodeFunc)(void *ptr, ttLibC_Jpeg *jpeg);

/**
 * make jpeg encoder
 * @param width   target width
 * @param height  target height
 * @param quality target quality 0 - 100 100 is best quality.
 * @return jpegEncoder object.
 */
ttLibC_JpegEncoder *ttLibC_JpegEncoder_make(
		uint32_t width,
		uint32_t height,
		uint32_t quality);

/**
 * encode frame.
 * @param encoder  jpeg encoder object.
 * @param yuv      source yuv420 data
 * @param callback callback func for jpeg encode.
 * @param ptr      pointer for user def value, which will call in callback.
 */
bool ttLibC_JpegEncoder_encode(
		ttLibC_JpegEncoder *encoder,
		ttLibC_Yuv420 *yuv,
		ttLibC_JpegEncodeFunc callback,
		void *ptr);

/**
 * update jpeg quality.
 * @param encoder jpeg encoder object.
 * @param quality ftarget quality 0 - 100
 * @return true:success false:error
 */
bool ttLibC_JpegEncoder_setQuality(
		ttLibC_JpegEncoder *encoder,
		uint32_t quality);

/**
 * close jpeg encoder.
 * @param encoder
 */
void ttLibC_JpegEncoder_close(ttLibC_JpegEncoder **encoder);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_ENCODER_JPEGENCODER_H_ */
