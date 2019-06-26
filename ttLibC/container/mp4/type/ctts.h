/*
 * ctts.h
 *
 *  Created on: 2016/10/30
 *      Author: taktod
 */

#ifndef TTLIBC_CONTAINER_MP4_TYPE_CTTS_H_
#define TTLIBC_CONTAINER_MP4_TYPE_CTTS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../../../ttLibC_predef.h"
#include "../mp4Atom.h"

typedef struct ttLibC_Container_Mp4_Ctts {
	ttLibC_Mp4Atom inherit_super;
	uint32_t entry_count;
	uint32_t sample_count;
	uint32_t sample_offset;
	uint32_t *data;
} ttLibC_Container_Mp4_Ctts;

typedef ttLibC_Container_Mp4_Ctts ttLibC_Ctts;

ttLibC_Mp4 TT_ATTRIBUTE_INNER *ttLibC_Ctts_make(
		uint8_t *data,
		size_t data_size,
		uint32_t timebase);

uint32_t TT_ATTRIBUTE_INNER ttLibC_Ctts_refCurrentOffset(ttLibC_Mp4 *mp4);
void TT_ATTRIBUTE_INNER ttLibC_Ctts_moveNext(ttLibC_Mp4 *mp4);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MP4_TYPE_CTTS_H_ */
