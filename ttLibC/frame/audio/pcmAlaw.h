/**
 * @file   pcmAlaw.h
 * @brief  pcm a-law frame information. G711.A-law
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/06
 */

#ifndef TTLIBC_FRAME_AUDIO_PCMALAW_H_
#define TTLIBC_FRAME_AUDIO_PCMALAW_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "audio.h"

/**
 * pcm alaw frame definition.
 */
typedef struct ttLibC_Frame_Audio_PcmAlaw {
	/** inherit data from ttLibC_Audio */
	ttLibC_Audio inherit_super;
} ttLibC_Frame_Audio_PcmAlaw;

typedef ttLibC_Frame_Audio_PcmAlaw ttLibC_PcmAlaw;

/**
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
		uint32_t timebase);

/**
 * make clone frame
 * always make copy buffer on it.
 * @param prev_frame reuse frame object.
 * @param src_frame  source of clone.
 */
ttLibC_PcmAlaw *ttLibC_PcmAlaw_clone(
		ttLibC_PcmAlaw *prev_frame,
		ttLibC_PcmAlaw *src_frame);

/**
 * close frame
 * @param frame
 */
void ttLibC_PcmAlaw_close(ttLibC_PcmAlaw **frame);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_FRAME_AUDIO_PCMALAW_H_ */
