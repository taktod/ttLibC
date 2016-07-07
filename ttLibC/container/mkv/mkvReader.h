/**
 * @file   mkvReader.h
 * @brief  mkv container reader.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/09/04
 */

#ifndef TTLIBC_CONTAINER_MKV_MKVREADER_H_
#define TTLIBC_CONTAINER_MKV_MKVREADER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../mkv.h"
#include "../../util/dynamicBufferUtil.h"
#include "../../util/stlMapUtil.h"

#include "mkvTag.h"

/**
 * detail definition of mkv reader.
 */
typedef struct ttLibC_ContainerReader_MkvReader_ {
	ttLibC_MkvReader inherit_super;
	uint32_t error_number;
	ttLibC_StlMap *tracks; // track id -> track.
	ttLibC_MkvTrack *track; // tmp pointer for analyzing track information.
	uint32_t timebase;
	uint64_t pts;
	ttLibC_MkvTag *tag;
	ttLibC_DynamicBuffer *tmp_buffer;
	bool in_reading;
} ttLibC_ContainerReader_MkvReader_;

typedef ttLibC_ContainerReader_MkvReader_ ttLibC_MkvReader_;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MKV_MKVREADER_H_ */
