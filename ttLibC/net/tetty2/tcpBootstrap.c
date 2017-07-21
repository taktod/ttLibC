/*
 * tcpBootstrap.c
 *
 *  Created on: 2017/07/21
 *      Author: taktod
 */

#ifdef __ENABLE_SOCKET__

#include "tcpBootstrap.h"
#include "../../util/tetty2/bootstrap.h"
#include "../tcp.h"
#include "../../_log.h"
#include <string.h>
#include <netdb.h>
#include <netinet/tcp.h>

static uint32_t project_id = 0x319a3;

typedef struct ttLibC_Net_TcpBootstrap {
	ttLibC_Tetty2Bootstrap_ inherit_super;
	bool so_keepalive;
	bool so_reuseaddr;
	bool tcp_nodelay;

	ttLibC_StlList *tcp_client_info_list;

	ttLibC_Fdset *fdset;
	ttLibC_Fdset *fdchkset;
} ttLibC_Net_TcpBootstrap;

typedef ttLibC_Net_TcpBootstrap ttLibC_TcpBootstrap;

static bool TcpBootstrap_check(
		ttLibC_Tetty2Bootstrap *bootstrap) {
	if(bootstrap == NULL) {
		return false;
	}
	ttLibC_Tetty2Bootstrap_ *bootstrap_ = (ttLibC_Tetty2Bootstrap_ *)bootstrap;
	return bootstrap_->project_id == project_id;
}

static void TcpBootstrap_closeClient(ttLibC_TcpClientInfo *client_info, ttLibC_TcpBootstrap *tcpBootstrap) {
	if(client_info == NULL) {
		return;
	}
	if(tcpBootstrap != NULL) {
		// remove from fdset
		ttLibC_Fdset_FD_CLR((ttLibC_SocketInfo *)client_info, tcpBootstrap->fdset);

		//	fire event
		ttLibC_Tetty2Info info;
		info.bootstrap_ptr = client_info;
		info.ptr = client_info->inherit_super.ptr;
		ttLibC_Tetty2Context_close_((ttLibC_Tetty2Bootstrap *)tcpBootstrap, &info);
		ttLibC_Tetty2Context_channelInactive_((ttLibC_Tetty2Bootstrap *)tcpBootstrap, &info);
		client_info->inherit_super.ptr = info.ptr;
	}
	// done
	ttLibC_TcpClient_close(&client_info);
}

static bool TcpBootstrap_closeAllClient(void *ptr, void *item) {
	ttLibC_TcpBootstrap *tcpBootstrap = (ttLibC_TcpBootstrap *)ptr;
	ttLibC_TcpClientInfo *client_info = (ttLibC_TcpClientInfo *)item;
	TcpBootstrap_closeClient(client_info, tcpBootstrap);
	return true;
}

static tetty2_errornum TcpBootstrap_close(
		ttLibC_Tetty2Bootstrap *bootstrap,
		ttLibC_Tetty2Context *ctx) {
	(void)ctx;
	if(!TcpBootstrap_check(bootstrap)) {
		return 99;
	}
	ttLibC_TcpBootstrap *target = (ttLibC_TcpBootstrap *)bootstrap;
	ttLibC_StlList_forEach(target->tcp_client_info_list, TcpBootstrap_closeAllClient, target);
	ttLibC_StlList_close(&target->tcp_client_info_list);
	ttLibC_TcpServerInfo *server_info = (ttLibC_TcpServerInfo *)target->inherit_super.tetty_info.bootstrap_ptr;
	ttLibC_TcpServer_close(&server_info);
	ttLibC_Fdset_close(&target->fdchkset);
	ttLibC_Fdset_close(&target->fdset);
	return 0;
}
static bool TcpBootstrap_flushAllClient(void *ptr, void *item) {
	ttLibC_TcpBootstrap *bootstrap = (ttLibC_TcpBootstrap *)ptr;
	ttLibC_TcpClientInfo *client_info = (ttLibC_TcpClientInfo *)item;
	if(client_info != NULL) {
		if(!ttLibC_TcpClient_flush(client_info)) {
			bootstrap->inherit_super.inherit_super.error_number = 21;
		}
	}
	return true;
}

