/**
 * @file   aac.h
 * @brief  
 * @author taktod
 * @date   2015/07/23
 */

#ifndef TTLIBC_FRAME_AUDIO_AAC_H_
#define TTLIBC_FRAME_AUDIO_AAC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "audio.h"

/**
 * aac type
 */
typedef enum {
	/** with global header. */
	AacType_raw,
	/** no global header. */
	AacType_adts,
} ttLibC_Aac_Type;

/**
 * aac frame definition
 */
typedef struct {
	/** inherit data from ttLibC_Audio */
	ttLibC_Audio inherit_super;
	/** frame type */
	ttLibC_Aac_Type type;
} ttLibC_Frame_Audio_Aac;

typedef ttLibC_Frame_Audio_Aac ttLibC_Aac;

/**
 * make aac frame.
 * @param prev_frame    reuse frame.
 * @param type          type of aac
 * @param sample_rate   sample rate of data
 * @param sample_num    sample num of data(1024 fixed?)
 * @param channel_num   channel number of data
 * @param data          aac data
 * @param data_size     aac data size
 * @param non_copy_mode true:hold the data pointer. false:data will copy.
 * @param pts           pts for aac data.
 * @param timebase      timebase number for pts.
 * @return aac object.
 */
ttLibC_Aac *ttLibC_Aac_make(
		ttLibC_Aac *prev_frame,
		ttLibC_Aac_Type type,
		uint32_t sample_rate,
		uint32_t sample_num,
		uint32_t channel_num,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase);

/**
 * analyze aac frame and make data.
 * @param prev_frame reuse frame
 * @param data       aac binary data.
 * @param data_size  data size
 * @param pts        pts for aac frame.
 * @param timebase   timebase for pts.
 * @return aac object
 */
ttLibC_Aac *ttLibC_Aac_makeFrame(
		ttLibC_Aac *prev_frame,
		void *data,
		size_t data_size,
		uint64_t pts,
		uint32_t timebase);

/**
 * close frame
 * @param frame
 */
void ttLibC_Aac_close(ttLibC_Aac **frame);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_FRAME_AUDIO_AAC_H_ */
