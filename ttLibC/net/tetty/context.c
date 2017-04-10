/*
 * @file   context.c
 * @brief  context for tetty.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2015/12/12
 */
#ifdef __ENABLE_SOCKET__

#include "context.h"
#include "bootstrap.h"
#include "../udp.h"
#include "../tcp.h"
#include "../../allocator.h"
#include "../../log.h"
#include "../../util/hexUtil.h"
#include "../../util/dynamicBufferUtil.h"
#include <string.h>
#include <unistd.h>

/*
 * update clientContextInformation.
 * @param ctx             target ttLibC_TettyContext
 * @param bootstrap       ttLibC_TettyBootstrap object.
 * @param channel_handler ttLibC_TettyChannelHandler object.
 * @param client_info     ttLibC_TcpClientInfo object.
 */
static void TettyContext_updateContextInfo(
		ttLibC_TettyContext_ *ctx,
		ttLibC_TettyBootstrap *bootstrap,
		ttLibC_TettyChannelHandler *channel_handler,
		ttLibC_SocketInfo *socket_info) {
	ctx->bootstrap = bootstrap;
	ctx->inherit_super.bootstrap = bootstrap;
	ctx->channel_handler = channel_handler;
	ctx->inherit_super.channel_handler = channel_handler;
	ctx->socket_info = socket_info;
	ctx->inherit_super.socket_info = socket_info;
}

/*
 * write data for one connection.
 * @param ctx       target context
 * @param data      data object.
 * @param data_size data object size.
 * @return error_num
 */
tetty_errornum ttLibC_TettyContext_channel_write(
		ttLibC_TettyContext *ctx,
		void *data,
		size_t data_size) {
	ttLibC_TettyContext_ ctx_;
	ttLibC_TettyContext_ *org_ctx_ = (ttLibC_TettyContext_ *)ctx;
	ctx_.bootstrap = org_ctx_->bootstrap;
	ctx_.channel_handler = NULL;
	ctx_.error_no = 0;
	ctx_.socket_info = org_ctx_->socket_info;
	ctx_.inherit_super.bootstrap = org_ctx_->inherit_super.bootstrap;
	ctx_.inherit_super.channel_handler = NULL;
	ctx_.inherit_super.socket_info = org_ctx_->inherit_super.socket_info;
	ttLibC_TettyContext_super_write((ttLibC_TettyContext *)&ctx_, data, data_size);
	return ctx_.error_no;
}

tetty_errornum ttLibC_TettyContext_channel_flush(ttLibC_TettyContext *ctx) {
	/*
	 * ttLibC_TettyContext_super_writeEach can expand the write target from ctx to bootstrap.
	 * in this case, target for flush must be all connection of bootstrap.
	 * and, flush for one context is not enough.
	 */
	tetty_errornum error_num = ttLibC_TettyBootstrap_channels_flush(ctx->bootstrap);
	return error_num;
}

tetty_errornum ttLibC_TettyContext_channel_writeAndFlush(
		ttLibC_TettyContext *ctx,
		void *data,
		size_t data_size) {
	tetty_errornum error_num = ttLibC_TettyContext_channel_write(ctx, data, data_size);
	if(error_num != 0) {
		return error_num;
	}
	error_num = ttLibC_TettyContext_channel_flush(ctx);
	return error_num;
}

/*
 * close data connection for target context.
 * @param ctx target context
 * @return error_num
 */
