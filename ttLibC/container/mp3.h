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

/**
 * definition of mp3 container.
 */
typedef struct ttLibC_Container_Mp3 {
	ttLibC_Container inherit_super;
} ttLibC_Container_Mp3;

/**
 * get frame from container.
 * @param mp3      mp3 container.
 * @param callback callback for getFrame
 * @param ptr      user def value pointer.
 */
bool ttLibC_Container_Mp3_getFrame(
		ttLibC_Container_Mp3 *mp3,
		ttLibC_getFrameFunc callback,
		void *ptr);

/**
 * close mp3 container.
 * @param mp3
 */
void ttLibC_Container_Mp3_close(ttLibC_Container_Mp3 **mp3);

// -------------------------------------------------------------- //

/**
 * definition of mp3ContainerReader
 */
typedef struct ttLibC_ContainerReader_Mp3Reader {
	ttLibC_ContainerReader inherit_super;
} ttLibC_ContainerReader_Mp3Reader;

typedef ttLibC_ContainerReader_Mp3Reader ttLibC_Mp3Reader;

/**
 * callback function for mp3ReadFunc
 * @param ptr  user def data pointer.
 * @param mp3  mp3 container data.
 * @return true: continue, false: stop.
 */
typedef bool (* ttLibC_Mp3ReadFunc)(void *ptr, ttLibC_Container_Mp3 *mp3);

/**
 * make mp3 container reader object.
 * @return mp3 container reader object.
 */
ttLibC_Mp3Reader *ttLibC_Mp3Reader_make();

/**
 * read mp3 binary data.
 * @param reader    mp3 reader object.
 * @param data      data
 * @param data_size size of data
 * @param callback  callback for read.
 * @param ptr       user def pointer object.
 * @return true: ok false: error.
 */
bool ttLibC_Mp3Reader_read(
		ttLibC_Mp3Reader *reader,
		void *data,
		size_t data_size,
		ttLibC_Mp3ReadFunc callback,
		void *ptr);

/**
 * close mp3 container reader object.
 * @reader
 */
void ttLibC_Mp3Reader_close(ttLibC_Mp3Reader **reader);

// -------------------------------------------------------------- //

/**
 * definition of mp3 writer object.
 */
typedef struct ttLibC_ContainerWriter_Mp3Writer {
	ttLibC_ContainerWriter inherit_super;
} ttLibC_ContainerWriter_Mp3Writer;

typedef ttLibC_ContainerWriter_Mp3Writer ttLibC_Mp3Writer;

/**
 * make mp3 writer object.
 * @return mp3Writer object.
 */
ttLibC_Mp3Writer *ttLibC_Mp3Writer_make();

/**
 * write mp3 frame data.
 * @param writer   mp3 writer object.
 * @param frame    target frame.
 * @param callback callback for binary data.
 * @param ptr      user def data pointer, which will call in callback.
 * @return true:success false:error
 */
bool ttLibC_Mp3Writer_write(
		ttLibC_Mp3Writer *writer,
		ttLibC_Frame *frame,
		ttLibC_ContainerWriteFunc callback,
		void *ptr);

/**
 * close mp3Writer.
 * @param writer
 */
void ttLibC_Mp3Writer_close(ttLibC_Mp3Writer **writer);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MP3_H_ */
