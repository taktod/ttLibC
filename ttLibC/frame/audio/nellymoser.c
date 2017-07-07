/*
 * @file   nellymoser.c
 * @brief  nellymoser frame information
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/06
 *
 * NOTE
 * data should be multi of 64 bytes?
 */

#include "nellymoser.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../ttLibC_predef.h"
#include "../../_log.h"

typedef ttLibC_Frame_Audio_Nellymoser ttLibC_Nellymoser_;

/*
 * make nellymoser frame.
 * @param prev_frame    reuse frame.
 * @param sample_rate   sample rate of data
 * @param sample_num    sample num of data
 * @param channel_num   channel number of data
 * @param data          nellymoser data
 * @param data_size     nellymoser data size
 * @param non_copy_mode true:hold the data pointer. false:data will copy.
 * @param pts           pts for nellymoser data.
 * @param timebase      timebase number for pts.
 * @return nellymoser object.
 */
ttLibC_Nellymoser TT_VISIBILITY_DEFAULT *ttLibC_Nellymoser_make(
		ttLibC_Nellymoser *prev_frame,
		uint32_t sample_rate,
		uint32_t sample_num,
		uint32_t channel_num,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	return (ttLibC_Nellymoser *)ttLibC_Audio_make(
			(ttLibC_Audio *)prev_frame,
			sizeof(ttLibC_Nellymoser_),
			frameType_nellymoser,
			sample_rate,
			sample_num,
			channel_num, // monoral only?
			data,
			data_size,
			non_copy_mode,
			pts,
			timebase);
}

/*
 * make clone frame
 * always make copy buffer on it.
 * @param prev_frame reuse frame object.
 * @param src_frame  source of clone.
 */
ttLibC_Nellymoser TT_VISIBILITY_DEFAULT *ttLibC_Nellymoser_clone(
		ttLibC_Nellymoser *prev_frame,
		ttLibC_Nellymoser *src_frame) {
	if(src_frame == NULL) {
		return NULL;
	}
	if(src_frame->inherit_super.inherit_super.type != frameType_nellymoser) {
		ERR_PRINT("try to clone non nellymoser frame.");
		return NULL;
	}
	if(prev_frame != NULL && prev_frame->inherit_super.inherit_super.type != frameType_nellymoser) {
		ERR_PRINT("try to use non nellymoser frame for reuse.");
		return NULL;
	}
	ttLibC_Nellymoser *nellymoser = ttLibC_Nellymoser_make(
			prev_frame,
			src_frame->inherit_super.sample_rate,
			src_frame->inherit_super.sample_num,
			src_frame->inherit_super.channel_num,
			src_frame->inherit_super.inherit_super.data,
			src_frame->inherit_super.inherit_super.buffer_size,
			false,
			src_frame->inherit_super.inherit_super.pts,
			src_frame->inherit_super.inherit_super.timebase);
	if(nellymoser != NULL) {
		nellymoser->inherit_super.inherit_super.id = src_frame->inherit_super.inherit_super.id;
	}
	return nellymoser;
}

/*
 * close frame
 * @param frame
 */
void TT_VISIBILITY_DEFAULT ttLibC_Nellymoser_close(ttLibC_Nellymoser **frame) {
	ttLibC_Audio_close_((ttLibC_Audio **)frame);
}