tetty_errornum ttLibC_TettyContext_close(
		ttLibC_TettyContext *ctx) {
	ttLibC_TettyContext_ *ctx_ = (ttLibC_TettyContext_ *)ctx;
	if(ctx_->bootstrap->channel_type == ChannelType_Udp) {
		ttLibC_TettyContext_ ctx;
		ctx.bootstrap = ctx_->bootstrap;
		ctx.channel_handler = NULL;
		ctx.error_no = 0;
		ctx.socket_info = ctx_->socket_info;
		ctx.inherit_super.bootstrap = ctx_->inherit_super.bootstrap;
		ctx.inherit_super.channel_handler = NULL;
		ctx.inherit_super.socket_info = ctx_->inherit_super.socket_info;
		ttLibC_TettyContext_super_close((ttLibC_TettyContext *)&ctx);
	}
	else {
		if(ctx_->socket_info == NULL) {
			ERR_PRINT("target context is not invidual channel.");
			return 0;
		}
		// do close.
		ttLibC_TettyBootstrap_closeClient_(ctx_->bootstrap, ctx_->socket_info);
		// remove from stlList. (only for tcp.)
		ttLibC_TettyBootstrap_ *bootstrap = (ttLibC_TettyBootstrap_ *)ctx_->bootstrap;
		ttLibC_StlList_remove(bootstrap->tcp_client_info_list, ctx_->socket_info);
	}
	return 0;
}

/**
 * find next target channel_handler and fire event.
 * @param ptr  tettyContext
 * @param item channel_handler
 * @return true:continue false:stop
 */
static bool TettyContext_callNextForEach(void *ptr, void *item) {
	ttLibC_TettyContext_ *ctx = ptr;
	ttLibC_TettyChannelHandler *channel_handler = item;
	if(ctx->channel_handler == NULL) {
		// in the case of NULL, fire the first found handler.
		switch(ctx->command) {
		case Command_channelActive:
			if(channel_handler->channelActive == NULL) {
				return true;
			}
			ctx->channel_handler = channel_handler;
			ctx->inherit_super.channel_handler = channel_handler;
			ctx->error_no = channel_handler->channelActive((ttLibC_TettyContext *)ctx);
			return false;
		case Command_channelInactive:
			if(channel_handler->channelInactive == NULL) {
				return true;
			}
			ctx->channel_handler = channel_handler;
			ctx->inherit_super.channel_handler = channel_handler;
			ctx->error_no = channel_handler->channelInactive((ttLibC_TettyContext *)ctx);
			return false;
		case Command_channelRead:
			if(channel_handler->channelRead == NULL) {
				return true;
			}
			ctx->channel_handler = channel_handler;
			ctx->inherit_super.channel_handler = channel_handler;
			ctx->error_no = channel_handler->channelRead((ttLibC_TettyContext *)ctx, ctx->data, ctx->data_size);
			return false;
		case Command_bind:
			if(channel_handler->bind == NULL) {
				return true;
			}
			ctx->channel_handler = channel_handler;
			ctx->inherit_super.channel_handler = channel_handler;
			ctx->error_no = channel_handler->bind((ttLibC_TettyContext *)ctx);
			return false;
		case Command_connect:
			if(channel_handler->connect == NULL) {
				return true;
			}
			ctx->channel_handler = channel_handler;
			ctx->inherit_super.channel_handler = channel_handler;
			ctx->error_no = channel_handler->connect((ttLibC_TettyContext *)ctx);
			return false;
		case Command_disconnect:
			if(channel_handler->disconnect == NULL) {
				return true;
			}
			ctx->channel_handler = channel_handler;
			ctx->inherit_super.channel_handler = channel_handler;
			ctx->error_no = channel_handler->disconnect((ttLibC_TettyContext *)ctx);
			return false;
		case Command_write:
			if(channel_handler->write == NULL) {
				return true;
			}
			ctx->channel_handler = channel_handler;
			ctx->inherit_super.channel_handler = channel_handler;
			ctx->error_no = channel_handler->write((ttLibC_TettyContext *)ctx, ctx->data, ctx->data_size);
			return false;
		case Command_flush:
			if(channel_handler->flush == NULL) {
				return true;
			}
			ctx->channel_handler = channel_handler;
			ctx->inherit_super.channel_handler = channel_handler;
			ctx->error_no = channel_handler->flush((ttLibC_TettyContext *)ctx);
			return false;
		case Command_close:
			if(channel_handler->close == NULL) {
				return true;
			}
			ctx->channel_handler = channel_handler;
			ctx->inherit_super.channel_handler = channel_handler;
			ctx->error_no = channel_handler->close((ttLibC_TettyContext *)ctx);
			return false;
		case Command_userEventTriggered:
			if(channel_handler->userEventTriggered == NULL) {
				return true;
			}
			ctx->channel_handler = channel_handler;
			ctx->inherit_super.channel_handler = channel_handler;
			ctx->error_no = channel_handler->userEventTriggered((ttLibC_TettyContext *)ctx, ctx->data, ctx->data_size);
			return false;
		default:
			break;
		}
	}
	if(ctx->channel_handler == channel_handler) {
		ctx->channel_handler = NULL; // found put null and fire next handler.
	}
	return true;
}

