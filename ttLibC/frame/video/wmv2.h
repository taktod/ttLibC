/**
 * @file   wmv2.h
 * @brief  wmv2 image frame information.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/06
 */

#ifndef TTLIBC_FRAME_VIDEO_WMV2_H_
#define TTLIBC_FRAME_VIDEO_WMV2_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "video.h"

/**
 * wmv2 frame definition
 */
typedef struct {
	/** inherit data from ttLibC_Video */
	ttLibC_Video inherit_super;
} ttLibC_Frame_Video_Wmv2;

typedef ttLibC_Frame_Video_Wmv2 ttLibC_Wmv2;

/**
 * make wmv2 frame
 * @param prev_frame    reuse frame
 * @param video_type    video type of wmv2
 * @param width         width
 * @param height        height
 * @param data          wmv2 data
 * @param data_size     wmv2 data size
 * @param non_copy_mode true:hold the data pointer. false:data will copy.
 * @param pts           pts for wmv2 data.
 * @param timebase      timebase number for pts.
 */
ttLibC_Wmv2 *ttLibC_Wmv2_make(
		ttLibC_Wmv2 *prev_frame,
		ttLibC_Video_Type video_type,
		uint32_t width,
		uint32_t height,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase);

/**
 * close frame
 * @param frame
 */
void ttLibC_Wmv2_close(ttLibC_Wmv2 **frame);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_FRAME_VIDEO_WMV2_H_ */
