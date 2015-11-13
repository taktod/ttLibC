/**
 * @file   mp3Frame.h
 * @brief  
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

typedef struct {
	ttLibC_Container_Mp3 inherit_super;
	ttLibC_Frame *frame; // ここでデータを保持させる。
} ttLibC_Container_Mp3Frame;

typedef ttLibC_Container_Mp3Frame ttLibC_Mp3Frame;

ttLibC_Mp3Frame *ttLibC_Mp3Frame_make(
		ttLibC_Mp3Frame *prev_frame,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase);

void ttLibC_Mp3Frame_close(ttLibC_Mp3Frame **frame);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MP3_MP3FRAME_H_ */
