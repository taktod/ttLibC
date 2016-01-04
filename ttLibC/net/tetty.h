/**
 * @file   tetty.h
 * @brief  network framework like netty.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2015/11/15
 */

#ifndef TTLIBC_NET_TETTY_H_
#define TTLIBC_NET_TETTY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "tcp.h"

/**
 * def for return val.
 * TODO make enum for error returns.
 */
typedef int32_t tetty_errornum;

/**
 * type of tetty channel
 */
typedef enum ttLibC_Tetty_ChannelType {
	ChannelType_Tcp,
	ChannelType_Udp
} ttLibC_Tetty_ChannelType;

/**
 * socket option for tetty.
 */
typedef enum ttLibC_Tetty_Option {
	Option_SO_KEEPALIVE,
	Option_SO_REUSEADDR,
	Option_TCP_NODELAY
} ttLibC_Tetty_Option;

/**
 * definition of bootstrap.
 */
typedef struct ttLibC_Net_TettyBootstrap {
	/** ref of error_flag */
	tetty_errornum error_flag;
	/** type of channel */
	ttLibC_Tetty_ChannelType channel_type;
	/** ref of server_info */
	ttLibC_TcpServerInfo *server_info;
} ttLibC_Net_TettyBootstrap;

typedef ttLibC_Net_TettyBootstrap ttLibC_TettyBootstrap;

struct ttLibC_Net_TettyChannelHandler;

typedef struct ttLibC_Net_TettyChannelHandler ttLibC_TettyChannelHandler;

/**
 * definition of context.
 */
typedef struct ttLibC_Net_TettyContext {
	/** ref for bootstrap */
	ttLibC_Net_TettyBootstrap *bootstrap;
	/** ref for working channel handler. */
	ttLibC_TettyChannelHandler *channel_handler;
	/** ref of target client_info */
	ttLibC_TcpClientInfo *client_info;
} ttLibC_Net_TettyContext;

typedef ttLibC_Net_TettyContext ttLibC_TettyContext;

typedef struct ttLibC_Net_TettyPromise {
	ttLibC_TettyBootstrap *bootstrap;
	bool is_done;
	bool is_success;
	void *return_val;
} ttLibC_Net_TettyPromise;

// future for native work.
typedef ttLibC_Net_TettyPromise ttLibC_TettyFuture;
// promise for user def work.
typedef ttLibC_Net_TettyPromise ttLibC_TettyPromise;

/**
 * function definition for channelHandler
 * @param ctx working context.
 * @return errornum 0:ok others:error number.
 */
typedef tetty_errornum (* ttLibC_Tetty_ContextFunc)(ttLibC_TettyContext *ctx);

/**
 * function definition for channelHandler
 * @param ctx       working context.
 * @param data      data object.
 * @param data_size data object size.
 * @return errornum 0:ok others:error number.
 */
typedef tetty_errornum (* ttLibC_Tetty_DataFunc)(ttLibC_TettyContext *ctx, void *data, size_t data_size);

/**
 * function definition for channelHandler
 * @param ctx      working context.
 * @param error_no error number for the exception.
 */
typedef void (* ttLibC_Tetty_ExceptionFunc)(ttLibC_TettyContext *ctx, int32_t error_no);

/**
 * definition of channel_handler.
 */
typedef struct ttLibC_Net_TettyChannelHandler {
	/** event for channel active */
	ttLibC_Tetty_ContextFunc channelActive;
	/** event for channel inactive */
	ttLibC_Tetty_ContextFunc channelInactive;
	/** event for read socket data */
	ttLibC_Tetty_DataFunc    channelRead;
	/** event for read socket data complete.(not supported.) */
//	ttLibC_Tetty_ContextFunc channelReadComplete;

	/** event for bind server or udp. */
	ttLibC_Tetty_ContextFunc bind;
	/** event for connect for tcp client. */
	ttLibC_Tetty_ContextFunc connect;
	/** event for disconnect for tcp client.  */
	ttLibC_Tetty_ContextFunc disconnect;
	/** event for close socket. */
	ttLibC_Tetty_ContextFunc close;
	/** event for write data to socket. */
	ttLibC_Tetty_DataFunc    write;
	/** event for write data flush.(not supported.) */
//	ttLibC_Tetty_ContextFunc flush;

	/** event for exception.(just ignore now.) */
	ttLibC_Tetty_ExceptionFunc exceptionCaught;
} ttLibC_Net_TettyChannelHandler;

/**
 * make bootstrap object.
 * @return bootstrap object.
 */
ttLibC_TettyBootstrap *ttLibC_TettyBootstrap_make();

/**
 * set thread group.(not support.)
 */