/*
 * call channel active to next channel_handler.
 * @param ctx
 * @return errornum
 */
tetty_errornum ttLibC_TettyContext_super_channelActive(ttLibC_TettyContext *ctx) {
	ttLibC_TettyContext_ *ctx_ = (ttLibC_TettyContext_ *)ctx;
	ctx_->command = Command_channelActive;
	ttLibC_TettyBootstrap_ *bootstrap = (ttLibC_TettyBootstrap_ *)ctx_->bootstrap;
	if(bootstrap == NULL) {
		LOG_PRINT("failed to ref the bootstrap.");
		return 0;
	}
	ttLibC_TettyChannelHandler *channel_handler = ctx_->channel_handler;
	ttLibC_StlList_forEach(bootstrap->pipeline, TettyContext_callNextForEach, ctx);
	ctx_->channel_handler = channel_handler;
	return ctx_->error_no;
}

/*
 * call channel inactive to next channel_handler.
 * @param ctx
 * @return errornum
 */
tetty_errornum ttLibC_TettyContext_super_channelInactive(ttLibC_TettyContext *ctx) {
	ttLibC_TettyContext_ *ctx_ = (ttLibC_TettyContext_ *)ctx;
	ctx_->command = Command_channelInactive;
	ttLibC_TettyBootstrap_ *bootstrap = (ttLibC_TettyBootstrap_ *)ctx_->bootstrap;
	if(bootstrap == NULL) {
		LOG_PRINT("failed to ref the bootstrap.");
		return 0;
	}
	ttLibC_TettyChannelHandler *channel_handler = ctx_->channel_handler;
	ttLibC_StlList_forEach(bootstrap->pipeline, TettyContext_callNextForEach, ctx);
	ctx_->channel_handler = channel_handler;
	return ctx_->error_no;
}

/*
 * call channel read to next channel_handler.
 * @param ctx
 * @param data
 * @param data_size
 * @return errornum
 */
tetty_errornum ttLibC_TettyContext_super_channelRead(ttLibC_TettyContext *ctx, void *data, size_t data_size) {
	ttLibC_TettyContext_ *ctx_ = (ttLibC_TettyContext_ *)ctx;
	ctx_->command = Command_channelRead;
	ttLibC_TettyBootstrap_ *bootstrap = (ttLibC_TettyBootstrap_ *)ctx_->bootstrap;
	if(bootstrap == NULL) {
		LOG_PRINT("failed to ref the bootstrap.");
		return 0;
	}
	ctx_->data = data;
	ctx_->data_size = data_size;
	ttLibC_TettyChannelHandler *channel_handler = ctx_->channel_handler;
	ttLibC_StlList_forEach(bootstrap->pipeline, TettyContext_callNextForEach, ctx);
	ctx_->channel_handler = channel_handler;
	return ctx_->error_no;
}

/*
 * call bind to next channel_handler.
 * @param ctx
 * @return errornum
 */
