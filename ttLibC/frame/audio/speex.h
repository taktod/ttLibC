/**
 * @file   speex.h
 * @brief  speex frame information
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/28
 */

#ifndef TTLIBC_FRAME_AUDIO_SPEEX_H_
#define TTLIBC_FRAME_AUDIO_SPEEX_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "audio.h"

/**
 * speex type
 */
typedef enum ttLibC_Speex_Type {
	SpeexType_header,
	SpeexType_comment,
	SpeexType_frame
} ttLibC_Speex_Type;

/**
 * speex frame definition
 */
typedef struct ttLibC_Frame_Audio_Speex {
	/** inherit data from ttLibC_Audio */
	ttLibC_Audio inherit_super;
	/** frame type */
	ttLibC_Speex_Type type;
} ttLibC_Frame_Audio_Speex;

typedef ttLibC_Frame_Audio_Speex ttLibC_Speex;

/**
 * make speex frame
 * @param prev_frame    reuse frame.
 * @param type          type of speex
 * @param sample_rate   sample rate of data.
 * @param sample_num    sample number of data.
 * @param channel_num   channel number of data. 1:monoral 2:stereo
 * @param data          speex data
 * @param data_size     speex data size
 * @param non_copy_mode true:hold the data pointer. false:data will copy.
 * @param pts           pts for speex data.
 * @param timebase      timebase number for pts.
 * @return speex object.
 */
ttLibC_Speex *ttLibC_Speex_make(
		ttLibC_Speex *prev_frame,
		ttLibC_Speex_Type type,
		uint32_t sample_rate,
		uint32_t sample_num,
		uint32_t channel_num,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase);

/**
 * make speex frame from byte data.
 * @param prev_frame reuse speex frame.
 * @param data       speex binary data.
 * @param data_size  data size
 * @param pts        pts for speex frame.
 * @param timebase   timebase for speex frame.
 * @return speex object.
 */
ttLibC_Speex *ttLibC_Speex_makeFrame(
		ttLibC_Speex *prev_frame,
		void *data,
		size_t data_size,
		uint64_t pts,
		uint32_t timebase);

/**
 * close frame
 * @param frame
 */
void ttLibC_Speex_close(ttLibC_Speex **frame);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_FRAME_AUDIO_SPEEX_H_ */
