/*
 * promise.c
 *
 *  Created on: 2017/07/21
 *      Author: taktod
 */

#include "promise.h"
#include "../../allocator.h"
#include "../../_log.h"

ttLibC_Tetty2Promise *ttLibC_Tetty2Promise_make_(ttLibC_Tetty2Bootstrap *bootstrap) {
	ttLibC_Tetty2Promise_ *promise = ttLibC_malloc(sizeof(ttLibC_Tetty2Promise_));
	if(promise == NULL) {
		return NULL;
	}
	promise->bootstrap = bootstrap;
	promise->listener = NULL;
	promise->ptr = NULL;
	promise->inherit_super.bootstrap = bootstrap;
	promise->inherit_super.is_done = false;
	promise->inherit_super.is_success = false;
	promise->inherit_super.return_val = NULL;
	return (ttLibC_Tetty2Promise *)promise;
}

void ttLibC_Tetty2Promise_addEventListener(
		ttLibC_Tetty2Promise *promise,
		ttLibC_Tetty2PromiseListener listener,
		void *ptr) {
	ttLibC_Tetty2Promise_ *promise_ = (ttLibC_Tetty2Promise_ *)promise;
	if(promise_ == NULL) {
		return;
	}
	promise_->listener = listener;
	promise_->ptr = ptr;
}

static void Tetty2Promise_setResult(
		ttLibC_Tetty2Promise *promise,
		void *return_val,
		bool is_success) {
	ttLibC_Tetty2Promise_ *promise_ = (ttLibC_Tetty2Promise_ *)promise;
	if(promise == NULL) {
		return;
	}
	promise_->inherit_super.is_done = true;
	promise_->inherit_super.is_success = is_success;
	promise_->inherit_super.return_val = return_val;
	if(promise_->listener != NULL) {
		promise_->listener(promise_->ptr, (ttLibC_Tetty2Promise *)promise_);
	}
}

void ttLibC_Tetty2Promise_setSuccess(
		ttLibC_Tetty2Promise *promise,
		void *data) {
	Tetty2Promise_setResult(
			promise,
			data,
			true);
}
void ttLibC_Tetty2Promise_setFailure(
		ttLibC_Tetty2Promise *promise,
		void *data) {
	Tetty2Promise_setResult(
			promise,
			data,
			false);
}
void ttLibC_Tetty2Promise_close(ttLibC_Tetty2Promise **promise) {
	ttLibC_Tetty2Promise_ *target = (ttLibC_Tetty2Promise_ *)*promise;
	if(target == NULL) {
		return;
	}
	if(!target->inherit_super.is_done) {
		target->inherit_super.is_done = true;
		target->inherit_super.is_success = false;
		if(target->listener != NULL) {
			target->listener(target->ptr, (ttLibC_Tetty2Promise *)target);
		}
	}
	ttLibC_free(target);
	*promise = NULL;
}
