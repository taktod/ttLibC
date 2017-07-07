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
#include "../../../ttLibC_predef.h"
#include "../../../util/ioUtil.h"
#include "../../../_log.h"
#include "../../../util/hexUtil.h"

ttLibC_Mp4 TT_VISIBILITY_HIDDEN *ttLibC_Trun_make(
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

void TT_VISIBILITY_HIDDEN ttLibC_Trun_setTrack(
		ttLibC_Mp4 *mp4,
		ttLibC_Mp4Track *track) {
	ttLibC_Trun *trun = (ttLibC_Trun *)mp4;
	trun->track = track;
	trun->current_composition_time_offset = 0;
	trun->current_duration = 0;
	trun->current_flags = 0;
	trun->current_pts = track->decode_time_duration;
	trun->current_size = 0;
	trun->current_pos = 0;
	trun->data = (uint8_t *)trun->inherit_super.inherit_super.inherit_super.data;
	trun->data_size = trun->inherit_super.inherit_super.inherit_super.buffer_size;
	ttLibC_Trun_moveNext((ttLibC_Mp4 *)trun);
}

uint64_t TT_VISIBILITY_HIDDEN ttLibC_Trun_refCurrentPts(ttLibC_Mp4 *mp4) {
	ttLibC_Trun *trun = (ttLibC_Trun *)mp4;
	return trun->current_pts;
}
uint32_t TT_VISIBILITY_HIDDEN ttLibC_Trun_refCurrentDelta(ttLibC_Mp4 *mp4) {
	ttLibC_Trun *trun = (ttLibC_Trun *)mp4;
	return trun->current_duration;
}
uint32_t TT_VISIBILITY_HIDDEN ttLibC_Trun_refCurrentPos(ttLibC_Mp4 *mp4) {
	ttLibC_Trun *trun = (ttLibC_Trun *)mp4;
	return trun->current_pos;
}
uint32_t TT_VISIBILITY_HIDDEN ttLibC_Trun_refCurrentSize(ttLibC_Mp4 *mp4) {
	ttLibC_Trun *trun = (ttLibC_Trun *)mp4;
	return trun->current_size;
}
uint32_t TT_VISIBILITY_HIDDEN ttLibC_Trun_refCurrentTimeOffset(ttLibC_Mp4 *mp4) {
	ttLibC_Trun *trun = (ttLibC_Trun *)mp4;
	return trun->current_composition_time_offset;
}
bool TT_VISIBILITY_HIDDEN ttLibC_Trun_isValid(ttLibC_Mp4 *mp4) {
	ttLibC_Trun *trun = (ttLibC_Trun *)mp4;
	return trun->sample_count != 0 || trun->data_size != 0;
}
bool TT_VISIBILITY_HIDDEN ttLibC_Trun_moveNext(ttLibC_Mp4 *mp4) {
	ttLibC_Trun *trun = (ttLibC_Trun *)mp4;
	ttLibC_Mp4Track *track = trun->track;
	uint8_t *buf = trun->data;
	size_t buf_size = trun->data_size;
	if(trun->sample_count == 0) {
		if(buf_size == 0) {
			// no more data.
			return false;
		}
		// in the case of more data. need to check traf binary.
		while(buf_size > 0) {
			uint32_t sz = be_uint32_t(*(uint32_t *)buf);
			buf += 4;
			buf_size -= 4;
			if(be_uint32_t(*(uint32_t *)buf) == Mp4Type_Trun) {
				buf += 4;
				buf_size -= 4;
				trun->flags = be_uint32_t(*(uint32_t *)buf);
				buf += 4;
				buf_size -= 4;
				trun->sample_count = be_uint32_t(*(uint32_t *)buf);
				buf += 4;
				buf_size -= 4;
				if((trun->flags & 0x000001) != 0) {
					trun->data_offset = be_uint32_t(*(uint32_t *)buf);
					buf += 4;
					buf_size -= 4;
				}
				else {
					trun->data_offset = 0;
				}
				if((trun->flags & 0x000004) != 0) {
					trun->first_sample_flags = be_uint32_t(*(uint32_t *)buf);
					buf += 4;
					buf_size -= 4;
				}
				else {
					if(track->tfhd_sample_flags != 0) {
						trun->first_sample_flags = track->tfhd_sample_flags;
					}
					else {
						trun->first_sample_flags = track->trex_sample_flags;
					}
				}
				trun->current_pos = trun->data_offset;
				trun->current_composition_time_offset = 0;
				trun->current_pts = track->decode_time_duration;
				trun->current_duration = 0;
				trun->current_flags = 0;
				trun->current_size = 0;
				break;
			}
			buf += (sz - 4);
			buf_size -= (sz - 4);
		}
	}
	-- trun->sample_count;
	trun->current_pts += trun->current_duration;
	trun->current_pos += trun->current_size;
	if((trun->flags & 0x000100) != 0) {
		trun->current_duration = be_uint32_t(*(uint32_t *)buf);
		buf += 4;
		buf_size -= 4;
	}
	else {
		if(track->tfhd_sample_duration != 0) {
			trun->current_duration = track->tfhd_sample_duration;
		}
		else {
			trun->current_duration = track->trex_sample_duration;
		}
	}
	track->decode_time_duration = trun->current_pts + trun->current_duration;
	if((trun->flags & 0x000200) != 0) {
		trun->current_size = be_uint32_t(*(uint32_t *)buf);
		buf += 4;
		buf_size -= 4;
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
		trun->current_flags = be_uint32_t(*(uint32_t *)buf);
		buf += 4;
		buf_size -= 4;
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
		trun->current_composition_time_offset = be_uint32_t(*(uint32_t *)buf);
		buf += 4;
		buf_size -= 4;
	}
	else {
		trun->current_composition_time_offset = 0;
	}
	trun->data = buf;
	trun->data_size = buf_size;
	return true;
}