//bool ttLibC_TettyBootstrap_group();

/**
 * set channel type
 * @param bootstrap    bootstrap object.
 * @param channel_type channelType_Tcp or channelType_Udp(udp is not supported yet.)
 * @return true:success false:error
 */
bool ttLibC_TettyBootstrap_channel(
		ttLibC_TettyBootstrap *bootstrap,
		ttLibC_Tetty_ChannelType channel_type);

/**
 * set channel option
 * @param bootstrap bootstrap object.
 * @param option    target option type.
 * @return true:success false:error
 */
bool ttLibC_TettyBootstrap_option(
		ttLibC_TettyBootstrap *bootstrap,
		ttLibC_Tetty_Option option);

/**
 * bind.
 * @param bootstrap bootstrap object.
 * @param port      port number for bind.
 * @return true:success false:error
 */
bool ttLibC_TettyBootstrap_bind(
		ttLibC_TettyBootstrap *bootstrap,
		int port);

/**
 * connect.
 * @param bootstrap bootstrap object
 * @param host      target host to connect
 * @param port      target port to connect
 * @return true:success false:error
 */
bool ttLibC_TettyBootstrap_connect(
		ttLibC_TettyBootstrap *bootstrap,
		const char *host,
		int port);

/**
 * do update task.
 * @param bootstrap bootstrap object.
 * @param wait_interval in micro sec.
 * @return true:new client_connection is established false:usual work.
 */
bool ttLibC_TettyBootstrap_update(
		ttLibC_TettyBootstrap *bootstrap,
		uint32_t wait_interval);

/**
 * close server socket.
 * @param bootstrap bootstrap object.
 */
void ttLibC_TettyBootstrap_closeServer(ttLibC_TettyBootstrap *bootstrap);

/**
 * close all client socket.
 * @param bootstrap bootstrap object.
 */
void ttLibC_TettyBootstrap_closeClients(ttLibC_TettyBootstrap *bootstrap);

/**
 * close all
 * @param bootstrap bootstrap object.
 */
void ttLibC_TettyBootstrap_close(ttLibC_TettyBootstrap **bootstrap);

/**
 * add channel handler.
 * @param bootstrap       bootstrap object.
 * @param channel_handler use def channel_handler object.
 */
void ttLibC_TettyBootstrap_pipeline_addLast(
		ttLibC_TettyBootstrap *bootstrap,
		ttLibC_TettyChannelHandler *channel_handler);

/**
 * remove channel handler.
 * @param bootstrap       bootstrap object.
 * @param channel_handler use def channel_handler object.
 */
void ttLibC_TettyBootstrap_pipeline_remove(
		ttLibC_TettyBootstrap *bootstrap,
		ttLibC_TettyChannelHandler *channel_handler);

/**
 * write data for all connect socket.(share one task for all connection.)
 * @param bootstrap
 * @param data
 * @param data_size
 * @return errornum
 */
tetty_errornum ttLibC_TettyBootstrap_channels_write(
		ttLibC_TettyBootstrap *bootstrap,
		void *data,
		size_t data_size);

/*
ttLibC_TettyFuture *ttLibC_TettyBootstrap_channels_writeFuture(
		ttLibC_TettyBootstrap *bootstrap,
		void *data,
		size_t data_size);
*/

/**
 * write data for all connect socket.(do all task for each connection.)
 * @param bootstrap
 * @param data
 * @param data_size
 * @return errornum
 */
tetty_errornum ttLibC_TettyBootstrap_channelEach_write(
		ttLibC_TettyBootstrap *bootstrap,
		void *data,
		size_t data_size);

/*
ttLibC_TettyFuture *ttLibC_TettyBootstrap_channelEach_writeFuture(
		ttLibC_TettyBootstrap *bootstrap,
		void *data,
		size_t data_size);
*/

/**
 * make promise
 * @param bootstrap
 */
ttLibC_TettyPromise *ttLibC_TettyBootstrap_makePromise(ttLibC_TettyBootstrap *bootstrap);

/**
 * get the close future.
 * @param bootstrap
 * @return future
 */
ttLibC_TettyFuture *ttLibC_TettyBootstrap_closeFuture(ttLibC_TettyBootstrap *bootstrap);

/**
 * write data for one connection.
 * @param ctx       target context
 * @param data      data object.
 * @param data_size data object size.
 * @return error_num
 */
tetty_errornum ttLibC_TettyContext_channel_write(
		ttLibC_TettyContext *ctx,
		void *data,
		size_t data_size);

/*
ttLibC_TettyFuture *ttLibC_TettyContext_channel_writeFuture(
		ttLibC_TettyContext *ctx,
		void *data,
		size_t data_size);
*/

