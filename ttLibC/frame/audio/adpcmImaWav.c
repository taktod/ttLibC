/*
 * @file   adpcmImaWav.c
 * @brief  adpcmImaWav frame information.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/06
 *
 * NOTE:
 * for adpcm_ima_wav...
 * 16bit first predictor(left)
 * 8bit first index
 * 8bit reserved zero
 * 16bit first predictor(right)
 * 8bit first index
 * 8bit reserved zero
 *
 * 'getChannel' and 'getSampleNum' functions are possible to make.
 */

#include "adpcmImaWav.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../log.h"

typedef ttLibC_Frame_Audio_AdpcmImaWav ttLibC_AdpcmImaWav_;

/*
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
		uint32_t timebase) {
	return (ttLibC_AdpcmImaWav *)ttLibC_Audio_make(
			(ttLibC_Audio *)prev_frame,
			sizeof(ttLibC_AdpcmImaWav_),
			frameType_adpcm_ima_wav,
			sample_rate,
			sample_num,
			channel_num,
			data,
			data_size,
			non_copy_mode,
			pts,
			timebase);
}

/*
 * close frame
 * @param frame
 */
void ttLibC_AdpcmImaWav_close(ttLibC_AdpcmImaWav **frame){
	ttLibC_Audio_close_((ttLibC_Audio **)frame);
}

