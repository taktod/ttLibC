/**
 * @file   promise.h
 * @brief  promise/future for tetty.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2015/12/17
 */

#ifndef TTLIBC_NET_TETTY_PROMISE_H_
#define TTLIBC_NET_TETTY_PROMISE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../../ttLibC_predef.h"
#include "../tetty.h"

/**
 * promise work type.
 */
typedef enum ttLibC_PromiseType {
	PromiseType_Promise,
	PromiseType_Future
} ttLibC_PromiseType;

/**
 * detail definition of promise/future.
 */
typedef struct ttLibC_Net_TettyPromise_ {
	ttLibC_TettyPromise inherit_super;
	ttLibC_PromiseType promise_type; // promise or future
	ttLibC_TettyBootstrap *bootstrap;
	bool is_done;
	bool is_success;
	void *return_val;
	ttLibC_TettyPromiseListener listener;
	void *ptr;
} ttLibC_Net_TettyPromise_;

typedef ttLibC_Net_TettyPromise_ ttLibC_TettyPromise_;

/**
 * make promise
 * @param bootstrap bootstrap object.
 * @return promise object.
 */
ttLibC_TettyPromise TT_ATTRIBUTE_INNER *ttLibC_TettyPromise_make_(ttLibC_TettyBootstrap *bootstrap);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_TETTY_PROMISE_H_ */
