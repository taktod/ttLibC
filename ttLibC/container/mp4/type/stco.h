/**
 * @file   stco.h
 * @brief  stco atom support.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/07/04
 */

#ifndef TTLIBC_CONTAINER_MP4_TYPE_STCO_H_
#define TTLIBC_CONTAINER_MP4_TYPE_STCO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../../../ttLibC_predef.h"
#include "../mp4Atom.h"

typedef struct ttLibC_Container_Mp4_Stco {
	ttLibC_Mp4Atom inherit_super;
	uint32_t entry_count;
	uint32_t *chunk_offset_data;
} ttLibC_Container_Mp4_Stco;

typedef ttLibC_Container_Mp4_Stco ttLibC_Stco;

// for 64bit, there is co64

ttLibC_Mp4 TT_ATTRIBUTE_INNER *ttLibC_Stco_make(
		uint8_t *data,
		size_t data_size,
		uint32_t timebase);

uint32_t TT_ATTRIBUTE_INNER ttLibC_Stco_refOffset(ttLibC_Mp4 *mp4);
uint32_t TT_ATTRIBUTE_INNER ttLibC_Stco_refNextOffset(ttLibC_Mp4 *mp4);
void TT_ATTRIBUTE_INNER ttLibC_Stco_moveNext(ttLibC_Mp4 *mp4);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MP4_TYPE_STCO_H_ */
