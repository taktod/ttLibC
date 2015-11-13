/**
 * @file   containerCommon.h
 * @brief  container support. common header for inner use.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/11/07
 */

#ifndef TTLIBC_CONTAINER_CONTAINERCOMMON_H_
#define TTLIBC_CONTAINER_CONTAINERCOMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "container.h"

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

// void ttLibC_Container_close(ttLibC_Container **container);

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

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_CONTAINERCOMMON_H_ */
