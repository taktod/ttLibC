/**
 * @file   flv1.h
 * @brief  flv1 image frame information
 *
 * this code is under 3-Cause BSD license
 *
 * @author taktod
 * @date   2015/08/06
 */

#ifndef TTLIBC_FRAME_VIDEO_FLV1_H_
#define TTLIBC_FRAME_VIDEO_FLV1_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "video.h"

typedef enum {
	Flv1Type_intra,
	Flv1Type_inner,
	Flv1Type_disposableInner,
} ttLibC_Flv1_Type;

/**
 * flv1 frame definition
 */
typedef struct {
	/** inherit data from ttLibC_Video */
	ttLibC_Video inherit_super;
	ttLibC_Flv1_Type type;
} ttLibC_Frame_Video_Flv1;

typedef ttLibC_Frame_Video_Flv1 ttLibC_Flv1;

/**
 * make flv1 frame
 * @param prev_frame    reuse frame
 * @param type          flv1 frame type
 * @param width         width
 * @param height        height
 * @param data          flv1 data
 * @param data_size     flv1 data size
 * @param non_copy_mode true:hold the data pointer. false:data will copy.
 * @param pts           pts for flv1 data.
 * @param timebase      timebase number for pts.
 */
ttLibC_Flv1 *ttLibC_Flv1_make(
		ttLibC_Flv1 *prev_frame,
		ttLibC_Flv1_Type type,
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
ttLibC_Flv1 *ttLibC_Flv1_clone(
		ttLibC_Flv1 *prev_frame,
		ttLibC_Flv1 *src_frame);

/**
 * check if the flv1 binary is key frame.
 * @param data      flv1 data
 * @param data_size flv1 data size
 * @return true: key frame false:inter frame
 */
bool ttLibC_Flv1_isKey(void *data, size_t data_size);

/**
 * analyze the width.
 * @param data      flv1 data
 * @param data_size flv1 data size
 * @return width  0 for error.
 */
uint32_t ttLibC_Flv1_getWidth(void *data, size_t data_size);

/**
 * analyze the height.
 * @param data      flv1 data
 * @param data_size flv1 data size
 * @return height  0 for error.
 */
uint32_t ttLibC_Flv1_getHeight(void *data, size_t data_size);

/**
 * analyze the frame information from flv1 binary
 * @param prev_frame    reuse frame
 * @param data          flv1 data
 * @param data_size     flv1 data size
 * @param non_copy_mode true:hold pointer. false:copy data.
 * @param pts           pts for flv1 frame
 * @param timebase      timebase for pts
 * @return flv1 frame
 */
ttLibC_Flv1 *ttLibC_Flv1_getFrame(
		ttLibC_Flv1 *prev_frame,
		uint8_t *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase);

/**
 * close frame
 * @param frame
 */
void ttLibC_Flv1_close(ttLibC_Flv1 **frame);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_FRAME_VIDEO_FLV1_H_ */
