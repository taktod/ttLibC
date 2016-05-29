/**
 * @file   websocket.h
 * @brief  websocket detail definition.
 *
 * this code is under 3-Cause BSD License.
 * 
 * @author taktod
 * @date   2016/05/22
 */

#ifndef TTLIBC_NET_CLIENT_WEBSOCKET_WEBSOCKET_H_
#define TTLIBC_NET_CLIENT_WEBSOCKET_WEBSOCKET_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../websocket.h"
#include "handshake.h"
#include "handler.h"

/**
 * detail definition of webSocket client.
 */
typedef struct ttLibC_Net_Client_WebSocket_ {
	ttLibC_WebSocket inherit_super;
	// bootstrap
	ttLibC_TettyBootstrap *bootstrap;
	// handlers
	ttLibC_WebSocketHandler *handler;
	ttLibC_WebSocketHandshakeHandler *handshake_handler;

	// connect server information
	char server[256];
	int port;
	char path[256];

	// promise for handshake works.
	ttLibC_TettyPromise *handshake_promise;
} ttLibC_Net_Client_WebSocket_;

typedef ttLibC_Net_Client_WebSocket_ ttLibC_WebSocket_;

/**
 * send ping to server.
 * @param socket
 * @note will receive pong from server.
 */
void ttLibC_WebSocket_sendPing(ttLibC_WebSocket *socket);

/**
 * send pong to server.
 * @param socket
 */
void ttLibC_WebSocket_sendPong(ttLibC_WebSocket *socket);

/**
 * send close to server.
 * @param socket
 */
void ttLibC_WebSocket_sendClose(ttLibC_WebSocket *socket);

/**
 * sendMessage for internal use.
 * @param socket
 * @param opcode
 * @param data
 * @param data_size
 */
void ttLibC_WebSocket__sendMessage(
		ttLibC_WebSocket *socket,
		ttLibC_WebSocketEvent_Opcode opcode,
		void *data,
		size_t data_size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_CLIENT_WEBSOCKET_WEBSOCKET_H_ */
