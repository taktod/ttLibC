/**
 * @file   ttLibC.h
 * @brief  misc.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/12
 */

#ifndef TTLIBC_TTLIBC_H_
#define TTLIBC_TTLIBC_H_

#ifdef __cplusplus
extern "C" {
#endif

const char *ttLibC_getLastError(int error_no);

/**
 * @return version string for ttLibC
 */
const char *ttLibC_getVersion();


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* TTLIBC_TTLIBC_H_ */
