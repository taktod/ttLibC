/**
 * @file   imageResampler.h
 * @brief  library for image resample.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/19
 */

#ifndef TTLIBC_RESAMPLER_IMAGERESAMPLER_H_
#define TTLIBC_RESAMPLER_IMAGERESAMPLER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../frame/video/yuv420.h"
#include "../frame/video/bgr.h"

/**
 * make yuv420 frame from bgr frame.
 * @param prev_frame reuse frame.
 * @param type       yuv420 type.
 * @param src_frame  src bgr frame.
 */
ttLibC_Yuv420 *ttLibC_ImageResampler_makeYuv420FromBgr(
		ttLibC_Yuv420 *prev_frame,
		ttLibC_Yuv420_Type type,
		ttLibC_Bgr *src_frame);

/**
 * make bgr frame from yuv420 frame.
 * @param prev_frame reuse frame.
 * @param type       bgr type.
 * @param src_frame  src yuv420 frame.
 */
ttLibC_Bgr *ttLibC_ImageResampler_makeBgrFromYuv420(
		ttLibC_Bgr *prev_frame,
		ttLibC_Bgr_Type type,
		ttLibC_Yuv420 *src_frame);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_RESAMPLER_IMAGERESAMPLER_H_ */
