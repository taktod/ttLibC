/*
 * @file   wmv2.c
 * @brief  wmv2 image frame information.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/06
 */

#include "wmv2.h"
#include "../../_log.h"

typedef ttLibC_Frame_Video_Wmv2 ttLibC_Wmv2_;

/*
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
		uint32_t timebase) {
	return (ttLibC_Wmv2 *)ttLibC_Video_make(
			(ttLibC_Video *)prev_frame,
			sizeof(ttLibC_Wmv2_),
			frameType_wmv2,
			video_type,
			width,
			height,
			data,
			data_size,
			non_copy_mode,
			pts,
			timebase);
}

/**
 * make clone frame
 * always make copy buffer on it.
 * @param prev_frame reuse frame object.
 * @param src_frame  source of clone.
 */
ttLibC_Wmv2 *ttLibC_Wmv2_clone(
		ttLibC_Wmv2 *prev_frame,
		ttLibC_Wmv2 *src_frame) {
	if(src_frame == NULL) {
		return NULL;
	}
	if(src_frame->inherit_super.inherit_super.type != frameType_wmv2) {
		ERR_PRINT("try to clone non wmv2 frame.");
		return NULL;
	}
	if(prev_frame != NULL && prev_frame->inherit_super.inherit_super.type != frameType_wmv2) {
		ERR_PRINT("try to use non wmv2 frame for reuse.");
		return NULL;
	}
	ttLibC_Wmv2 *wmv2 = ttLibC_Wmv2_make(
			prev_frame,
			src_frame->inherit_super.type,
			src_frame->inherit_super.width,
			src_frame->inherit_super.height,
			src_frame->inherit_super.inherit_super.data,
			src_frame->inherit_super.inherit_super.buffer_size,
			false,
			src_frame->inherit_super.inherit_super.pts,
			src_frame->inherit_super.inherit_super.timebase);
	if(wmv2 != NULL) {
		wmv2->inherit_super.inherit_super.id = src_frame->inherit_super.inherit_super.id;
	}
	return wmv2;
}

/*
 * close frame
 * @param frame
 */
void ttLibC_Wmv2_close(ttLibC_Wmv2 **frame) {
	ttLibC_Video_close_((ttLibC_Video **)frame);
}

