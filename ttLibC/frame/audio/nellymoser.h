/**
 * @file   nellymoser.h
 * @brief  nellymoser frame information
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/06
 */

#ifndef TTLIBC_FRAME_AUDIO_NELLYMOSER_H_
#define TTLIBC_FRAME_AUDIO_NELLYMOSER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "audio.h"

/**
 * nelymoser frame definition
 */
typedef struct ttLibC_Frame_Audio_Nellymoser {
	/** inherit data from ttLibC_Audio */
	ttLibC_Audio inherit_super;
} ttLibC_Frame_Audio_Nellymoser;

typedef ttLibC_Frame_Audio_Nellymoser ttLibC_Nellymoser;

/**
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
ttLibC_Nellymoser *ttLibC_Nellymoser_make(
		ttLibC_Nellymoser *prev_frame,
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
void ttLibC_Nellymoser_close(ttLibC_Nellymoser **frame);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_FRAME_AUDIO_NELLYMOSER_H_ */
