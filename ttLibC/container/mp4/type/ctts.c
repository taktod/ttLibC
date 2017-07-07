/*
 * ctts.c
 *
 *  Created on: 2016/10/30
 *      Author: taktod
 */

#include "ctts.h"
#include "../../../ttLibC_predef.h"
#include "../../../util/ioUtil.h"
#include "../../../_log.h"

ttLibC_Mp4 *ttLibC_Ctts_make(
		uint8_t *data,
		size_t data_size,
		uint32_t timebase) {
	ttLibC_Ctts *ctts = (ttLibC_Ctts *)ttLibC_Mp4Atom_make(
			NULL,
			data,
			data_size,
			false,
			0,
			timebase,
			Mp4Type_Ctts);
	if(ctts == NULL) {
		return NULL;
	}
	uint32_t *buf = (uint32_t *)ctts->inherit_super.inherit_super.inherit_super.data;
	buf += 3;
	ctts->entry_count = be_uint32_t(*buf);
	ctts->data = buf + 1;
	ctts->sample_count = 0;
	ctts->sample_offset = 0;
	ttLibC_Ctts_moveNext((ttLibC_Mp4 *)ctts);
	return (ttLibC_Mp4 *)ctts;
}

uint32_t ttLibC_Ctts_refCurrentOffset(ttLibC_Mp4 *mp4) {
	ttLibC_Ctts *ctts = (ttLibC_Ctts *)mp4;
	if(ctts == NULL) {
		return 0;
	}
	return ctts->sample_offset;
}

void ttLibC_Ctts_moveNext(ttLibC_Mp4 *mp4) {
	ttLibC_Ctts *ctts = (ttLibC_Ctts *)mp4;
	if(ctts == NULL) {
		return;
	}
	if(ctts->sample_count > 1) {
		-- ctts->sample_count;
	}
	else {
		ctts->sample_offset = 0;
		ctts->sample_count = 0;
		if(ctts->entry_count > 0) {
			ctts->sample_count = be_uint32_t(*ctts->data);
			++ ctts->data;
			ctts->sample_offset = be_uint32_t(*ctts->data);
			++ ctts->data;
			-- ctts->entry_count;
		}
	}
}