tetty_errornum ttLibC_TettyContext_super_bind(ttLibC_TettyContext *ctx) {
	ttLibC_TettyContext_ *ctx_ = (ttLibC_TettyContext_ *)ctx;
	ctx_->command = Command_bind;
	ttLibC_TettyBootstrap_ *bootstrap = (ttLibC_TettyBootstrap_ *)ctx_->bootstrap;
	if(bootstrap == NULL) {
		LOG_PRINT("failed to ref the bootstrap.");
		return 0;
	}
	ttLibC_TettyChannelHandler *channel_handler = ctx_->channel_handler;
	ttLibC_StlList_forEachReverse(bootstrap->pipeline, TettyContext_callNextForEach, ctx);
	ctx_->channel_handler = channel_handler;
	return ctx_->error_no;
}

/*
 * call connect to next channel_handler.
 * @param ctx
 * @return errornum
 */
tetty_errornum ttLibC_TettyContext_super_connect(ttLibC_TettyContext *ctx) {
	ttLibC_TettyContext_ *ctx_ = (ttLibC_TettyContext_ *)ctx;
	ctx_->command = Command_connect;
	ttLibC_TettyBootstrap_ *bootstrap = (ttLibC_TettyBootstrap_ *)ctx_->bootstrap;
	if(bootstrap == NULL) {
		LOG_PRINT("failed to ref the bootstrap.");
		return 0;
	}
	ttLibC_TettyChannelHandler *channel_handler = ctx_->channel_handler;
	ttLibC_StlList_forEachReverse(bootstrap->pipeline, TettyContext_callNextForEach, ctx);
	ctx_->channel_handler = channel_handler;
	return ctx_->error_no;
}

/*
 * call disconnect to next channel_handler.
 * @param ctx
 * @return errornum
 */
tetty_errornum ttLibC_TettyContext_super_disconnect(ttLibC_TettyContext *ctx) {
	ttLibC_TettyContext_ *ctx_ = (ttLibC_TettyContext_ *)ctx;
	ctx_->command = Command_disconnect;
	ttLibC_TettyBootstrap_ *bootstrap = (ttLibC_TettyBootstrap_ *)ctx_->bootstrap;
	if(bootstrap == NULL) {
		LOG_PRINT("failed to ref the bootstrap.");
		return 0;
	}
	ttLibC_TettyChannelHandler *channel_handler = ctx_->channel_handler;
	ttLibC_StlList_forEachReverse(bootstrap->pipeline, TettyContext_callNextForEach, ctx);
	ctx_->channel_handler = channel_handler;
	return ctx_->error_no;
}

/*
 * call close to next channel_handler.
 * @param ctx
 * @return errornum
 */
tetty_errornum ttLibC_TettyContext_super_close(ttLibC_TettyContext *ctx) {
	ttLibC_TettyContext_ *ctx_ = (ttLibC_TettyContext_ *)ctx;
	ctx_->command = Command_close;
	ttLibC_TettyBootstrap_ *bootstrap = (ttLibC_TettyBootstrap_ *)ctx_->bootstrap;
	if(bootstrap == NULL) {
		LOG_PRINT("failed to ref the bootstrap.");
		return 0;
	}
	ttLibC_TettyChannelHandler *channel_handler = ctx_->channel_handler;
	ttLibC_StlList_forEachReverse(bootstrap->pipeline, TettyContext_callNextForEach, ctx);
	ctx_->channel_handler = channel_handler;
	return ctx_->error_no;
}

/**
 * call userEventTriggered to next channel_handler.
 * @param ctx
 * @param data
 * @param data_size
 * @return error_no
 */
tetty_errornum ttLibC_TettyContext_super_userEventTriggered(
		ttLibC_TettyContext *ctx,
		void *data,
		size_t data_size) {
	ttLibC_TettyContext_ *ctx_ = (ttLibC_TettyContext_ *)ctx;
	ctx_->command = Command_userEventTriggered;
	ttLibC_TettyBootstrap_ *bootstrap = (ttLibC_TettyBootstrap_ *)ctx_->bootstrap;
	if(bootstrap == NULL) {
		LOG_PRINT("failed to ref the bootstrap.");
		return 0;
	}
	ctx_->data = data;
	ctx_->data_size = data_size;
	ttLibC_TettyChannelHandler *channel_handler = ctx_->channel_handler;
	ttLibC_StlList_forEach(bootstrap->pipeline, TettyContext_callNextForEach, ctx);
	ctx_->channel_handler = channel_handler;
	return ctx_->error_no;
}

