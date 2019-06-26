/**
 * @file   forkUtil.h
 * @brief  util for process fork
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/09/28
 */

#ifndef TTLIBC_UTIL_FORKUTIL_H_
#define TTLIBC_UTIL_FORKUTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../ttLibC_predef.h"
#include <signal.h>

void TT_ATTRIBUTE_API ttLibC_ForkUtil_setup();
pid_t TT_ATTRIBUTE_API ttLibC_ForkUtil_fork();

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_UTIL_FORKUTIL_H_ */
