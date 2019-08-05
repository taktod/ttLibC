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
 * resize image with libyuv
 * @param dest_frame
 * @param src_frame
 * @param mode
 * @param sub_mode
 * @return bool.
 */
bool TT_ATTRIBUTE_API ttLibC_LibyuvResampler_resize(
		ttLibC_Video *dest_frame,
		ttLibC_Video *src_frame,
		ttLibC_LibyuvFilter_Mode mode,
		ttLibC_LibyuvFilter_Mode sub_mode);

/**
 * rotate yuv image with libyuv
 * @param prev_frame
 * @param src_frame
 * @param mode
 * @return bool.
 */
bool TT_ATTRIBUTE_API ttLibC_LibyuvResampler_rotate(
		ttLibC_Video *dest_frame,
		ttLibC_Video *src_frame,
		ttLibC_LibyuvRotate_Mode mode);

/**
 * convert to bgr
 * @param dest_frame
 * @param src_frame
 * @return bool.
 */
bool TT_ATTRIBUTE_API ttLibC_LibyuvResampler_ToBgr(
		ttLibC_Bgr   *dest_frame,
		ttLibC_Video *src_frame);

/**
 * convert to yuv
 * @param dest_frame
 * @param src_frame
 * @return bool.
 */
bool TT_ATTRIBUTE_API ttLibC_LibyuvResampler_ToYuv420(
		ttLibC_Yuv420 *dest_frame,
		ttLibC_Video  *src_frame);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_RESAMPLER_LIBYUVRESAMPLER_H_ */