/**
 * put binary data on write cache buffer.
 */
static bool TettyContext_write_contextWriteBuffer(
		ttLibC_TcpClientInfo *client_info,
		void *data,
		size_t data_size) {
	if(client_info->write_buffer == NULL) {
		client_info->write_buffer = ttLibC_DynamicBuffer_make();
	}
	return ttLibC_DynamicBuffer_append(client_info->write_buffer, data, data_size);
}

/*
 * callback for broadcast writing.
 * @param ptr  context
 * @param item client_info
 * @return true:to continue
 */
static bool TettyContext_super_write_callback(void *ptr, void *item) {
	ttLibC_TettyContext_ *ctx_ = (ttLibC_TettyContext_ *)ptr;
	TettyContext_write_contextWriteBuffer((ttLibC_TcpClientInfo *)item, ctx_->data, ctx_->data_size);
	// TODO do something in the case of error.
	return true;
}

/*
 * call write to next channel_handler.
 * @param ctx
 * @param data
 * @param data_size
 * @return errornum
 */
tetty_errornum ttLibC_TettyContext_super_write(ttLibC_TettyContext *ctx, void *data, size_t data_size) {
	ttLibC_TettyContext_ *ctx_ = (ttLibC_TettyContext_ *)ctx;
	ctx_->command = Command_write;
	ttLibC_TettyBootstrap_ *bootstrap = (ttLibC_TettyBootstrap_ *)ctx_->bootstrap;
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
	ttLibC_TettyChannelHandler *channel_handler = ctx_->channel_handler;
	if(ttLibC_StlList_forEachReverse(bootstrap->pipeline, TettyContext_callNextForEach, ctx)) {
		// if finish the iterator, we need to write.
		if(ctx_->data != NULL) {
			switch(bootstrap->channel_type) {
			default:
			case ChannelType_Tcp:
				if(ctx_->socket_info == NULL) {
					// broadcast writing.
					ttLibC_StlList_forEach(bootstrap->tcp_client_info_list, TettyContext_super_write_callback, ctx_);
				}
				else {
					// user must call flush later to send message.
					TettyContext_write_contextWriteBuffer((ttLibC_TcpClientInfo *)ctx_->socket_info, ctx_->data, ctx_->data_size);
				}
				break;
			case ChannelType_Udp:
				{
					ttLibC_UdpSocket_write((ttLibC_UdpSocketInfo *)bootstrap->socket_info, (ttLibC_DatagramPacket *)data);
				}
				break;
			}
			ctx_->data = NULL;
			ctx_->data_size = 0;
		}
	}
	ctx_->channel_handler = channel_handler;
	return ctx_->error_no;
}

/*
 * call flush to next channel_handler.
 * @param ctx
 * @return errornum
 */
tetty_errornum ttLibC_TettyContext_super_flush(ttLibC_TettyContext *ctx) {
	ttLibC_TettyContext_ *ctx_ = (ttLibC_TettyContext_ *)ctx;
	ctx_->command = Command_flush;
	ttLibC_TettyBootstrap_ *bootstrap = (ttLibC_TettyBootstrap_ *)ctx_->bootstrap;
	if(bootstrap == NULL) {
		LOG_PRINT("failed to ref the bootstrap.");
		return 0;
	}
	ttLibC_TettyChannelHandler *channel_handler = ctx_->channel_handler;
	ttLibC_StlList_forEachReverse(bootstrap->pipeline, TettyContext_callNextForEach, ctx);
	ctx_->channel_handler = channel_handler;
	return ctx_->error_no;
}

/*
 * call exceptionCaught to next channel_handler.
 * @param ctx
 * @param error_no
 */
void ttLibC_TettyContext_super_exceptionCaught(ttLibC_TettyContext *ctx, tetty_errornum error_no) {
	(void)ctx;
	(void)error_no;
}

