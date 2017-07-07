/**
 * @file   stsz.c
 * @brief  stsz atom support.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/07/04
 */

#include "stsz.h"
#include "../../../ttLibC_predef.h"
#include "../../../util/ioUtil.h"

ttLibC_Mp4 TT_VISIBILITY_HIDDEN *ttLibC_Stsz_make(
		uint8_t *data,
		size_t data_size,
		uint32_t timebase) {
	ttLibC_Stsz *stsz = (ttLibC_Stsz *)ttLibC_Mp4Atom_make(
			NULL,
			data,
			data_size,
			false,
			0,
			timebase,
			Mp4Type_Stsz);
	if(stsz == NULL) {
		return NULL;
	}
	uint32_t *buf = (uint32_t *)stsz->inherit_super.inherit_super.inherit_super.data;
	buf += 3;
	stsz->sample_size = be_uint32_t(*buf);
	stsz->sample_count = be_uint32_t(*(buf + 1));
	stsz->entry_size_data = buf + 2;
	return (ttLibC_Mp4 *)stsz;
}
uint32_t TT_VISIBILITY_HIDDEN ttLibC_Stsz_refCurrentSampleSize(ttLibC_Mp4 *mp4) {
	ttLibC_Stsz *stsz = (ttLibC_Stsz *)mp4;
	if(stsz->sample_count > 0) {
		if(stsz->sample_size == 0) {
			return be_uint32_t(*stsz->entry_size_data);
		}
		else {
			return stsz->sample_size;
		}
	}
	else {
		return 0;
	}
}
void TT_VISIBILITY_HIDDEN ttLibC_Stsz_moveNext(ttLibC_Mp4 *mp4) {
	ttLibC_Stsz *stsz = (ttLibC_Stsz *)mp4;
	-- stsz->sample_count;
	if(stsz->sample_size == 0) {
		++ stsz->entry_size_data;
	}
}

