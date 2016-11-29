/**
 * @file   container.h
 * @brief  container support.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/12
 */

#ifndef TTLIBC_CONTAINER_CONTAINER_H_
#define TTLIBC_CONTAINER_CONTAINER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "../frame/frame.h"

/**
 * container type.
 */
typedef enum ttLibC_Container_Type {
	containerType_flv,
	containerType_mkv,
	containerType_mp3,
	containerType_mp4,
	containerType_mpegts,
	containerType_riff,
	containerType_wav,
	containerType_webm
} ttLibC_Container_Type;

/**
 * detail definition of container.
 */
typedef struct ttLibC_Container {
	ttLibC_Container_Type type;
	void    *data;
	size_t   data_size;
	size_t   buffer_size;
	bool     is_non_copy;
	uint64_t pts;
	uint32_t timebase;
} ttLibC_Container;

/**
 * callback function to use each container to get frame object.
 * @param ptr   user def pointer.
 * @param frame analyzed frame from container.
 * @return true: work properly, false: error occured.
 */
typedef bool (* ttLibC_getFrameFunc)(void *ptr, ttLibC_Frame *frame);

/**
 * get frame from container object.
 * @param container container object.
 * @param callback  callback function
 * @param ptr       user def pointer object.
 * @return true:success false:error
 */
bool ttLibC_Container_getFrame(
		ttLibC_Container *container,
		ttLibC_getFrameFunc callback,
		void *ptr);

// -------------------------------------------------------------- //

/**
 * definition of container reader.
 */
typedef struct ttLibC_ContainerReader {
	ttLibC_Container_Type type;
} ttLibC_ContainerReader;

typedef bool (* ttLibC_ContainerReadFunc)(void *ptr, ttLibC_Container *container);

bool ttLibC_ContainerReader_read(
		ttLibC_ContainerReader *reader,
		void  *data,
		size_t data_size,
		ttLibC_ContainerReadFunc callback,
		void  *ptr);

/**
 * close container reader
 * @param reader
 */
void ttLibC_ContainerReader_close(ttLibC_ContainerReader **reader);

// -------------------------------------------------------------- //

#define containerWriter_normal                 0x00
#define containerWriter_enable_dts             0x20

/*
#define containerWriter_keyFrame_division   0x00
#define containerWriter_innerFrame_division 0x01
#define containerWriter_allFrame_division   0x02
*/

// split with all keyFrame
#define containerWriter_allKeyFrame_split      0x10
// split with keyFrame
#define containerWriter_keyFrame_split         0x00
// split with inner Frame, for h264 & h265, split with p frame which is not refed by b frame.
#define containerWriter_innerFrame_split       0x01
// split with p frame.
#define containerWriter_pFrame_split           0x02
// split with disposable b frame.
#define containerWriter_disposableBFrame_split 0x04
// split with b frame.
#define containerWriter_bFrame_split           0x08

#define ttLibC_ContainerWriter_Mode uint32_t

/**
 * definition of container writer.
 */
typedef struct ttLibC_ContainerWriter {
	ttLibC_Container_Type type;
	uint64_t pts;
	uint32_t timebase;
	ttLibC_ContainerWriter_Mode mode;
} ttLibC_ContainerWriter;

/**
 * callback of writer function.
 * @param ptr       user def pointer object.
 * @param data      data to write
 * @param data_size data sizes
 * @return true:continue false:stop
 */
typedef bool (* ttLibC_ContainerWriteFunc)(void *ptr, void *data, size_t size);

bool ttLibC_ContainerWriter_write(
		ttLibC_ContainerWriter *writer,
		ttLibC_Frame *frame,
		ttLibC_ContainerWriteFunc callback,
		void *ptr);

/**
 * close container writer
 * @param writer
 */
void ttLibC_ContainerWriter_close(ttLibC_ContainerWriter **writer);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_CONTAINER_H_ */
