/*
 * context.c
 *
 *  Created on: 2017/07/21
 *      Author: taktod
 */

#include "context.h"
#include "bootstrap.h"
#include "../../ttLibC_predef.h"
#include "../../allocator.h"
#include "../../_log.h"
#include "../dynamicBufferUtil.h"
#include <stdio.h>
#include <string.h>

static void Tetty2Context_updateContextInfo(
		ttLibC_Tetty2Context_ *ctx,
		ttLibC_Tetty2Bootstrap *bootstrap,
		ttLibC_Tetty2ChannelHandler *channel_handler,
		ttLibC_Tetty2Info *tetty_info) {
	ctx->inherit_super.bootstrap = bootstrap;
	ctx->inherit_super.channel_handler = channel_handler;
	ctx->inherit_super.tetty_info = tetty_info;
}

static void Tetty2Context_copyContextInfo(
		ttLibC_Tetty2Context_ *target_ctx,
		ttLibC_Tetty2Context_ *src_ctx) {
	Tetty2Context_updateContextInfo(
			target_ctx,
			src_ctx->inherit_super.bootstrap,
			NULL,
			src_ctx->inherit_super.tetty_info);
}

tetty2_errornum TT_VISIBILITY_DEFAULT ttLibC_Tetty2Context_channel_write(
		ttLibC_Tetty2Context *ctx,
		void *data,
		size_t data_size) {
	ttLibC_Tetty2Context_ ctx_;
	Tetty2Context_copyContextInfo(&ctx_, (ttLibC_Tetty2Context_ *)ctx);
	ctx_.error_no = 0;
	ttLibC_Tetty2Context_super_write((ttLibC_Tetty2Context *)&ctx_, data, data_size);
	return ctx_.error_no;
}

tetty2_errornum TT_VISIBILITY_DEFAULT ttLibC_Tetty2Context_channel_flush(ttLibC_Tetty2Context *ctx) {
	ttLibC_Tetty2Context_ ctx_;
	Tetty2Context_copyContextInfo(&ctx_, (ttLibC_Tetty2Context_ *)ctx);
	ctx_.error_no = 0;
	ttLibC_Tetty2Context_super_flush((ttLibC_Tetty2Context *)&ctx_);
	return ctx_.error_no;
}

tetty2_errornum TT_VISIBILITY_DEFAULT ttLibC_Tetty2Context_channel_writeAndFlush(
		ttLibC_Tetty2Context *ctx,
		void *data,
		size_t data_size) {
	tetty2_errornum error_num = ttLibC_Tetty2Context_channel_write(ctx, data, data_size);
	if(error_num != 0) {
		return error_num;
	}
	error_num = ttLibC_Tetty2Context_channel_flush(ctx);
	return error_num;
}

tetty2_errornum TT_VISIBILITY_DEFAULT ttLibC_Tetty2Context_close(ttLibC_Tetty2Context *ctx) {
	ttLibC_Tetty2Context_ gen_ctx;
	Tetty2Context_copyContextInfo(&gen_ctx, (ttLibC_Tetty2Context_ *)ctx);
	gen_ctx.error_no = 0;
	return ttLibC_Tetty2Context_super_close((ttLibC_Tetty2Context *)&gen_ctx);
}

static bool Tetty2Context_callNextContext(
		ttLibC_Tetty2Context_ *ctx,
		ttLibC_Tetty2ChannelHandler *channel_handler,
		ttLibC_Tetty2_ContextFunc func,
		bool check_bootstrap) {
	if(check_bootstrap) {
		if(ctx->inherit_super.bootstrap->error_number != 0) {
			return false;
		}
	}
	if(func == NULL) {
		return true;
	}
	ctx->inherit_super.channel_handler = channel_handler;
	ctx->error_no = func((ttLibC_Tetty2Context *)ctx);
	return false;
}

