/**
 * @file   x265Encoder.h
 * @brief  encode h265 with x265
 *
 * this code is under GPLv3 license.
 *
 * @author taktod
 * @date   2016/02/26
 */

#ifndef TTLIBC_ENCODER_X265ENCODER_H_
#define TTLIBC_ENCODER_X265ENCODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../frame/video/h265.h"
#include "../frame/video/yuv420.h"

/**
 * x265 encoder definition
 */
typedef struct ttLibC_Encoder_X265Encoder {
	/** target width */
	uint32_t width;
	/** target height */
	uint32_t height;
} ttLibC_Encoder_X265Encoder;

typedef ttLibC_Encoder_X265Encoder ttLibC_X265Encoder;

/**
 * callback function for x265 encoder.
 * @param ptr
 * @param h265
 */
typedef bool (* ttLibC_X265EncodeFunc)(void *ptr, ttLibC_H265 *h265);

ttLibC_X265Encoder *ttLibC_X265Encoder_make(
		uint32_t width,
		uint32_t height);

bool ttLibC_X265Encoder_encode(
		ttLibC_X265Encoder *encoder,
		ttLibC_Yuv420 *yuv420,
		ttLibC_X265EncodeFunc callback,
		void *ptr);

void ttLibC_X265Encoder_close(ttLibC_X265Encoder **encoder);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_ENCODER_X265ENCODER_H_ */