static tetty2_errornum TcpBootstrap_flush(
		ttLibC_Tetty2Bootstrap *bootstrap,
		ttLibC_Tetty2Context *ctx) {
	if(!TcpBootstrap_check(bootstrap)) {
		return 99;
	}
	if(ctx == NULL) {
		ERR_PRINT("need to have context.");
		return 11;
	}
	ttLibC_TcpBootstrap *tcpBootstrap = (ttLibC_TcpBootstrap *)bootstrap;
	if(ctx->tetty_info->bootstrap_ptr == tcpBootstrap->inherit_super.tetty_info.bootstrap_ptr) {
		// for all context in bootstrap.
		ttLibC_StlList_forEach(tcpBootstrap->tcp_client_info_list, TcpBootstrap_flushAllClient, tcpBootstrap);
		return tcpBootstrap->inherit_super.inherit_super.error_number;
	}
	else {
		ttLibC_TcpClientInfo *client_info = (ttLibC_TcpClientInfo *)ctx->tetty_info->bootstrap_ptr;
		if(!ttLibC_TcpClient_flush(client_info)) {
			return 10;
		}
	}
	return 0;
}

static bool TcpBootstrap_writeAllClient(void *ptr, void *item) {
	ttLibC_Tetty2Context_ *ctx_ = (ttLibC_Tetty2Context_ *)ptr;
	ttLibC_TcpClientInfo *client_info = (ttLibC_TcpClientInfo *)item;
	if(!ttLibC_TcpClient_write(
			client_info,
			ctx_->data,
			ctx_->data_size)) {
		ctx_->inherit_super.bootstrap->error_number = 10;
	}
	return true;
}

static tetty2_errornum TcpBootstrap_write(
		ttLibC_Tetty2Bootstrap *bootstrap,
		ttLibC_Tetty2Context *ctx) {
	if(!TcpBootstrap_check(bootstrap)) {
		return 99;
	}
	if(ctx == NULL) {
		ERR_PRINT("need to have context");
		return 11;
	}
	ttLibC_TcpBootstrap *tcpBootstrap = (ttLibC_TcpBootstrap *)bootstrap;
	if(ctx->tetty_info->bootstrap_ptr == tcpBootstrap->inherit_super.tetty_info.bootstrap_ptr) {
		// write for all client_context
		ttLibC_StlList_forEach(tcpBootstrap->tcp_client_info_list, TcpBootstrap_writeAllClient, ctx);
		return tcpBootstrap->inherit_super.inherit_super.error_number;
	}
	else {
		ttLibC_Tetty2Context_ *ctx_ = (ttLibC_Tetty2Context_ *)ctx;
		ttLibC_TcpClientInfo *client_info = (ttLibC_TcpClientInfo *)ctx->tetty_info->bootstrap_ptr;
		if(!ttLibC_TcpClient_write(
				client_info,
				ctx_->data,
				ctx_->data_size)) {
			return 10;
		}
	}
	return 0;
}

ttLibC_Tetty2Bootstrap *ttLibC_TcpBootstrap_make() {
	ttLibC_TcpBootstrap *bootstrap = (ttLibC_TcpBootstrap *)ttLibC_Tetty2Bootstrap_make(sizeof(ttLibC_TcpBootstrap));
	if(bootstrap == NULL) {
		return NULL;
	}
	// false for all option for default
	bootstrap->so_keepalive = false;
	bootstrap->so_reuseaddr = false;
	bootstrap->tcp_nodelay = false;
	// client_list
	bootstrap->tcp_client_info_list = ttLibC_StlList_make();
	// use select with fdset
	bootstrap->fdchkset = ttLibC_Fdset_make();
	bootstrap->fdset    = ttLibC_Fdset_make();
	ttLibC_Fdset_FD_ZERO(bootstrap->fdset);
	// apply extra event
	bootstrap->inherit_super.close_event = TcpBootstrap_close;
	bootstrap->inherit_super.write_event = TcpBootstrap_write;
	bootstrap->inherit_super.flush_event = TcpBootstrap_flush;
	bootstrap->inherit_super.project_id = project_id; // set project_id
	return (ttLibC_Tetty2Bootstrap *)bootstrap;
}

