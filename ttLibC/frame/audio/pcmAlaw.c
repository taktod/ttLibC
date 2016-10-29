/*
 * @file   pcmAlaw.c
 * @brief  pcm a-law frame information. G711.A-law
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/06
 */

#include "pcmAlaw.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../log.h"

typedef ttLibC_Frame_Audio_PcmAlaw ttLibC_PcmAlaw_;

/*
 * make pcm_alaw frame.
 * @param prev_frame    reuse frame.
 * @param sample_rate   sample rate of data
 * @param sample_num    sample num of data
 * @param channel_num   channel number of data
 * @param data          pcm_alaw data
 * @param data_size     pcm_alaw data size
 * @param non_copy_mode true:hold the data pointer. false:data will copy.
 * @param pts           pts for pcm_alaw data.
 * @param timebase      timebase number for pts.
 * @return pcm_alaw object.
 */
ttLibC_PcmAlaw *ttLibC_PcmAlaw_make(
		ttLibC_PcmAlaw *prev_frame,
		uint32_t sample_rate,
		uint32_t sample_num,
		uint32_t channel_num,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	return (ttLibC_PcmAlaw *)ttLibC_Audio_make(
			(ttLibC_Audio *)prev_frame,
			sizeof(ttLibC_PcmAlaw_),
			frameType_pcm_alaw,
			sample_rate,
			sample_num,
			channel_num,
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
ttLibC_PcmAlaw *ttLibC_PcmAlaw_clone(
		ttLibC_PcmAlaw *prev_frame,
		ttLibC_PcmAlaw *src_frame) {
	if(src_frame == NULL) {
		return NULL;
	}
	if(src_frame->inherit_super.inherit_super.type != frameType_pcm_alaw) {
		ERR_PRINT("try to clone non pcmAlaw frame.");
		return NULL;
	}
	if(prev_frame != NULL && prev_frame->inherit_super.inherit_super.type != frameType_pcm_alaw) {
		ERR_PRINT("try to use non pcmAlaw frame for reuse.");
		return NULL;
	}
	ttLibC_PcmAlaw *pcmAlaw = ttLibC_PcmAlaw_make(
			prev_frame,
			src_frame->inherit_super.sample_rate,
			src_frame->inherit_super.sample_num,
			src_frame->inherit_super.channel_num,
			src_frame->inherit_super.inherit_super.data,
			src_frame->inherit_super.inherit_super.buffer_size,
			false,
			src_frame->inherit_super.inherit_super.pts,
			src_frame->inherit_super.inherit_super.timebase);
	if(pcmAlaw != NULL) {
		pcmAlaw->inherit_super.inherit_super.id = src_frame->inherit_super.inherit_super.id;
	}
	return pcmAlaw;
}

/*
 * close frame
 * @param frame
 */
void ttLibC_PcmAlaw_close(ttLibC_PcmAlaw **frame) {
	ttLibC_Audio_close_((ttLibC_Audio **)frame);
}