/**
 * close data connection for target context.
 * @param ctx target context
 * @return error_num
 */
tetty_errornum ttLibC_TettyContext_close(ttLibC_TettyContext *ctx);

/**
 * call channel active to next channel_handler.
 * @param ctx
 * @return errornum
 */
tetty_errornum ttLibC_TettyContext_super_channelActive(ttLibC_TettyContext *ctx);

/**
 * call channel inactive to next channel_handler.
 * @param ctx
 * @return errornum
 */
tetty_errornum ttLibC_TettyContext_super_channelInactive(ttLibC_TettyContext *ctx);

/**
 * call channel read to next channel_handler.
 * @param ctx
 * @param data
 * @param data_size
 * @return errornum
 */
tetty_errornum ttLibC_TettyContext_super_channelRead(
		ttLibC_TettyContext *ctx,
		void *data,
		size_t data_size);

/**
 * call channel read complete to next channel_handler.
 * @param ctx
 * @return errornum
 */
//tetty_errornum ttLibC_TettyContext_super_channelReadComplete(ttLibC_TettyContext *ctx);

/**
 * call bind to next channel_handler.
 * @param ctx
 * @return errornum
 */
tetty_errornum ttLibC_TettyContext_super_bind(ttLibC_TettyContext *ctx);

/**
 * call connect to next channel_handler.
 * @param ctx
 * @return errornum
 */
tetty_errornum ttLibC_TettyContext_super_connect(ttLibC_TettyContext *ctx);

/**
 * call disconnect to next channel_handler.
 * @param ctx
 * @return errornum
 */
tetty_errornum ttLibC_TettyContext_super_disconnect(ttLibC_TettyContext *ctx);

/**
 * call close to next channel_handler.
 * @param ctx
 * @return errornum
 */
tetty_errornum ttLibC_TettyContext_super_close(ttLibC_TettyContext *ctx);

/**
 * call write to next channel_handler.
 * @param ctx
 * @param data
 * @param data_size
 * @return errornum
 */
tetty_errornum ttLibC_TettyContext_super_write(
		ttLibC_TettyContext *ctx,
		void *data,
		size_t data_size);

/**
 * call flush to next channel_handler.
 * @param ctx
 * @return errornum
 */
//tetty_errornum ttLibC_TettyContext_super_flush(ttLibC_TettyContext *ctx);

/**
 * call exceptionCaught to next channel_handler.
 * @param ctx
 * @param error_no
 */
void ttLibC_TettyContext_super_exceptionCaught(
		ttLibC_TettyContext *ctx,
		tetty_errornum error_no);

/**
 * call write to next channel_hander as broadcast multi call.
 * @param ctx
 * @param data
 * @param data_size
 * @return error_no
 */
tetty_errornum ttLibC_TettyContext_super_writeEach(
		ttLibC_TettyContext *ctx,
		void *data,
		size_t data_size);

/**
 * callback function definition of promise event listener.
 * @param ptr     user def pointer.
 * @param promise target promise/future.
 */
typedef void (* ttLibC_TettyPromiseListener)(ttLibC_TettyPromise *promise);

/**
 * sync until promise/future done.
 * @param promise target promise
 */
//void ttLibC_TettyPromise_sync(ttLibC_TettyPromise *promise); // rethrow exception.

/**
 * await until promise/future done
 * @param promise target promise/future
 */
void ttLibC_TettyPromise_await(ttLibC_TettyPromise *promise); // not throw exception.

/**
 * await util promise/future done or timeout.
 * @param promise         target promise/future.
 * @param timeout_milisec timeout time length in mili sec.
 */
void ttLibC_TettyPromise_awaitFor(ttLibC_TettyPromise *promise, uint32_t timeout_milisec);

/**
 * event listener for promise/future done.
 * @param promise  target promise/future
 * @param listener listener
 */
void ttLibC_TettyPromise_addEventListener(
		ttLibC_TettyPromise *promise,
		ttLibC_TettyPromiseListener listener);

/**
 * notify success to promise.
 * @param promise target promise
 * @param data    sub information.
 * TODO do I need to add is_non_copy?
 */
void ttLibC_TettyPromise_setSuccess(ttLibC_TettyPromise *promise, void *data);

/**
 * notify failed to promise.
 * @param promise target promise
 * @param data    sub information.
 * TODO do I need to add is_non_copy?
 */
void ttLibC_TettyPromise_setFailure(ttLibC_TettyPromise *promise, void *data);

/**
 * close generated promise
 * @param promise target promise
 */
void ttLibC_TettyPromise_close(ttLibC_TettyPromise **promise);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_TETTY_H_ */
