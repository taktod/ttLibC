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
typedef enum {
	containerType_flv,
	containerType_mkv,
	containerType_mp3,
	containerType_mp4,
	containerType_mpegts,
	containerType_riff,
	containerType_wav,
} ttLibC_Container_Type;

/**
 * detail definition of container.
 */
typedef struct {
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
 * common function for container make.
 * @param prev_container reuse container object.
 * @param container_size memory allocate size for container object.
 * @param container_type target container_type
 * @param data           data
 * @param data_size      data_size
 * @param non_copy_mode  true:hold the pointer / false:copy memory data.
 * @param pts            pts for container
 * @param timebase       timebase for pts.
 * @return container object.
 */
ttLibC_Container *ttLibC_Container_make(
		ttLibC_Container *prev_container,
		size_t container_size,
		ttLibC_Container_Type container_type,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase);

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

/**
 * close container
 * @param container
 */
void ttLibC_Container_close(ttLibC_Container **container);

// -------------------------------------------------------------- //

/**
 * definition of container reader.
 */
typedef struct {
	ttLibC_Container_Type type;
} ttLibC_ContainerReader;

/**
 * common work for containerReader make.
 * use inner only.
 * @param container_type target container type.
 * @param reader_size    sizeof object.
 * @return reader object.
 */
ttLibC_ContainerReader *ttLibC_ContainerReader_make(
		ttLibC_Container_Type container_type,
		size_t reader_size);

/**
 * close container reader
 * @param reader
 */
void ttLibC_ContainerReader_close(ttLibC_ContainerReader **reader);

// -------------------------------------------------------------- //

/**
 * definition of container writer.
 */
typedef struct {
	ttLibC_Container_Type type;
	uint64_t pts;
	uint32_t timebase;
} ttLibC_ContainerWriter;

/**
 * callback of writer function.
 * @param ptr       user def pointer object.
 * @param data      data to write
 * @param data_size data sizes
 * @return true:continue false:stop
 */
typedef bool (* ttLibC_ContainerWriterFunc)(void *ptr, void *data, size_t size);

/**
 * common work for containerWriter make
 * use inner only.
 * @param container_type target container type.
 * @param writer_size    sizeof object.
 * @param timebase       timebase for writer.
 * @return writer object.
 */
ttLibC_ContainerWriter *ttLibC_ContainerWriter_make(
		ttLibC_Container_Type container_type,
		size_t writer_size,
		uint32_t timebase);

/**
 * close container writer
 * @param writer
 */
void ttLibC_ContainerWriter_close(ttLibC_ContainerWriter **writer);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_CONTAINER_H_ */
