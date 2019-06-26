/**
 * @file   handler.c
 * @brief  
 * @author taktod
 * @date   2016/05/17
 */

#ifdef __ENABLE_SOCKET__

#include "handler.h"
#include "../../../allocator.h"
#include "../../../_log.h"
#include "websocket.h"
#include <string.h>

static tetty_errornum WebSocketHandler_channelActive(ttLibC_TettyContext *ctx) {
	// call onopen.
	ttLibC_WebSocket *socket = (ttLibC_WebSocket *)ctx->socket_info->ptr;
	if(socket->onopen != NULL) {
		ttLibC_WebSocketEvent event;
		event.data = NULL;
		event.data_size = 0;
		event.type = 0;
		event.target = socket;
		socket->onopen(&event);
	}
	return 0;
}

static tetty_errornum WebSocketHandler_channelRead(
		ttLibC_TettyContext *ctx,
		void *data,
		size_t data_size) {
	ttLibC_WebSocketHandler *handler = (ttLibC_WebSocketHandler *)ctx->channel_handler;
	// update read_buffer
	ttLibC_DynamicBuffer_append(handler->read_buffer, data, data_size);
	if(handler->in_reading) {
		return 0;
	}
	handler->in_reading = true;
	while(ttLibC_DynamicBuffer_refSize(handler->read_buffer) > 0) {
		// pointer for received data.
		uint8_t *buf = ttLibC_DynamicBuffer_refData(handler->read_buffer);
		// in the case of header reading.
		if(handler->status == State_header) {
			bool is_last_chunk = (*buf & 0x80) != 0x00;
			ttLibC_WebSocketEvent_Opcode opcode = (*buf & 0x0F);
			uint32_t need_size = 2;
			// at least we need 2bytes.
			if(ttLibC_DynamicBuffer_refSize(handler->read_buffer) < need_size) {
				// if less, return and do it later.
				handler->in_reading = false;
				return 0;
			}
			bool is_masked = false;
			is_masked = (*(buf + 1) & 0x80) != 0x00;
			uint64_t read_size = *(buf + 1) & 0x7F;
			if(is_masked) {
				// if masked need 4byte more.
				need_size += 4;
			}
			switch(read_size) {
			case 0x7E:
				// size is defined in 16bit int.
				need_size += 2;
				if(ttLibC_DynamicBuffer_refSize(handler->read_buffer) < need_size) {
					handler->in_reading = false;
					return 0;
				}
				read_size = ((*(buf + 2) & 0xFF) << 8) | (*(buf + 3) & 0xFF);
				buf += 4;
				break;
			case 0x7F:
				// size is defined in 64bit int.
				need_size += 8;
				if(ttLibC_DynamicBuffer_refSize(handler->read_buffer) < need_size) {
					handler->in_reading = false;
					return 0;
				}
				read_size = \
						(*(buf + 2) & 0xFFL) << 56 | \
						(*(buf + 3) & 0xFFL) << 48 | \
						(*(buf + 4) & 0xFFL) << 40 | \
						(*(buf + 5) & 0xFFL) << 32 | \
						(*(buf + 6) & 0xFFL) << 24 | \
						(*(buf + 7) & 0xFFL) << 16 | \
						(*(buf + 8) & 0xFFL) << 8 | \
						(*(buf + 9) & 0xFFL);
				buf += 10;
				break;
			default:
				if(ttLibC_DynamicBuffer_refSize(handler->read_buffer) < need_size) {
					handler->in_reading = false;
					return 0;
				}
				buf += 2;
				break;
			}
			// now we have enough data size.
			// get mask bytes.
			if(is_masked) {
				for(int i = 0;i < 4;++ i) {
					handler->mask[i] = buf[i];
				}
			}
			if(!handler->is_last_chunk // prev data is not last chunk.
			&& is_last_chunk // now last chunk.
			&& opcode != WebSocketOpcode_continue) { // however, current opcode is not continue.
				ERR_PRINT("some chunk is missing in websocket data stream.");
			}
			if(opcode != WebSocketOpcode_continue) {
				// not continue means now is the beginning of chunk, so empty recv_buffer.
				ttLibC_DynamicBuffer_empty(handler->recv_buffer);
			}
			else {
				opcode = handler->opcode;
			}
			// now ready, update handler information.
			ttLibC_DynamicBuffer_markAsRead(handler->read_buffer, need_size);
			handler->is_last_chunk = is_last_chunk;
			handler->is_masked = is_masked;
			handler->opcode = opcode;
			handler->current_size = read_size;
			handler->status = State_body;
			buf = ttLibC_DynamicBuffer_refData(handler->read_buffer);
		}
		// current work for data body.
		if(ttLibC_DynamicBuffer_refSize(handler->read_buffer) < (size_t)handler->current_size) {
			// need more data. do later.
			handler->in_reading = false;
			return 0;
		}
		if(handler->is_masked) {
			// unmasked data.
			uint8_t *b = buf;
			for(int i = 0;i < handler->current_size;++ i) {
				*b = *b ^ handler->mask[i % 4];
			}
		}
		// copy data.
		ttLibC_DynamicBuffer_append(handler->recv_buffer, buf, handler->current_size);
		ttLibC_DynamicBuffer_markAsRead(handler->read_buffer, handler->current_size);
		ttLibC_DynamicBuffer_clear(handler->read_buffer);
		if(handler->is_last_chunk) {
			// if current is last chunk. data is ready.
			ttLibC_WebSocket *socket = (ttLibC_WebSocket *)ctx->socket_info->ptr;
			if(handler->opcode == WebSocketOpcode_text) {
				// for the text, put null byte.
				uint8_t null_str = 0x00;
				ttLibC_DynamicBuffer_append(handler->recv_buffer, &null_str, 1);
			}
			// call event callback.
			ttLibC_WebSocketEvent event;
			event.data = ttLibC_DynamicBuffer_refData(handler->recv_buffer);
			event.data_size = ttLibC_DynamicBuffer_refSize(handler->recv_buffer);
			event.target = socket;
			event.type = handler->opcode;
			switch(event.type) {
			case WebSocketOpcode_close:
				if(socket->onclose != NULL) {
					socket->onclose(&event);
				}
				ctx->bootstrap->error_number = 1; // put error for bootstrap, to stop working.
				break;
			case WebSocketOpcode_ping:
				if(socket->onmessage != NULL) {
					socket->onmessage(&event);
				}
				// if got ping, return pong.
				ttLibC_WebSocket_sendPong(socket);
				break;
			case WebSocketOpcode_pong:
				if(socket->onmessage != NULL) {
					socket->onmessage(&event);
				}
				break;
			default:
				if(socket->onmessage != NULL) {
					socket->onmessage(&event);
				}
				break;
			}
		}
		handler->status = State_header;
	}
	handler->in_reading = false;
	return 0;
}

