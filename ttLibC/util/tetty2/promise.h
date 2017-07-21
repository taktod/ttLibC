/*
 * promise.h
 *
 *  Created on: 2017/07/21
 *      Author: taktod
 */

#ifndef TTLIBC_UTIL_TETTY2_PROMISE_H_
#define TTLIBC_UTIL_TETTY2_PROMISE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../tetty2.h"

typedef struct ttLibC_Util_Tetty2Promise_ {
	ttLibC_Tetty2Promise inherit_super;
	ttLibC_Tetty2Bootstrap *bootstrap;
	ttLibC_Tetty2PromiseListener listener;
	void *ptr;
} ttLibC_Util_Tetty2Promise_;

typedef ttLibC_Util_Tetty2Promise_ ttLibC_Tetty2Promise_;

ttLibC_Tetty2Promise *ttLibC_Tetty2Promise_make_(ttLibC_Tetty2Bootstrap *bootstrap);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_UTIL_TETTY2_PROMISE_H_ */
