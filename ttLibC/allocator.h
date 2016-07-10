/**
 * @file   allocator.h
 * @brief  custom memory control for memory leak.
 *
 * this code is under 3-Cause BSD license
 *
 * @author taktod
 * @date   2015/09/07
 *
 * @see http://qiita.com/shimacpyon/items/9f18dbac318059aa6ed8
 */

#ifndef TTLIBC_ALLOCATOR_H_
#define TTLIBC_ALLOCATOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * ttLibC_malloc
 * @param size allocate size
 * @return memory pointer
 */
#define ttLibC_malloc(size) ttLibC_Allocator_malloc(size, __FILE__, __LINE__, __func__)

/**
 * ttLibC_calloc
 * @param n    allocator object num
 * @param size allocator unit size
 * @return memory pointer
 */
#define ttLibC_calloc(n, size) ttLibC_Allocator_calloc(n, size, __FILE__, __LINE__, __func__)

/**
 * ttLibC_free
 * @param ptr memory pointer
 */
#define ttLibC_free(ptr) ttLibC_Allocator_free(ptr)

/**
 * malloc with information.
 * @param size      allocate size
 * @param file_name caller file name
 * @param line      caller line number
 * @param func_name caller func name
 * @return memory pointer
 */
void *ttLibC_Allocator_malloc(size_t size, const char *file_name, int line, const char *func_name);

/**
 * calloc with information.
 * @param n         allocator object num
 * @param size      allocator unit size
 * @param file_name caller file name
 * @param line      caller line number
 * @param func_name caller func name
 * @return memory pointer
 */
void *ttLibC_Allocator_calloc(size_t n, size_t size, const char *file_name, int line, const char *func_name);

/**
 * free with information
 * @param ptr memory pointer
 */
void ttLibC_Allocator_free(void *ptr);

/**
 * initialize information table.
 * @return true:success false:error(ignore info collecting.)
 */
bool ttLibC_Allocator_init();

/**
 * dump current memory information.
 * @return total size of allocate.
 */
size_t ttLibC_Allocator_dump();

/**
 * close information table.
 */
void ttLibC_Allocator_close();

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_ALLOCATOR_H_ */
