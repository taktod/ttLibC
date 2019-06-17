/*
 * handler.h
 *
 *  Created on: 2017/07/21
 *      Author: taktod
 */

#ifndef TTLIBC_NET_CLIENT_WEBSOCKET2_HANDLER_H_
#define TTLIBC_NET_CLIENT_WEBSOCKET2_HANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../../../ttLibC_predef.h"
#include "../websocket.h"
#include "../../tetty2/tcpBootstrap.h"
#include "../../../util/dynamicBufferUtil.h"

typedef enum {
	State_header,
	State_body
} ttLibC_WebSocketHandler_State;

typedef struct ttLibC_Net_Client_WebSocket_Handler {
	ttLibC_Tetty2ChannelHandler channel_handler;
	// raw data from server.
	ttLibC_DynamicBuffer *read_buffer;
	// decoded message.
	ttLibC_DynamicBuffer *recv_buffer;

	// data for current data.
	bool is_last_chunk;
	bool is_masked;
	uint8_t mask[4];
	ttLibC_WebSocketEvent_Opcode opcode;
	int64_t current_size;
	ttLibC_WebSocketHandler_State status;
	bool in_reading;
} ttLibC_Net_Client_WebSocket_Handler;

typedef ttLibC_Net_Client_WebSocket_Handler ttLibC_WebSocketHandler;

ttLibC_WebSocketHandler TT_ATTRIBUTE_INNER *ttLibC_WebSocketHandler_make();
void TT_ATTRIBUTE_INNER ttLibC_WebSocketHandler_close(ttLibC_WebSocketHandler **handler);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_CLIENT_WEBSOCKET2_HANDLER_H_ */
