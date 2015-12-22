/**
 * @file   pcmf32.h
 * @brief  pcmf32 frame information
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/08
 */

#ifndef TTLIBC_FRAME_AUDIO_PCMF32_H_
#define TTLIBC_FRAME_AUDIO_PCMF32_H_

#include "audio.h"

/**
 * pcmf32 type
 */
typedef enum ttLibC_PcmF32_Type {
	/** 32bit float interleave pcm. lrlrlr.... */
	PcmF32Type_interleave,
	/** 32bit float planar pcm. lll...rrr... */
	PcmF32Type_planar,
} ttLibC_PcmF32_Type;

/**
 * pcmf32 frame definition
 */
typedef struct ttLibC_Frame_Audio_PcmF32 {
	/** inherit data from ttLibC_Audio */
	ttLibC_Audio inherit_super;
	/** pcmf32 type */
	ttLibC_PcmF32_Type type;
	/** ref pointer for left channel */
	uint8_t *l_data;
	/** stride size(data_size) of left channel. */
	uint32_t l_stride;
	/** step to next left sample. */
	uint32_t l_step;
	/** ref pointer for right channel */
	uint8_t *r_data;
	/** stride size(data_size) of right channel. */
	uint32_t r_stride;
	/** step to next right sample. */
	uint32_t r_step;
} ttLibC_Frame_Audio_PcmF32;

typedef ttLibC_Frame_Audio_PcmF32 ttLibC_PcmF32;

/**
 * make pcmf32 frame
 * @param prev_frame    reuse frame. if NULL, make new frame object.
 * @param type          type of pcmf32
 * @param sample_rate   sample rate of data.
 * @param sample_num    sample number of data.
 * @param channel_num   channel number of data. 1:monoral 2:stereo
 * @param data          pcm data
 * @param data_size     pcm data size
 * @param l_data        pointer for l_data
 * @param l_stride      stride(data_size) for l_data
 * @param r_data        pointer for r_data
 * @param r_stride      stride(data_size) for r_data
 * @param non_copy_mode true:hold the data pointer. false:data will copy.
 * @param pts           pts for pcm data.
 * @param timebase      timebase number for pts.
 * @return pcmf32 object.
 */
ttLibC_PcmF32 *ttLibC_PcmF32_make(
		ttLibC_PcmF32 *prev_frame,
		ttLibC_PcmF32_Type type,
		uint32_t sample_rate,
		uint32_t sample_num,
		uint32_t channel_num,
		void *data,
		size_t data_size,
		void *l_data,
		uint32_t l_stride,
		void *r_data,
		uint32_t r_stride,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase);

/**
 * close frame
 * @param frame
 */
void ttLibC_PcmF32_close(ttLibC_PcmF32 **frame);

#endif /* TTLIBC_FRAME_AUDIO_PCMF32_H_ */
