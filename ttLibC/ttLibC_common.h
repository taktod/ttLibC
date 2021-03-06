/**
 * @file   ttLibC_common.h
 * @brief  misc.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2016/07/10
 */

#ifndef TTLIBC_TTLIBC_COMMON_H_
#define TTLIBC_TTLIBC_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ttLibC.h"

/**
 * setup error.
 */
Error_e ttLibC_updateError(Error_Target_e target, Error_e error);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_TTLIBC_COMMON_H_ */
