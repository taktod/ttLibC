/*
 * libyuvResampler.h
 *
 *  Created on: 2017/04/27
 *      Author: taktod
 */

#ifndef TTLIBC_RESAMPLER_LIBYUVRESAMPLER_H_
#define TTLIBC_RESAMPLER_LIBYUVRESAMPLER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../ttLibC_predef.h"
#include "../frame/video/yuv420.h"
#include "../frame/video/bgr.h"

typedef enum ttLibC_LibyuvFilter_Mode {
	LibyuvFilter_None,
	LibyuvFilter_Linear,
	LibyuvFilter_Bilinear,
	LibyuvFilter_Box
} ttLibC_LibyuvFilter_Mode;

typedef enum ttLibC_LibyuvRotate_Mode {
	LibyuvRotate_0,
	LibyuvRotate_90,
	LibyuvRotate_180,
	LibyuvRotate_270
} ttLibC_LibyuvRotate_Mode;

/**
 * resize yuv image with libyuv
 * @param prev_frame
 * @param width
 * @param height
 * @param src_frame
 * @param y_mode
 * @param u_mode
 * @param v_mode
 * @return scaled yuv image.
 */
ttLibC_Yuv420 TT_ATTRIBUTE_API *ttLibC_LibyuvResampler_resize(
		ttLibC_Yuv420 *prev_frame,
		uint32_t width,
		uint32_t height,
		ttLibC_Yuv420 *src_frame,
		ttLibC_LibyuvFilter_Mode y_mode,
		ttLibC_LibyuvFilter_Mode u_mode,
		ttLibC_LibyuvFilter_Mode v_mode);

/**
 * rotate yuv image with libyuv
 * @param prev_frame
 * @param src_frame
 * @param mode
 * @return rotate yuv image.
 */
ttLibC_Yuv420 TT_ATTRIBUTE_API *ttLibC_LibyuvResampler_rotate(
		ttLibC_Yuv420 *prev_frame,
		ttLibC_Yuv420 *src_frame,
		ttLibC_LibyuvRotate_Mode mode);

/**
 * convert from yuv to bgr
 * @param prev_frame
 * @param src_frame
 * @param bgr_type
 * @return new bgr frame.
 */
ttLibC_Bgr TT_ATTRIBUTE_API *ttLibC_LibyuvResampler_ToBgr(
		ttLibC_Bgr *prev_frame,
		ttLibC_Yuv420 *src_frame,
		ttLibC_Bgr_Type bgr_type);

/**
 * convert from yuv to bgr
 * @param prev_frame
 * @param src_frame
 * @param bgr_type
 * @return new bgr frame.
 */
ttLibC_Yuv420 TT_ATTRIBUTE_API *ttLibC_LibyuvResampler_ToYuv420(
		ttLibC_Yuv420 *prev_frame,
		ttLibC_Bgr *src_frame,
		ttLibC_Yuv420_Type yuv420_type);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_RESAMPLER_LIBYUVRESAMPLER_H_ */
