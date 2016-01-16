/*
 * @file   tcp.c
 * @brief  base for tcp server.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2015/09/28
 */

#include "tcp.h"
#include "tcpmisc.h"

#include <sys/types.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/tcp.h>
#include "../allocator.h"
#include "../log.h"

ttLibC_TcpServerInfo *ttLibC_TcpServer_make(
		uint64_t ip,
		uint16_t port) {
	ttLibC_TcpServerInfo *server_info = (ttLibC_TcpServerInfo *)ttLibC_malloc(sizeof(ttLibC_TcpServerInfo));
	if(server_info == NULL) {
		ERR_PRINT("failed to allocate ttLibC_TcpServerInfo.");
		return NULL;
	}
	memset(server_info, 0, sizeof(ttLibC_TcpServerInfo));
	server_info->inherit_super.socket = -1;
	server_info->inherit_super.addr.sin_family = AF_INET;
	server_info->inherit_super.addr.sin_addr.s_addr = htonl(ip);
	server_info->inherit_super.addr.sin_port = htons(port);
	server_info->use_reuse_addr  = false;
	server_info->use_keep_alive  = false;
	server_info->use_tcp_nodelay = false;
	return server_info;
}

bool ttLibC_TcpServer_open(ttLibC_TcpServerInfo *server_info) {
	if(server_info == NULL) {
		return false;
	}
	server_info->inherit_super.socket = socket(PF_INET, SOCK_STREAM, 0);
	if(server_info->inherit_super.socket == -1) {
		ERR_PRINT("failed to open socket.");
		return false;
	}
	// set the options for server.
	int optval = 1;
	// TODO need to check return value.
	if(server_info->use_keep_alive) {
		setsockopt(server_info->inherit_super.socket, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
	}
	if(server_info->use_reuse_addr) {
		setsockopt(server_info->inherit_super.socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	}
	if(server_info->use_tcp_nodelay) {
		setsockopt(server_info->inherit_super.socket, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
	}
	if(bind(
			server_info->inherit_super.socket,
			(struct sockaddr *)&server_info->inherit_super.addr,
			sizeof(server_info->inherit_super.addr)) == -1) {
		ERR_PRINT("failed to bind.");
		return false;
	}
	if(listen(server_info->inherit_super.socket, 5) == -1) {
		ERR_PRINT("failed to listen");
		return false;
	}
	return true;
}

ttLibC_TcpClientInfo *ttLibC_TcpServer_wait(
		ttLibC_TcpServerInfo *server_info) {
	ttLibC_TcpClientInfo_ *client_info = (ttLibC_TcpClientInfo_ *)ttLibC_malloc(sizeof(ttLibC_TcpClientInfo_));
	if(client_info == NULL) {
		ERR_PRINT("failed to allocate client_info.");
		return NULL;
	}
	memset(client_info, 0, sizeof(ttLibC_TcpClientInfo_));
	client_info->write_buffer = NULL;
	while(true) {
		socklen_t client_addr_len = sizeof(client_info->inherit_super.addr);
		client_info->inherit_super.socket = accept(
				server_info->inherit_super.socket,
				(struct sockaddr *)&client_info->inherit_super.addr,
				&client_addr_len);
		if(client_info->inherit_super.socket != -1) {
			break;
		}
		if(errno == EINTR) {
			continue;
		}
		ERR_PRINT("failed to accept.");
		ttLibC_free(client_info);
		return NULL;
	}
	return (ttLibC_TcpClientInfo *)client_info;
}

void ttLibC_TcpServer_close(ttLibC_TcpServerInfo **server_info) {
	ttLibC_TcpServerInfo *target = *server_info;
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.socket != -1) {
		close(target->inherit_super.socket);
		target->inherit_super.socket = -1;
	}
	ttLibC_free(target);
	*server_info = NULL;
}

void ttLibC_TcpClient_close(ttLibC_TcpClientInfo **client_info) {
	ttLibC_TcpClientInfo_ *target = (ttLibC_TcpClientInfo_ *)*client_info;
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.socket != -1) {
		close(target->inherit_super.socket);
		target->inherit_super.socket = -1;
	}
	ttLibC_DynamicBuffer_close(&target->write_buffer);
	ttLibC_free(target);
	*client_info = NULL;
}

