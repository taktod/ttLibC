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
 * close frame
 * @param frame
 */
void ttLibC_Audio_close(ttLibC_Audio **frame);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_FRAME_AUDIO_AUDIO_H_ */
