/*
 * rtmpConnection.c
 *
 *  Created on: 2017/07/21
 *      Author: taktod
 */

#ifdef __ENABLE_SOCKET__

#include "rtmpConnection.h"
#include "../../../_log.h"
#include "../../../allocator.h"
#include "../../../util/amfUtil.h"
#include "../../tetty2/tcpBootstrap.h"
#include "message/acknowledgement.h"
#include "message/amf0Command.h"

#include <string.h>

ttLibC_RtmpConnection TT_ATTRIBUTE_API *ttLibC_RtmpConnection_make() {
	ttLibC_RtmpConnection_ *conn = ttLibC_malloc(sizeof(ttLibC_RtmpConnection_));
	if(conn == NULL) {
		return NULL;
	}
	conn->bootstrap = NULL;
	conn->client_handler = NULL;
	conn->command_handler = NULL;
	conn->decoder = NULL;
	conn->encoder = NULL;
	conn->handshake = NULL;

	conn->callback = NULL;
	conn->ptr = NULL;
	return (ttLibC_RtmpConnection *)conn;
}

static void RtmpConnection_connectCallback(void *ptr, ttLibC_Tetty2Promise *promise) {
	if(!promise->is_success) {
		return;
	}
	ttLibC_RtmpConnection_ *conn = (ttLibC_RtmpConnection_ *)ptr;
	ttLibC_Amf0Command *command = (ttLibC_Amf0Command *)promise->return_val;
	if(command->obj2 != NULL) {
		if(conn->callback != NULL) {
			conn->callback(conn->ptr, command->obj2);
		}
	}
}

void TT_ATTRIBUTE_API ttLibC_RtmpConnection_addEventListener(
		ttLibC_RtmpConnection *conn,
		ttLibC_RtmpEventFunc callback,
		void *ptr) {
	ttLibC_RtmpConnection_ *conn_ = (ttLibC_RtmpConnection_ *)conn;
	if(conn_ == NULL) {
		return;
	}
	conn_->callback = callback;
	conn_->ptr = ptr;
}

bool TT_ATTRIBUTE_API ttLibC_RtmpConnection_connect(
		ttLibC_RtmpConnection *conn,
		const char *address) {
	ttLibC_RtmpConnection_ *conn_ = (ttLibC_RtmpConnection_ *)conn;
	if(conn_ == NULL) {
		return false;
	}
	// check server address, port, application
	char server[256];
	char app[256];
	int32_t port = 1935;
	char server_path[256];
	if(strstr(address, "rtmp://")
	&& sscanf(address, "rtmp://%s", server_path)
	&& strcmp(address, "rtmp://")) {
		char *p;
		p = strchr(server_path, '/');
		if(p != NULL) {
			strcpy(app, p + 1);
			*p = '\0';
			strcpy(server, server_path);
		}
		else {
			strcpy(app, "");
			strcpy(server, server_path);
		}
		p = strchr(server, ':');
		if(p != NULL) {
			port = atoi(p + 1);
			if(port <= 0) {
				port = 1935;
			}
			*p = '\0';
		}
	}
	else {
		ERR_PRINT("failed to analyze address.");
		return false;
	}
	// setup tetty bootstrap.
	conn_->bootstrap = ttLibC_TcpBootstrap_make();
	ttLibC_TcpBootstrap_setOption(conn_->bootstrap, Tetty2Option_SO_KEEPALIVE);
	ttLibC_TcpBootstrap_setOption(conn_->bootstrap, Tetty2Option_TCP_NODELAY);

	// setup pipeline.
	conn_->handshake = ttLibC_RtmpHandshake_make();
	ttLibC_Tetty2Bootstrap_pipeline_addLast(conn_->bootstrap, conn_->handshake);
	conn_->decoder = ttLibC_RtmpDecoder_make();
	ttLibC_Tetty2Bootstrap_pipeline_addLast(conn_->bootstrap, conn_->decoder);
	conn_->encoder = ttLibC_RtmpEncoder_make();
	ttLibC_Tetty2Bootstrap_pipeline_addLast(conn_->bootstrap, conn_->encoder);
	conn_->command_handler = ttLibC_RtmpCommandHandler_make();
	ttLibC_Tetty2Bootstrap_pipeline_addLast(conn_->bootstrap, conn_->command_handler);
	conn_->client_handler = ttLibC_RtmpClientHandler_make();
	ttLibC_Tetty2Bootstrap_pipeline_addLast(conn_->bootstrap, conn_->client_handler);

	// do connect.
	ttLibC_TcpBootstrap_connect(conn_->bootstrap, server, port);

	// wait until handshake done.
	ttLibC_Tetty2Promise *handshake_promise = ttLibC_RtmpHandshake_getHandshakePromise(conn_->bootstrap, conn_->handshake);

	while(true) {
		ttLibC_TcpBootstrap_update(conn_->bootstrap, 10000);
		if(handshake_promise->is_done) {
			break;
		}
	}
//	ttLibC_Tetty2Promise_await(handshake_promise);
	ttLibC_Tetty2Promise_close(&handshake_promise);
	if(conn_->bootstrap->error_number != 0) {
		return false;
	}

	// send connect message.
	ttLibC_Amf0Command *connect = ttLibC_Amf0Command_connect(address, app);
	ttLibC_Tetty2Promise *connect_promise = ttLibC_Tetty2Bootstrap_makePromise(conn_->bootstrap);

	ttLibC_Tetty2Promise_addEventListener(connect_promise, RtmpConnection_connectCallback, conn_);
	connect->promise = connect_promise;

	ttLibC_Tetty2Bootstrap_write(conn_->bootstrap, connect, sizeof(ttLibC_Amf0Command));
	ttLibC_Tetty2Bootstrap_flush(conn_->bootstrap);
	ttLibC_Amf0Command_close(&connect);

	while(true) {
		ttLibC_TcpBootstrap_update(conn_->bootstrap, 10000);
		if(connect_promise->is_done) {
			break;
		}
	}
//	ttLibC_Tetty2Promise_await(connect_promise);
	ttLibC_Tetty2Promise_close(&connect_promise);
	return conn_->bootstrap->error_number == 0;
}

