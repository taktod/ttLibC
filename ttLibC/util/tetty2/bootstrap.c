/*
 * bootstrap.c
 *
 *  Created on: 2017/07/21
 *      Author: taktod
 */

#include "bootstrap.h"
#include "promise.h"
#include "../../ttLibC_predef.h"
#include "../../allocator.h"
#include "../../_log.h"
#include <string.h>


ttLibC_Tetty2Bootstrap TT_VISIBILITY_HIDDEN *ttLibC_Tetty2Bootstrap_make(size_t bootstrap_size) {
	ttLibC_Tetty2Bootstrap_ *bootstrap = ttLibC_malloc(bootstrap_size);
	if(bootstrap == NULL) {
		return NULL;
	}
	bootstrap->inherit_super.error_number = 0;
	bootstrap->pipeline = ttLibC_StlList_make();
	bootstrap->write_event = NULL;
	bootstrap->close_event = NULL;
	bootstrap->flush_event = NULL;
	bootstrap->tetty_info.bootstrap_ptr = NULL;
	bootstrap->tetty_info.ptr = NULL;
	if(bootstrap->pipeline == NULL) {
		ttLibC_free(bootstrap);
		return NULL;
	}
	return (ttLibC_Tetty2Bootstrap *)bootstrap;
}

void TT_VISIBILITY_DEFAULT ttLibC_Tetty2Bootstrap_close(ttLibC_Tetty2Bootstrap **bootstrap) {
	ttLibC_Tetty2Bootstrap_ *target = (ttLibC_Tetty2Bootstrap_ *)*bootstrap;
	if(target == NULL) {
		return;
	}
	// call extra close event for each bootstrap.
	if(target->close_event != NULL) {
		target->close_event((ttLibC_Tetty2Bootstrap *)target, NULL);
	}
	ttLibC_StlList_close(&target->pipeline);
	ttLibC_free(target);
	*bootstrap = NULL;
}

void TT_VISIBILITY_DEFAULT ttLibC_Tetty2Bootstrap_pipeline_addLast(
		ttLibC_Tetty2Bootstrap *bootstrap,
		void *channel_handler) {
	ttLibC_Tetty2Bootstrap_ *bootstrap_ = (ttLibC_Tetty2Bootstrap_ *)bootstrap;
	if(bootstrap_ == NULL) {
		return;
	}
	ttLibC_StlList_addLast(bootstrap_->pipeline, channel_handler);
}

void TT_VISIBILITY_DEFAULT ttLibC_Tetty2Bootstrap_pipeline_remove(
		ttLibC_Tetty2Bootstrap *bootstrap,
		void *channel_handler) {
	ttLibC_Tetty2Bootstrap_ *bootstrap_ = (ttLibC_Tetty2Bootstrap_ *)bootstrap;
	if(bootstrap_ == NULL) {
		return;
	}
	ttLibC_StlList_remove(bootstrap_->pipeline, channel_handler);
}

tetty2_errornum TT_VISIBILITY_DEFAULT ttLibC_Tetty2Bootstrap_userEventTriggered(
		ttLibC_Tetty2Bootstrap *bootstrap,
		void *data,
		size_t data_size) {
	if(bootstrap == NULL) {
		return 0;
	}
	ttLibC_Tetty2Bootstrap_ *bootstrap_ = (ttLibC_Tetty2Bootstrap_ *)bootstrap;
	return ttLibC_Tetty2Context_userEventTriggered_(bootstrap, &bootstrap_->tetty_info, data, data_size);
}

tetty2_errornum TT_VISIBILITY_DEFAULT ttLibC_Tetty2Bootstrap_write(
		ttLibC_Tetty2Bootstrap *bootstrap,
		void *data,
		size_t data_size) {
	if(bootstrap == NULL) {
		return 0;
	}
	ttLibC_Tetty2Bootstrap_ *bootstrap_ = (ttLibC_Tetty2Bootstrap_ *)bootstrap;
	return ttLibC_Tetty2Context_write_(bootstrap, &bootstrap_->tetty_info, data, data_size);
}

tetty2_errornum TT_VISIBILITY_DEFAULT ttLibC_Tetty2Bootstrap_flush(ttLibC_Tetty2Bootstrap *bootstrap) {
	if(bootstrap == NULL) {
		return 0;
	}
	ttLibC_Tetty2Bootstrap_ *bootstrap_ = (ttLibC_Tetty2Bootstrap_ *)bootstrap;
	return ttLibC_Tetty2Context_flush_(bootstrap, &bootstrap_->tetty_info);
}

tetty2_errornum TT_VISIBILITY_DEFAULT ttLibC_Tetty2Bootstrap_writeAndFlush(
		ttLibC_Tetty2Bootstrap *bootstrap,
		void *data,
		size_t data_size) {
	tetty2_errornum error_num = ttLibC_Tetty2Bootstrap_write(
			bootstrap,
			data,
			data_size);
	if(error_num != 0) {
		return error_num;
	}
	return ttLibC_Tetty2Bootstrap_flush(bootstrap);
}

ttLibC_Tetty2Promise TT_VISIBILITY_DEFAULT *ttLibC_Tetty2Bootstrap_makePromise(ttLibC_Tetty2Bootstrap *bootstrap) {
	if(bootstrap->error_number != 0) {
		return NULL;
	}
	return ttLibC_Tetty2Promise_make_(bootstrap);
}
