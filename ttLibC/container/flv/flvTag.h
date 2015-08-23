/**
 * @file   flvTag.h
 * @brief  
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/12
 */

#ifndef TTLIBC_CONTAINER_FLV_FLVTAG_H_
#define TTLIBC_CONTAINER_FLV_FLVTAG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../flv.h"

typedef struct {
	// 共通項目を取り出しておく。
	ttLibC_Flv inherit_super;
	uint32_t track_id; // 1固定
	// size -> ttLibC_Containerで保持する。
	// timestamp -> 同上
	// track_id -> ここに含める (本来の意味でのtrackIdはttLibC_Flv_Typeで代用する予定)
	// でいいかな。
	ttLibC_Frame *frame; // 本来ならvideoFrameとaudioFrameのみで十分だが、ここで保持させることで解放時にうまく解放できるようにする。
} ttLibC_Container_FlvTag;

typedef ttLibC_Container_FlvTag ttLibC_FlvTag;

ttLibC_FlvTag *ttLibC_FlvTag_make(
		ttLibC_FlvTag *prev_tag,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		ttLibC_Flv_Type type,
		uint32_t track_id);

void ttLibC_FlvTag_close(ttLibC_FlvTag **tag);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_FLV_FLVTAG_H_ */
