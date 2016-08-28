/**
 * @file   vorbis.h
 * @brief  vorbis frame information
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/06
 */

#ifndef TTLIBC_FRAME_AUDIO_VORBIS_H_
#define TTLIBC_FRAME_AUDIO_VORBIS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "audio.h"

/**
 * vorbis type
 */
typedef enum ttLibC_Vorbis_Type {
	VorbisType_identification,
	VorbisType_comment,
	VorbisType_setup,
	VorbisType_frame,
} ttLibC_Vorbis_Type;

/**
 * vorbis frame definition
 */
typedef struct ttLibC_Frame_Audio_Vorbis {
	/** inherit data from ttLibC_Audio */
	ttLibC_Audio inherit_super;
	/** frame type */
	ttLibC_Vorbis_Type type;
	/** block data from identification frame. */
	uint32_t block0;
	uint32_t block1;
} ttLibC_Frame_Audio_Vorbis;

typedef ttLibC_Frame_Audio_Vorbis ttLibC_Vorbis;

/**
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
		uint32_t timebase);

/**
 * make clone frame
 * always make copy buffer on it.
 * @param prev_frame reuse frame object.
 * @param src_frame  source of clone.
 */
ttLibC_Vorbis *ttLibC_Vorbis_clone(
		ttLibC_Vorbis *prev_frame,
		ttLibC_Vorbis *src_frame);

/**
 * make vorbis frame from byte data.
 */
ttLibC_Vorbis *ttLibC_Vorbis_getFrame(
		ttLibC_Vorbis *prev_frame,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase);

/**
 * close frame
 * @param frame
 */
void ttLibC_Vorbis_close(ttLibC_Vorbis **frame);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_FRAME_AUDIO_VORBIS_H_ */
