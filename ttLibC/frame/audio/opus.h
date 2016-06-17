/**
 * @file   opus.h
 * @brief  opus frame information
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/01
 */

#ifndef TTLIBC_FRAME_AUDIO_OPUS_H_
#define TTLIBC_FRAME_AUDIO_OPUS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "audio.h"

/**
 * opus type
 */
typedef enum ttLibC_Opus_Type {
	OpusType_header,
	OpusType_comment,
	OpusType_frame
} ttLibC_Opus_Type;

/**
 * opus frame definition
 */
typedef struct ttLibC_Frame_Audio_Opus {
	/** inherit data from ttLibC_Audio */
	ttLibC_Audio inherit_super;
	/** frame type */
	ttLibC_Opus_Type type;
} ttLibC_Frame_Audio_Opus;

typedef ttLibC_Frame_Audio_Opus ttLibC_Opus;

/**
 * make opus frame
 * @param prev_frame    reuse frame.
 * @param type          type of opus
 * @param sample_rate   sample rate of data.
 * @param sample_num    sample number of data.
 * @param channel_num   channel number of data. 1:monoral 2:stereo
 * @param data          opus data
 * @param data_size     opus data size
 * @param non_copy_mode true:hold the data pointer. false:data will copy.
 * @param pts           pts for opus data.
 * @param timebase      timebase number for pts.
 * @return opus object.
 */
ttLibC_Opus *ttLibC_Opus_make(
		ttLibC_Opus *prev_frame,
		ttLibC_Opus_Type type,
		uint32_t sample_rate,
		uint32_t sample_num,
		uint32_t channel_num,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase);

/**
 * make opus frame from byte data.
 * @param prev_frame reuse opus frame.
 * @param data       opus binary data.
 * @param data_size  data size
 * @param pts        pts for opus frame.
 * @param timebase   timebase for opus frame.
 */
ttLibC_Opus *ttLibC_Opus_makeFrame(
		ttLibC_Opus *prev_frame,
		void *data,
		size_t data_size,
		uint64_t pts,
		uint32_t timebase);

/**
 * close frame
 * @param frame
 */
void ttLibC_Opus_close(ttLibC_Opus **frame);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_FRAME_AUDIO_OPUS_H_ */