static bool Tetty2Context_callNextData(
		ttLibC_Tetty2Context_ *ctx,
		ttLibC_Tetty2ChannelHandler *channel_handler,
		ttLibC_Tetty2_DataFunc func) {
	if(ctx->inherit_super.bootstrap->error_number != 0) {
		return false;
	}
	if(func == NULL) {
		return true;
	}
	ctx->inherit_super.channel_handler = channel_handler;
	ctx->error_no = func((ttLibC_Tetty2Context *)ctx, ctx->data, ctx->data_size);
	return false;
}

static bool Tetty2Context_callNextForEach(void *ptr, void *item) {
	ttLibC_Tetty2Context_ *ctx = ptr;
	ttLibC_Tetty2ChannelHandler *channel_handler = item;
	if(ctx->inherit_super.channel_handler == NULL) {
		switch(ctx->command) {
		case Tetty2Command_channelActive:
			return Tetty2Context_callNextContext(
					ctx,
					channel_handler,
					channel_handler->channelActive,
					true);
		case Tetty2Command_channelInactive:
			return Tetty2Context_callNextContext(
					ctx,
					channel_handler,
					channel_handler->channelInactive,
					false);
		case Tetty2Command_channelRead:
			return Tetty2Context_callNextData(
					ctx,
					channel_handler,
					channel_handler->channelRead);
		case Tetty2Command_write:
			return Tetty2Context_callNextData(
					ctx,
					channel_handler,
					channel_handler->write);
		case Tetty2Command_flush:
			return Tetty2Context_callNextContext(
					ctx,
					channel_handler,
					channel_handler->flush,
					true);
		case Tetty2Command_close:
			return Tetty2Context_callNextContext(
					ctx,
					channel_handler,
					channel_handler->flush,
					false);
		case Tetty2Command_userEventTriggered:
			return Tetty2Context_callNextData(
					ctx,
					channel_handler,
					channel_handler->userEventTriggered);
		default:
			break;
		}
	}
	if(ctx->inherit_super.channel_handler == channel_handler) {
		ctx->inherit_super.channel_handler = NULL;
	}
	return true;
}

static tetty2_errornum Tetty2Context_callSuper(
		ttLibC_Tetty2Context *ctx,
		ttLibC_Tetty2ContextCommand command) {
	ttLibC_Tetty2Context_ *ctx_ = (ttLibC_Tetty2Context_ *)ctx;
	ctx_->command = command;
	ttLibC_Tetty2Bootstrap_ *bootstrap = (ttLibC_Tetty2Bootstrap_ *)ctx_->inherit_super.bootstrap;
	if(bootstrap == NULL) {
		LOG_PRINT("failed to ref the bootstrap.");
		return 0;
	}
	ttLibC_Tetty2ChannelHandler *channel_handler = ctx_->inherit_super.channel_handler;
	ttLibC_StlList_forEach(bootstrap->pipeline, Tetty2Context_callNextForEach, ctx);
	ctx_->inherit_super.channel_handler = channel_handler;
	return ctx_->error_no;
}

tetty2_errornum TT_VISIBILITY_DEFAULT ttLibC_Tetty2Context_super_channelActive(ttLibC_Tetty2Context *ctx) {
	return Tetty2Context_callSuper(ctx, Tetty2Command_channelActive);
}
tetty2_errornum TT_VISIBILITY_DEFAULT ttLibC_Tetty2Context_super_channelInactive(ttLibC_Tetty2Context *ctx) {
	return Tetty2Context_callSuper(ctx, Tetty2Command_channelInactive);
}
tetty2_errornum TT_VISIBILITY_DEFAULT ttLibC_Tetty2Context_super_channelRead(
		ttLibC_Tetty2Context *ctx,
		void *data,
		size_t data_size) {
	ttLibC_Tetty2Context_ *ctx_ = (ttLibC_Tetty2Context_ *)ctx;
	ctx_->data = data;
	ctx_->data_size = data_size;
	return Tetty2Context_callSuper(ctx, Tetty2Command_channelRead);
}
tetty2_errornum TT_VISIBILITY_DEFAULT ttLibC_Tetty2Context_super_close(ttLibC_Tetty2Context *ctx) {
	return Tetty2Context_callSuper(ctx, Tetty2Command_close);
}

