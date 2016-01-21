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

#include <sys/time.h>

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
	promise->ptr          = NULL;
	promise->inherit_super.bootstrap  = bootstrap;
	promise->inherit_super.is_done    = false;
	promise->inherit_super.is_success = false;
	return (ttLibC_TettyPromise *)promise;
}

/**
 * await until promise/future done
 * @param promise target promise/future
 */
void ttLibC_TettyPromise_await(ttLibC_TettyPromise *promise) {
	ttLibC_TettyPromise_* promise_ = (ttLibC_TettyPromise_ *)promise;
	if(promise == NULL) {
		return;
	}
	// wait until done...
	while(!promise_->is_done) {
		ttLibC_TettyBootstrap_update(promise_->bootstrap, 100000);
		if(promise_->bootstrap->error_number != 0) {
			// if error occured, update promise as error.
			if(!promise_->is_done) {
				promise_->is_done    = true;
				promise_->is_success = false;
				promise_->inherit_super.is_done    = true;
				promise_->inherit_super.is_success = false;
				if(promise_->listener != NULL) {
					promise_->listener(promise_->ptr, (ttLibC_TettyPromise *)promise_);
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

/**
 * await util promise/future done or timeout.
 * @param promise         target promise/future.
 * @param timeout_milisec timeout time length in mili sec.
 */
void ttLibC_TettyPromise_awaitFor(ttLibC_TettyPromise *promise, uint32_t timeout_milisec) {
	ttLibC_TettyPromise_* promise_ = (ttLibC_TettyPromise_ *)promise;
	if(promise == NULL) {
		return;
	}
	// get start time.
	struct timeval start_time, end_time;
	gettimeofday(&start_time, NULL);

	while(!promise_->is_done) {
		ttLibC_TettyBootstrap_update(promise_->bootstrap, 100000);
		// get current time.
		gettimeofday(&end_time, NULL);
		time_t diffsec = difftime(end_time.tv_sec, start_time.tv_sec);
		suseconds_t diffsub = end_time.tv_usec - start_time.tv_usec;
		double milisec = diffsec * 1e3 + diffsub * 1e-3;
		if(milisec > timeout_milisec) {
			// timeout done.
			break;
		}
		if(promise_->bootstrap->error_number != 0) {
			// if error occured, update promise as error.
			if(!promise_->is_done) {
				promise_->is_done    = true;
				promise_->is_success = false;
				promise_->inherit_super.is_done    = true;
				promise_->inherit_super.is_success = false;
				if(promise_->listener != NULL) {
					promise_->listener(promise_->ptr, (ttLibC_TettyPromise *)promise_);
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

/**
 * event listener for promise/future done.
 * @param promise  target promise/future
 * @param listener listener
 */
void ttLibC_TettyPromise_addEventListener(
		ttLibC_TettyPromise *promise,
		ttLibC_TettyPromiseListener listener,
		void *ptr) {
	if(promise == NULL) {
		return;
	}
	ttLibC_TettyPromise_ *promise_ = (ttLibC_TettyPromise_ *)promise;
	promise_->listener = listener;
	promise_->ptr = ptr;
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
	if(promise == NULL) {
		return;
	}
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
		promise_->listener(promise_->ptr, (ttLibC_TettyPromise *)promise_);
	}
}

/*
 * notify failed to promise.
 * @param promise target promise
 * @param data    sub information.
 */
void ttLibC_TettyPromise_setFailure(ttLibC_TettyPromise *promise, void *data) {
	ttLibC_TettyPromise_ *promise_ = (ttLibC_TettyPromise_ *)promise;
	if(promise == NULL) {
		return;
	}
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
		promise_->listener(promise_->ptr, (ttLibC_TettyPromise *)promise_);
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
