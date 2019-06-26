/**
 * @file   yuv420.h
 * @brief  yuv420 image frame information.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/18
 */

#ifndef TTLIBC_FRAME_VIDEO_YUV420_H_
#define TTLIBC_FRAME_VIDEO_YUV420_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../../ttLibC_predef.h"
#include "video.h"

/**
 * yuv420 type
 */
typedef enum ttLibC_Yuv420_Type {
	/** planar data. yyy... uu... vv...*/
	Yuv420Type_planar,
	/** semiPlanar data. yyy... uvuv... */
	Yuv420Type_semiPlanar,
	/** planar data for yvu. yyy... vv... uu... */
	Yvu420Type_planar,
	/** semiPlanar data for yvu. yyy... vuvu... */
	Yvu420Type_semiPlanar,
} ttLibC_Yuv420_Type;

/**
 * yuv420 frame definition.
 */
typedef struct ttLibC_Frame_Video_Yuv420 {
	/** inherit data from ttLibC_Video */
	ttLibC_Video inherit_super;
	/** yuv420 type */
	ttLibC_Yuv420_Type type;
	/** ref pointer for y_data. */
	uint8_t *y_data;
	/** stride data for y-width */
	uint32_t y_stride;
	/** step for next y pixel. */
	uint32_t y_step;
	/** ref pointer for u_data. */
	uint8_t *u_data;
	/** stride data for u-width */
	uint32_t u_stride;
	/** step for next u pixel. */
	uint32_t u_step;
	/** ref pointer for v_data. */
	uint8_t *v_data;
	/** stride data for v-width */
	uint32_t v_stride;
	/** step for next v pixel. */
	uint32_t v_step;
} ttLibC_Frame_Video_Yuv420;

typedef ttLibC_Frame_Video_Yuv420 ttLibC_Yuv420;

/**
 * make yuv420 frame
 * @param prev_frame    reuse frame object. if NULL, create new one.
 * @param type          type of yuv420
 * @param width         width of image
 * @param height        height of image
 * @param data          yuv420 data
 * @param data_size     data size
 * @param y_data        pointer for y_data
 * @param y_stride      stride for each line for y_data
 * @param u_data        pointer for u_data
 * @param u_stride      stride for each line for u_data
 * @param v_data        pointer for v_data
 * @param v_stride      stride for each line for v_data
 * @param non_copy_mode true:hold the data pointer. false:data will copy
 * @param pts           pts for image
 * @param timebase      timebase number for pts.
 * @return yuv420 object.
 */
ttLibC_Yuv420 TT_ATTRIBUTE_API *ttLibC_Yuv420_make(
		ttLibC_Yuv420 *prev_frame,
		ttLibC_Yuv420_Type type,
		uint32_t width,
		uint32_t height,
		void *data,
		size_t data_size,
		void *y_data,
		uint32_t y_stride,
		void *u_data,
		uint32_t u_stride,
		void *v_data,
		uint32_t v_stride,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase);

/**
 * make clone frame.
 * always make copy buffer on it.
 * @param prev_frame reuse frame object.
 * @param src_frame  source of clone.
 */
ttLibC_Yuv420 TT_ATTRIBUTE_API *ttLibC_Yuv420_clone(
		ttLibC_Yuv420 *prev_frame,
		ttLibC_Yuv420 *src_frame);

/**
 * close frame
 * @param frame
 */
void TT_ATTRIBUTE_API ttLibC_Yuv420_close(ttLibC_Yuv420 **frame);

/**
 * generate empty frame
 * @param sub_type type of yuv
 * @param width    width of image
 * @param height   height of image
 */
ttLibC_Yuv420 TT_ATTRIBUTE_API *ttLibC_Yuv420_makeEmptyFrame(
		ttLibC_Yuv420_Type sub_type,
		uint32_t           width,
		uint32_t           height);

/**
 * generate empty frame
 * @param prev_frame reuse frame
 * @param sub_type   type of yuv
 * @param width      width of image
 * @param height     height of image
 */
ttLibC_Yuv420 TT_ATTRIBUTE_API *ttLibC_Yuv420_makeEmptyFrame2(
		ttLibC_Yuv420     *prev_frame,
		ttLibC_Yuv420_Type sub_type,
		uint32_t           width,
		uint32_t           height);

/**
 * get minimum size of binary buffer.
 * @param yuv      target yuv frame
 * @param callback binary buffer callback
 * @param ptr      user def pointer.
 * @return true / false
 */
bool TT_ATTRIBUTE_API ttLibC_Yuv420_getMinimumBinaryBuffer(
		ttLibC_Yuv420 *yuv,
		ttLibC_FrameBinaryFunc callback,
		void *ptr);

/**
 * reset changed data.
 * @param yuv target bgr frame.
 */
void TT_ATTRIBUTE_API ttLibC_Yuv420_resetData(ttLibC_Yuv420 *yuv);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_FRAME_VIDEO_YUV420_H_ */
