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

#include "../ttLibC_predef.h"
#include "../frame/video/h265.h"
#include "../frame/video/yuv420.h"

typedef enum ttLibC_X265Encoder_FrameType {
	X265FrameType_Auto     = 0x0000,
	X265FrameType_IDR      = 0x0001,
	X265FrameType_I        = 0x0002,
	X265FrameType_P        = 0x0003,
	X265FrameType_Bref     = 0x0004,
	X265FrameType_B        = 0x0005,
} ttLibC_X265Encoder_FrameType;

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

ttLibC_X265Encoder TT_ATTRIBUTE_API *ttLibC_X265Encoder_make(
		uint32_t width,
		uint32_t height);

ttLibC_X265Encoder TT_ATTRIBUTE_API *ttLibC_X265Encoder_make_ex(
		uint32_t width,
		uint32_t height,
		uint32_t bitrate);

bool TT_ATTRIBUTE_API ttLibC_X265Encoder_getDefaultX265ApiAndParam(
		void **api,
		void **param,
		const char *preset,
		const char *tune,
		uint32_t width,
		uint32_t height);

ttLibC_X265Encoder TT_ATTRIBUTE_API *ttLibC_X265Encoder_makeWithX265ApiAndParam(void *api, void *param);

bool TT_ATTRIBUTE_API ttLibC_X265Encoder_forceNextFrameType(
		ttLibC_X265Encoder *encoder,
		ttLibC_X265Encoder_FrameType frame_type);

bool TT_ATTRIBUTE_API ttLibC_X265Encoder_encode(
		ttLibC_X265Encoder *encoder,
		ttLibC_Yuv420 *yuv420,
		ttLibC_X265EncodeFunc callback,
		void *ptr);

void TT_ATTRIBUTE_API ttLibC_X265Encoder_close(ttLibC_X265Encoder **encoder);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_ENCODER_X265ENCODER_H_ */
