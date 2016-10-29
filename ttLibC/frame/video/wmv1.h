/**
 * @file   wmv1.h
 * @brief  wmv1 image frame information.
 *
 * this code is under 3-Cause BSD license
 *
 * @author taktod
 * @date   2015/08/06
 */

#ifndef TTLIBC_FRAME_VIDEO_WMV1_H_
#define TTLIBC_FRAME_VIDEO_WMV1_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "video.h"

/**
 * wmv1 frame definition
 */
typedef struct ttLibC_Frame_Video_Wmv1 {
	/** inherit data from ttLibC_Video */
	ttLibC_Video inherit_super;
} ttLibC_Frame_Video_Wmv1;

typedef ttLibC_Frame_Video_Wmv1 ttLibC_Wmv1;

/**
 * make wmv1 frame
 * @param prev_frame    reuse frame
 * @param video_type    video type of wmv1
 * @param width         width
 * @param height        height
 * @param data          wmv1 data
 * @param data_size     wmv1 data size
 * @param non_copy_mode true:hold the data pointer. false:data will copy.
 * @param pts           pts for wmv1 data.
 * @param timebase      timebase number for pts.
 */
ttLibC_Wmv1 *ttLibC_Wmv1_make(
		ttLibC_Wmv1 *prev_frame,
		ttLibC_Video_Type video_type,
		uint32_t width,
		uint32_t height,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase);

/**
 * make clone frame
 * always make copy buffer on it.
 * @param prev_frame reuse frame object.
 * @param src_frame  source of clone.
 */
ttLibC_Wmv1 *ttLibC_Wmv1_clone(
		ttLibC_Wmv1 *prev_frame,
		ttLibC_Wmv1 *src_frame);

/**
 * close frame
 * @param frame
 */
void ttLibC_Wmv1_close(ttLibC_Wmv1 **frame);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_FRAME_VIDEO_WMV1_H_ */
