/*
 * @file   mp3Reader.h
 * @brief  
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

typedef struct {
	ttLibC_Mp3Reader inherit_super;
	ttLibC_Mp3Frame *frame;
	uint8_t *tmp_buffer;
	size_t tmp_buffer_size;
	size_t tmp_buffer_next_pos;
	uint64_t current_pts;
	uint32_t timebase;
} ttLibC_ContainerReader_Mp3Reader_;

typedef ttLibC_ContainerReader_Mp3Reader_ ttLibC_Mp3Reader_;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MP3_MP3READER_H_ */
