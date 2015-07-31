/**
 * @file   bit.h
 * @brief  bit work support.
 *
 * this code is under 3-Cause BSD license
 *
 * @author taktod
 * @date   2015/08/01
 */

#ifndef TTLIBC_UTIL_BIT_H_
#define TTLIBC_UTIL_BIT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * data for bitReader.
 */
typedef struct {
} ttLibC_Util_BitReader;

typedef ttLibC_Util_BitReader ttLibC_BitReader;

/**
 * make bit reader object.
 * @param data
 * @param data_size
 * @return bit reader object.
 */
ttLibC_BitReader *ttLibC_BitReader_make(void *data, size_t data_size);

/**
 * get bit from bit reader.
 * @param reader  target bit reader object
 * @param bit_num bit size for read.
 * @return number
 */
uint32_t ttLibC_BitReader_bit(ttLibC_BitReader *reader, uint32_t bit_num);

/**
 * get exp golomb from bit reader.
 * @param reader target bit reader
 * @param sign   true: signed false:unsigned
 * @return value
 */
int32_t  ttLibC_BitReader_expGolomb(ttLibC_BitReader *reader, bool sign);

/**
 * get ebmp value from bit reader.
 * @param reader target bit reader
 * @param is_tag true: read as tag, false:read as number.
 * @return number
 */
uint64_t ttLibC_BitReader_ebml(ttLibC_BitReader *reader, bool is_tag);

/**
 * close bit reader.
 * @param reader
 */
void ttLibC_BitReader_close(ttLibC_BitReader **reader);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_UTIL_BIT_H_ */
