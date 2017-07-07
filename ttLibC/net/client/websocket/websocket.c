/*
 * @file   websocket.c
 * @brief  websocket detail definition.
 *
 * this code is under 3-Cause BSD License.
 * 
 * @author taktod
 * @date   2016/05/17
 */

#ifdef __ENABLE_SOCKET__
#include "../websocket.h"
#include <string.h>

#include "../../../ttLibC_predef.h"
#include "../../../allocator.h"
#include "../../../_log.h"
#include "../../tetty.h"
#include "../../../util/ioUtil.h"
#include "../../../util/hexUtil.h"

#include <sys/time.h>
#include "websocket.h"

/*
 * make websocket client object.
 * @param address  target websocket server address. ex:) ws://localhost:8080/test
 * @return ttLibC_WebSocket object.
 */
ttLibC_WebSocket TT_VISIBILITY_DEFAULT *ttLibC_WebSocket_make(const char *address) {
	// make struct object.
	ttLibC_WebSocket_ *socket = ttLibC_malloc(sizeof(ttLibC_WebSocket_));
	if(socket == NULL) {
		return NULL;
	}
	// analyze address string.
	char server_path[256];
	if(strstr(address, "wss://")
	&& sscanf(address, "wss://%s", server_path)
	&& strcmp(address, "wss://")) {
		ERR_PRINT("secure socket is not support.");
		ttLibC_free(socket);
		return NULL;
	}
	else if(strstr(address, "ws://")
	&& sscanf(address, "ws://%s", server_path)
	&& strcmp(address, "ws://")) {
		socket->port = 80; // default port = 80
		char *p;
		p = strchr(server_path, '/');
		if(p != NULL) {
			strcpy(socket->path, p + 1);
			*p = '\0';
			strcpy(socket->server, server_path);
		}
		else {
			strcpy(socket->path, "");
			strcpy(socket->server, server_path);
		}
		p = strchr(socket->server, ':');
		if(p != NULL) {
			socket->port = atoi(p + 1);
			if(socket->port <= 0) {
				socket->port = 80;
			}
			*p = '\0';
		}
	}
	else {
		ERR_PRINT("address is corrupt.");
		ttLibC_free(socket);
		return NULL;
	}
	socket->bootstrap = ttLibC_TettyBootstrap_make();

	socket->handshake_handler = ttLibC_WebSocketHandshakeHandler_make();
	ttLibC_TettyBootstrap_pipeline_addLast(socket->bootstrap, (ttLibC_TettyChannelHandler *)socket->handshake_handler);
	socket->handler = ttLibC_WebSocketHandler_make();
	ttLibC_TettyBootstrap_pipeline_addLast(socket->bootstrap, (ttLibC_TettyChannelHandler *)socket->handler);

	ttLibC_TettyBootstrap_connect(socket->bootstrap, socket->server, socket->port);

	// clear event.
	socket->inherit_super.onclose = NULL;
	socket->inherit_super.onerror = NULL;
	socket->inherit_super.onmessage = NULL;
	socket->inherit_super.onopen = NULL;
	socket->inherit_super.ptr = NULL;

	// fire handshake task.
	ttLibC_WebSocketHandshakeHandler_doHandshake(
			socket->handshake_handler,
			(ttLibC_WebSocket *)socket);

	return (ttLibC_WebSocket *)socket;
}

/**
 * update connection event.
 * @param socket        ttLibC_WebSocket object.
 * @param wait_interval interval in micro sec.
 * @return true:success false:error
 */
bool TT_VISIBILITY_DEFAULT ttLibC_WebSocket_update(
		ttLibC_WebSocket *socket,
		uint32_t wait_interval) {
	ttLibC_WebSocket_ *socket_ = (ttLibC_WebSocket_ *)socket;
	if(socket_ == NULL) {
		return false;
	}
	ttLibC_TettyBootstrap_update(socket_->bootstrap, wait_interval);
	if(socket_->bootstrap->error_number != 0) {
		ERR_PRINT("error:%d", socket_->bootstrap->error_number);
		return false;
	}
	return true;
}

/**
 * send text message to server.
 * @param socket
 * @param message
 */
void TT_VISIBILITY_DEFAULT ttLibC_WebSocket_sendText(
		ttLibC_WebSocket *socket,
		const char *message) {
	ttLibC_WebSocket__sendMessage(
			socket,
			WebSocketOpcode_text,
			(void *)message,
			strlen(message));
}

/**
 * send binary message to server.
 * @param socket
 * @param data
 * @param data_size
 */
