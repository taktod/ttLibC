/**
 * @file   mp3.h
 * @brief  mp3 frame information
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/21
 */

#ifndef TTLIBC_FRAME_AUDIO_MP3_H_
#define TTLIBC_FRAME_AUDIO_MP3_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "audio.h"

/**
 * mp3 type
 */
typedef enum ttLibC_Mp3_Type {
	Mp3Type_tag,
	Mp3Type_id3,
	Mp3Type_frame
} ttLibC_Mp3_Type;

/**
 * mp3 frame definition
 */
typedef struct ttLibC_Frame_Audio_Mp3 {
	/** inherit data from ttLibC_Audio */
	ttLibC_Audio inherit_super;
	/** frame type */
	ttLibC_Mp3_Type type;
} ttLibC_Frame_Audio_Mp3;

typedef ttLibC_Frame_Audio_Mp3 ttLibC_Mp3;

/**
 * make mp3 frame
 * @param prev_frame    reuse frame.
 * @param type          type of mp3
 * @param sample_rate   sample rate of data.
 * @param sample_num    sample number of data.
 * @param channel_num   channel number of data. 1:monoral 2:stereo
 * @param data          mp3 data
 * @param data_size     mp3 data size
 * @param non_copy_mode true:hold the data pointer. false:data will copy.
 * @param pts           pts for mp3 data.
 * @param timebase      timebase number for pts.
 * @return mp3 object.
 */
ttLibC_Mp3 *ttLibC_Mp3_make(
		ttLibC_Mp3 *prev_frame,
		ttLibC_Mp3_Type type,
		uint32_t sample_rate,
		uint32_t sample_num,
		uint32_t channel_num,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase);

/**
 * make clone frame
 * always make copy buffer on it.
 * @param prev_frame reuse frame object.
 * @param src_frame  source of clone.
 */
ttLibC_Mp3 *ttLibC_Mp3_clone(
		ttLibC_Mp3 *prev_frame,
		ttLibC_Mp3 *src_frame);

/**
 * check the mp3 frame type from data buffer.
 * @param data      mp3 binary data.
 * @param data_size data size.
 * @return Mp3Type value.
 */
ttLibC_Mp3_Type ttLibC_Mp3_getMp3Type(
		void *data,
		size_t data_size);

/**
 * make mp3 frame from byte data.
<<<<<<< HEAD
 * @param prev_frame reuse mp3 frame.
 * @param data       mp3 binary data
 * @param data_size  data size
 * @param pts        pts for mp3 frame.
 * @param timebase   timebase for mp3 frame.
=======
 * @param prev_frame    reuse mp3 frame.
 * @param data          mp3 binary data
 * @param data_size     data size
 * @param non_copy_mode true:hold the data pointer. false:data will copy.
 * @param pts           pts for mp3 frame.
 * @param timebase      timebase for mp3 frame.
>>>>>>> 9bbbf00f57e1bb3b2a36a3faa606dbefb135e8a0
 * @return mp3 object.
 */
ttLibC_Mp3 *ttLibC_Mp3_getFrame(
		ttLibC_Mp3 *prev_frame,
		void *data,
		size_t data_size,
<<<<<<< HEAD
=======
		bool non_copy_mode,
>>>>>>> 9bbbf00f57e1bb3b2a36a3faa606dbefb135e8a0
		uint64_t pts,
		uint32_t timebase);

/**
 * close frame
 * @param frame
 */
void ttLibC_Mp3_close(ttLibC_Mp3 **frame);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_FRAME_AUDIO_MP3_H_ */
