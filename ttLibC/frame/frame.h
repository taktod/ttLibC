/**
 * @file   frame.h
 * @brief  frame information.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/18
 */

#ifndef TTLIBC_FRAME_FRAME_H_
#define TTLIBC_FRAME_FRAME_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * frame type.
 */
typedef enum {
	/** ttLibC_Bgr video frame */
	frameType_bgr,
	/** ttLibC_Yuv420 video frame */
	frameType_yuv420,
	/** ttLibC_H264 video frame */
	frameType_h264,
	/** ttLibC_PcmS16 audio frame */
	frameType_pcmS16,
	/** ttLibC_Mp3 audio frame */
	frameType_mp3,
	/** ttLibC_Aac audio frame */
	frameType_aac,
	/** ttLibC_Speex audio frame */
	frameType_speex,
	/** ttLibC_Opus audio frame */
	frameType_opus,
} ttLibC_Frame_Type;

/**
 * base definition of frame.
 */
typedef struct {
	/** frame type information */
	ttLibC_Frame_Type type;
	/**
	 * pts information
	 * 1 pts = 1sec / timebase
	 */
	uint64_t pts;
	/**
	 * timebase information
	 * 1/timebase is the unit of pts.
	 * for example, 1000 means 1pts = 1mili sec.
	 */
	uint32_t timebase;
	/** data of frame */
	void *data;
	/** data size of frame */
	size_t data_size;
	/**
	 * actual size of frame
	 * data_size can have extra space.
	 */
	size_t buffer_size;
	/**
	 * flag for instanced data or not.
	 * if false, data will be freed on close method.
	 */
	bool is_non_copy;
} ttLibC_Frame;

/**
 * close frame
 * @param frame
 */
void ttLibC_Frame_close(ttLibC_Frame **frame);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_FRAME_FRAME_H_ */