tetty2_errornum TT_VISIBILITY_DEFAULT ttLibC_Tetty2Context_super_write(
		ttLibC_Tetty2Context *ctx,
		void *data,
		size_t data_size) {
	ttLibC_Tetty2Context_ *ctx_ = (ttLibC_Tetty2Context_ *)ctx;
	ctx_->command = Tetty2Command_write;
	ttLibC_Tetty2Bootstrap_ *bootstrap = (ttLibC_Tetty2Bootstrap_ *)ctx_->inherit_super.bootstrap;
	if(bootstrap == NULL) {
		LOG_PRINT("failed to ref the bootstrap.");
		return 0;
	}
	if(bootstrap->inherit_super.error_number != 0) {
		// if errored, do nothing.
		return 0;
	}
	ctx_->data = data;
	ctx_->data_size = data_size;
	ttLibC_Tetty2ChannelHandler *channel_handler = ctx_->inherit_super.channel_handler;
	if(ttLibC_StlList_forEach(bootstrap->pipeline, Tetty2Context_callNextForEach, ctx)) {
		// if finish the iterator, we need to write
		if(ctx_->data != NULL) {
			// data is available
			if(bootstrap->write_event != NULL) {
				bootstrap->write_event(
						(ttLibC_Tetty2Bootstrap *)bootstrap,
						ctx);
			}
			ctx_->data = NULL;
			ctx_->data_size = 0;
		}
	}
	ctx_->inherit_super.channel_handler = channel_handler;
	return ctx_->error_no;
}

tetty2_errornum TT_VISIBILITY_DEFAULT ttLibC_Tetty2Context_super_flush(ttLibC_Tetty2Context *ctx) {
	ttLibC_Tetty2Context_ *ctx_ = (ttLibC_Tetty2Context_ *)ctx;
	ctx_->command = Tetty2Command_flush;
	ttLibC_Tetty2Bootstrap_ *bootstrap = (ttLibC_Tetty2Bootstrap_ *)ctx_->inherit_super.bootstrap;
	if(bootstrap == NULL) {
		LOG_PRINT("failed to ref the bootstrap.");
		return 0;
	}
	ttLibC_Tetty2ChannelHandler *channel_handler = ctx_->inherit_super.channel_handler;
	if(ttLibC_StlList_forEach(bootstrap->pipeline, Tetty2Context_callNextForEach, ctx)) {
		// if finish the iterator, we need to flush
		if(bootstrap->flush_event != NULL) {
			bootstrap->flush_event(
					(ttLibC_Tetty2Bootstrap *)bootstrap,
					ctx);
		}
	}
	ctx_->inherit_super.channel_handler = channel_handler;
	return ctx_->error_no;
}
tetty2_errornum TT_VISIBILITY_DEFAULT ttLibC_Tetty2Context_super_exceptionCaught(
		ttLibC_Tetty2Context *ctx,
		tetty2_errornum error_no) {
	(void)ctx;
	(void)error_no;
	return 0;
}
tetty2_errornum TT_VISIBILITY_DEFAULT ttLibC_Tetty2Context_super_userEventTriggered(
		ttLibC_Tetty2Context *ctx,
		void *data,
		size_t data_size) {
	ttLibC_Tetty2Context_ *ctx_ = (ttLibC_Tetty2Context_ *)ctx;
	ctx_->data = data;
	ctx_->data_size = data_size;
	return Tetty2Context_callSuper(ctx, Tetty2Command_userEventTriggered);
}

