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

/**
 * def for return val.
 * TODO make enum for error returns.
 */
typedef int32_t tetty_errornum;

/**
 * type of tetty channel
 */
typedef enum {
	ChannelType_Tcp,
	ChannelType_Udp
} ttLibC_Tetty_ChannelType;

/**
 * socket option for tetty.
 */
typedef enum {
	Option_SO_KEEPALIVE,
	Option_SO_REUSEADDR,
	Option_TCP_NODELAY
} ttLibC_Tetty_Option;

/**
 * definition of bootstrap.
 */
typedef struct {
	/** ref of error_flag */
	tetty_errornum error_flag;
} ttLibC_Net_TettyBootstrap;

typedef ttLibC_Net_TettyBootstrap ttLibC_TettyBootstrap;

/**
 * definition of context.
 */
typedef struct {
	/** ref for bootstrap */
	ttLibC_Net_TettyBootstrap *bootstrap;
	/** ref for working channel handler. */
	void *channel_handler;
} ttLibC_Net_TettyContext;

typedef ttLibC_Net_TettyContext ttLibC_TettyContext;

/**
 * function definition for channelHandler
 * @param ctx working context.
 * @return errornum 0:ok others:error number.
 */
typedef tetty_errornum (* ttLibC_Tetty_ContextFunc)  (ttLibC_TettyContext *ctx);

/**
 * function definition for channelHandler
 * @param ctx       working context.
 * @param data      data object.
 * @param data_size data object size.
 * @return errornum 0:ok others:error number.
 */
typedef tetty_errornum (* ttLibC_Tetty_DataFunc)     (ttLibC_TettyContext *ctx, void *data, size_t data_size);

/**
 * function definition for channelHandler
 * @param ctx      working context.
 * @param error_no error number for the exception.
 */
typedef void    (* ttLibC_Tetty_ExceptionFunc)(ttLibC_TettyContext *ctx, int32_t error_no);

/**
 * definition of channel_handler.
 */
typedef struct {
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

typedef ttLibC_Net_TettyChannelHandler ttLibC_TettyChannelHandler;

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
 * do sync task.
 * @param bootstrap bootstrap object.
 * @return true:new client_connection is established false:usual work.
 */
bool ttLibC_TettyBootstrap_sync(ttLibC_TettyBootstrap *bootstrap);

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

ttLibC_TettyChannelHandler *A_make();
void A_close(ttLibC_TettyChannelHandler **handler);
ttLibC_TettyChannelHandler *B_make();
void B_close(ttLibC_TettyChannelHandler **handler);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_TETTY_H_ */