/**
 * callback for ttLibC_TettyContext_super_writeEach
 * @param ptr  context
 * @param item client_info
 * @return true:continue false:stop
 */
static bool TettyContext_super_writeEach_callback(void *ptr, void *item) {
	ttLibC_TettyContext_ *ctx_ = (ttLibC_TettyContext_ *)ptr;
	ttLibC_TcpClientInfo *client_info = (ttLibC_TcpClientInfo *)item;
	// fire write for each client_info
	void *data = ctx_->data;
	size_t data_size = ctx_->data_size;
	ttLibC_TettyContext_ ctx;

	ctx.bootstrap = ctx_->bootstrap;
	ctx.channel_handler = ctx_->channel_handler;
	ctx.socket_info = (ttLibC_SocketInfo *)client_info;
	ctx.inherit_super.bootstrap = ctx_->inherit_super.bootstrap;
	ctx.inherit_super.channel_handler = ctx_->inherit_super.channel_handler;
	ctx.inherit_super.socket_info = (ttLibC_SocketInfo *)client_info;
	ttLibC_TettyContext_super_write((ttLibC_TettyContext *)&ctx, data, data_size);
	return true;
}

/**
 * call write to next channel_hander as broadcast multi call.
 * @param ctx
 * @param data
 * @param data_size
 * @return error_no
 */
tetty_errornum ttLibC_TettyContext_super_writeEach(ttLibC_TettyContext *ctx, void *data, size_t data_size) {
	ttLibC_TettyContext_ *ctx_ = (ttLibC_TettyContext_ *)ctx;
	ttLibC_TettyBootstrap_ *bootstrap = (ttLibC_TettyBootstrap_ *)ctx_->bootstrap;
	if(bootstrap == NULL) {
		LOG_PRINT("failed to ref the bootstrap.");
		return 0;
	}
	ctx_->data = data;
	ctx_->data_size = data_size;
	ttLibC_StlList_forEach(bootstrap->tcp_client_info_list, TettyContext_super_writeEach_callback, ctx_);
	return 0;
}

/*
 * call for channelActive from bootstrap
 * @param bootstrap
 * @param client_info
 * @return
 */
tetty_errornum ttLibC_TettyContext_channelActive_(
		ttLibC_TettyBootstrap *bootstrap,
		ttLibC_SocketInfo *socket_info) {
	ttLibC_TettyContext_ ctx;
	TettyContext_updateContextInfo(
			&ctx,
			bootstrap,
			NULL,
			socket_info);
	ttLibC_TettyContext_super_channelActive((ttLibC_TettyContext *)&ctx);
	return ctx.error_no;
}

/*
 * call for channelInactive from bootstrap
 * @param bootstrap
 * @param client_info
 * @return
 */
tetty_errornum ttLibC_TettyContext_channelInactive_(
		ttLibC_TettyBootstrap *bootstrap,
		ttLibC_SocketInfo *socket_info) {
	ttLibC_TettyContext_ ctx;
	TettyContext_updateContextInfo(
			&ctx,
			bootstrap,
			NULL,
			socket_info);
	ttLibC_TettyContext_super_channelInactive((ttLibC_TettyContext *)&ctx);
	return ctx.error_no;
}

/*
 * call for channelRead from bootstrap
 * @param bootstrap
 * @param client_info
 * @param data
 * @param data_size
 * @return
 */
tetty_errornum ttLibC_TettyContext_channelRead_(
		ttLibC_TettyBootstrap *bootstrap,
		ttLibC_SocketInfo *socket_info,
		void *data,
		size_t data_size) {
	ttLibC_TettyContext_ ctx;
	TettyContext_updateContextInfo(
			&ctx,
			bootstrap,
			NULL,
			socket_info);
	ttLibC_TettyContext_super_channelRead((ttLibC_TettyContext *)&ctx, data, data_size);
	return ctx.error_no;
}

/*
 * call for bind from bootstrap
 * @param bootstrap
 * @return
 */
