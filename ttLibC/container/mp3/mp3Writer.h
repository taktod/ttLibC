/**
 * @file   mp3Writer.h
 * @brief  mp3Frame writer to make binary data.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/23
 */

#ifndef TTLIBC_CONTAINER_MP3_MP3WRITER_H_
#define TTLIBC_CONTAINER_MP3_MP3WRITER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../mp3.h"
#include "mp3Frame.h"

/**
 * mp3writer detail definition.
 */
typedef struct ttLibC_ContainerWriter_Mp3Writer_ {
	ttLibC_Mp3Writer inherit_super;
	bool is_first;
} ttLibC_ContainerWriter_Mp3Writer_;

typedef ttLibC_ContainerWriter_Mp3Writer_ ttLibC_Mp3Writer_;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MP3_MP3WRITER_H_ */
