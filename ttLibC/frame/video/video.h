/**
 * @file   video.h
 * @brief  video frame information.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/18
 */

#ifndef TTLIBC_FRAME_VIDEO_VIDEO_H_
#define TTLIBC_FRAME_VIDEO_VIDEO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../frame.h"

/**
 * video frame type.
 */
typedef enum {
	/** key frame */
	videoType_key,
	/** inner frame */
	videoType_inner,
	/** information frame */
	videoType_info
} ttLibC_Video_Type;

/**
 * additional definition of video frame.
 */
typedef struct {
	/** inherit data from ttLibC_Frame */
	ttLibC_Frame inherit_super;
	/** width */
	uint32_t width;
	/** height */
	uint32_t height;
	/** videoType information */
	ttLibC_Video_Type type;
} ttLibC_Frame_Video;

typedef ttLibC_Frame_Video ttLibC_Video;

/**
 * make video frame
 * @param prev_frame    reuse frame.
 * @param frame_size    allocate frame size.
 * @param frame_type    type of frame.
 * @param type          type of videoframe
 * @param width         width
 * @param height        height
 * @param data          data
 * @param data_size     data size
 * @param non_copy_mode true: hold the data pointer. false:copy the data.
 * @param pts           pts for data.
 * @param timebase      timebase number for pts.
 * @return video frame object.
 */
ttLibC_Video *ttLibC_Video_make(
		ttLibC_Video *prev_frame,
		size_t frame_size,
		ttLibC_Frame_Type frame_type,
		ttLibC_Video_Type type,
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
ttLibC_Video *ttLibC_Video_clone(
		ttLibC_Video *prev_frame,
		ttLibC_Video *src_frame);

/**
 * close frame(use internal)
 * @param frame
 */
void ttLibC_Video_close_(ttLibC_Video **frame);

/**
 * close frame
 * @param frame
 */
void ttLibC_Video_close(ttLibC_Video **frame);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_FRAME_VIDEO_VIDEO_H_ */
