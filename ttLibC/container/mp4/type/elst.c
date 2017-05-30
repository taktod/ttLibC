/*
 * elst.c
 *
 *  Created on: 2016/12/08
 *      Author: taktod
 */

#include "elst.h"
#include "../../../util/ioUtil.h"
#include "../../../_log.h"

ttLibC_Mp4 *ttLibC_Elst_make(
		uint8_t *data,
		size_t data_size,
		uint32_t timebase) {
	ttLibC_Elst *elst = (ttLibC_Elst *)ttLibC_Mp4Atom_make(
			NULL,
			data,
			data_size,
			false,
			0,
			timebase,
			Mp4Type_Elst);
	if(elst == NULL) {
		return NULL;
	}
	uint32_t *buf = (uint32_t *)elst->inherit_super.inherit_super.inherit_super.data;
	buf += 2;
	elst->version = be_uint32_t(*buf);
	++ buf;
	elst->entry_count = be_uint32_t(*buf);
	++ buf;
	elst->pts = 0;
	elst->mediatime = 0;
	if(elst->entry_count > 0) {
		-- elst->entry_count;
		if(elst->version == 0x00) {
			elst->pts = be_uint32_t(*buf);
			++ buf;
			elst->mediatime = be_uint32_t(*buf);
			++ buf;
		}
		else {
			ERR_PRINT("not support 64bit elst now.");
			ttLibC_Mp4Atom_close((ttLibC_Mp4Atom **)&elst);
			return NULL;
		}
		++ buf;
	}
	elst->data = buf;
	return (ttLibC_Mp4 *)elst;
}

uint32_t ttLibC_Elst_refCurrentMediatime(ttLibC_Mp4 *mp4) {
	ttLibC_Elst *elst = (ttLibC_Elst *)mp4;
	if(elst == NULL) {
		return 0;
	}
	return elst->mediatime;
}

void ttLibC_Elst_moveNext(ttLibC_Mp4 *mp4, uint64_t pts) {
	ttLibC_Elst *elst = (ttLibC_Elst *)mp4;
	if(elst == NULL) {
		return;
	}
	if(elst->entry_count > 0 && elst->pts <= pts) {
		-- elst->entry_count;
		uint32_t *buf = elst->data;
		if(elst->version != 0x00) {
		}
		else {
			elst->pts = be_uint32_t(*buf);
			++ buf;
			elst->mediatime = be_uint32_t(*buf);
			++ buf;
		}
		++ buf;
		elst->data = buf;
	}
}
