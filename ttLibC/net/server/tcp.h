/**
 * @file   tcp.h
 * @brief  base for tcp server.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2015/09/28
 *
 * @see https://www.ipa.go.jp/security/awareness/vendor/programmingv1/b07_04.html
 */

#ifndef TTLIBC_NET_SERVER_TCP_H_
#define TTLIBC_NET_SERVER_TCP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct ttLibC_TcpServerInfo {
	int wait_socket;
	struct sockaddr_in server_addr;
	bool use_reuse_addr;
	bool use_keep_alive;
	bool use_tcp_nodelay;
	void *ptr;
} ttLibC_TcpServerInfo;

typedef struct ttLibC_TcpClientInfo {
	int data_socket;
	struct sockaddr_in data_addr;
	void *ptr;
} ttLibC_TcpClientInfo;

ttLibC_TcpServerInfo *ttLibC_TcpServer_make(
		uint64_t ip,
		uint16_t port);

bool ttLibC_TcpServer_open(ttLibC_TcpServerInfo *server_info);

ttLibC_TcpClientInfo *ttLibC_TcpServer_wait(
		ttLibC_TcpServerInfo *server_info);

void ttLibC_TcpServer_close(ttLibC_TcpServerInfo **server_info);

void ttLibC_TcpClient_close(ttLibC_TcpClientInfo **client_info);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_SERVER_TCP_H_ */
