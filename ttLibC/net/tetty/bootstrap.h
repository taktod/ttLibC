/**
 * @file   bootstrap.h
 * @brief  bootstrap for tetty.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2015/11/24
 */

#ifndef TTLIBC_NET_TETTY_BOOTSTRAP_H_
#define TTLIBC_NET_TETTY_BOOTSTRAP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../tetty.h"
#include "../../util/stlListUtil.h"
#include "context.h"

#include "../tcp.h"

/**
 * bootstrap detail definition
 */
typedef struct ttLibC_Net_TettyBootstrap_ {
	ttLibC_TettyBootstrap inherit_super;
	/** channel type tcp or udp */
	ttLibC_Tetty_ChannelType channel_type;
	/** for socket option */
	bool so_keepalive;
	bool so_reuseaddr;
	bool tcp_nodelay;

	/** channel handler pipeline. */
	ttLibC_StlList *pipeline;

	/** bootstrap socket(waitsocket for server or data socket for udp.) */
	ttLibC_SocketInfo *socket_info;
	/** client socket list(for tcp only). */
	ttLibC_StlList *tcp_client_info_list;

	/** fdset object for select. */
	fd_set fdset;
	/** fdset object for select work. */
	fd_set fdchkset;

	ttLibC_TettyFuture *close_future;
} ttLibC_Net_TettyBootstrap_;

typedef ttLibC_Net_TettyBootstrap_ ttLibC_TettyBootstrap_;

/**
 * close target client.
 * @param bootstrap
 * @param client_info
 * @return true:ok false:error
 */
bool ttLibC_TettyBootstrap_closeClient_(
		ttLibC_TettyBootstrap *bootstrap,
		ttLibC_SocketInfo *socket_info);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_TETTY_BOOTSTRAP_H_ */
