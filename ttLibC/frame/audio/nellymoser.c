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
#include "../../log.h"

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
ttLibC_Nellymoser *ttLibC_Nellymoser_make(
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
 * close frame
 * @param frame
 */
void ttLibC_Nellymoser_close(ttLibC_Nellymoser **frame) {
	ttLibC_Audio_close_((ttLibC_Audio **)frame);
}
