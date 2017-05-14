/**
 * @file   msH264Encoder.cpp
 * @brief  windows native h264 encoder.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2017/05/13
 */

#ifndef TTLIBC_ENCODER_MSH264ENCODER_H_
#define TTLIBC_ENCODER_MSH264ENCODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <ttLibC/frame/video/h264.h>
#include <ttLibC/frame/video/yuv420.h>

typedef struct ttLibC_Encoder_MsH264Encoder {
	uint32_t width;
	uint32_t height;
	uint32_t bitrate;
} ttLibC_Encoder_MsH264Encoder;

typedef ttLibC_Encoder_MsH264Encoder ttLibC_MsH264Encoder;

typedef bool (*ttLibC_MsH264EncodeFunc)(void *ptr, ttLibC_H264 *h264);
typedef bool (*ttLibC_MsH264EncodeNameFunc)(void *ptr, const char *name);

ttLibC_MsH264Encoder *ttLibC_MsH264Encoder_make(
	const char *target,
	uint32_t width,
	uint32_t height,
	uint32_t bitrate);

bool ttLibC_MsH264Encoder_listEncoders(
	ttLibC_MsH264EncodeNameFunc callback,
	void *ptr);

bool ttLibC_MsH264Encoder_encode(
	ttLibC_MsH264Encoder *encoder,
	ttLibC_Yuv420 *frame,
	ttLibC_MsH264EncodeFunc callback,
	void *ptr);

void ttLibC_MsH264Encoder_close(ttLibC_MsH264Encoder **encoder);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_ENCODER_MSH264ENCODER_H_ */
