/**
 * @file   mp3Frame.h
 * @brief  mp3frame container.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/23
 */

#ifndef TTLIBC_CONTAINER_MP3_MP3FRAME_H_
#define TTLIBC_CONTAINER_MP3_MP3FRAME_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../containerCommon.h"
#include "../mp3.h"

/**
 * detail definition of container Mp3Frame.
 */
typedef struct {
	ttLibC_Container_Mp3 inherit_super;
	ttLibC_Frame *frame; // frame object to hold data.
} ttLibC_Container_Mp3Frame;

typedef ttLibC_Container_Mp3Frame ttLibC_Mp3Frame;

/**
 * make mp3Frame container.
 * @param prev_frame    reuse mp3Frame container.
 * @param data          binary data for mp3Frame container.
 * @param data_size     data size
 * @param non_copy_mode true:hold data is ref, false: copy data.
 * @param pts           timestamp
 * @param timebase      timebase
 * @return mp3frame container object.
 */
ttLibC_Mp3Frame *ttLibC_Mp3Frame_make(
		ttLibC_Mp3Frame *prev_frame,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase);

/**
 * close mp3frame container.
 * @param frame mp3Frame container object.
 */
void ttLibC_Mp3Frame_close(ttLibC_Mp3Frame **frame);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MP3_MP3FRAME_H_ */
