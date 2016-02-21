/**
 * @file   vtCompressSessionH264Encoder.h
 * @brief  osx or ios native h264 encode.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/02/20
 */

#ifndef TTLIBC_ENCODER_VTCOMPRESSSESSIONH264ENCODER_H_
#define TTLIBC_ENCODER_VTCOMPRESSSESSIONH264ENCODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../frame/video/h264.h"
#include "../frame/video/yuv420.h"

typedef struct ttLibC_Encoder_VtConpressionSession_VtH264Encoder {
	/** target width */
	uint32_t width;
	/** target height */
	uint32_t height;
} ttLibC_Encoder_VtConpressionSession_VtH264Encoder;

typedef ttLibC_Encoder_VtConpressionSession_VtH264Encoder ttLibC_VtH264Encoder;

/**
 * callback function for vtH264Encoder
 * @param ptr  user def value pointer.
 * @param h264 encoded h264 frame.
 */
typedef bool (* ttLibC_VtH264EncodeFunc)(void *ptr, ttLibC_H264 *h264);

/**
 * make vtH264Encoder.
 * @param width
 * @param height
 * @return ttLibCVtH264Encoder object.
 */
ttLibC_VtH264Encoder *ttLibC_VtH264Encoder_make(
		uint32_t width,
		uint32_t height);

/**
 * make vtH264Encoder.
 * @param width
 * @param height
 * @param fps
 * @param bitrate
 * @param is_baseline true:baseline false:main
 */
ttLibC_VtH264Encoder *ttLibC_VtH264Encoder_make_ex(
		uint32_t width,
		uint32_t height,
		uint32_t fps,
		uint32_t bitrate,
		bool is_baseline);

bool ttLibC_VtH264Encoder_encode(
		ttLibC_VtH264Encoder *encoder,
		ttLibC_Yuv420 *yuv420,
		ttLibC_VtH264EncodeFunc callback,
		void *ptr);

void ttLibC_VtH264Encoder_close(ttLibC_VtH264Encoder **encoder);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_ENCODER_VTCOMPRESSSESSIONH264ENCODER_H_ */
