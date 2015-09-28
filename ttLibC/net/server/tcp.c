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

#include <sys/types.h>
#include <strings.h>
#include <errno.h>
#include "../../allocator.h"
#include "../../log.h"

ttLibC_TcpServerInfo *ttLibC_TcpServer_make(
		uint64_t ip,
		uint16_t port) {
	ttLibC_TcpServerInfo *server_info = (ttLibC_TcpServerInfo *)ttLibC_malloc(sizeof(ttLibC_TcpServerInfo));
	if(server_info == NULL) {
		ERR_PRINT("failed to allocate ttLibC_TcpServerInfo.");
		return NULL;
	}
	memset(server_info, 0, sizeof(ttLibC_TcpServerInfo));
	server_info->wait_socket = -1;
	server_info->server_addr.sin_family = AF_INET;
	server_info->server_addr.sin_addr.s_addr = htonl(ip);
	server_info->server_addr.sin_port = htons(port);
	return server_info;
}

bool ttLibC_TcpServer_open(ttLibC_TcpServerInfo *server_info) {
	if(server_info == NULL) {
		return false;
	}
	server_info->wait_socket = socket(PF_INET, SOCK_STREAM, 0);
	if(server_info->wait_socket == -1) {
		ERR_PRINT("failed to open socket.");
		return false;
	}
	if(bind(
			server_info->wait_socket,
			(struct sockaddr *)&server_info->server_addr,
			sizeof(server_info->server_addr)) == -1) {
		ERR_PRINT("failed to bind.");
		return false;
	}
	if(listen(server_info->wait_socket, 5) == -1) {
		ERR_PRINT("failed to listen");
		return false;
	}
	return true;
}

ttLibC_TcpClientInfo *ttLibC_TcpServer_wait(
		ttLibC_TcpServerInfo *server_info) {
	// clientInfoをつくって応答しなければならない。
	ttLibC_TcpClientInfo *client_info = (ttLibC_TcpClientInfo *)ttLibC_malloc(sizeof(ttLibC_TcpClientInfo));
	if(client_info == NULL) {
		ERR_PRINT("failed to allocate client_info.");
		return NULL;
	}
	memset(client_info, 0, sizeof(ttLibC_TcpClientInfo));
	while(true) {
		socklen_t client_addr_len = sizeof(client_info->client_addr);
		client_info->data_socket = accept(
				server_info->wait_socket,
				(struct sockaddr *)&client_info->client_addr,
				&client_addr_len);
		if(client_info->data_socket != -1) {
			break;
		}
		if(errno == EINTR) {
			continue;
		}
		ERR_PRINT("failed to accept.");
		ttLibC_free(client_info);
	}
	return client_info;
}

void ttLibC_TcpServer_close(ttLibC_TcpServerInfo **server_info) {
	ttLibC_TcpServerInfo *target = *server_info;
	if(target == NULL) {
		return;
	}
	if(target->wait_socket != -1) {
		close(target->wait_socket);
		target->wait_socket = -1;
	}
	ttLibC_free(target);
	*server_info = NULL;
}

void ttLibC_TcpClient_close(ttLibC_TcpClientInfo **client_info) {
	ttLibC_TcpClientInfo *target = *client_info;
	if(target == NULL) {
		return;
	}
	if(target->data_socket != -1) {
		close(target->data_socket);
		target->data_socket = -1;
	}
	ttLibC_free(target);
	*client_info = NULL;
}

