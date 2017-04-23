/**
 * @file   tcp.h
 * @brief  base for tcp.
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

#include "net.h"
#include <stdio.h>
#include <stdbool.h>
#include "../util/dynamicBufferUtil.h"

/**
 * definition for server information.
 */
typedef struct ttLibC_TcpServerInfo {
	ttLibC_SocketInfo inherit_super;
	bool use_reuse_addr;
	bool use_keep_alive;
	bool use_tcp_nodelay;
} ttLibC_Net_TcpServerInfo;

typedef ttLibC_Net_TcpServerInfo ttLibC_TcpServerInfo;

/**
 * definition of tcp client information.
 */
typedef struct ttLibC_TcpClientInfo {
	ttLibC_SocketInfo inherit_super;
	ttLibC_DynamicBuffer *write_buffer;
} ttLibC_Net_TcpClientInfo;

typedef ttLibC_Net_TcpClientInfo ttLibC_TcpClientInfo;

/**
 * make tcp wait socket
 * @param ip
 * @param port
 * @return ttLibC_TcpServerInfo object
 */
ttLibC_TcpServerInfo *ttLibC_TcpServer_make(
		uint64_t ip,
		uint16_t port);

/**
 * bind tcp socket.
 * @param server_info
 * @return true / false
 */
bool ttLibC_TcpServer_open(ttLibC_TcpServerInfo *server_info);

/**
 * accept tcp connection from client.
 * @param server_info
 * @return ttLibC_TcpClientInfo object.
 */
ttLibC_TcpClientInfo *ttLibC_TcpServer_wait(
		ttLibC_TcpServerInfo *server_info);

/**
 * close server socket
 * @param server_info
 */
void ttLibC_TcpServer_close(ttLibC_TcpServerInfo **server_info);

/**
 * make tcp client socket
 * @param host
 * @param por
 * @return ttLibC_TcpClientInfo2 object.
 */
ttLibC_TcpClientInfo *ttLibC_TcpClient_make(
		const char *host,
		int port);

int ttLibC_TcpClient_connect(ttLibC_TcpClientInfo *client_info);

int64_t ttLibC_TcpClient_read(
		ttLibC_TcpClientInfo *client_info,
		void * data,
		size_t data_size);

bool ttLibC_TcpClient_write(ttLibC_TcpClientInfo *client_info);

int ttLibC_TcpClient_setSockOpt(
		ttLibC_TcpClientInfo *client_info,
		int target,
		int option,
		const void *value,
		size_t value_size);

/**
 * close client socket
 * @param client_info
 */
void ttLibC_TcpClient_close(ttLibC_TcpClientInfo **client_info);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_SERVER_TCP_H_ */
