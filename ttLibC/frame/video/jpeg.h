/**
 * @file   jpeg.h
 * @brief  jpeg image frame information
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/11/05
 */

#ifndef TTLIBC_FRAME_VIDEO_JPEG_H_
#define TTLIBC_FRAME_VIDEO_JPEG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "video.h"

typedef struct {
	/** inherit data from ttLibC_Video */
	ttLibC_Video inherit_super;
} ttLibC_Frame_Video_Jpeg;

typedef ttLibC_Frame_Video_Jpeg ttLibC_Jpeg;

/**
 * make jpeg frame
 * @param prev_frame    reuse frame
 * @param width         width
 * @param height        height
 * @param data          jpeg data
 * @param data_size     jpeg data size
 * @param non_copy_mode true:hold the data pointer. false:data will copy.
 * @param pts           pts for jpeg data.
 * @param timebase      timebase number for pts
 */
ttLibC_Jpeg *ttLibC_Jpeg_make(
		ttLibC_Jpeg *prev_frame,
		uint32_t width,
		uint32_t height,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase);

/**
 * close frame
 * @param frame
 */
void ttLibC_Jpeg_close(ttLibC_Jpeg **frame);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_FRAME_VIDEO_JPEG_H_ */
