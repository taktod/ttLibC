/**
 * @file   context.h
 * @brief  context for tetty.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2015/12/12
 */

#ifndef TTLIBC_NET_TETTY_CONTEXT_H_
#define TTLIBC_NET_TETTY_CONTEXT_H_

#include "../tetty.h"
#include "../server/tcp.h"
#include "../../util/stlListUtil.h"

/**
 * command list.
 */
typedef enum ttLibC_TettyContextCommand {
	Command_channelActive,
	Command_channelInactive,
	Command_channelRead,
//	Command_channelReadComplete,
	Command_bind,
	Command_connect,
	Command_disconnect,
	Command_close,
	Command_write,
//	Command_flush
} ttLibC_TettyContextCommand;

/**
 * detail definition of context.
 */
typedef struct ttLibC_Net_TettyContext_ {
	ttLibC_TettyContext inherit_super;
	/** bootstrap */
	ttLibC_TettyBootstrap *bootstrap;

	/** target client_info */
	ttLibC_TcpClientInfo *client_info;
	/** current channel_handler object */
	ttLibC_TettyChannelHandler *channel_handler;

	/** target command */
	ttLibC_TettyContextCommand command;
	/** data */
	void *data;
	/** data_size */
	size_t data_size;
	/** error number. */
	int32_t error_no;
} ttLibC_Net_TettyContext_;

typedef ttLibC_Net_TettyContext_ ttLibC_TettyContext_;

/**
 * call for channelActive from bootstrap
 * @param bootstrap
 * @param client_info
 * @return
 */
tetty_errornum ttLibC_TettyContext_channelActive_(
		ttLibC_TettyBootstrap *bootstrap,
		ttLibC_TcpClientInfo *client_info);

/**
 * call for channelInactive from bootstrap
 * @param bootstrap
 * @param client_info
 * @return
 */
tetty_errornum ttLibC_TettyContext_channelInactive_(
		ttLibC_TettyBootstrap *bootstrap,
		ttLibC_TcpClientInfo *client_info);

/**
 * call for channelRead from bootstrap
 * @param bootstrap
 * @param client_info
 * @param data
 * @param data_size
 * @return
 */
tetty_errornum ttLibC_TettyContext_channelRead_(
		ttLibC_TettyBootstrap *bootstrap,
		ttLibC_TcpClientInfo *client_info,
		void *data,
		size_t data_size);

/**
 * call for bind from bootstrap
 * @param bootstrap
 * @return
 */
tetty_errornum ttLibC_TettyContext_bind_(ttLibC_TettyBootstrap *bootstrap);

/**
 * call for connect from bootstrap
 * @param bootstrap
 * @param client_info
 * @return
 */
tetty_errornum ttLibC_TettyContext_connect_(
		ttLibC_TettyBootstrap *bootstrap,
		ttLibC_TcpClientInfo *client_info);

/**
 * call for disconnect from bootstrap
 * @param bootstrap
 * @param client_info
 * @return
 */
tetty_errornum ttLibC_TettyContext_disconnect_(
		ttLibC_TettyBootstrap *bootstrap,
		ttLibC_TcpClientInfo *client_info);

/**
 * call for write from bootstrap
 * @param bootstrap
 * @param client_info if client_info is NULL, broadcast else write for one client.
 * @param data
 * @param data_size
 * @return
 */
tetty_errornum ttLibC_TettyContext_channel_write_(
		ttLibC_TettyBootstrap *bootstrap,
		ttLibC_TcpClientInfo *client_info,
		void *data,
		size_t data_size);

/**
 * call for close from bootstrap
 * @param bootstrap
 * @param client_info
 * @return
 */
tetty_errornum ttLibC_TettyContext_close_(
		ttLibC_TettyBootstrap *bootstrap,
		ttLibC_TcpClientInfo *client_info);

#endif /* TTLIBC_NET_TETTY_CONTEXT_H_ */
