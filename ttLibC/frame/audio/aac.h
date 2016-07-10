/**
 * @file   aac.h
 * @brief  aac frame information
 *
 * this code is under 3-Cause BSD license.
 *
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
typedef enum ttLibC_Aac_Type {
	/** with global header. */
	AacType_raw,
	/** no global header. */
	AacType_adts,
} ttLibC_Aac_Type;

/**
 * aac object(profile) information.
 */
typedef enum ttLibC_Aac_Object {
	AacObject_Main = 1,
	AacObject_Low  = 2,
	AacObject_SSR  = 3,
	AacObject_LTP  = 4,

	AacObject_SBR      = 5,
	AacObject_Scalable = 6,
	AacObject_TwinVQ   = 7,
	AacObject_CELP     = 8,
	AacObject_HVXC     = 9
} ttLibC_Aac_Object;

/**
 * aac frame definition
 */
typedef struct ttLibC_Frame_Audio_Aac {
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
 * @param dsi_info      decoder specific info(global header for low data.)
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
		uint32_t timebase,
		uint64_t dsi_info);

/**
 * make clone frame.
 * always make copy buffer on it.
 * @param prev_frame reuse frame object.
 * @param src_frame  source of clone.
 */
ttLibC_Aac *ttLibC_Aac_clone(
		ttLibC_Aac *prev_frame,
		ttLibC_Aac *src_frame);

/**
 * analyze aac frame and make data.
 * NOTE: this function for adts format only.
 * @param prev_frame    reuse frame
 * @param data          aac binary data.
 * @param data_size     data size
 * @param non_copy_mode true:hold the data pointer. false:data will copy.
 * @param pts           pts for aac frame.
 * @param timebase      timebase for pts.
 * @return aac object
 */
ttLibC_Aac *ttLibC_Aac_getFrame(
		ttLibC_Aac *prev_frame,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase);

/**
 * get adts header from aac information.
 * @param target_aac target aac object.
 * @param data       written target buffer.
 * @param data_size  buffer size
 * @return 0:error others:written size.
 */
size_t ttLibC_Aac_readAdtsHeader(
		ttLibC_Aac *target_aac,
		void *data,
		size_t data_size);

/**
 * calcurate crc32 value for configdata.
 * if this value is changed, the configuration of aac is changed.
 * @param aac target aac object.
 * @return value of crc32. 0 for error.
 */
uint32_t ttLibC_Aac_getConfigCrc32(ttLibC_Aac *aac);

/**
 * get dsi buffer for aac data.
 * @param aac
 * @param data
 * @param data_size
 * @return write size. 0 for error.
 */
size_t ttLibC_Aac_readDsiInfo(
		ttLibC_Aac *aac,
		void *data,
		size_t data_size);

/**
 * make dsi buffer from information.
 * @param object_type
 * @param sample_rate
 * @param channel_num
 * @param data
 * @param data_size
 * @return size of generate dsi information.
 */
size_t ttLibC_Aac_getDsiInfo(
		ttLibC_Aac_Object object_type,
		uint32_t sample_rate,
		uint32_t channel_num,
		void *data,
		size_t data_size);

/**
 * close frame
 * @param frame
 */
void ttLibC_Aac_close(ttLibC_Aac **frame);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_FRAME_AUDIO_AAC_H_ */
