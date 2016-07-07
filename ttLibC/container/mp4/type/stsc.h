/**
 * @file   stsc.h
 * @brief  stsc atom support.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/07/04
 */

#ifndef TTLIBC_CONTAINER_MP4_TYPE_STSC_H_
#define TTLIBC_CONTAINER_MP4_TYPE_STSC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../mp4Atom.h"

typedef struct ttLibC_Container_Mp4_Stsc {
	ttLibC_Mp4Atom inherit_super;
	uint32_t entry_count;
	uint32_t current_count;
	uint32_t current_samples_in_chunk;
	uint32_t current_sample_description_ref;
	uint32_t first_chunk;
	uint32_t samples_in_chunk;
	uint32_t sample_description_ref;
	uint32_t *data;
} ttLibC_Container_Mp4_Stsc;

typedef ttLibC_Container_Mp4_Stsc ttLibC_Stsc;

ttLibC_Mp4 *ttLibC_Stsc_make(
		uint8_t *data,
		size_t data_size,
		uint32_t timebase);

uint32_t ttLibC_Stsc_refChunkSampleNum(ttLibC_Mp4 *mp4);
uint32_t ttLibC_Stsc_refSampleDescriptionRef(ttLibC_Mp4 *mp4);
void ttLibC_Stsc_moveNext(ttLibC_Mp4 *mp4);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MP4_TYPE_STSC_H_ */
