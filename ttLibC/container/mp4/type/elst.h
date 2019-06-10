/*
 * elst.h
 *
 *  Created on: 2016/12/08
 *      Author: taktod
 */

#ifndef TTLIBC_CONTAINER_MP4_TYPE_ELST_H_
#define TTLIBC_CONTAINER_MP4_TYPE_ELST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../mp4Atom.h"

typedef struct ttLibC_Container_Mp4_Elst {
	ttLibC_Mp4Atom inherit_super;
	uint32_t version;
	uint32_t entry_count;
	uint64_t pts;
	uint64_t mediatime;
	uint64_t start_interval;
	bool     error_flag;
} ttLibC_Container_Mp4_Elst;

typedef ttLibC_Container_Mp4_Elst ttLibC_Elst;

ttLibC_Mp4 *ttLibC_Elst_make(
		uint8_t *data,
		size_t data_size,
		uint32_t timebase);

uint64_t ttLibC_Elst_refCurrentMediatime(ttLibC_Mp4 *mp4);
 bool ttLibC_Elst_refErrorFlag(ttLibC_Mp4 *mp4);
uint64_t ttLibC_Elst_refStartInterval(ttLibC_Mp4 *mp4, uint32_t timebase);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MP4_TYPE_ELST_H_ */
