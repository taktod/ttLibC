/*
 * @file   bootstrap.c
 * @brief  bootstrap for tetty.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2015/11/24
 */
#ifdef __ENABLE_SOCKET__

#include <unistd.h>
#include <string.h>

#include "../tcp.h"
#include "../udp.h"
#include "bootstrap.h"
#include "promise.h"
#include "../../allocator.h"
#include "../../log.h"
#include "../../util/forkUtil.h"
#include <netdb.h>
#include <netinet/tcp.h>

/*
 * make bootstrap object.
 * @return bootstrap object.
 */
ttLibC_TettyBootstrap *ttLibC_TettyBootstrap_make() {
	ttLibC_TettyBootstrap_ *bootstrap = ttLibC_malloc(sizeof(ttLibC_TettyBootstrap_));
	bootstrap->channel_type = ChannelType_Tcp;
	bootstrap->inherit_super.channel_type = ChannelType_Tcp;
	bootstrap->pipeline = ttLibC_StlList_make();
	bootstrap->socket_info = NULL;
	bootstrap->inherit_super.socket_info = NULL;
	bootstrap->tcp_client_info_list = ttLibC_StlList_make();
	bootstrap->so_keepalive = false;
	bootstrap->so_reuseaddr = false;
	bootstrap->tcp_nodelay  = false;
	bootstrap->inherit_super.error_number = 0;
	bootstrap->close_future = NULL;
	ttLibC_SocketInfo_FD_ZERO(&bootstrap->fdset);
//	FD_ZERO(&bootstrap->fdset);
	return (ttLibC_TettyBootstrap *)bootstrap;
}

/*
 * set channel type
 * @param bootstrap    bootstrap object.
 * @param channel_type channelType_Tcp or channelType_Udp(udp is not supported yet.)
 * @return true:success false:error
 */
bool ttLibC_TettyBootstrap_channel(
		ttLibC_TettyBootstrap *bootstrap,
		ttLibC_Tetty_ChannelType channel_type) {
	ttLibC_TettyBootstrap_ *bootstrap_ = (ttLibC_TettyBootstrap_ *)bootstrap;
	bootstrap_->channel_type = channel_type;
	bootstrap_->inherit_super.channel_type = channel_type;
	return true;
}

/*
 * set channel option
 * @param bootstrap bootstrap object.
 * @param option    target option type.
 * @return true:success false:error
 */
bool ttLibC_TettyBootstrap_option(
		ttLibC_TettyBootstrap *bootstrap,
		ttLibC_Tetty_Option option) {
	ttLibC_TettyBootstrap_ *bootstrap_ = (ttLibC_TettyBootstrap_ *)bootstrap;
	switch(option) {
	case Option_SO_KEEPALIVE:
		bootstrap_->so_keepalive = true;
		break;
	case Option_SO_REUSEADDR:
		bootstrap_->so_reuseaddr = true;
		break;
	case Option_TCP_NODELAY:
		bootstrap_->tcp_nodelay = true;
		break;
	}
	return true;
}

/*
 * bind.
 * @param bootstrap bootstrap object.
 * @param port      port number for bind.
 * @return true:success false:error
 */
