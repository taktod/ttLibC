/**
 * @file   websocket.h
 * @brief  support for websocket client.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/05/13
 */

#ifndef TTLIBC_NET_CLIENT_WEBSOCKET_H_
#define TTLIBC_NET_CLIENT_WEBSOCKET_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../../ttLibC_predef.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * definition of opcode.
 */
typedef enum ttLibC_WebSocketEvent_Opcode {
	WebSocketOpcode_continue = 0x00,
	WebSocketOpcode_text     = 0x01,
	WebSocketOpcode_binary   = 0x02,
	WebSocketOpcode_close    = 0x08,
	WebSocketOpcode_ping     = 0x09,
	WebSocketOpcode_pong     = 0x0A
} ttLibC_WebSocketEvent_Opcode;

typedef struct ttLibC_Net_Client_WebSocket ttLibC_WebSocket;

/**
 * websocketevent structure.
 */
typedef struct ttLibC_Net_Client_WebSocketEvent {
	ttLibC_WebSocketEvent_Opcode type;
	ttLibC_WebSocket *target;
	void *data;
	size_t data_size;
} ttLibC_Net_Client_WebSocketEvent;

typedef ttLibC_Net_Client_WebSocketEvent ttLibC_WebSocketEvent;

/**
 * definition of event func.
 */
typedef bool (* ttLibC_WebSocketEventFunc)(ttLibC_WebSocketEvent *event);

/**
 * definition of websocket structure.
 */
typedef struct ttLibC_Net_Client_WebSocket {
	/** event for close */
	ttLibC_WebSocketEventFunc onclose;
	/** event for receiving message^ */
	ttLibC_WebSocketEventFunc onmessage;
	ttLibC_WebSocketEventFunc onopen;
	ttLibC_WebSocketEventFunc onerror;
	void *ptr; // you can put any data for ref.
} ttLibC_Net_Client_WebSocket;

// note: defined previously.
//typedef struct ttLibC_Net_Client_WebSocket ttLibC_WebSocket;

/**
 * make websocket client object.
 * @param address  target websocket server address. ex:) ws://localhost:8080/test
 * @return ttLibC_WebSocket object.
 */
ttLibC_WebSocket TT_ATTRIBUTE_API *ttLibC_WebSocket_make(const char *address);

/**
 * update connection event.
 * @param socket        ttLibC_WebSocket object.
 * @param wait_interval interval in micro sec.
 * @return true:success false:error
 */
bool TT_ATTRIBUTE_API ttLibC_WebSocket_update(
		ttLibC_WebSocket *socket,
		uint32_t wait_interval);

/**
 * send text message to server.
 * @param socket
 * @param message
 */
void TT_ATTRIBUTE_API ttLibC_WebSocket_sendText(
		ttLibC_WebSocket *socket,
		const char *message);

/**
 * send binary message to server.
 * @param socket
 * @param data
 * @param data_size
 */
void TT_ATTRIBUTE_API ttLibC_WebSocket_sendBinary(
		ttLibC_WebSocket *socket,
		void *data,
		size_t data_size);
/*
 * these function is used for internal only, now.
void ttLibC_WebSocket_sendPing(ttLibC_WebSocket *socket);
void ttLibC_WebSocket_sendPong(ttLibC_WebSocket *socket);
void ttLibC_WebSocket_sendClose(ttLibC_WebSocket *socket);
*/

/**
 * close websocket.
 * @param socket
 */
void TT_ATTRIBUTE_API ttLibC_WebSocket_close(ttLibC_WebSocket **socket);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_CLIENT_WEBSOCKET_H_ */
