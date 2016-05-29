/**
 * @file   handler.h
 * @brief  client handler
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/05/17
 */

#ifndef TTLIBC_NET_CLIENT_WEBSOCKET_HANDLER_H_
#define TTLIBC_NET_CLIENT_WEBSOCKET_HANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../websocket.h"
#include "../../tetty.h"
#include "../../../util/dynamicBufferUtil.h"

typedef enum {
	State_header,
	State_body
} ttLibC_WebSocketHandler_State;

typedef struct ttLibC_Net_Client_WebSocket_Handler {
	ttLibC_TettyChannelHandler channel_handler;
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
} ttLibC_Net_Client_WebSocket_Handler;

typedef ttLibC_Net_Client_WebSocket_Handler ttLibC_WebSocketHandler;

ttLibC_WebSocketHandler *ttLibC_WebSocketHandler_make();
void ttLibC_WebSocketHandler_close(ttLibC_WebSocketHandler **handler);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_CLIENT_WEBSOCKET_HANDLER_H_ */
