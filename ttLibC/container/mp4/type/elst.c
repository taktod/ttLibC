/*
 * elst.c
 *
 *  Created on: 2016/12/08
 *      Author: taktod
 */

#include "elst.h"
#include "../../../ttLibC_predef.h"
#include "../../../util/ioUtil.h"
#include "../../../_log.h"

ttLibC_Mp4 TT_VISIBILITY_HIDDEN *ttLibC_Elst_make(
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
	elst->error_flag = false;
	elst->pts = 0;
	elst->mediatime = 0;
	elst->start_interval = 0;
	for(uint32_t i = 0;i < elst->entry_count;++ i) {
		uint32_t pts = be_uint32_t(*buf);
		++ buf;
		uint32_t mediatime = be_uint32_t(*buf);
		++ buf;
		++ buf;
		if(mediatime == 0xFFFFFFFF) {
			// special value for skipping time(based on reader->timebase).
			if(i != 0) {
				ERR_PRINT("mediatime = 0xFFFFFFFF should be found in first elst value.");
				elst->error_flag = true;
			}
			elst->start_interval = pts;
		}
		else {
			if(elst->pts != 0) {
				ERR_PRINT("found multiple mediatime definition.");
				elst->error_flag = true;
			}
			elst->pts = pts;
			elst->mediatime = mediatime;
		}
	}
	return (ttLibC_Mp4 *)elst;
}

uint64_t TT_VISIBILITY_HIDDEN ttLibC_Elst_refCurrentMediatime(ttLibC_Mp4 *mp4) {
	ttLibC_Elst *elst = (ttLibC_Elst *)mp4;
	if(elst == NULL) {
		return 0;
	}
	return elst->mediatime;
}

bool TT_VISIBILITY_HIDDEN ttLibC_Elst_refErrorFlag(ttLibC_Mp4 *mp4) {
	ttLibC_Elst *elst = (ttLibC_Elst *)mp4;
	if(elst == NULL) {
		return 0;
	}
	return elst->error_flag;
}

uint64_t TT_VISIBILITY_HIDDEN ttLibC_Elst_refStartInterval(ttLibC_Mp4 *mp4, uint32_t timebase) {
	ttLibC_Elst *elst = (ttLibC_Elst *)mp4;
	if(elst == NULL) {
		return 0;
	}
	return elst->start_interval * timebase / elst->inherit_super.inherit_super.inherit_super.timebase;
}
