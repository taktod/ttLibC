/*
 * @file   vorbis.c
 * @brief  vorbis frame information
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/06
 */

#include "vorbis.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../log.h"

typedef ttLibC_Frame_Audio_Vorbis ttLibC_Vorbis_;

/*
 * make vorbis frame.
 * @param prev_frame    reuse frame.
 * @param type          vorbis frame type.
 * @param sample_rate   sample rate of data
 * @param sample_num    sample num of data
 * @param channel_num   channel number of data
 * @param data          vorbis data
 * @param data_size     vorbis data size
 * @param non_copy_mode true:hold the data pointer. false:data will copy.
 * @param pts           pts for vorbis data.
 * @param timebase      timebase number for pts.
 * @return vorbis object.
 */
ttLibC_Vorbis *ttLibC_Vorbis_make(
		ttLibC_Vorbis *prev_frame,
		ttLibC_Vorbis_Type type,
		uint32_t sample_rate,
		uint32_t sample_num,
		uint32_t channel_num,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	ttLibC_Vorbis_ *vorbis = (ttLibC_Vorbis_ *)ttLibC_Audio_make(
			(ttLibC_Audio *)prev_frame,
			sizeof(ttLibC_Vorbis_),
			frameType_vorbis,
			sample_rate,
			sample_num,
			channel_num,
			data,
			data_size,
			non_copy_mode,
			pts,
			timebase);
	if(vorbis != NULL) {
		vorbis->type = type;
	}
	return (ttLibC_Vorbis *)vorbis;
}

/*
 * close frame
 * @param frame
 */
void ttLibC_Vorbis_close(ttLibC_Vorbis **frame) {
	ttLibC_Audio_close_((ttLibC_Audio **)frame);
}


