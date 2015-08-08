/**
 * @file   pcmMulaw.h
 * @brief  pcm mu-law frame information. G711.mu-law
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/06
 */

#ifndef TTLIBC_FRAME_AUDIO_PCMMULAW_H_
#define TTLIBC_FRAME_AUDIO_PCMMULAW_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "audio.h"

/**
 * pcm mulaw frame.
 */
typedef struct {
	/** inherit data from ttLibC_Audio */
	ttLibC_Audio inherit_super;
} ttLibC_Frame_Audio_PcmMulaw;

typedef ttLibC_Frame_Audio_PcmMulaw ttLibC_PcmMulaw;

/**
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
ttLibC_PcmMulaw *ttLibC_PcmMulaw_make(
		ttLibC_PcmMulaw *prev_frame,
		uint32_t sample_rate,
		uint32_t sample_num,
		uint32_t channel_num,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase);

/**
 * close frame
 * @param frame
 */
void ttLibC_PcmMulaw_close(ttLibC_PcmMulaw **frame);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_FRAME_AUDIO_PCMMULAW_H_ */