void TT_VISIBILITY_DEFAULT ttLibC_WebSocket_sendBinary(
		ttLibC_WebSocket *socket,
		void *data,
		size_t data_size) {
	ttLibC_WebSocket__sendMessage(
			socket,
			WebSocketOpcode_binary,
			data,
			data_size);
}

// note below 3 functions can be hold any kind of message. however, skip.
void ttLibC_WebSocket_sendPing(ttLibC_WebSocket *socket) {
	ttLibC_WebSocket__sendMessage(
			socket, WebSocketOpcode_ping,
			NULL,
			0);
}

void ttLibC_WebSocket_sendPong(ttLibC_WebSocket *socket) {
	ttLibC_WebSocket__sendMessage(
			socket, WebSocketOpcode_pong,
			NULL,
			0);
}

void ttLibC_WebSocket_sendClose(ttLibC_WebSocket *socket) {
	ttLibC_WebSocket__sendMessage(
			socket, WebSocketOpcode_close,
			NULL,
			0);
}

void ttLibC_WebSocket__sendMessage(
		ttLibC_WebSocket *socket,
		ttLibC_WebSocketEvent_Opcode opcode,
		void *data,
		size_t data_size) {
	ttLibC_WebSocket_ *socket_ = (ttLibC_WebSocket_ *)socket;
	if(socket_->handshake_promise == NULL || !socket_->handshake_promise->is_done) {
		ERR_PRINT("try to send message before handshake done.");
		return;
	}
	// note: to support huge size of data.
	// we can devide message into small chunks.
	// but not support now.
	/* example from rfc6455 docs.
0x01 0x03 0x48 0x65 0x6c ( "Hel" )
0x80 0x02 0x6c 0x6f ( "lo" )
	 */
	uint8_t buf[256];
	size_t send_size = 0;
	uint8_t *b8 = buf;
	*b8 =  0x80 | opcode; // flag for message begining and opcode.
	++ b8;
	++ send_size;
	// data from client should be masked.
	if(data_size < 126) {
		*b8 = 0x80 | (data_size & 0xFF);
		++ b8;
		++ send_size;
	}
	else if(data_size < 0xFFFF) {
		// data size with 16bit int.
		*b8 = 0xFE;
		++ b8;
		uint16_t *b16 = (uint16_t *)b8;
		*b16 = be_uint16_t((uint16_t)data_size);
		b8 += 2;
		send_size += 3;
	}
	else if(data_size < 0xFFFFFFFFFFFFFFFFL) {
		// data size with 64bit int.
		*b8 = 0xFF;
		++ b8;
		uint64_t *b64 = (uint64_t *)b8;
		*b64 = be_uint64_t((uint64_t)data_size);
		b8 += 8;
		send_size += 9;
	}
	else {
		// not possible to be here.
		ERR_PRINT("unsupported data length.");
		return;
	}
	ttLibC_TettyBootstrap_channels_write(socket_->bootstrap, buf, send_size);
	// make mask bits from current unixtime.
	uint8_t mask[4];
	struct timeval mask_time;
	gettimeofday(&mask_time, NULL);
	mask[0] = mask_time.tv_sec & 0xFF;
	mask[1] = mask_time.tv_usec & 0xFF;
	mask[2] = (mask_time.tv_sec >> 8) & 0xFF;
	mask[3] = (mask_time.tv_usec >> 8) & 0xFF;
	// send mask
	ttLibC_TettyBootstrap_channels_write(socket_->bootstrap, mask, 4);
	// write data with masking.
	uint8_t *data_buf = data;
	for(size_t i = 0;i < data_size;) {
		buf[i % 256] = mask[i % 4] ^ data_buf[i];
		++ i;
		if(i != 0 && i % 256 == 0) {
			ttLibC_TettyBootstrap_channels_write(socket_->bootstrap, buf, 256);
		}
	}
	ttLibC_TettyBootstrap_channels_write(socket_->bootstrap, buf, data_size % 256);
	// all done, flush and send.
	ttLibC_TettyBootstrap_channels_flush(socket_->bootstrap);
}

void TT_VISIBILITY_DEFAULT ttLibC_WebSocket_close(ttLibC_WebSocket **socket) {
	ttLibC_WebSocket_ *target = (ttLibC_WebSocket_ *)*socket;
	if(target == NULL) {
		return;
	}
	ttLibC_TettyPromise_close(&target->handshake_promise);
	ttLibC_TettyBootstrap_close(&target->bootstrap);
	// channel handler close after bootstrap done.
	ttLibC_WebSocketHandler_close(&target->handler);
	ttLibC_WebSocketHandshakeHandler_close(&target->handshake_handler);
	ttLibC_free(target);
	*socket = NULL;
}

#endif

