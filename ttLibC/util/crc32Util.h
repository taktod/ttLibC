/**
 * @file   crc32Util.h
 * @brief  crc32 support
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/13
 */

#ifndef TTLIBC_UTIL_CRC32UTIL_H_
#define TTLIBC_UTIL_CRC32UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * definition for crc32.
 */
typedef struct {
	/** calcurate value. */
	uint64_t crc;
} ttLibC_Util_Crc32;

typedef ttLibC_Util_Crc32 ttLibC_Crc32;

/**
 * make crc32
 * @param initial_data
 * memo:for mpegts initial_data 0xFFFFFFFFL
 * @return crc32 object.
 */
ttLibC_Crc32 *ttLibC_Crc32_make(uint32_t initial_data);

/**
 * update crc32 with byte data.
 * @param crc32 crc32 object.
 * @param byte  update byte value.
 */
void ttLibC_Crc32_update(ttLibC_Crc32 *crc32, uint8_t byte);

/**
 * get value
 * @param crc32
 */
uint32_t ttLibC_Crc32_getValue(ttLibC_Crc32 *crc32);

/**
 * close crc32 object.
 * @param crc32
 */
void ttLibC_Crc32_close(ttLibC_Crc32 **crc32);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_UTIL_CRC32UTIL_H_ */
