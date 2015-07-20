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
	/** inter frame */
	videoType_inter
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
 * close frame
 * @param frame
 */
void ttLibC_Video_close(ttLibC_Video **frame);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_FRAME_VIDEO_VIDEO_H_ */
