/**
 * @file   handshake.c
 * @brief  handshake
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/05/17
 */

#ifdef __ENABLE_SOCKET__

#include "handshake.h"
#include "../../../ttLibC_predef.h"
#include "../../../allocator.h"
#include "../../../_log.h"
#include "../../../util/hexUtil.h"
#include "../../../util/dynamicBufferUtil.h"
#include "websocket.h"
#include <string.h>

static tetty_errornum WebSocketHandshakeHandler_channelActive(ttLibC_TettyContext *ctx) {
	(void)ctx;
	return 0;
}

static tetty_errornum WebSocketHandshakeHandler_write(
		ttLibC_TettyContext *ctx,
		void *data,
		size_t data_size) {
	if(ctx->socket_info == NULL) {
		ERR_PRINT("socket info is missing, something wrong.");
		ctx->bootstrap->error_number = -1;
		return -1;
	}
	if(ctx->socket_info->ptr == NULL) {
		ttLibC_WebSocketHandshakeHandler *handler = (ttLibC_WebSocketHandshakeHandler *)ctx->channel_handler;
		// put socket on socket_info->ptr.
		ctx->socket_info->ptr = handler->socket;
	}
	return ttLibC_TettyContext_super_write(ctx, data, data_size);
}

static tetty_errornum WebSocketHandshakeHandler_channelRead(
		ttLibC_TettyContext *ctx,
		void *data,
		size_t data_size) {
	/*
	 * ex from server.
	HTTP/1.1 101 Web Socket Protocol Handshake
	Upgrade: WebSocket
	Connection: Upgrade
	Sec-WebSocket-accept: OsmsjfZRFzSF5pDk3Fi9Nijtjfk=
	 */
	ttLibC_WebSocket_ *socket = (ttLibC_WebSocket_ *)ctx->socket_info->ptr;
	if(socket->handshake_promise->is_done) {
		return ttLibC_TettyContext_super_channelRead(ctx, data, data_size);
	}
	// TODO check sha and base64 encoding for validation.
	// just now, skip it.
	ttLibC_TettyPromise_setSuccess(socket->handshake_promise, NULL);
	ttLibC_TettyContext_super_channelActive(ctx); // call channel active for other pipeline channel_handler.
	return 0;
}

ttLibC_WebSocketHandshakeHandler *ttLibC_WebSocketHandshakeHandler_make() {
	ttLibC_WebSocketHandshakeHandler *handler = ttLibC_malloc(sizeof(ttLibC_WebSocketHandshakeHandler));
	if(handler == NULL) {
		return NULL;
	}
	memset(handler, 0, sizeof(ttLibC_WebSocketHandshakeHandler));
	handler->channel_handler.channelActive = WebSocketHandshakeHandler_channelActive;
	handler->channel_handler.channelRead = WebSocketHandshakeHandler_channelRead;
	handler->channel_handler.write = WebSocketHandshakeHandler_write;
	return handler;
}

void ttLibC_WebSocketHandshakeHandler_close(ttLibC_WebSocketHandshakeHandler **handler) {
	ttLibC_WebSocketHandshakeHandler *target = (ttLibC_WebSocketHandshakeHandler *)*handler;
	if(target == NULL) {
		return;
	}
	ttLibC_free(target);
	*handler = NULL;
}

ttLibC_TettyPromise *ttLibC_WebSocketHandshakeHandler_doHandshake(
		ttLibC_WebSocketHandshakeHandler *handshake,
		ttLibC_WebSocket *socket) {
	ttLibC_WebSocket_ *socket_ = (ttLibC_WebSocket_ *)socket;
	if(socket_ == NULL || handshake == NULL) {
		return NULL;
	}
	// make promise and fire event.
	ttLibC_TettyPromise *promise = ttLibC_TettyBootstrap_makePromise(socket_->bootstrap);
	handshake->socket = socket;
	socket_->handshake_promise = promise;
	// now try to send handshake data.(Http message)
	char buffer[256];
	sprintf(buffer, "GET /%s HTTP/1.1\r\n", socket_->path);
	ttLibC_TettyBootstrap_channels_write(socket_->bootstrap, buffer, strlen(buffer));
	sprintf(buffer, "Host: %s:%d\r\n", socket_->server, socket_->port);
	ttLibC_TettyBootstrap_channels_write(socket_->bootstrap, buffer, strlen(buffer));
	sprintf(buffer, "Connection: Upgrade\r\n");
	ttLibC_TettyBootstrap_channels_write(socket_->bootstrap, buffer, strlen(buffer));
	sprintf(buffer, "Pragma: no-cache\r\n");
	ttLibC_TettyBootstrap_channels_write(socket_->bootstrap, buffer, strlen(buffer));
	sprintf(buffer, "Cache-Control: no-cache\r\n");
	ttLibC_TettyBootstrap_channels_write(socket_->bootstrap, buffer, strlen(buffer));
	sprintf(buffer, "Upgrade: websocket\r\n");
	ttLibC_TettyBootstrap_channels_write(socket_->bootstrap, buffer, strlen(buffer));
	sprintf(buffer, "Sec-WebSocket-Version: 13\r\n");
	ttLibC_TettyBootstrap_channels_write(socket_->bootstrap, buffer, strlen(buffer));
	// TODO not to use fixed key.
	sprintf(buffer, "Sec-WebSocket-Key: 1/bsuDHSmjGgQm+QiHz1IQ==\r\n");
	ttLibC_TettyBootstrap_channels_write(socket_->bootstrap, buffer, strlen(buffer));
	sprintf(buffer, "Sec-WebSocket-Extensions:\r\n\r\n");
	ttLibC_TettyBootstrap_channels_write(socket_->bootstrap, buffer, strlen(buffer));
	ttLibC_TettyBootstrap_channels_flush(socket_->bootstrap);
	return promise;
}

#endif
