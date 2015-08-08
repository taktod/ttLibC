/**
 * @file   adpcmImaWav.h
 * @brief  adpcmImaWav frame information.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/06
 */

#ifndef TTLIBC_FRAME_AUDIO_ADPCMIMAWAV_H_
#define TTLIBC_FRAME_AUDIO_ADPCMIMAWAV_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "audio.h"

/**
 * adpcm_ima_wav frame definition
 */
typedef struct {
	/** inherit data from ttLibC_Audio */
	ttLibC_Audio inherit_super;
} ttLibC_Frame_Audio_AdpcmImaWav;

typedef ttLibC_Frame_Audio_AdpcmImaWav ttLibC_AdpcmImaWav;

/**
 * make adpcm_ima_wav frame.
 * @param prev_frame    reuse frame.
 * @param sample_rate   sample rate of data
 * @param sample_num    sample num of data
 * @param channel_num   channel number of data
 * @param data          adpcm_ima_wav data
 * @param data_size     adpcm_ima_wav data size
 * @param non_copy_mode true:hold the data pointer. false:data will copy.
 * @param pts           pts for adpcm_ima_wav data.
 * @param timebase      timebase number for pts.
 * @return adpcm_ima_wav object.
 */
ttLibC_AdpcmImaWav *ttLibC_AdpcmImaWav_make(
		ttLibC_AdpcmImaWav *prev_frame,
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
void ttLibC_AdpcmImaWav_close(ttLibC_AdpcmImaWav **frame);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_FRAME_AUDIO_ADPCMIMAWAV_H_ */
