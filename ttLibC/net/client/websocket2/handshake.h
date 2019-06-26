/*
 * handshake.h
 *
 *  Created on: 2017/07/21
 *      Author: taktod
 */

#ifndef TTLIBC_NET_CLIENT_WEBSOCKET2_HANDSHAKE_H_
#define TTLIBC_NET_CLIENT_WEBSOCKET2_HANDSHAKE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../../../ttLibC_predef.h"
#include "../websocket.h"
#include "../../tetty2/tcpBootstrap.h"

typedef struct ttLibC_Net_Client_WebSocket_HandshakeHandler {
	ttLibC_Tetty2ChannelHandler channel_handler;
	ttLibC_WebSocket *socket;
} ttLibC_Net_Client_WebSocket_HandshakeHandler;

typedef ttLibC_Net_Client_WebSocket_HandshakeHandler ttLibC_WebSocketHandshakeHandler;

ttLibC_WebSocketHandshakeHandler TT_ATTRIBUTE_INNER *ttLibC_WebSocketHandshakeHandler_make();
void ttLibC_WebSocketHandshakeHandler_close(ttLibC_WebSocketHandshakeHandler **handler);

ttLibC_Tetty2Promise TT_ATTRIBUTE_INNER *ttLibC_WebSocketHandshakeHandler_doHandshake(
		ttLibC_WebSocketHandshakeHandler *handshake,
		ttLibC_WebSocket *socket);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_CLIENT_WEBSOCKET2_HANDSHAKE_H_ */
