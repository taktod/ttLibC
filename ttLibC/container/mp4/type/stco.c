/**
 * @file   stco.c
 * @brief  stco atom support.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/07/04
 */

#include "stco.h"
#include "../../../util/ioUtil.h"
#include "../../../_log.h"

ttLibC_Mp4 TT_ATTRIBUTE_INNER *ttLibC_Stco_make(
		uint8_t *data,
		size_t data_size,
		uint32_t timebase) {
	ttLibC_Stco *stco = (ttLibC_Stco *)ttLibC_Mp4Atom_make(
			NULL,
			data,
			data_size,
			false,
			0,
			timebase,
			Mp4Type_Stco);
	if(stco == NULL) {
		return NULL;
	}
	uint32_t *buf = (uint32_t *)stco->inherit_super.inherit_super.inherit_super.data;
	buf += 3;
	stco->entry_count = be_uint32_t(*buf);
	stco->chunk_offset_data = buf + 1;
	return (ttLibC_Mp4 *)stco;

}

uint32_t TT_ATTRIBUTE_INNER ttLibC_Stco_refOffset(ttLibC_Mp4 *mp4) {
	ttLibC_Stco *stco = (ttLibC_Stco *)mp4;
	if(stco->entry_count > 0) {
		return be_uint32_t(*stco->chunk_offset_data);
	}
	else {
		return 0;
	}
}

uint32_t TT_ATTRIBUTE_INNER ttLibC_Stco_refNextOffset(ttLibC_Mp4 *mp4) {
	ttLibC_Stco *stco = (ttLibC_Stco *)mp4;
	if(stco->entry_count > 1) {
		return be_uint32_t(*(stco->chunk_offset_data + 1));
	}
	else {
		return 0;
	}
}

void TT_ATTRIBUTE_INNER ttLibC_Stco_moveNext(ttLibC_Mp4 *mp4) {
	ttLibC_Stco *stco = (ttLibC_Stco *)mp4;
	if(stco->entry_count == 0) {
		return;
	}
	-- stco->entry_count;
	++ stco->chunk_offset_data;
}