bool ttLibC_TettyBootstrap_bind(
		ttLibC_TettyBootstrap *bootstrap,
		int port) {
	ttLibC_TettyBootstrap_ *bootstrap_ = (ttLibC_TettyBootstrap_ *)bootstrap;
	// TODO this is for tcp, for udp we need another code.
	switch(bootstrap_->channel_type) {
	default:
	case ChannelType_Tcp:
		{
			ttLibC_TcpServerInfo *server_info = ttLibC_TcpServer_make(0x00000000UL, port);
			if(server_info == NULL) {
				return false;
			}
			server_info->use_keep_alive = bootstrap_->so_keepalive;
			server_info->use_reuse_addr = bootstrap_->so_reuseaddr;
			server_info->use_tcp_nodelay = bootstrap_->tcp_nodelay;
			bootstrap_->socket_info = (ttLibC_SocketInfo *)server_info;
			bootstrap_->inherit_super.socket_info = bootstrap_->socket_info; // hold ref of server_info

			if(!ttLibC_TcpServer_open((ttLibC_TcpServerInfo *)bootstrap_->socket_info)) {
				ERR_PRINT("failed to open socket.");
				bootstrap->error_number = -1; // bind failed.
				return false;
			}
		}
		break;
	case ChannelType_Udp:
		{
			ttLibC_UdpSocketInfo *socket_info = ttLibC_UdpSocket_make(port);
			if(socket_info == NULL) {
				return false;
			}
			if(!ttLibC_UdpSocket_open(socket_info)) {
				ERR_PRINT("failed to open udpSocket.");
				return false;
			}
			bootstrap_->socket_info = (ttLibC_SocketInfo *)socket_info;
			bootstrap_->inherit_super.socket_info = bootstrap_->socket_info; // hold ref of socket_info
		}
		break;
	}
	// call pipeline->bind
	ttLibC_TettyContext_bind_((ttLibC_TettyBootstrap *)bootstrap_);
	// update fd_set
	ttLibC_SocketInfo_FD_SET(bootstrap_->socket_info, &bootstrap_->fdset);
	// for udp, call channel active.(context is none.)
	if(bootstrap_->channel_type == ChannelType_Udp) {
		ttLibC_TettyContext_channelActive_((ttLibC_TettyBootstrap *)bootstrap_, NULL);
	}
	return true;
}

/*
 * connect.
 * @param bootstrap bootstrap object
 * @param host      target host to connect
 * @param port      target port to connect
 * @return true:success false:error
 */
