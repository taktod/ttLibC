/**
 * @file   hexUtil.h
 * @brief  library deal with hex data.
 *
 * this code is under 3-Cause BSD license.
 *
 * @see    hexUtilTest()
 * @author taktod
 * @date   2015/07/18
 */

#ifndef TTLIBC_UTIL_HEXUTIL_H_
#define TTLIBC_UTIL_HEXUTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "../log.h"

/**
 * dump memory data.
 * @param ptr            target data
 * @param length         data length
 * @param separator_flag insert space among each byte
 */
void ttLibC_HexUtil_dump(void *ptr, size_t length, bool separator_flag);

/**
 * make void* buffer according to hex string
 * @param target	hex string
 * @param ptr		data to store
 * @param length	size of ptr
 * @return	size of filled data
 */
uint32_t ttLibC_HexUtil_makeBuffer(const char *target, void *ptr, size_t length);

/**
 * dump memory data. only for debug compile
 * @param ptr            target data
 * @param length         data length
 * @param separator_flag insert space among each byte
 */
#if __DEBUG_FLAG__ == 1
#	define	LOG_DUMP(ptr, length, separator_flag) ttLibC_HexUtil_dump(ptr, length, separator_flag)
#else
#	define	LOG_DUMP(ptr, length, separator_flag)
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_UTIL_HEXUTIL_H_ */
