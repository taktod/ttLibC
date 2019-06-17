/**
 * @file   trun.h
 * @brief  trun atom support.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/07/07
 */

#ifndef TTLIBC_CONTAINER_MP4_TYPE_TRUN_H_
#define TTLIBC_CONTAINER_MP4_TYPE_TRUN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../../../ttLibC_predef.h"
#include "../mp4Atom.h"

typedef struct ttLibC_Container_Mp4_Trun {
	ttLibC_Mp4Atom inherit_super;
	uint32_t sample_count;
	uint32_t flags;
	uint32_t data_offset;
	uint32_t first_sample_flags;

	uint64_t current_pts;
	uint32_t current_duration;
	uint64_t current_pos;
	uint32_t current_size;
	uint32_t current_flags;
	uint32_t current_composition_time_offset;
	uint8_t *data;
	size_t   data_size;
	ttLibC_Mp4Track *track;
} ttLibC_Container_Mp4_Trun;

typedef ttLibC_Container_Mp4_Trun ttLibC_Trun;

ttLibC_Mp4 TT_ATTRIBUTE_INNER *ttLibC_Trun_make(
		uint8_t *data,
		size_t data_size);

void TT_ATTRIBUTE_INNER ttLibC_Trun_setTrack(
		ttLibC_Mp4 *mp4,
		ttLibC_Mp4Track *track);

uint64_t TT_ATTRIBUTE_INNER ttLibC_Trun_refCurrentPts(ttLibC_Mp4 *mp4);
uint32_t TT_ATTRIBUTE_INNER ttLibC_Trun_refCurrentDelta(ttLibC_Mp4 *mp4);
uint32_t TT_ATTRIBUTE_INNER ttLibC_Trun_refCurrentPos(ttLibC_Mp4 *mp4);
uint32_t TT_ATTRIBUTE_INNER ttLibC_Trun_refCurrentSize(ttLibC_Mp4 *mp4);
uint32_t TT_ATTRIBUTE_INNER ttLibC_Trun_refCurrentTimeOffset(ttLibC_Mp4 *mp4);
bool TT_ATTRIBUTE_INNER ttLibC_Trun_moveNext(ttLibC_Mp4 *mp4);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MP4_TYPE_TRUN_H_ */