bool ttLibC_TettyBootstrap_connect(
		ttLibC_TettyBootstrap *bootstrap,
		const char *host,
		int port) {
	ttLibC_TettyBootstrap_ *bootstrap_ = (ttLibC_TettyBootstrap_ *)bootstrap;
	if(bootstrap_->channel_type == ChannelType_Udp) {
		ERR_PRINT("connect is not support in udp.");
		return false;
	}
	ttLibC_TcpClientInfo *client_info = ttLibC_TcpClient_make(host, port);
	if(client_info == NULL) {
		bootstrap->error_number = -3;
		return false;
	}
	int optval = 1;
	if(bootstrap_->so_keepalive) {
		ttLibC_TcpClient_setSockOpt(client_info, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
	}
/*	if(bootstrap_->so_reuseaddr) {
		ttLibC_TcpClient_setSockOpt(client_info, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	}*/
	if(bootstrap_->tcp_nodelay) {
		ttLibC_TcpClient_setSockOpt(client_info, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
	}
	// connect
	if(ttLibC_TcpClient_connect(client_info) == -1) {
		ERR_PRINT("failed to connect.");
		ttLibC_free(client_info);
		bootstrap->error_number = -4;
		return false;
	}
	// update fd_set
	ttLibC_SocketInfo_FD_SET((ttLibC_SocketInfo *)client_info, &bootstrap_->fdset);
	// put it on the list.
	ttLibC_StlList_addLast(bootstrap_->tcp_client_info_list, client_info);
	// call pipeline->connect
	ttLibC_TettyContext_connect_(bootstrap, (ttLibC_SocketInfo *)client_info);
	// call pipeline->channelActive
	ttLibC_TettyContext_channelActive_(bootstrap, (ttLibC_SocketInfo *)client_info);
	return true;
}

/**
 * do sync task for each client_connection.
 */
static bool TettyBootstrap_updateEach(void *ptr, void *item) {
	ttLibC_TettyBootstrap_ *bootstrap_ = (ttLibC_TettyBootstrap_ *)ptr;
	ttLibC_TcpClientInfo *client_info = (ttLibC_TcpClientInfo *)item;
	// check fd_set
	if(ttLibC_SocketInfo_FD_ISSET((ttLibC_SocketInfo *)client_info, &bootstrap_->fdchkset)) {
		uint8_t buffer[1024];
		size_t read_size = 0;
		memset(buffer, 0, sizeof(buffer));
		read_size = ttLibC_TcpClient_read(client_info, buffer, sizeof(buffer));
		if(read_size == 0) {
			// closed.
			ttLibC_TettyBootstrap_closeClient_((ttLibC_TettyBootstrap *)bootstrap_, (ttLibC_SocketInfo *)client_info);
			// remove from stl list.
			ttLibC_StlList_remove(bootstrap_->tcp_client_info_list, client_info);
		}
		else {
			// call pipeline->channelRead
			ttLibC_TettyContext_channelRead_((ttLibC_TettyBootstrap *)bootstrap_, (ttLibC_SocketInfo *)client_info, buffer, read_size);
		}
	}
	return true;
}

static bool TettyBootstrap_checkFdMaxValue(void *ptr, void *item) {
	int *pfd_max = (int *)ptr;
	*pfd_max = ttLibC_SocketInfo_updateFDMax((ttLibC_SocketInfo *)item, *pfd_max);
	return true;
}

/*
 * do sync task.
 * @param bootstrap bootstrap object.
 * @return true:new client_connection is established false:usual work.
 */
bool ttLibC_TettyBootstrap_update(
		ttLibC_TettyBootstrap *bootstrap,
		uint32_t wait_interval) {
	ttLibC_TettyBootstrap_ *bootstrap_ = (ttLibC_TettyBootstrap_ *)bootstrap;
	if(bootstrap_->inherit_super.error_number != 0) {
		return false;
	}

	if(bootstrap_->socket_info == NULL && bootstrap_->tcp_client_info_list->size == 0) {
		// no more socket.
		bootstrap_->inherit_super.error_number = -5;
		return false;
	}

	// check fdset wait for 0.01 sec.
	struct timeval timeout;
//	memcpy(&bootstrap_->fdchkset, &bootstrap_->fdset, sizeof(1));
	ttLibC_SocketInfo_FD_COPY(&bootstrap_->fdchkset, &bootstrap_->fdset);

	timeout.tv_sec = wait_interval / 1000000;
	timeout.tv_usec = wait_interval % 1000000;
	bool response = false;

	// check all socket to decide fd_max
	int fd_max = 0;
	if(bootstrap_->socket_info != NULL) {
//		fd_max = bootstrap_->socket_info->socket;
		fd_max = ttLibC_SocketInfo_updateFDMax(bootstrap_->socket_info, fd_max);
	}
	ttLibC_StlList_forEach(bootstrap_->tcp_client_info_list, TettyBootstrap_checkFdMaxValue, &fd_max);
	++ fd_max;

	if(select(fd_max, &bootstrap_->fdchkset, NULL, NULL, &timeout)) {
		// there is read wait socket.
		if(bootstrap_->socket_info != NULL) {
			// check server wait_socket.
			if(FD_ISSET(bootstrap_->socket_info->socket, &bootstrap_->fdchkset)) {
				switch(bootstrap_->channel_type) {
				default:
				case ChannelType_Tcp:
					{
						ttLibC_TcpClientInfo *client_info = ttLibC_TcpServer_wait((ttLibC_TcpServerInfo *)bootstrap_->socket_info);
						if(client_info == NULL) {
							ERR_PRINT("failed to make client socket.");
							bootstrap_->inherit_super.error_number = -6;
							return true;
						}
						// set tcp_nodelay and SO_KEEPALIVE
						int optval = 1;
						if(bootstrap_->so_keepalive) {
							ttLibC_TcpClient_setSockOpt(client_info, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
						}
/*						if(bootstrap_->so_reuseaddr) {
							ttLibC_TcpClient_setSockOpt(client_info, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
						}*/
						if(bootstrap_->tcp_nodelay) {
							ttLibC_TcpClient_setSockOpt(client_info, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
						}
						// update fdset with new data_socket.
						ttLibC_SocketInfo_FD_SET((ttLibC_SocketInfo *)client_info, &bootstrap_->fdset);
						// call pipeline->channelActive
						ttLibC_TettyContext_channelActive_(bootstrap, (ttLibC_SocketInfo *)client_info);
						ttLibC_StlList_addLast(bootstrap_->tcp_client_info_list, client_info);
						response = true;
					}
					break;
				case ChannelType_Udp:
					{
						// for recv we need to acquire data at once. and udp max = 65536
						uint8_t buf[65536];
						ttLibC_DatagramPacket *packet = ttLibC_DatagramPacket_make(buf, 65536);
						/*size_t read_size = */ttLibC_UdpSocket_read((ttLibC_UdpSocketInfo *)bootstrap_->socket_info, packet);
						// call pipeline_channelRead
						ttLibC_TettyContext_channelRead_(
								bootstrap,
								&packet->socket_info,
								packet,
								sizeof(ttLibC_DatagramPacket));
						ttLibC_DatagramPacket_close(&packet);
					}
					break;
				}
			}
		}
		// not udp
		if(bootstrap_->channel_type != ChannelType_Udp && bootstrap_->tcp_client_info_list != NULL) {
			// check client sockets
			ttLibC_StlList_forEach(bootstrap_->tcp_client_info_list, TettyBootstrap_updateEach, bootstrap_);
		}
	}
	return response;
}

/*
 * close target client.
 * @param bootstrap
 * @param client_info
 * @return true:ok false:error
 */
bool ttLibC_TettyBootstrap_closeClient_(
		ttLibC_TettyBootstrap *bootstrap,
		ttLibC_SocketInfo *socket_info) {
	ttLibC_TettyBootstrap_ *bootstrap_ = (ttLibC_TettyBootstrap_ *)bootstrap;
	if(bootstrap_->channel_type == ChannelType_Udp) {
		// for udp there is no close.
		return false;
	}
	// remove this socket from fdset.
	ttLibC_SocketInfo_FD_CLR(socket_info, &bootstrap_->fdset);
	// call pipeline->disconnect
	ttLibC_TettyContext_disconnect_((ttLibC_TettyBootstrap *)bootstrap_, socket_info);
	// call pipeline->close
	ttLibC_TettyContext_close_((ttLibC_TettyBootstrap *)bootstrap_, socket_info);
	// call pipeline->channelInactive
	ttLibC_TettyContext_channelInactive_((ttLibC_TettyBootstrap *)bootstrap_, socket_info);
	// close client_info.
	ttLibC_TcpClient_close((ttLibC_TcpClientInfo **)&socket_info);
	// done.
	return true;
}

/*
 * close each client connection.
 * @param ptr  bootstrap
 * @param item client_info
 * @return true:continue false:stop the stl list loop.
 */
static bool TettyBootstrap_closeEach(void *ptr, void *item) {
	return ttLibC_TettyBootstrap_closeClient_(
			(ttLibC_TettyBootstrap *)ptr,
			(ttLibC_SocketInfo *)item);
}

/*
 * close server socket.
 * @param bootstrap bootstrap object.
 */
void ttLibC_TettyBootstrap_closeServer(ttLibC_TettyBootstrap *bootstrap) {
	ttLibC_TettyBootstrap_ *bootstrap_ = (ttLibC_TettyBootstrap_ *)bootstrap;
	if(bootstrap_->socket_info != NULL) {
		// remove from fdset.
		ttLibC_SocketInfo_FD_CLR(bootstrap_->socket_info, &bootstrap_->fdset);
		switch(bootstrap_->channel_type) {
		default:
		case ChannelType_Tcp:
			// stop server wait socket.
			ttLibC_TcpServer_close((ttLibC_TcpServerInfo **)&bootstrap_->socket_info);
			break;
		case ChannelType_Udp:
			// call pipeline->close
			ttLibC_TettyContext_close_((ttLibC_TettyBootstrap *)bootstrap_, NULL);
			// call pipeline->channelInactive
			ttLibC_TettyContext_channelInactive_((ttLibC_TettyBootstrap *)bootstrap_, NULL);
			ttLibC_UdpSocket_close((ttLibC_UdpSocketInfo **)&bootstrap_->socket_info);
			break;
		}
	}
}

/*
 * close all client socket.
 * @param bootstrap bootstrap object.
 */
void ttLibC_TettyBootstrap_closeClients(ttLibC_TettyBootstrap *bootstrap) {
	ttLibC_TettyBootstrap_ *bootstrap_ = (ttLibC_TettyBootstrap_ *)bootstrap;
	if(bootstrap_->tcp_client_info_list != NULL) {
		ttLibC_StlList_forEach(bootstrap_->tcp_client_info_list, TettyBootstrap_closeEach, bootstrap_);
	}
}

/*
 * close all
 * @param bootstrap bootstrap object.
 */
void ttLibC_TettyBootstrap_close(ttLibC_TettyBootstrap **bootstrap) {
	ttLibC_TettyBootstrap_ *target = (ttLibC_TettyBootstrap_ *)*bootstrap;
	if(target == NULL) {
		return;
	}
	ttLibC_TettyBootstrap_closeClients((ttLibC_TettyBootstrap *)target);
	ttLibC_TettyBootstrap_closeServer((ttLibC_TettyBootstrap *)target);
	ttLibC_StlList_close(&target->tcp_client_info_list);
	ttLibC_StlList_close(&target->pipeline);
	if(target->close_future != NULL) {
		ttLibC_TettyPromise_ *promise = (ttLibC_TettyPromise_ *)target->close_future;
		promise->promise_type = PromiseType_Promise;
		ttLibC_TettyPromise_close((ttLibC_TettyPromise **)&promise);
	}
	ttLibC_free(target);
	*bootstrap = NULL;
}

/*
 * add channel handler.
 * @param bootstrap       bootstrap object.
 * @param channel_handler use def channel_handler object.
 */
void ttLibC_TettyBootstrap_pipeline_addLast(
		ttLibC_TettyBootstrap *bootstrap,
		ttLibC_TettyChannelHandler *channel_handler) {
	ttLibC_TettyBootstrap_ *bootstrap_ = (ttLibC_TettyBootstrap_ *)bootstrap;
	ttLibC_StlList_addLast(bootstrap_->pipeline, channel_handler);
}

/*
 * remove channel handler.
 * @param bootstrap       bootstrap object.
 * @param channel_handler use def channel_handler object.
 */
void ttLibC_TettyBootstrap_pipeline_remove(
		ttLibC_TettyBootstrap *bootstrap,
		ttLibC_TettyChannelHandler *channel_handler) {
	ttLibC_TettyBootstrap_ *bootstrap_ = (ttLibC_TettyBootstrap_ *)bootstrap;
	ttLibC_StlList_remove(bootstrap_->pipeline, channel_handler);
}

static bool TettyBootstrap_pipeline_each_fireUserEventTriggered_callback(void *ptr, void *item) {
	ttLibC_TettyContext_ *ctx = (ttLibC_TettyContext_ *)ptr;
	ttLibC_TcpClientInfo *client_info = item;
	ttLibC_TettyContext_userEventTriggered_(ctx->bootstrap, (ttLibC_SocketInfo *)client_info, ctx->data, ctx->data_size);
	return true;
}

/*
 * user defined event trigger.
 * @param bootstrap bootstrap object.
 * @param data      passing data
 * @param data_size passing data_size
 */
tetty_errornum ttLibC_TettyBootstrap_pipeline_fireUserEventTriggered(
		ttLibC_TettyBootstrap *bootstrap,
		ttLibC_SocketInfo *socket_info,
		void *data,
		size_t data_size) {
	ttLibC_TettyBootstrap_ *bootstrap_ = (ttLibC_TettyBootstrap_ *)bootstrap;
	if(socket_info != NULL) {
		return ttLibC_TettyContext_userEventTriggered_(bootstrap, socket_info, data, data_size);
	}
	switch(bootstrap_->channel_type) {
	case ChannelType_Tcp:
		{
			ttLibC_TettyContext_ ctx;
			ctx.data = data;
			ctx.data_size = data_size;
			ctx.bootstrap = bootstrap;
			ttLibC_StlList_forEach(bootstrap_->tcp_client_info_list, TettyBootstrap_pipeline_each_fireUserEventTriggered_callback, &ctx);
		}
		return 0;
	default:
	case ChannelType_Udp:
		return ttLibC_TettyContext_userEventTriggered_(bootstrap, socket_info, data, data_size);
	}
}

/*
 * write data for all client socket.(share one task for all connection.)
 * @param bootstrap
 * @param data
 * @param data_size
 * @return errornum
 */
tetty_errornum ttLibC_TettyBootstrap_channels_write(
		ttLibC_TettyBootstrap *bootstrap,
		void *data,
		size_t data_size) {
	if(bootstrap->error_number != 0) {
		return bootstrap->error_number;
	}
	return ttLibC_TettyContext_channel_write_(
			bootstrap,
			NULL,
			data,
			data_size);
}

/**
 * write data for each client socket.(callback for stlList)
 */
static bool TettyBootstrap_channelEach_write_callback(void *ptr, void *item) {
	ttLibC_TettyContext_ *ctx = (ttLibC_TettyContext_ *)ptr;
	ttLibC_TcpClientInfo *client_info = item;
	ttLibC_TettyContext_channel_write_(ctx->bootstrap, (ttLibC_SocketInfo *)client_info, ctx->data, ctx->data_size);
	return true;
}

/*
 * write data for all connect socket.(do all task for each connection.)
 * @param bootstrap
 * @param data
 * @param data_size
 * @return errornum
 */
tetty_errornum ttLibC_TettyBootstrap_channelEach_write(
		ttLibC_TettyBootstrap *bootstrap,
		void *data,
		size_t data_size) {
	if(bootstrap->error_number != 0) {
		return bootstrap->error_number;
	}
	ttLibC_TettyContext_ ctx;
	ctx.bootstrap = bootstrap;
	ctx.data = data;
	ctx.data_size = data_size;
	ttLibC_TettyBootstrap_ *bootstrap_ = (ttLibC_TettyBootstrap_ *)bootstrap;
	ttLibC_StlList_forEach(bootstrap_->tcp_client_info_list, TettyBootstrap_channelEach_write_callback, &ctx);
	return 0;
}

static bool TettyBootstrap_channelEach_callPipelineFlush_callback(void *ptr, void *item) {
	// call pipeline->flush
	ttLibC_TettyContext_flush_((ttLibC_TettyBootstrap *)ptr, (ttLibC_SocketInfo *)item);
	return true;
}

static bool TettyBootstrap_channelEach_flush_callback(void *ptr, void *item) {
	(void)ptr;
	return ttLibC_TcpClient_write((ttLibC_TcpClientInfo *)item);
}

tetty_errornum ttLibC_TettyBootstrap_channels_flush(ttLibC_TettyBootstrap *bootstrap) {
	ttLibC_TettyBootstrap_ *bootstrap_ = (ttLibC_TettyBootstrap_ *)bootstrap;
	// call pipeline->flush
	ttLibC_StlList_forEach(bootstrap_->tcp_client_info_list, TettyBootstrap_channelEach_callPipelineFlush_callback, bootstrap);
	// update socket reading.
	ttLibC_TettyBootstrap_update(bootstrap, 0);
	// do flush buffers.
	ttLibC_StlList_forEach(bootstrap_->tcp_client_info_list, TettyBootstrap_channelEach_flush_callback, NULL);
	return 0;
}

tetty_errornum ttLibC_TettyBootstrap_channels_writeAndFlush(
		ttLibC_TettyBootstrap *bootstrap,
		void *data,
		size_t data_size) {
	tetty_errornum error_num = ttLibC_TettyBootstrap_channels_write(bootstrap, data, data_size);
	if(error_num != 0) {
		return error_num;
	}
	error_num = ttLibC_TettyBootstrap_channels_flush(bootstrap);
	return error_num;
}

tetty_errornum ttLibC_TettyBootstrap_channelEach_writeAndFlush(
		ttLibC_TettyBootstrap *bootstrap,
		void *data,
		size_t data_size) {
	tetty_errornum error_num = ttLibC_TettyBootstrap_channelEach_write(
			bootstrap, data, data_size);
	if(error_num != 0) {
		return error_num;
	}
	error_num = ttLibC_TettyBootstrap_channels_flush(bootstrap);
	return error_num;
}

/*
 * make promise
 * @param bootstrap
 */
ttLibC_TettyPromise *ttLibC_TettyBootstrap_makePromise(ttLibC_TettyBootstrap *bootstrap) {
	if(bootstrap->error_number != 0) {
		return NULL;
	}
	return ttLibC_TettyPromise_make_(bootstrap);
}

/*
 * get the close future.
 * @param bootstrap
 * @return future
 */
ttLibC_TettyFuture *ttLibC_TettyBootstrap_closeFuture(ttLibC_TettyBootstrap *bootstrap) {
	ttLibC_TettyBootstrap_ *bootstrap_ = (ttLibC_TettyBootstrap_ *)bootstrap;
	ttLibC_TettyPromise_ *future = (ttLibC_TettyPromise_ *)ttLibC_TettyPromise_make_(bootstrap);
	future->promise_type = PromiseType_Future;
	bootstrap_->close_future = (ttLibC_TettyFuture *)future;
	return (ttLibC_TettyFuture *)future;
}

#endif
