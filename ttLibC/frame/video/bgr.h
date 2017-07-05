/**
 * @file   bgr.h
 * @brief  bgr image frame information
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/18
 */

#ifndef TTLIBC_FRAME_VIDEO_BGR_H_
#define TTLIBC_FRAME_VIDEO_BGR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "video.h"

/**
 * bgr type.
 */
typedef enum ttLibC_Bgr_Type {
	/** 24bit bgr */
	BgrType_bgr,
	/** unknown bgr */
	BgrType_abgr,
	/** 32bit bgra */
	BgrType_bgra
} ttLibC_Bgr_Type;

/**
 * bgr frame definition.
 */
typedef struct ttLibC_Frame_Video_Bgr {
	/** inherit data from ttLibC_Video */
	ttLibC_Video inherit_super;
	/** bgr type */
	ttLibC_Bgr_Type type;
	/** pixel unit size. */
	uint32_t unit_size;
	/** ref pointer for data. */
	uint8_t *data;
	/** stride data for width. */
	uint32_t width_stride;
} ttLibC_Frame_Video_Bgr;

typedef ttLibC_Frame_Video_Bgr ttLibC_Bgr;

/**
 * make bgr frame
 * @param prev_frame    reuse frame object. if NULL, create new one.
 * @param type          type of bgr
 * @param width         width of image
 * @param height        height of image
 * @param width_stride  width stride bytes size
 * @param data          bgr data
 * @param data_size     bgr data size
 * @param non_copy_mode true:hold the data pointer. false:data will copy.
 * @param pts           pts for image.
 * @param timebase      timebase number for pts.
 * @return bgr object.
 */
ttLibC_Bgr *ttLibC_Bgr_make(
		ttLibC_Bgr *prev_frame,
		ttLibC_Bgr_Type type,
		uint32_t width,
		uint32_t height,
		uint32_t width_stride,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase);

/**
 * make clone frame.
 * always make copy buffer on it.
 * @param prev_frame reuse frame object.
 * @param src_frame  source of clone.
 */
ttLibC_Bgr *ttLibC_Bgr_clone(
		ttLibC_Bgr *prev_frame,
		ttLibC_Bgr *src_frame);

/**
 * close frame
 * @param frame
 */
void ttLibC_Bgr_close(ttLibC_Bgr **frame);

/**
 * generate empty frame
 * @param sub_type type of bgr
 * @param width    width of image
 * @param height   height of image
 */
ttLibC_Bgr *ttLibC_Bgr_makeEmptyFrame(
		ttLibC_Bgr_Type sub_type,
		uint32_t        width,
		uint32_t        height);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_FRAME_VIDEO_BGR_H_ */
