/**
 * @file   vp6.h
 * @brief  vp6 image frame information.
 *
 * this code is under 3-Cause BSD license
 *
 * @author taktod
 * @date   2015/08/06
 */

#ifndef TTLIBC_FRAME_VIDEO_VP6_H_
#define TTLIBC_FRAME_VIDEO_VP6_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "video.h"

/**
 * vp6 frame definition
 */
typedef struct ttLibC_Frame_Video_Vp6 {
	/** inherit data from ttLibC_Video */
	ttLibC_Video inherit_super;
} ttLibC_Frame_Video_Vp6;

typedef ttLibC_Frame_Video_Vp6 ttLibC_Vp6;

/**
 * make vp6 frame
 * @param prev_frame    reuse frame
 * @param video_type    video type of vp6
 * @param width         width
 * @param height        height
 * @param data          vp6 data
 * @param data_size     vp6 data size
 * @param non_copy_mode true:hold the data pointer. false:data will copy.
 * @param pts           pts for vp6 data.
 * @param timebase      timebase number for pts.
 */
ttLibC_Vp6 *ttLibC_Vp6_make(
		ttLibC_Vp6 *prev_frame,
		ttLibC_Video_Type video_type,
		uint32_t width,
		uint32_t height,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase);

/**
 * check if the vp6 binary is key frame.
 * @param data      vp6 data
 * @param data_size vp6 data size
 * @return true: key frame false:inter frame
 */
bool ttLibC_Vp6_isKey(void *data, size_t data_size);

/**
 * analyze the width information from vp6 binary.
 * @param prev_frame ref for prev analyzed vp6 frame.
 * @param data       vp6 data
 * @param data_size  vp6 data size
 * @return 0:error or width size.
 */
uint32_t ttLibC_Vp6_getWidth(ttLibC_Vp6 *prev_frame, uint8_t *data, size_t data_size);

/**
 * analyze the height information from vp6 binary.
 * @param prev_frame ref for prev analyzed vp6 frame.
 * @param data       vp6 data
 * @param data_size  vp6 data size
 * @return 0:error or height size.
 */
uint32_t ttLibC_Vp6_getHeight(ttLibC_Vp6 *prev_frame, uint8_t *data, size_t data_size);

/**
 * make frame object from vp6 binary data.
 * @param prev_frame    ref for prev analyzed vp6 frame.
 * @param data          vp6 data
 * @param data_size     vp6 data size
 * @param non_copy_mode true:hold pointer. false:copy data.
 * @param pts           pts for vp6 frame.
 * @param timebase      timebase for pts.
 * @return vp6 frame
 */
ttLibC_Vp6 *ttLibC_Vp6_getFrame(
		ttLibC_Vp6 *prev_frame,
		uint8_t *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase);

/**
 * close frame
 * @param frame
 */
void ttLibC_Vp6_close(ttLibC_Vp6 **frame);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_FRAME_VIDEO_VP6_H_ */
