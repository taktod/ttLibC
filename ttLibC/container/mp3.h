/**
 * @file   mp3.h
 * @brief  mp3 container support
 *
 * this code is under 3-Cause BSD license
 *
 * @author taktod
 * @date   2015/08/23
 */

#ifndef TTLIBC_CONTAINER_MP3_H_
#define TTLIBC_CONTAINER_MP3_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "container.h"

typedef struct {
	ttLibC_Container inherit_super;
} ttLibC_Container_Mp3;

bool ttLibC_Container_Mp3_getFrame(
		ttLibC_Container_Mp3 *mp3,
		ttLibC_getFrameFunc callback,
		void *ptr);

void ttLibC_Container_Mp3_close(ttLibC_Container_Mp3 **mp3);

// -------------------------------------------------------------- //

typedef struct {
	ttLibC_ContainerReader inherit_super;
} ttLibC_ContainerReader_Mp3Reader;

typedef ttLibC_ContainerReader_Mp3Reader ttLibC_Mp3Reader;

typedef bool (* ttLibC_Mp3ReadFunc)(void *ptr, ttLibC_Container_Mp3 *mp3);

ttLibC_Mp3Reader *ttLibC_Mp3Reader_make();

bool ttLibC_Mp3Reader_read(
		ttLibC_Mp3Reader *reader,
		void *data,
		size_t data_size,
		ttLibC_Mp3ReadFunc callback,
		void *ptr);

void ttLibC_Mp3Reader_close(ttLibC_Mp3Reader **reader);

// -------------------------------------------------------------- //

typedef struct {
	ttLibC_ContainerWriter inherit_super;
} ttLibC_ContainerWriter_Mp3Writer;

typedef ttLibC_ContainerWriter_Mp3Writer ttLibC_Mp3Writer;

ttLibC_Mp3Writer *ttLibC_Mp3Writer_make();

bool ttLibC_Mp3Writer_write(
		ttLibC_Mp3Writer *writer,
		ttLibC_Frame *frame,
		ttLibC_ContainerWriteFunc callback,
		void *ptr);

void ttLibC_Mp3Writer_close(ttLibC_Mp3Writer **writer);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MP3_H_ */