static tetty_errornum WebSocketHandler_write(
		ttLibC_TettyContext *ctx,
		void *data,
		size_t data_size) {
	if(ctx->socket_info == NULL) {
		// update write call to use socket_info.
		ttLibC_TettyBootstrap_channelEach_write(ctx->bootstrap, data, data_size);
		return 0;
	}
	return ttLibC_TettyContext_super_write(ctx, data, data_size);
}

ttLibC_WebSocketHandler TT_ATTRIBUTE_INNER *ttLibC_WebSocketHandler_make() {
	ttLibC_WebSocketHandler *handler = ttLibC_malloc(sizeof(ttLibC_WebSocketHandler));
	if(handler == NULL) {
		return NULL;
	}
	memset(handler, 0, sizeof(ttLibC_WebSocketHandler));
	handler->channel_handler.channelActive = WebSocketHandler_channelActive;
	handler->channel_handler.channelRead = WebSocketHandler_channelRead;
	handler->channel_handler.write = WebSocketHandler_write;

	handler->is_last_chunk = true;
	handler->is_masked = false;
	handler->read_buffer = ttLibC_DynamicBuffer_make();
	handler->recv_buffer = ttLibC_DynamicBuffer_make();
	handler->current_size = 0;
	handler->status = State_header;
	handler->in_reading = false;
	return handler;
}

void TT_ATTRIBUTE_INNER ttLibC_WebSocketHandler_close(ttLibC_WebSocketHandler **handler) {
	ttLibC_WebSocketHandler *target = (ttLibC_WebSocketHandler *)*handler;
	if(target == NULL) {
		return;
	}
	ttLibC_DynamicBuffer_close(&target->read_buffer);
	ttLibC_DynamicBuffer_close(&target->recv_buffer);
	ttLibC_free(target);
	*handler = NULL;
}

#endif


