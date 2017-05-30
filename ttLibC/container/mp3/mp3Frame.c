/*
 * @file   mp3Frame.c
 * @brief  mp3frame container.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/23
 */

#include "mp3Frame.h"
#include <stdlib.h>
#include "../../_log.h"
#include "../../allocator.h"

ttLibC_Mp3Frame *ttLibC_Mp3Frame_make(
		ttLibC_Mp3Frame *prev_mp3frame,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	ttLibC_Frame *prev_frame = NULL;
	if(prev_frame) {
		prev_frame = prev_mp3frame->frame;
	}
	ttLibC_Mp3Frame *frame = (ttLibC_Mp3Frame *)ttLibC_Container_make(
			(ttLibC_Container *)prev_mp3frame,
			sizeof(ttLibC_Mp3Frame),
			containerType_mp3,
			data,
			data_size,
			non_copy_mode,
			pts,
			timebase);
	if(frame != NULL) {
		frame->frame = prev_frame;
	}
	return frame;
}

bool ttLibC_Container_Mp3_getFrame(
		ttLibC_Container_Mp3 *mp3,
		ttLibC_getFrameFunc callback,
		void *ptr) {
	ttLibC_Mp3Frame *mp3frame = (ttLibC_Mp3Frame *)mp3;
	return callback(ptr, (ttLibC_Frame *)mp3frame->frame);
}

void ttLibC_Container_Mp3_close(ttLibC_Container_Mp3 **mp3) {
	ttLibC_Mp3Frame_close((ttLibC_Mp3Frame **)mp3);
}

void ttLibC_Mp3Frame_close(ttLibC_Mp3Frame **frame) {
	ttLibC_Mp3Frame *target = *frame;
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.inherit_super.type != containerType_mp3) {
		ERR_PRINT("container type is not mp3.");
		return;
	}
	ttLibC_Frame_close(&target->frame);
	if(!target->inherit_super.inherit_super.is_non_copy) {
		ttLibC_free(target->inherit_super.inherit_super.data);
	}
	ttLibC_free(target);
	*frame = NULL;
}
