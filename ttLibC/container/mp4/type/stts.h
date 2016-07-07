/**
 * @file   stts.h
 * @brief  stts atom support.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/07/04
 */

#ifndef TTLIBC_CONTAINER_MP4_TYPE_STTS_H_
#define TTLIBC_CONTAINER_MP4_TYPE_STTS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../mp4Atom.h"

typedef struct ttLibC_Container_Mp4_Stts {
	ttLibC_Mp4Atom inherit_super;
	uint32_t entry_count;
	uint32_t current_count; // = 0, get next data.
	uint32_t current_delta;
	uint64_t current_pts;
	uint32_t *data;
} ttLibC_Container_Mp4_Stts;

typedef ttLibC_Container_Mp4_Stts ttLibC_Stts;

ttLibC_Mp4 *ttLibC_Stts_make(
		uint8_t *data,
		size_t data_size,
		uint32_t timebase);

uint32_t ttLibC_Stts_refCurrentDelta(ttLibC_Mp4 *mp4);
uint32_t ttLibC_Stts_refCurrentPts(ttLibC_Mp4 *mp4);
void ttLibC_Stts_moveNext(ttLibC_Mp4 *mp4);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MP4_TYPE_STTS_H_ */
