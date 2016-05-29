/**
 * @file   handshake.h
 * @brief  handshake
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/05/17
 */

#ifndef TTLIBC_NET_CLIENT_WEBSOCKET_HANDSHAKE_H_
#define TTLIBC_NET_CLIENT_WEBSOCKET_HANDSHAKE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../websocket.h"
#include "../../tetty.h"

typedef struct ttLibC_Net_Client_WebSocket_HandshakeHandler {
	ttLibC_TettyChannelHandler channel_handler;
	ttLibC_WebSocket *socket;
} ttLibC_Net_Client_WebSocket_HandshakeHandler;

typedef ttLibC_Net_Client_WebSocket_HandshakeHandler ttLibC_WebSocketHandshakeHandler;

ttLibC_WebSocketHandshakeHandler *ttLibC_WebSocketHandshakeHandler_make();
void ttLibC_WebSocketHandshakeHandler_close(ttLibC_WebSocketHandshakeHandler **handler);

ttLibC_TettyPromise *ttLibC_WebSocketHandshakeHandler_doHandshake(
		ttLibC_WebSocketHandshakeHandler *handshake,
		ttLibC_WebSocket *socket);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_CLIENT_WEBSOCKET_HANDSHAKE_H_ */
