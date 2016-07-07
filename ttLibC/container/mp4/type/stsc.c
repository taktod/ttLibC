/**
 * @file   stsc.c
 * @brief  stsc atom support.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/07/04
 */

#include "stsc.h"
#include "../../../util/ioUtil.h"
#include "../../../log.h"

ttLibC_Mp4 *ttLibC_Stsc_make(
		uint8_t *data,
		size_t data_size,
		uint32_t timebase) {
	ttLibC_Stsc *stsc = (ttLibC_Stsc *)ttLibC_Mp4Atom_make(
			NULL,
			data,
			data_size,
			false,
			0,
			timebase,
			Mp4Type_Stsc);
	if(stsc == NULL) {
		return NULL;
	}
	uint32_t *buf = (uint32_t *)stsc->inherit_super.inherit_super.inherit_super.data;
	buf += 3;
	stsc->entry_count = be_uint32_t(*buf);
	++ buf;
	stsc->first_chunk = be_uint32_t(*buf);
	++ buf;
	stsc->samples_in_chunk = be_uint32_t(*buf);
	++ buf;
	stsc->sample_description_ref = be_uint32_t(*buf);
	++ buf;
	stsc->data = buf;
	stsc->current_count = 1;
	stsc->current_samples_in_chunk = stsc->samples_in_chunk;
	stsc->current_sample_description_ref = stsc->sample_description_ref;
	ttLibC_Stsc_moveNext((ttLibC_Mp4 *)stsc);
	return (ttLibC_Mp4 *)stsc;
}

uint32_t ttLibC_Stsc_refChunkSampleNum(ttLibC_Mp4 *mp4) {
	// stsc is only one, which will return sample num forever.
	ttLibC_Stsc *stsc = (ttLibC_Stsc *)mp4;
	return stsc->current_samples_in_chunk;
}

uint32_t ttLibC_Stsc_refSampleDescriptionRef(ttLibC_Mp4 *mp4) {
	ttLibC_Stsc *stsc = (ttLibC_Stsc *)mp4;
	return stsc->current_sample_description_ref;
}

void ttLibC_Stsc_moveNext(ttLibC_Mp4 *mp4) {
	ttLibC_Stsc *stsc = (ttLibC_Stsc *)mp4;
	stsc->current_count ++;
	if(stsc->current_count > stsc->first_chunk) {
		if(stsc->entry_count > 0) {
			stsc->first_chunk = be_uint32_t(*stsc->data);
			++ stsc->data;
			stsc->current_samples_in_chunk = stsc->samples_in_chunk;
			stsc->current_sample_description_ref = stsc->sample_description_ref;
			stsc->samples_in_chunk = be_uint32_t(*stsc->data);
			++ stsc->data;
			stsc->sample_description_ref = be_uint32_t(*stsc->data);
			++ stsc->data;
			-- stsc->entry_count;
		}
	}
}
