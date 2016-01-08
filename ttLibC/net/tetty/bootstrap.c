/*
 * @file   bootstrap.c
 * @brief  bootstrap for tetty.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2015/11/24
 */

#include <unistd.h>
#include <string.h>

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
	bootstrap->server_info = NULL;
	bootstrap->inherit_super.server_info = NULL;
	bootstrap->client_info_list = ttLibC_StlList_make();
	bootstrap->so_keepalive = false;
	bootstrap->so_reuseaddr = false;
	bootstrap->tcp_nodelay  = false;
	bootstrap->inherit_super.error_flag = 0;
	bootstrap->close_future = NULL;
	FD_ZERO(&bootstrap->fdset);
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

	bootstrap_->server_info = ttLibC_TcpServer_make(0x00000000UL, port);
	if(bootstrap_->server_info == NULL) {
		return false;
	}
	bootstrap_->server_info->use_keep_alive = bootstrap_->so_keepalive;
	bootstrap_->server_info->use_reuse_addr = bootstrap_->so_reuseaddr;
	bootstrap_->server_info->use_tcp_nodelay = bootstrap_->tcp_nodelay;
	bootstrap_->inherit_super.server_info = bootstrap_->server_info; // hold ref of server_info

	if(!ttLibC_TcpServer_open(bootstrap_->server_info)) {
		ERR_PRINT("failed to open socket.");
		bootstrap->error_flag = -1; // bind failed.
		return false;
	}
	// call pipeline->bind
	ttLibC_TettyContext_bind_((ttLibC_TettyBootstrap *)bootstrap_);
	// update fd_set
	FD_SET(bootstrap_->server_info->wait_socket, &bootstrap_->fdset);
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
	struct hostent *servhost = NULL;
	servhost = gethostbyname(host);
	if(servhost == NULL) {
		ERR_PRINT("failed to get ip addres from [%s]", host);
		return false;
	}
	ttLibC_TcpClientInfo *client_info = ttLibC_malloc(sizeof(ttLibC_TcpClientInfo));
	if(client_info == NULL) {
		ERR_PRINT("failed to make client info object.");
		bootstrap->error_flag = -2;
		return false;
	}
	memset(&client_info->data_addr, 0, sizeof(client_info->data_addr));

	memcpy(&client_info->data_addr.sin_addr, servhost->h_addr_list[0], servhost->h_length);
	client_info->data_addr.sin_family = AF_INET;
	client_info->data_addr.sin_port = htons(port);
	// make socket
	if((client_info->data_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		ERR_PRINT("failed to make socket.");
		ttLibC_free(client_info);
		bootstrap->error_flag = -3;
		return false;
	}
	int optval = 1;
	if(bootstrap_->so_keepalive) {
		setsockopt(client_info->data_socket, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
	}
/*	if(bootstrap_->so_reuseaddr) {
		setsockopt(client_info->data_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	}*/
	if(bootstrap_->tcp_nodelay) {
		setsockopt(client_info->data_socket, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
	}
	// connect
	if(connect(client_info->data_socket, (struct sockaddr *)&client_info->data_addr, sizeof(client_info->data_addr)) == -1) {
		ERR_PRINT("failed to connect.");
		ttLibC_free(client_info);
		bootstrap->error_flag = -4;
		return false;
	}
	// update fd_set
	FD_SET(client_info->data_socket, &bootstrap_->fdset);
	// put it on the list.
	ttLibC_StlList_addLast(bootstrap_->client_info_list, client_info);
	// call pipeline->connect
	ttLibC_TettyContext_connect_(bootstrap, client_info);
	// call pipeline->channelActive
	ttLibC_TettyContext_channelActive_(bootstrap, client_info);
	return true;
}

/**
 * do sync task for each client_connection.
 */
static bool TettyBootstrap_updateEach(void *ptr, void *item) {
	ttLibC_TettyBootstrap_ *bootstrap_ = (ttLibC_TettyBootstrap_ *)ptr;
	ttLibC_TcpClientInfo *client_info = (ttLibC_TcpClientInfo *)item;
	// check fd_set
	if(FD_ISSET(client_info->data_socket, &bootstrap_->fdchkset)) {
		uint8_t buffer[1024];
		size_t read_size = 0;
		memset(buffer, 0, sizeof(buffer));
		read_size = read(client_info->data_socket, buffer, sizeof(buffer));
		if(read_size == 0) {
			// closed.
			ttLibC_TettyBootstrap_closeClient_((ttLibC_TettyBootstrap *)bootstrap_, client_info);
			// remove from stl list.
			ttLibC_StlList_remove(bootstrap_->client_info_list, client_info);
		}
		else {
			// call pipeline->channelRead
			ttLibC_TettyContext_channelRead_((ttLibC_TettyBootstrap *)bootstrap_, client_info, buffer, read_size);
		}
	}
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
	if(bootstrap_->inherit_super.error_flag != 0) {
		return false;
	}

	if(bootstrap_->server_info == NULL && bootstrap_->client_info_list->size == 0) {
		// no more socket.
		bootstrap_->inherit_super.error_flag = -5;
		return false;
	}

	// check fdset wait for 0.01 sec.
	struct timeval timeout;
	memcpy(&bootstrap_->fdchkset, &bootstrap_->fdset, sizeof(fd_set));

	timeout.tv_sec = 0;
	timeout.tv_usec = wait_interval;
	bool response = false;

	if(select(FD_SETSIZE, &bootstrap_->fdchkset, NULL, NULL, &timeout)) {
		// there is read wait socket.
		if(bootstrap_->server_info != NULL) {
			// check server wait_socket.
			if(FD_ISSET(bootstrap_->server_info->wait_socket, &bootstrap_->fdchkset)) {
				// server socketはclient_infoを作成する必要がある。
				ttLibC_TcpClientInfo *client_info = ttLibC_TcpServer_wait(bootstrap_->server_info);
				if(client_info == NULL) {
					ERR_PRINT("failed to make client socket.");
					bootstrap_->inherit_super.error_flag = -6;
					return true;
				}
				// update fdset with new data_socket.
				FD_SET(client_info->data_socket, &bootstrap_->fdset);
				// call pipeline->channelActive
				ttLibC_TettyContext_channelActive_(bootstrap, client_info);
				ttLibC_StlList_addLast(bootstrap_->client_info_list, client_info);
				response = true;
			}
		}
		if(bootstrap_->client_info_list != NULL) {
			// check client sockets
			ttLibC_StlList_forEach(bootstrap_->client_info_list, TettyBootstrap_updateEach, bootstrap_);
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
		ttLibC_TcpClientInfo *client_info) {
	ttLibC_TettyBootstrap_ *bootstrap_ = (ttLibC_TettyBootstrap_ *)bootstrap;
	// remove this socket from fdset.
	FD_CLR(client_info->data_socket, &bootstrap_->fdset);
	// call pipeline->disconnect
	ttLibC_TettyContext_disconnect_((ttLibC_TettyBootstrap *)bootstrap_, client_info);
	// call pipeline->close
	ttLibC_TettyContext_close_((ttLibC_TettyBootstrap *)bootstrap_, client_info);
	// call pipeline->channelInactive
	ttLibC_TettyContext_channelInactive_((ttLibC_TettyBootstrap *)bootstrap_, client_info);
	// close client_info.
	ttLibC_TcpClient_close(&client_info);
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
			(ttLibC_TcpClientInfo *)item);
}

/*
 * close server socket.
 * @param bootstrap bootstrap object.
 */
void ttLibC_TettyBootstrap_closeServer(ttLibC_TettyBootstrap *bootstrap) {
	ttLibC_TettyBootstrap_ *bootstrap_ = (ttLibC_TettyBootstrap_ *)bootstrap;
	if(bootstrap_->server_info != NULL) {
		// remove from fdset.
		FD_CLR(bootstrap_->server_info->wait_socket, &bootstrap_->fdset);
		// stop server wait socket.
		ttLibC_TcpServer_close(&bootstrap_->server_info);
	}
}

/*
 * close all client socket.
 * @param bootstrap bootstrap object.
 */
void ttLibC_TettyBootstrap_closeClients(ttLibC_TettyBootstrap *bootstrap) {
	ttLibC_TettyBootstrap_ *bootstrap_ = (ttLibC_TettyBootstrap_ *)bootstrap;
	if(bootstrap_->client_info_list != NULL) {
		ttLibC_StlList_forEach(bootstrap_->client_info_list, TettyBootstrap_closeEach, bootstrap_);
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
	ttLibC_StlList_close(&target->client_info_list);
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
	if(bootstrap->error_flag != 0) {
		return bootstrap->error_flag;
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
	ttLibC_TettyContext_channel_write_(ctx->bootstrap, client_info, ctx->data, ctx->data_size);
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
	if(bootstrap->error_flag != 0) {
		return bootstrap->error_flag;
	}
	ttLibC_TettyContext_ ctx;
	ctx.bootstrap = bootstrap;
	ctx.data = data;
	ctx.data_size = data_size;
	ttLibC_TettyBootstrap_ *bootstrap_ = (ttLibC_TettyBootstrap_ *)bootstrap;
	ttLibC_StlList_forEach(bootstrap_->client_info_list, TettyBootstrap_channelEach_write_callback, &ctx);
	return 0;
}

/*
 * make promise
 * @param bootstrap
 */
ttLibC_TettyPromise *ttLibC_TettyBootstrap_makePromise(ttLibC_TettyBootstrap *bootstrap) {
	if(bootstrap->error_flag != 0) {
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
