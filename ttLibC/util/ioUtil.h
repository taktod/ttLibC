/**
 * @file   ioUtil.h
 * @brief  handle input/output
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/09
 */

#ifndef TTLIBC_UTIL_IOUTIL_H_
#define TTLIBC_UTIL_IOUTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#if !defined(__LITTLE_ENDIAN__) && (defined(WORDS_BIGENDIAN) || defined(__BIG_ENDIAN__))
#define le_uint16_t(s) ( (uint16_t) ( ((s & 0xFF) << 8) | ((s >> 8) & 0x00FF) ) )
#define be_uint16_t(s) ( (uint16_t) s )
#define le_uint24_t(m) ( (uint32_t) ( ((m & 0xFF) << 16) | (m & 0x00FF00) | ((m >> 16) & 0x0000FF) ) )
#define be_uint24_t(m) ( (uint32_t) ( m & 0x00FFFFFF ) )
#define le_uint32_t(i) ( (uint32_t) ( ((i & 0xFF) << 24) | ((i & 0xFF00) << 8) | ((i >> 8) & 0x0000FF00) | ((i >> 24) & 0x000000FF) ) )
#define be_uint32_t(i) ( (uint32_t) i )
#define le_uint64_t(l) ( (uint64_t) ( ((l & 0xFFL) << 56) | ((l & 0xFF00L) << 40) | ((l & 0xFF0000L) << 24) | ((l & 0xFF000000L) << 8) | ((l >> 8) & 0xFF000000) | ((l >> 24) & 0x00FF0000) | ((l >> 40) & 0x0000FF00) | ((l >> 56) & 0x000000FF) ) )
#define be_uint64_t(l) ( (uint64_t) l )

#define le_int16_t(s) ( (int16_t) ( ((s & 0xFF) << 8) | ((s >> 8) & 0x00FF) ) )
#define be_int16_t(s) ( (int16_t) s )
#define le_int32_t(i) ( (int32_t) ( ((i & 0xFF) << 24) | ((i & 0xFF00) << 8) | ((i >> 8) & 0x0000FF00) | ((i >> 24) & 0x000000FF) ) )
#define be_int32_t(i) ( (int32_t) i )
#define le_int64_t(l) ( (int64_t) ( ((l & 0xFFL) << 56) | ((l & 0xFF00L) << 40) | ((l & 0xFF0000L) << 24) | ((l & 0xFF000000L) << 8) | ((l >> 8) & 0xFF000000) | ((l >> 24) & 0x00FF0000) | ((l >> 40) & 0x0000FF00) | ((l >> 56) & 0x000000FF) ) )
#define be_int64_t(l) ( (int64_t) l )
#else

#define be_uint16_t(s) ( (uint16_t) ( ((s & 0xFF) << 8) | ((s >> 8) & 0x00FF) ) )
#define le_uint16_t(s) ( (uint16_t) s )
#define be_uint24_t(m) ( (uint32_t) ( ((m & 0xFF) << 16) | (m & 0x00FF00) | ((m >> 16) & 0x0000FF) ) )
#define le_uint24_t(m) ( (uint32_t) ( m & 0x00FFFFFF ) )
#define be_uint32_t(i) ( (uint32_t) ( ((i & 0xFF) << 24) | ((i & 0xFF00) << 8) | ((i >> 8) & 0x0000FF00) | ((i >> 24) & 0x000000FF) ) )
#define le_uint32_t(i) ( (uint32_t) i )
#define be_uint64_t(l) ( (uint64_t) ( ((l & 0xFFL) << 56) | ((l & 0xFF00L) << 40) | ((l & 0xFF0000L) << 24) | ((l & 0xFF000000L) << 8) | ((l >> 8) & 0xFF000000) | ((l >> 24) & 0x00FF0000) | ((l >> 40) & 0x0000FF00) | ((l >> 56) & 0x000000FF) ) )
#define le_uint64_t(l) ( (uint64_t) l )

#define be_int16_t(s) ( (int16_t) ( ((s & 0xFF) << 8) | ((s >> 8) & 0x00FF) ) )
#define le_int16_t(s) ( (int16_t) s )
#define be_int32_t(i) ( (int32_t) ( ((i & 0xFF) << 24) | ((i & 0xFF00) << 8) | ((i >> 8) & 0x0000FF00) | ((i >> 24) & 0x000000FF) ) )
#define le_int32_t(i) ( (int32_t) i )
#define be_int64_t(l) ( (int64_t) ( ((l & 0xFFL) << 56) | ((l & 0xFF00L) << 40) | ((l & 0xFF0000L) << 24) | ((l & 0xFF000000L) << 8) | ((l >> 8) & 0xFF000000) | ((l >> 24) & 0x00FF0000) | ((l >> 40) & 0x0000FF00) | ((l >> 56) & 0x000000FF) ) )
#define le_int64_t(l) ( (int64_t) l )
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_UTIL_IOUTIL_H_ */
