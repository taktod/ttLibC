/**
 * @file  imageResizer.h
 * @brief library or image resizing.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2016/9/10
 */

#ifndef TTLIBC_RESAMPLER_IMAGERESIZER_H_
#define TTLIBC_RESAMPLER_IMAGERESIZER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../ttLibC_predef.h"
#include "../frame/video/yuv420.h"
#include "../frame/video/bgr.h"

/**
 * resize image
 * @param dest_frame
 * @param src_frame
 * @param is_quick   pickup left top point instead to calculate average weight.
 * @return bool true:success false:error.
 */
bool TT_ATTRIBUTE_API ttLibC_ImageResizer_resize(
		ttLibC_Video *dest_frame,
		ttLibC_Video *src_frame,
		bool is_quick);

/**
 * resize yuv image.
 * @param prev_frame reuse image object
 * @param type       target yuv420 image type.
 * @param width      target width
 * @param height     target height
 * @param src_frame
 * @param is_quick
 * @return scaled yuv image.
 */
ttLibC_Yuv420 TT_ATTRIBUTE_API *ttLibC_ImageResizer_resizeYuv420(
		ttLibC_Yuv420 *prev_frame,
		ttLibC_Yuv420_Type type,
		uint32_t width,
		uint32_t height,
		ttLibC_Yuv420 *src_frame,
		bool is_quick);

/**
 * resize bgr image.
 * @param prev_frame
 * @param type
 * @param width
 * @param height
 * @param src_frame
 * @return scaled bgr image.
 */
ttLibC_Bgr TT_ATTRIBUTE_API *ttLibC_ImageResizer_resizeBgr(
		ttLibC_Bgr *prev_frame,
		ttLibC_Bgr_Type type,
		uint32_t width,
		uint32_t height,
		ttLibC_Bgr *src_frame);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_RESAMPLER_IMAGERESIZER_H_ */
