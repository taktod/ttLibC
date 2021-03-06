/**
 * @file   pcms16.h
 * @brief  pcms16 frame information.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/19
 */

#ifndef TTLIBC_FRAME_AUDIO_PCMS16_H_
#define TTLIBC_FRAME_AUDIO_PCMS16_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "audio.h"

/**
 * pcms16 type
 */
typedef enum ttLibC_PcmS16_Type {
	/** 16bit little endian signed int interleave pcm. lrlrlr....
	 * l:left value. r:right value.
	 */
	PcmS16Type_littleEndian,
	/** 16bit big endian signed int interleave pcm. lrlrlr.... */
	PcmS16Type_bigEndian,
	/** 16bit little endian signed int planar pcm. lll.. rrr.. */
	PcmS16Type_littleEndian_planar,
	/** 16bit big endian signed int planar pcm. lll.. rrr.. */
	PcmS16Type_bigEndian_planar
} ttLibC_PcmS16_Type;

/**
 * pcms16 frame definition
 */
typedef struct ttLibC_Frame_Audio_PcmS16 {
	/** inherit data from ttLibC_Audio */
	ttLibC_Audio inherit_super;
	/** pcms16 type */
	ttLibC_PcmS16_Type type;
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
} ttLibC_Frame_Audio_PcmS16;

typedef ttLibC_Frame_Audio_PcmS16 ttLibC_PcmS16;

/**
 * make pcms16 frame
 * @param prev_frame    reuse frame. if NULL, make new frame object.
 * @param type          type of pcms16
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
 * @return pcms16 object.
 */
ttLibC_PcmS16 *ttLibC_PcmS16_make(
		ttLibC_PcmS16 *prev_frame,
		ttLibC_PcmS16_Type type,
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
 * make clone frame.
 * always make copy buffer on it.
 * @param prev_frame reuse frame object.
 * @param src_frame  source of clone.
 */
ttLibC_PcmS16 *ttLibC_PcmS16_clone(
		ttLibC_PcmS16 *prev_frame,
		ttLibC_PcmS16 *src_frame);

/**
 * close frame
 * @param frame
 */
void ttLibC_PcmS16_close(ttLibC_PcmS16 **frame);

/**
 * reset changed data.
 * @param pcm target pcm frame.
 */
void ttLibC_PcmS16_resetData(ttLibC_PcmS16 *pcm);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_FRAME_AUDIO_PCMS16_H_ */
