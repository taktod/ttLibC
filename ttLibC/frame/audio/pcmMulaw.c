/*
 * @file   pcmMulaw.c
 * @brief  pcm mu-law frame information. G711.mu-law
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/06
 */

#include "pcmMulaw.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../_log.h"

typedef ttLibC_Frame_Audio_PcmMulaw ttLibC_PcmMulaw_;

/*
 * make pcm_mulaw frame.
 * @param prev_frame    reuse frame.
 * @param sample_rate   sample rate of data
 * @param sample_num    sample num of data
 * @param channel_num   channel number of data
 * @param data          pcm_mulaw data
 * @param data_size     pcm_mulaw data size
 * @param non_copy_mode true:hold the data pointer. false:data will copy.
 * @param pts           pts for pcm_mulaw data.
 * @param timebase      timebase number for pts.
 * @return pcm_mulaw object.
 */
ttLibC_PcmMulaw TT_ATTRIBUTE_API *ttLibC_PcmMulaw_make(
		ttLibC_PcmMulaw *prev_frame,
		uint32_t sample_rate,
		uint32_t sample_num,
		uint32_t channel_num,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	return (ttLibC_PcmMulaw *)ttLibC_Audio_make(
			(ttLibC_Audio *)prev_frame,
			sizeof(ttLibC_PcmMulaw_),
			frameType_pcm_mulaw,
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
ttLibC_PcmMulaw TT_ATTRIBUTE_API *ttLibC_PcmMulaw_clone(
		ttLibC_PcmMulaw *prev_frame,
		ttLibC_PcmMulaw *src_frame) {
	if(src_frame == NULL) {
		return NULL;
	}
	if(src_frame->inherit_super.inherit_super.type != frameType_pcm_mulaw) {
		ERR_PRINT("try to clone non pcmMulaw frame.");
		return NULL;
	}
	if(prev_frame != NULL && prev_frame->inherit_super.inherit_super.type != frameType_pcm_mulaw) {
		ERR_PRINT("try to use non pcmMulaw frame for reuse.");
		return NULL;
	}
	ttLibC_PcmMulaw *pcmMulaw = ttLibC_PcmMulaw_make(
			prev_frame,
			src_frame->inherit_super.sample_rate,
			src_frame->inherit_super.sample_num,
			src_frame->inherit_super.channel_num,
			src_frame->inherit_super.inherit_super.data,
			src_frame->inherit_super.inherit_super.buffer_size,
			false,
			src_frame->inherit_super.inherit_super.pts,
			src_frame->inherit_super.inherit_super.timebase);
	if(pcmMulaw != NULL) {
		pcmMulaw->inherit_super.inherit_super.id = src_frame->inherit_super.inherit_super.id;
	}
	return pcmMulaw;
}

/*
 * close frame
 * @param frame
 */
void TT_ATTRIBUTE_API ttLibC_PcmMulaw_close(ttLibC_PcmMulaw **frame) {
	ttLibC_Audio_close_((ttLibC_Audio **) frame);
}