bool ttLibC_TcpBootstrap_setOption(
		ttLibC_Tetty2Bootstrap *bootstrap,
		ttLibC_Tetty2_TcpOption option) {
	if(!TcpBootstrap_check(bootstrap)) {
		return false;
	}
	ttLibC_TcpBootstrap *tcpBootstrap = (ttLibC_TcpBootstrap *)bootstrap;
	switch(option) {
	case Tetty2Option_SO_KEEPALIVE:
		tcpBootstrap->so_keepalive = true;
		break;
	case Tetty2Option_SO_REUSEADDR:
		tcpBootstrap->so_reuseaddr = true;
		break;
	case Tetty2Option_TCP_NODELAY:
		tcpBootstrap->tcp_nodelay = true;
		break;
	default:
		return false;
	}
	return true;
}

bool ttLibC_TcpBootstrap_connect(
		ttLibC_Tetty2Bootstrap *bootstrap,
		const char *host,
		int port) {
	if(!TcpBootstrap_check(bootstrap)) {
		return false;
	}
	ttLibC_TcpBootstrap *tcpBootstrap = (ttLibC_TcpBootstrap *)bootstrap;
	ttLibC_TcpClientInfo *client_info = ttLibC_TcpClient_make(host, port);
	if(client_info == NULL) {
		bootstrap->error_number = -3;
		return false;
	}
	int optval = 1;
	if(tcpBootstrap->so_keepalive) {
		ttLibC_TcpClient_setSockOpt(client_info, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
	}
/*	if(tcpBootstrap->so_reuseaddr) {
		ttLibC_TcpClient_setSockOpt(client_info, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	}*/
	if(tcpBootstrap->tcp_nodelay) {
		ttLibC_TcpClient_setSockOpt(client_info, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
	}
	// connect
	if(ttLibC_TcpClient_connect(client_info) == -1) {
		ERR_PRINT("failed to connect.");
		ttLibC_TcpClient_close(&client_info);
		bootstrap->error_number = -4;
		return false;
	}
	// update fd_set
	ttLibC_Fdset_FD_SET((ttLibC_SocketInfo *)client_info, tcpBootstrap->fdset);
	// put it on the list.
	ttLibC_StlList_addLast(tcpBootstrap->tcp_client_info_list, client_info);
	// call pipeline->channelActive
	ttLibC_Tetty2Info info;
	info.bootstrap_ptr = client_info;
	info.ptr = NULL;
	ttLibC_Tetty2Context_channelActive_(bootstrap, &info);
	client_info->inherit_super.ptr = info.ptr;
	return true;
}

bool ttLibC_TcpBootstrap_bind(
		ttLibC_Tetty2Bootstrap *bootstrap,
		int port) {
	if(!TcpBootstrap_check(bootstrap)) {
		return false;
	}
	ttLibC_TcpBootstrap *tcpBootstrap = (ttLibC_TcpBootstrap *)bootstrap;
	ttLibC_TcpServerInfo *server_info = ttLibC_TcpServer_make(0x00000000UL, port);
	if(server_info == NULL) {
		return false;
	}
	server_info->use_keep_alive  = tcpBootstrap->so_keepalive;
	server_info->use_reuse_addr  = tcpBootstrap->so_reuseaddr;
	server_info->use_tcp_nodelay = tcpBootstrap->tcp_nodelay;
	tcpBootstrap->inherit_super.tetty_info.bootstrap_ptr = server_info;

	if(!ttLibC_TcpServer_open(server_info)) {
		ERR_PRINT("failed to open socket.");
		tcpBootstrap->inherit_super.inherit_super.error_number = -1;
		return false;
	}
	ttLibC_Fdset_FD_SET((ttLibC_SocketInfo *)server_info, tcpBootstrap->fdset);
	return true;
}

static bool TcpBootstrap_checkFdMaxValue(void *ptr, void *item) {
	int *pfd_max = (int *)ptr;
	*pfd_max = ttLibC_Fdset_updateFDMax((ttLibC_SocketInfo *)item, *pfd_max);
	return true;
}
static bool TcpBootstrap_updateEach(void *ptr, void *item) {
	ttLibC_TcpBootstrap *tcpBootstrap = (ttLibC_TcpBootstrap *)ptr;
	ttLibC_TcpClientInfo *client_info = (ttLibC_TcpClientInfo *)item;
	if(ttLibC_Fdset_FD_ISSET((ttLibC_SocketInfo *)client_info, tcpBootstrap->fdchkset)) {
		uint8_t buffer[65536];
		size_t read_size;
		memset((void *)buffer, 0, sizeof(buffer));
		read_size = ttLibC_TcpClient_read(client_info, buffer, sizeof(buffer));
		ttLibC_Tetty2Info info;
		info.bootstrap_ptr = client_info;
		info.ptr = client_info->inherit_super.ptr;
		if(read_size == 0) {
			ttLibC_StlList_remove(tcpBootstrap->tcp_client_info_list, client_info);
			TcpBootstrap_closeClient(client_info, tcpBootstrap);
			// return false, to stop forEach operation.
			return false;
		}
		else {
			ttLibC_Tetty2Context_channelRead_((ttLibC_Tetty2Bootstrap *)tcpBootstrap, &info, buffer, read_size);
		}
		client_info->inherit_super.ptr = info.ptr;
	}
	return true;
}

bool ttLibC_TcpBootstrap_update(
		ttLibC_Tetty2Bootstrap *bootstrap,
		uint32_t wait_interval) {
	// return true:do anything false:do nothing
	// to check error,check bootstrap error_number.
	if(!TcpBootstrap_check(bootstrap)) {
		return false;
	}
	ttLibC_TcpBootstrap *tcpBootstrap = (ttLibC_TcpBootstrap *)bootstrap;
	ttLibC_SocketInfo *server_info = (ttLibC_SocketInfo *)tcpBootstrap->inherit_super.tetty_info.bootstrap_ptr;
	if(tcpBootstrap->inherit_super.inherit_super.error_number != 0) {
		// status is error, don't do anything.
		return false;
	}
	if(server_info == NULL && tcpBootstrap->tcp_client_info_list->size == 0) {
		// no more client, tcp is done.
		tcpBootstrap->inherit_super.inherit_super.error_number = -5;
		return false;
	}
	ttLibC_Fdset_FD_COPY(tcpBootstrap->fdchkset, tcpBootstrap->fdset);

	// check fd_max
	int fd_max = 0;
	if(server_info != NULL) {
		fd_max = ttLibC_Fdset_updateFDMax(server_info, fd_max);
	}
	ttLibC_StlList_forEach(tcpBootstrap->tcp_client_info_list, TcpBootstrap_checkFdMaxValue, &fd_max);
	++ fd_max;
	if(ttLibC_Fdset_select(fd_max, tcpBootstrap->fdchkset, NULL, NULL, wait_interval)) {
		// we have some socket which we need to read
		if(server_info != NULL) {
			// check server
			if(ttLibC_Fdset_FD_ISSET(server_info, tcpBootstrap->fdchkset)) {
				ttLibC_TcpClientInfo *client_info = ttLibC_TcpServer_wait((ttLibC_TcpServerInfo *)server_info);
				if(client_info == NULL) {
					ERR_PRINT("failed to make client socket.");
					tcpBootstrap->inherit_super.inherit_super.error_number = -6;
					return true;
				}
				int optval = 1;
				if(tcpBootstrap->so_keepalive) {
					ttLibC_TcpClient_setSockOpt(client_info, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
				}
//				if(tcpBootstrap->so_reuseaddr) {
//					ttLibC_TcpClient_setSockOpt(client_info, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
//				}
				if(tcpBootstrap->tcp_nodelay) {
					ttLibC_TcpClient_setSockOpt(client_info, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
				}
				// update fdset with new data_socket.
				ttLibC_Fdset_FD_SET((ttLibC_SocketInfo *)client_info, tcpBootstrap->fdset);
				// call pipeline->channelActive
				ttLibC_Tetty2Info info;
				info.bootstrap_ptr = client_info;
				info.ptr = client_info->inherit_super.ptr;
				ttLibC_Tetty2Context_channelActive_(bootstrap, &info);
				client_info->inherit_super.ptr = info.ptr;
				ttLibC_StlList_addLast(tcpBootstrap->tcp_client_info_list, client_info);
			}
		}
		ttLibC_StlList_forEach(tcpBootstrap->tcp_client_info_list, TcpBootstrap_updateEach, tcpBootstrap);
		return true;
	}
	return false;
}

#endif

