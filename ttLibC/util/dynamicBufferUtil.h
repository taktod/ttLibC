/**
 * @file   dynamicBufferUtil.h
 * @brief  expandable data buffer.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/10/06
 */

#ifndef TTLIBC_UTIL_DYNAMICBUFFERUTIL_H_
#define TTLIBC_UTIL_DYNAMICBUFFERUTIL_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * data for dynamic buffer,
 * @note data is for ref.
 */
typedef struct ttLibC_Util_DynamicBuffer {
	/** memory size */
	size_t buffer_size;
	/** hold memory size, (less than buffer_size) */
	size_t target_size;
} ttLibC_Util_DynamicBuffer;

typedef ttLibC_Util_DynamicBuffer ttLibC_DynamicBuffer;

/**
 * make buffer
 * @return dynamicBuffer object.
 */
ttLibC_DynamicBuffer* ttLibC_DynamicBuffer_make();

/**
 * append data on the end of buffer.
 * @param buffer    target dynamic buffer object.
 * @param data      append data.
 * @param data_size append data size
 */
bool ttLibC_DynamicBuffer_append(
		ttLibC_DynamicBuffer *buffer,
		uint8_t *data,
		size_t data_size);

/**
 * set read pointer.
 * read_size will be deleted on clear function.
 * @param buffer    target dynamic buffer object.
 * @param read_size size of read.
 */
bool ttLibC_DynamicBuffer_markAsRead(
		ttLibC_DynamicBuffer *buffer,
		size_t read_size);

/**
 * ref the data pointer.
 * @param buffer target dynamic buffer object.
 * @return data pointer.
 */
uint8_t *ttLibC_DynamicBuffer_refData(ttLibC_DynamicBuffer *buffer);

/**
 * ref the data size.
 * @param buffer target dynamic buffer object.
 * @return data size.
 */
size_t ttLibC_DynamicBuffer_refSize(ttLibC_DynamicBuffer *buffer);

/**
 * reset the read pointer.
 * @param buffer target dynamic buffer object.
 */
bool ttLibC_DynamicBuffer_reset(ttLibC_DynamicBuffer *buffer);

/**
 * clear read size and shift the data.
 * @param buffer target dynamic buffer object.
 */
bool ttLibC_DynamicBuffer_clear(ttLibC_DynamicBuffer *buffer);

/**
 * set empty for writing buffer.
 */
bool ttLibC_DynamicBuffer_empty(ttLibC_DynamicBuffer *buffer);

/**
 * alloc specific size of memory for dynamic buffer.
 * @param buffer target dynamic buffer object.
 * @param size   target size
 */
bool ttLibC_DynamicBuffer_alloc(
		ttLibC_DynamicBuffer *buffer,
		size_t size);

/**
 * write data on the dynamic buffer on specific position.
 * if the data is overflowed, error.
 * @param buffer    target dynamic buffer object.
 * @param write_pos write start position. relative number from first byte.
 * @param data      write data.
 * @param data_size write data size.
 */
bool ttLibC_DynamicBuffer_write(
		ttLibC_DynamicBuffer *buffer,
		size_t write_pos,
		uint8_t *data,
		size_t data_size);

/**
 * close and release memory object.
 * @param buffer target dynamic buffer object.
 */
void ttLibC_DynamicBuffer_close(ttLibC_DynamicBuffer **buffer);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_UTIL_DYNAMICBUFFERUTIL_H_ */
