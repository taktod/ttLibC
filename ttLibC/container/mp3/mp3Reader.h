/*
 * @file   mp3Reader.h
 * @brief  mp3Frame reader from binary data.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/23
 */

#ifndef TTLIBC_CONTAINER_MP3_MP3READER_H_
#define TTLIBC_CONTAINER_MP3_MP3READER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../mp3.h"

#include "mp3Frame.h"
#include "../../util/dynamicBufferUtil.h"

/**
 * mp3Reader detail definition.
 */
typedef struct ttLibC_ContainerReader_Mp3Reader_ {
	ttLibC_Mp3Reader inherit_super;
	ttLibC_Mp3Frame *frame;
	ttLibC_DynamicBuffer *tmp_buffer;
	uint64_t current_pts;
	uint32_t timebase;
	bool is_reading;
} ttLibC_ContainerReader_Mp3Reader_;

typedef ttLibC_ContainerReader_Mp3Reader_ ttLibC_Mp3Reader_;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MP3_MP3READER_H_ */
