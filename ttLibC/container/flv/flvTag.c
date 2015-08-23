/*
 * @file   flvTag.c
 * @brief  
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/12
 */

#include "type/headerTag.h"
#include "type/audioTag.h"
#include "type/metaTag.h"
#include "type/videoTag.h"

#include "flvTag.h"
#include <stdlib.h>
#include "../../log.h"

ttLibC_FlvTag *ttLibC_FlvTag_make(
		ttLibC_FlvTag *prev_tag,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		ttLibC_Flv_Type type,
		uint32_t track_id) {
	ttLibC_Frame *prev_frame = NULL;
	if(prev_tag != NULL) {
		prev_frame = prev_tag->frame;
	}
	ttLibC_FlvTag *tag = (ttLibC_FlvTag *)ttLibC_Container_make(
			(ttLibC_Container *)prev_tag,
			sizeof(union {
				ttLibC_FlvHeaderTag headerTag;
				ttLibC_FlvAudioTag  audioTag;
				ttLibC_FlvMetaTag   metaTag;
				ttLibC_FlvVideoTag  videoTag;
			}),
			containerType_flv,
			data,
			data_size,
			non_copy_mode,
			pts,
			timebase);
	if(tag != NULL) {
		tag->frame = prev_frame;
		tag->track_id = track_id;
		tag->inherit_super.type = type;
	}
	return tag;
}

bool ttLibC_Flv_getFrame(ttLibC_Flv *flv, ttLibC_getFrameFunc callback, void *ptr) {
	switch(flv->type) {
	case FlvType_audio:
		return ttLibC_FlvAudioTag_getFrame((ttLibC_FlvAudioTag *)flv, callback, ptr);
	case FlvType_header:
	case FlvType_meta:
		// 取得するデータはない
		return true;
	case FlvType_video:
		return ttLibC_FlvVideoTag_getFrame((ttLibC_FlvVideoTag *)flv, callback, ptr);
	}
}

void ttLibC_Flv_close(ttLibC_Flv **flv) {
	ttLibC_FlvTag_close((ttLibC_FlvTag **)flv);
}

void ttLibC_FlvTag_close(ttLibC_FlvTag **tag) {
	ttLibC_FlvTag *target = *tag;
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.inherit_super.type != containerType_flv) {
		ERR_PRINT("container type is not flv.");
		return;
	}
	ttLibC_Frame_close(&target->frame);
	if(!target->inherit_super.inherit_super.is_non_copy) {
		free(target->inherit_super.inherit_super.data);
	}
	free(target);
	*tag = NULL;
}
