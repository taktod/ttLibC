/**
 * @file   trun.c
 * @brief  trun atom support.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/07/07
 */

#include "trun.h"
#include "../../../util/ioUtil.h"
#include "../../../log.h"
#include "../../../util/hexUtil.h"

ttLibC_Mp4 *ttLibC_Trun_make(
		uint8_t *data,
		size_t data_size) {
	ttLibC_Trun *trun = (ttLibC_Trun *)ttLibC_Mp4Atom_make(
			NULL,
			data,
			data_size,
			false,
			0,
			1000,
			Mp4Type_Trun);
	if(trun == NULL) {
		return NULL;
	}
	trun->current_composition_time_offset = 0;
	trun->current_duration = 0;
	trun->current_flags = 0;
	trun->current_pts = 0;
	trun->current_size = 0;
	trun->data = NULL;
	trun->data_offset = 0;
	trun->first_sample_flags = 0;
	trun->flags = 0;
	trun->sample_count = 0;
	trun->track = NULL;
	return (ttLibC_Mp4 *)trun;
}

void ttLibC_Trun_setTrack(
		ttLibC_Mp4 *mp4,
		ttLibC_Mp4Track *track) {
	ttLibC_Trun *trun = (ttLibC_Trun *)mp4;
	mp4->inherit_super.pts = track->decode_time_duration;
	mp4->inherit_super.timebase = track->timebase;
	trun->track = track;
	uint32_t *buf = (uint32_t *)trun->inherit_super.inherit_super.inherit_super.data;
	buf += 2;
	trun->flags = be_uint32_t(*buf);
	++ buf;
	trun->sample_count = be_uint32_t(*buf);
	++ buf;
	if((trun->flags & 0x000001) != 0) {
		trun->data_offset = be_uint32_t(*buf);
		++ buf;
	}
	else {
		// ?
		trun->data_offset = track->tfhd_base_data_offset;
	}
	if((trun->flags & 0x000004) != 0) {
		trun->first_sample_flags = be_uint32_t(*buf);
		++ buf;
	}
	else {
		if(track->tfhd_sample_flags != 0) {
			trun->first_sample_flags = track->tfhd_sample_flags;
		}
		else {
			trun->first_sample_flags = track->trex_sample_flags;
		}
	}
	trun->current_composition_time_offset = 0;
	trun->current_duration = 0;
	trun->current_flags = 0;
	trun->current_pts = trun->inherit_super.inherit_super.inherit_super.pts;
	trun->current_size = 0;
	trun->current_pos = trun->data_offset;
	trun->data = buf;
	ttLibC_Trun_moveNext((ttLibC_Mp4 *)trun);
}

uint64_t ttLibC_Trun_refCurrentPts(ttLibC_Mp4 *mp4) {
	ttLibC_Trun *trun = (ttLibC_Trun *)mp4;
	return trun->current_pts;
}
uint32_t ttLibC_Trun_refCurrentDelta(ttLibC_Mp4 *mp4) {
	ttLibC_Trun *trun = (ttLibC_Trun *)mp4;
	return trun->current_duration;
}
uint32_t ttLibC_Trun_refCurrentPos(ttLibC_Mp4 *mp4) {
	ttLibC_Trun *trun = (ttLibC_Trun *)mp4;
	return trun->current_pos;
}
uint32_t ttLibC_Trun_refCurrentSize(ttLibC_Mp4 *mp4) {
	ttLibC_Trun *trun = (ttLibC_Trun *)mp4;
	return trun->current_size;
}
bool ttLibC_Trun_moveNext(ttLibC_Mp4 *mp4) {
	ttLibC_Trun *trun = (ttLibC_Trun *)mp4;
	ttLibC_Mp4Track *track = trun->track;
	uint32_t *buf = trun->data;
	if(trun->sample_count == 0) {
		return false; // no more.
	}
	-- trun->sample_count;
	// now update information.
	trun->current_pts += trun->current_duration;
	if((trun->flags & 0x000100) != 0) {
		trun->current_duration = be_uint32_t(*buf);
		++ buf;
	}
	else {
		if(track->tfhd_sample_duration != 0) {
			trun->current_duration = track->tfhd_sample_duration;
		}
		else {
			trun->current_duration = track->trex_sample_duration;
		}
	}
	trun->current_pos += trun->current_size;
	if((trun->flags & 0x000200) != 0) {
		trun->current_size = be_uint32_t(*buf);
		++ buf;
	}
	else {
		if(track->tfhd_sample_size != 0) {
			trun->current_size = track->tfhd_sample_size;
		}
		else {
			trun->current_size = track->trex_sample_size;
		}
	}
	if((trun->flags & 0x000400) != 0) {
		trun->current_flags = be_uint32_t(*buf);
		++ buf;
	}
	else {
		if(track->tfhd_sample_flags != 0) {
			trun->current_flags = track->tfhd_sample_flags;
		}
		else {
			trun->current_flags = track->trex_sample_flags;
		}
	}
	if((trun->flags & 0x000800) != 0) {
		// sample_composition time presented
		trun->current_composition_time_offset = be_uint32_t(*buf);
		++ buf;
	}
	else {

	}
	trun->data = buf;
	return true;
}