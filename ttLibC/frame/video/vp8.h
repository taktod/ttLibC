/**
 * @file   vp8.h
 * @brief  vp8 image frame information
 *
 * this code is under 3-Cause BSD license
 *
 * @author taktod
 * @date   2015/07/31
 */

#ifndef TTLIBC_FRAME_VIDEO_VP8_H_
#define TTLIBC_FRAME_VIDEO_VP8_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../../ttLibC_predef.h"
#include "video.h"

/**
 * vp8 frame definition
 */
typedef struct ttLibC_Frame_Video_Vp8 {
	/** inherit data from ttLibC_Video */
	ttLibC_Video inherit_super;
} ttLibC_Frame_Video_Vp8;

typedef ttLibC_Frame_Video_Vp8 ttLibC_Vp8;

/**
 * make vp8 frame
 * @param prev_frame    reuse frame
 * @param video_type    video type of vp8
 * @param width         width
 * @param height        height
 * @param data          vp8 data
 * @param data_size     vp8 data size
 * @param non_copy_mode true:hold the data pointer. false:data will copy.
 * @param pts           pts for vp8 data.
 * @param timebase      timebase number for pts.
 */
ttLibC_Vp8 TT_ATTRIBUTE_API *ttLibC_Vp8_make(
		ttLibC_Vp8 *prev_frame,
		ttLibC_Video_Type video_type,
		uint32_t width,
		uint32_t height,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase);

/**
 * make clone frame.
 * always make copy buffer on it.
 * @param prev_frame reuse frame object.
 * @param src_frame  source of clone.
 */
ttLibC_Vp8 TT_ATTRIBUTE_API *ttLibC_Vp8_clone(
		ttLibC_Vp8 *prev_frame,
		ttLibC_Vp8 *src_frame);

/**
 * check if the vp8 binary is key frame.
 * @param data      vp8 data
 * @param data_size vp8 data size
 * @return true: key frame false:inter frame
 */
bool TT_ATTRIBUTE_API ttLibC_Vp8_isKey(void *data, size_t data_size);

/**
 * analyze the width information from vp8 binary.
 * @param prev_frame ref for prev analyzed vp8 frame.
 * @param data       vp8 data
 * @param data_size  vp8 data size
 * @return 0:error or width size.
 */
uint32_t TT_ATTRIBUTE_API ttLibC_Vp8_getWidth(ttLibC_Vp8 *prev_frame, uint8_t *data, size_t data_size);

/**
 * analyze the height information from vp8 binary.
 * @param prev_frame ref for prev analyzed vp8 frame.
 * @param data       vp8 data
 * @param data_size  vp8 data size
 * @return 0:error or height size.
 */
uint32_t TT_ATTRIBUTE_API ttLibC_Vp8_getHeight(ttLibC_Vp8 *prev_frame, uint8_t *data, size_t data_size);

/**
 * make frame object from vp8 binary data.
 * @param prev_frame    ref for prev analyzed vp8 frame.
 * @param data          vp8 data
 * @param data_size     vp8 data size
 * @param non_copy_mode true:hold pointer. false:copy data.
 * @param pts           pts for vp8 frame.
 * @param timebase      timebase for pts.
 * @return vp8 frame
 */
ttLibC_Vp8 TT_ATTRIBUTE_API *ttLibC_Vp8_getFrame(
		ttLibC_Vp8 *prev_frame,
		uint8_t *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase);

/**
 * close frame
 * @param frame
 */
void TT_ATTRIBUTE_API ttLibC_Vp8_close(ttLibC_Vp8 **frame);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_FRAME_VIDEO_VP8_H_ */
