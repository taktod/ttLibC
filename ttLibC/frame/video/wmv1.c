/*
 * @file   wmv1.c
 * @brief  wmv1 image frame information.
 *
 * this code is under 3-Cause BSD license
 *
 * @author taktod
 * @date   2015/08/06
 */

#include "wmv1.h"
#include "../../log.h"

typedef ttLibC_Frame_Video_Wmv1 ttLibC_Wmv1_;

/*
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
		uint32_t timebase) {
	return (ttLibC_Wmv1 *)ttLibC_Video_make(
			(ttLibC_Video *)prev_frame,
			sizeof(ttLibC_Wmv1_),
			frameType_vp6,
			video_type,
			width,
			height,
			data,
			data_size,
			non_copy_mode,
			pts,
			timebase);
}

/*
 * close frame
 * @param frame
 */
void ttLibC_Wmv1_close(ttLibC_Wmv1 **frame) {
	ttLibC_Video_close_((ttLibC_Video **)frame);
}