bool TT_ATTRIBUTE_API ttLibC_RtmpConnection_update(ttLibC_RtmpConnection* conn, uint32_t wait_interval) {
	ttLibC_RtmpConnection_ *conn_ = (ttLibC_RtmpConnection_ *)conn;
	if(conn_ == NULL) {
		return false;
	}
	ttLibC_RtmpClientHandler *client_handler = conn_->client_handler;
	if(client_handler != NULL) {
		if(client_handler->bytesRead - client_handler->bytesReadAcked >= client_handler->bytesReadWindow) {
			ttLibC_Acknowledgement *acknowledgement = ttLibC_Acknowledgement_make((uint32_t)client_handler->bytesRead);
			ttLibC_Tetty2Bootstrap_write(conn_->bootstrap, acknowledgement, sizeof(ttLibC_Acknowledgement));
			ttLibC_Tetty2Bootstrap_flush(conn_->bootstrap);
			ttLibC_Acknowledgement_close(&acknowledgement);
			client_handler->bytesReadAcked = client_handler->bytesRead;
		}
	}
	ttLibC_TcpBootstrap_update(conn_->bootstrap, wait_interval);
	if(conn_->bootstrap->error_number != 0) {
		ERR_PRINT("error:%d", conn_->bootstrap->error_number);
		return false;
	}
	return true;
}

void TT_ATTRIBUTE_API ttLibC_RtmpConnection_close(ttLibC_RtmpConnection **conn) {
	ttLibC_RtmpConnection_ *target = (ttLibC_RtmpConnection_ *)*conn;
	if(target == NULL) {
		return;
	}
	ttLibC_Tetty2Bootstrap_close(&target->bootstrap);
	ttLibC_RtmpHandshake_close(&target->handshake);
	ttLibC_RtmpDecoder_close(&target->decoder);
	ttLibC_RtmpEncoder_close(&target->encoder);
	ttLibC_RtmpCommandHandler_close(&target->command_handler);
	ttLibC_RtmpClientHandler_close(&target->client_handler);
	ttLibC_free(target);
	*conn = NULL;
}

#endif

