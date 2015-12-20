/*
 * @file   promise.c
 * @brief  promise/future for tetty.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2015/12/17
 */

#include "promise.h"
#include "../../allocator.h"
#include "../../log.h"

/*
 * make promise
 * @param bootstrap bootstrap object.
 * @return promise object.
 */
ttLibC_TettyPromise *ttLibC_TettyPromise_make_(ttLibC_TettyBootstrap *bootstrap) {
	ttLibC_TettyPromise_ *promise = ttLibC_malloc(sizeof(ttLibC_TettyPromise_));
	if(promise == NULL) {
		return NULL;
	}
	promise->promise_type = PromiseType_Promise;
	promise->bootstrap    = bootstrap;
	promise->is_done      = false;
	promise->is_success   = false;
	promise->listener     = NULL;
	promise->inherit_super.bootstrap  = bootstrap;
	promise->inherit_super.is_done    = false;
	promise->inherit_super.is_success = false;
	return (ttLibC_TettyPromise *)promise;
}

/*
 * sync until promise/future done.
 * @param promise target promise
 */
void ttLibC_TettyPromise_sync(ttLibC_TettyPromise *promise) {
	ttLibC_TettyPromise_* promise_ = (ttLibC_TettyPromise_ *)promise;
	// wait until done...
	while(!promise_->is_done) {
		ttLibC_TettyBootstrap_update(promise_->bootstrap);
		if(promise_->bootstrap->error_flag != 0) {
			// if error occured, update promise as error.
			if(!promise_->is_done) {
				promise_->is_done    = true;
				promise_->is_success = false;
				promise_->inherit_super.is_done    = true;
				promise_->inherit_super.is_success = false;
				if(promise_->listener != NULL) {
					promise_->listener((ttLibC_TettyPromise *)promise_);
				}
			}
			else {
				promise_->is_done    = true;
				promise_->is_success = false;
				promise_->inherit_super.is_done    = true;
				promise_->inherit_super.is_success = false;
			}
			break;
		}
	}
}
//void ttLibC_TettyPromise_await();

/**
 * event listener for promise/future done.
 * @param promise  target promise/future
 * @param listener listener
 */
void ttLibC_TettyPromise_addEventListener(
		ttLibC_TettyPromise *promise,
		ttLibC_TettyPromiseListener listener) {
	ttLibC_TettyPromise_ *promise_ = (ttLibC_TettyPromise_ *)promise;
	promise_->listener = listener;
}

/*
 * notify success to promise.
 * @param promise target promise
 * @param data    sub information.
 */
void ttLibC_TettyPromise_setSuccess(
		ttLibC_TettyPromise *promise,
		void *data) {
	ttLibC_TettyPromise_ *promise_ = (ttLibC_TettyPromise_ *)promise;
	if(promise_->promise_type != PromiseType_Promise) {
		ERR_PRINT("set function works on promise only.");
		return;
	}
	promise_->is_done = true;
	promise_->is_success = true;
	promise_->return_val = data;
	promise_->inherit_super.is_done = true;
	promise_->inherit_super.is_success = true;
	promise_->inherit_super.return_val = data;
	if(promise_->listener != NULL) {
		promise_->listener((ttLibC_TettyPromise *)promise_);
	}
}

/*
 * notify failed to promise.
 * @param promise target promise
 * @param data    sub information.
 */
void ttLibC_TettyPromise_setFailure(ttLibC_TettyPromise *promise, void *data) {
	ttLibC_TettyPromise_ *promise_ = (ttLibC_TettyPromise_ *)promise;
	if(promise_->promise_type != PromiseType_Promise) {
		ERR_PRINT("set function works on promise only.");
		return;
	}
	promise_->is_done = true;
	promise_->is_success = false;
	promise_->return_val = data;
	promise_->inherit_super.is_done = true;
	promise_->inherit_super.is_success = false;
	promise_->inherit_super.return_val = data;
	if(promise_->listener != NULL) {
		promise_->listener((ttLibC_TettyPromise *)promise_);
	}
}

/*
 * close generated promise
 * @param promise target promise
 */
void ttLibC_TettyPromise_close(ttLibC_TettyPromise **promise) {
	ttLibC_TettyPromise_ *target = (ttLibC_TettyPromise_ *)*promise;
	if(target == NULL) {
		return;
	}
	if(target->promise_type == PromiseType_Future) {
		ERR_PRINT("future is not allow to close by user.");
		return;
	}
	ttLibC_free(target);
	*promise = NULL;
}
