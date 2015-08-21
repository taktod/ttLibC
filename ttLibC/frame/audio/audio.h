/**
 * @file   audio.h
 * @brief  audio frame information.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/19
 */

#ifndef TTLIBC_FRAME_AUDIO_AUDIO_H_
#define TTLIBC_FRAME_AUDIO_AUDIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../frame.h"

/**
 * additional definition of audio frame.
 */
typedef struct {
	/** inherit data from ttLibC_Frame */
	ttLibC_Frame inherit_super;
	/** sample rate for audio frame. */
	uint32_t sample_rate;
	/** sample num for audio frame. */
	uint32_t sample_num;
	/** channel number for audio frame. 1:monoral 2:stereo */
	uint32_t channel_num;
} ttLibC_Frame_Audio;

typedef ttLibC_Frame_Audio ttLibC_Audio;

/**
 * make audio frame.
 * @param prev_frame    reuse frame.
 * @param frame_size    allocate frame size
 * @param frame_type    type of frame
 * @param sample_rate   sample rate
 * @param sample_num    sample number
 * @param channel_num   channel number
 * @param data          data
 * @param data_size     data size
 * @param non_copy_mode true:hold the data pointer, false:data will copy.
 * @param pts           pts for data.
 * @param timebase      timebase for pts.
 */
ttLibC_Audio *ttLibC_Audio_make(
		ttLibC_Audio *prev_frame,
		size_t frame_size,
		ttLibC_Frame_Type frame_type,
		uint32_t sample_rate,
		uint32_t sample_num,
		uint32_t channel_num,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase);

/**
 * make clone frame.
 * always make copy buffer on it.
 * @param prev_frame reuse frame object.
 * @param src_frame  source of clone.
 */
ttLibC_Audio *ttLibC_Audio_clone(
		ttLibC_Audio *prev_frame,
		ttLibC_Audio *src_frame);

/**
 * close frame(use internal)
 * @param frame
 */
void ttLibC_Audio_close_(ttLibC_Audio **frame);

/**
 * close frame
 * @param frame
 */
void ttLibC_Audio_close(ttLibC_Audio **frame);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_FRAME_AUDIO_AUDIO_H_ */
