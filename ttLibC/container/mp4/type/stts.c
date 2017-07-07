/**
 * @file   stts.c
 * @brief  stts atom support.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/07/04
 */

#include "stts.h"
#include "../../../ttLibC_predef.h"
#include "../../../util/ioUtil.h"
#include "../../../_log.h"

ttLibC_Mp4 TT_VISIBILITY_HIDDEN *ttLibC_Stts_make(
		uint8_t *data,
		size_t data_size,
		uint32_t timebase) {
	ttLibC_Stts *stts = (ttLibC_Stts *)ttLibC_Mp4Atom_make(
			NULL,
			data,
			data_size,
			false,
			0,
			timebase,
			Mp4Type_Stts);
	if(stts == NULL) {
		return NULL;
	}
	uint32_t *buf = (uint32_t *)stts->inherit_super.inherit_super.inherit_super.data;
	buf += 3;
	stts->entry_count = be_uint32_t(*buf);
	stts->data = buf + 1;
	stts->current_count = 0;
	stts->current_delta = 0;
	stts->current_pts = 0;
	ttLibC_Stts_moveNext((ttLibC_Mp4 *)stts);
	return (ttLibC_Mp4 *)stts;
}

uint32_t TT_VISIBILITY_HIDDEN ttLibC_Stts_refCurrentDelta(ttLibC_Mp4 *mp4) {
	ttLibC_Stts *stts = (ttLibC_Stts *)mp4;
	return stts->current_delta;
}
uint32_t TT_VISIBILITY_HIDDEN ttLibC_Stts_refCurrentPts(ttLibC_Mp4 *mp4) {
	ttLibC_Stts *stts = (ttLibC_Stts *)mp4;
	return stts->current_pts;
}

void TT_VISIBILITY_HIDDEN ttLibC_Stts_moveNext(ttLibC_Mp4 *mp4) {
	ttLibC_Stts *stts = (ttLibC_Stts *)mp4;
	stts->current_pts += ttLibC_Stts_refCurrentDelta(mp4);
	if(stts->current_count > 1) {
		-- stts->current_count;
	}
	else {
		stts->current_delta = 0;
		stts->current_count = 0;
		if(stts->entry_count > 0) {
			stts->current_count = be_uint32_t(*stts->data);
			++ stts->data;
			stts->current_delta = be_uint32_t(*stts->data);
			++ stts->data;
			-- stts->entry_count;
		}
	}
}
