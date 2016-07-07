/**
 * @file   stsz.h
 * @brief  stsz atom support.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/07/04
 */

#ifndef TTLIBC_CONTAINER_MP4_TYPE_STSZ_H_
#define TTLIBC_CONTAINER_MP4_TYPE_STSZ_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../mp4Atom.h"

typedef struct ttLibC_Container_Mp4_Stsz {
	ttLibC_Mp4Atom inherit_super;
	uint32_t sample_size;
	uint32_t sample_count;
	uint32_t *entry_size_data;
} ttLibC_Container_Mp4_Stsz;

typedef ttLibC_Container_Mp4_Stsz ttLibC_Stsz;

ttLibC_Mp4 *ttLibC_Stsz_make(
		uint8_t *data,
		size_t data_size,
		uint32_t timebase);
uint32_t ttLibC_Stsz_refCurrentSampleSize(ttLibC_Mp4 *mp4);
void ttLibC_Stsz_moveNext(ttLibC_Mp4 *mp4);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MP4_TYPE_STSZ_H_ */