tetty2_errornum TT_VISIBILITY_HIDDEN ttLibC_Tetty2Context_channelActive_(
		ttLibC_Tetty2Bootstrap *bootstrap,
		ttLibC_Tetty2Info *tetty_info) {
	ttLibC_Tetty2Context_ ctx;
	ctx.error_no = 0;
	Tetty2Context_updateContextInfo(
			&ctx,
			bootstrap,
			NULL,
			tetty_info);
	ttLibC_Tetty2Context_super_channelActive((ttLibC_Tetty2Context *)&ctx);
	return ctx.error_no;
}

tetty2_errornum TT_VISIBILITY_HIDDEN ttLibC_Tetty2Context_channelInactive_(
		ttLibC_Tetty2Bootstrap *bootstrap,
		ttLibC_Tetty2Info *tetty_info) {
	ttLibC_Tetty2Context_ ctx;
	ctx.error_no = 0;
	Tetty2Context_updateContextInfo(
			&ctx,
			bootstrap,
			NULL,
			tetty_info);
	ttLibC_Tetty2Context_super_channelInactive((ttLibC_Tetty2Context *)&ctx);
	return ctx.error_no;
}

tetty2_errornum TT_VISIBILITY_HIDDEN ttLibC_Tetty2Context_channelRead_(
		ttLibC_Tetty2Bootstrap *bootstrap,
		ttLibC_Tetty2Info *tetty_info,
		void *data,
		size_t data_size) {
	ttLibC_Tetty2Context_ ctx;
	ctx.error_no = 0;
	Tetty2Context_updateContextInfo(
			&ctx,
			bootstrap,
			NULL,
			tetty_info);
	ttLibC_Tetty2Context_super_channelRead((ttLibC_Tetty2Context *)&ctx, data, data_size);
	return ctx.error_no;
}

tetty2_errornum TT_VISIBILITY_HIDDEN ttLibC_Tetty2Context_write_(
		ttLibC_Tetty2Bootstrap *bootstrap,
		ttLibC_Tetty2Info *tetty_info,
		void *data,
		size_t data_size) {
	ttLibC_Tetty2Context_ ctx;
	ctx.error_no = 0;
	Tetty2Context_updateContextInfo(
			&ctx,
			bootstrap,
			NULL,
			tetty_info);
	ttLibC_Tetty2Context_super_write((ttLibC_Tetty2Context *)&ctx, data, data_size);
	return ctx.error_no;
}

tetty2_errornum TT_VISIBILITY_HIDDEN ttLibC_Tetty2Context_flush_(
		ttLibC_Tetty2Bootstrap *bootstrap,
		ttLibC_Tetty2Info *tetty_info) {
	ttLibC_Tetty2Context_ ctx;
	ctx.error_no = 0;
	Tetty2Context_updateContextInfo(
			&ctx,
			bootstrap,
			NULL,
			tetty_info);
	ttLibC_Tetty2Context_super_flush((ttLibC_Tetty2Context *)&ctx);
	return ctx.error_no;
}

tetty2_errornum TT_VISIBILITY_HIDDEN ttLibC_Tetty2Context_close_(
		ttLibC_Tetty2Bootstrap *bootstrap,
		ttLibC_Tetty2Info *tetty_info) {
	ttLibC_Tetty2Context_ ctx;
	ctx.error_no = 0;
	Tetty2Context_updateContextInfo(
			&ctx,
			bootstrap,
			NULL,
			tetty_info);
	ttLibC_Tetty2Context_super_close((ttLibC_Tetty2Context *)&ctx);
	return ctx.error_no;
}

tetty2_errornum TT_VISIBILITY_HIDDEN ttLibC_Tetty2Context_userEventTriggered_(
		ttLibC_Tetty2Bootstrap *bootstrap,
		ttLibC_Tetty2Info *tetty_info,
		void *data,
		size_t data_size) {
	ttLibC_Tetty2Context_ ctx;
	ctx.error_no = 0;
	Tetty2Context_updateContextInfo(
			&ctx,
			bootstrap,
			NULL,
			tetty_info);
	ttLibC_Tetty2Context_super_userEventTriggered((ttLibC_Tetty2Context *)&ctx, data, data_size);
	return ctx.error_no;
}