tetty_errornum ttLibC_TettyContext_bind_(ttLibC_TettyBootstrap *bootstrap) {
	ttLibC_TettyContext_ ctx;
	TettyContext_updateContextInfo(
			&ctx,
			bootstrap,
			NULL,
			NULL);
	ttLibC_TettyContext_super_bind((ttLibC_TettyContext *)&ctx);
	return ctx.error_no;
}

/*
 * call for connect from bootstrap
 * @param bootstrap
 * @param client_info
 * @return
 */
tetty_errornum ttLibC_TettyContext_connect_(
		ttLibC_TettyBootstrap *bootstrap,
		ttLibC_SocketInfo *socket_info) {
	ttLibC_TettyContext_ ctx;
	TettyContext_updateContextInfo(
			&ctx,
			bootstrap,
			NULL,
			socket_info);
	ttLibC_TettyContext_super_connect((ttLibC_TettyContext *)&ctx);
	return ctx.error_no;
}

/*
 * call for disconnect from bootstrap
 * @param bootstrap
 * @param client_info
 * @return
 */
tetty_errornum ttLibC_TettyContext_disconnect_(
		ttLibC_TettyBootstrap *bootstrap,
		ttLibC_SocketInfo *socket_info) {
	ttLibC_TettyContext_ ctx;
	TettyContext_updateContextInfo(
			&ctx,
			bootstrap,
			NULL,
			socket_info);
	ttLibC_TettyContext_super_disconnect((ttLibC_TettyContext *)&ctx);
	return ctx.error_no;
}

/*
 * call for write from bootstrap
 * @param bootstrap
 * @param client_info if client_info is NULL, broadcast else write for one client.
 * @param data
 * @param data_size
 * @return
 */
tetty_errornum ttLibC_TettyContext_channel_write_(
		ttLibC_TettyBootstrap *bootstrap,
		ttLibC_SocketInfo *socket_info,
		void *data,
		size_t data_size) {
	ttLibC_TettyContext_ ctx;
	TettyContext_updateContextInfo(
			&ctx,
			bootstrap,
			NULL,
			socket_info);
	ttLibC_TettyContext_super_write((ttLibC_TettyContext *)&ctx, data, data_size);
	return ctx.error_no;
}

/*
 * call for flush from bootstrap
 * @param bootstrap
 * @param client_info
 * @return
 */
tetty_errornum ttLibC_TettyContext_flush_(
		ttLibC_TettyBootstrap *bootstrap,
		ttLibC_SocketInfo *socket_info) {
	ttLibC_TettyContext_ ctx;
	TettyContext_updateContextInfo(
			&ctx,
			bootstrap,
			NULL,
			socket_info);
	ttLibC_TettyContext_super_flush((ttLibC_TettyContext *)&ctx);
	return ctx.error_no;
}

/*
 * call for close from bootstrap
 * @param bootstrap
 * @param client_info
 * @return
 */
tetty_errornum ttLibC_TettyContext_close_(
		ttLibC_TettyBootstrap *bootstrap,
		ttLibC_SocketInfo *socket_info) {
	ttLibC_TettyContext_ ctx;
	TettyContext_updateContextInfo(
			&ctx,
			bootstrap,
			NULL,
			socket_info);
	ttLibC_TettyContext_super_close((ttLibC_TettyContext *)&ctx);
	return ctx.error_no;
}

/**
 * call for userEventTriggered
 */
tetty_errornum ttLibC_TettyContext_userEventTriggered_(
		ttLibC_TettyBootstrap *bootstrap,
		ttLibC_SocketInfo *socket_info,
		void *data,
		size_t data_size) {
	ttLibC_TettyContext_ ctx;
	TettyContext_updateContextInfo(
			&ctx,
			bootstrap,
			NULL,
			socket_info);
	ttLibC_TettyContext_super_userEventTriggered(
			(ttLibC_TettyContext *)&ctx,
			data,
			data_size);
	return ctx.error_no;
}

#endif
