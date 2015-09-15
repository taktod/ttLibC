/*
 * @file   netConnection.c
 * @brief  netConnection functions.
 *
 * This code is under 3-Cause BSD license
 *
 * @author taktod
 * @date   2015/08/26
 */

#ifdef __ENABLE_FILE__
#include "netConnection.h"
#include "message.h"
#include "../../log.h"
#include "../../allocator.h"
#include "../../util/hexUtil.h"
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <stdlib.h>
#include <string.h>

static bool RtmpConnection_writeCallback(void *ptr, void *data, size_t data_size) {
	ttLibC_RtmpConnection_ *conn = (ttLibC_RtmpConnection_ *)ptr;
	if(conn->send_size + data_size > conn->chunk_size) {
		uint8_t *dat = data;
		ttLibC_RtmpHeader *header = ttLibC_RtmpHeader_getCurrentHeader((ttLibC_RtmpConnection *)conn);
		if(header == NULL) {
			return false;
		}
		// size is bigger than chunk_size. need to insert type3 header.
		size_t first_size = conn->chunk_size - conn->send_size;
		write(conn->sock, dat, first_size);
		conn->send_size = 0;
		ttLibC_RtmpHeader_write(RtmpHeaderType_3, header, RtmpConnection_writeCallback, conn);
		dat += first_size;
		size_t left_size = data_size - first_size;
		write(conn->sock, dat, left_size);
		conn->send_size = left_size;
	}
	else {
		size_t write_size = write(conn->sock, data, data_size);
		conn->send_size += data_size;
	}
	return true;
}

static bool RtmpConnection_clientHandler(
		void *ptr,
		ttLibC_RtmpMessage *message) {
	ttLibC_RtmpConnection_ *conn = (ttLibC_RtmpConnection_ *)ptr;
	switch(message->header->message_type) {
	case RtmpMessageType_setChunkSize:
		{
			ttLibC_RtmpSetChunkSize *setChunkSize_message = (ttLibC_RtmpSetChunkSize *)message;
			conn->chunk_size = setChunkSize_message->value;
		}
		break;
	case RtmpMessageType_abortMessage:
	case RtmpMessageType_acknowledgement:
	case RtmpMessageType_userControlMessage:
		{
			ttLibC_RtmpUserControlMessage *user_control_message = (ttLibC_RtmpUserControlMessage *)message;
			switch(user_control_message->event_type) {
//			case RtmpEventType_StreamBegin:
//			case RtmpEventType_StreamEof:
//			case RtmpEventType_StreamDry:
//			case RtmpEventType_ClientBufferLength:
//			case RtmpEventType_RecordedStreamBegin:
//			case RtmpEventType_Unknown5:
			case RtmpEventType_Ping:
				{
					bool write_result = true;
					// pingの場合はpongをつくって応答しなければならない。
					uint32_t time = user_control_message->value;
					ttLibC_RtmpUserControlMessage *pong = (ttLibC_RtmpUserControlMessage *)ttLibC_RtmpMessage_userControlMessage(
							(ttLibC_RtmpConnection *)conn,
							RtmpEventType_Pong);
					pong->value = time;
					write_result = ttLibC_RtmpMessage_write(
							(ttLibC_RtmpConnection *)conn,
							(ttLibC_RtmpMessage *)pong,
							RtmpConnection_writeCallback,
							conn);
					// メッセージとして送る。
					ttLibC_RtmpMessage_close((ttLibC_RtmpMessage **)&pong);
					return write_result;
				}
				break;
//			case RtmpEventType_Pong:
//			case RtmpEventType_Unknown8:
//			case RtmpEventType_PingSwfVerification:
//			case RtmpEventType_PongSwfVerification:
//			case RtmpEventType_BufferEmpty:
//			case RtmpEventType_BufferFull:
//				break;
			default:
				break;
			}
		}
		break;
	case RtmpMessageType_windowAcknowledgementSize:
		break;
	case RtmpMessageType_setPeerBandwidth:
		break;
	case RtmpMessageType_audioMessage:
	case RtmpMessageType_videoMessage:
	case RtmpMessageType_amf3DataMessage:
	case RtmpMessageType_amf3SharedObjectMessage:
	case RtmpMessageType_amf3Command:
	case RtmpMessageType_amf0DataMessage:
	case RtmpMessageType_amf0SharedObjectMessage:
		break;
	case RtmpMessageType_amf0Command:
		{
			ttLibC_RtmpAmf0Command *amf0Command_message = (ttLibC_RtmpAmf0Command *)message;
			uint32_t command_id = (uint32_t)(*((double *)amf0Command_message->command_id->object));
			const char *command_name = amf0Command_message->command_name->object;
			if(strcmp(command_name, "_result") == 0 && conn->command_id == command_id) {
				conn->command_id ++;
				conn->event_callback(conn->event_ptr, amf0Command_message->command_param2);
			}
		}
		break;
	case RtmpMessageType_aggregateMessage:
		break;
	}
	return true;
}

static bool RtmpConnection_checkServerResponse(
		ttLibC_Net_RtmpConnection_ *conn,
		ttLibC_RtmpEventFunc callback,
		void *ptr) {
	uint8_t buffer[256];
	do {
		ssize_t res = read(conn->sock, buffer, 256);
		if(res > 0) {
			if(!ttLibC_RtmpMessage_read(
					(ttLibC_RtmpConnection *)conn,
					buffer,
					res,
					RtmpConnection_clientHandler,
					conn)) {
				break;
			}
		}
		else if(res == 0) {
			// response 0 = disconnect from server.
			LOG_PRINT("disconnect from server.");
			break;
		}
		else {
			// timeout or err. quit loop.
			break;
		}
	}while(true);
	return true;
}

static bool RtmpConnection_connect(ttLibC_RtmpConnection_ *conn) {
	struct hostent *servhost = NULL;
	struct sockaddr_in server;
	servhost = gethostbyname(conn->inherit_super.server);
	if(servhost == NULL) {
		ERR_PRINT("failed to get ipaddress from [%s]", conn->inherit_super.server);
	}
	memset(&server, 0, sizeof(server));
	// TODO work in linux or window?
	memcpy(&server.sin_addr, servhost->h_addr_list[0], servhost->h_length);
	server.sin_family = AF_INET;
	server.sin_port = htons(conn->inherit_super.port);
	// setup ok.
	if((conn->sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		ERR_PRINT("failed to make socket");
		return false;
	}
	// put timeout for recv.
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 500000;
	setsockopt(conn->sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(tv));

	// need timeout for send?

	if(connect(conn->sock, (struct sockaddr *)&server, sizeof(server)) == -1) {
		ERR_PRINT("failed to connect server.");
		return false;
	}
	return true;
}

static bool RtmpConnection_handshake(ttLibC_RtmpConnection_ *conn) {
	char buf[1536];
	uint8_t *b = (uint8_t *)buf;
	// c1
	buf[0] = 0x03;
	if(write(conn->sock, buf, 1) < 0) {
		ERR_PRINT("failed to send c1.");
		return false;
	}
	// c2
	if(write(conn->sock, buf, 1536) < 0) {
		ERR_PRINT("failed to send c2.");
		return false;
	}
	// s1
	ssize_t size = read(conn->sock, buf, 1);
	if(size < 0) {
		ERR_PRINT("failed to recv s1");
		return false;
	}
	if(size == 0) {
		ERR_PRINT("disconnected during s1");
		return false;
	}
	// s2
	uint32_t left_size = 1536;
	size_t read_size = 0;
	do {
		size = read(conn->sock, b + read_size, 1536 - read_size);
		if(size < 0) {
			ERR_PRINT("failed to recv s2");
			return false;
		}
		if(size == 0) {
			ERR_PRINT("disconnected during s2");
			return false;
		}
		left_size -= size;
		read_size += size;
	}while(left_size > 0);
	// c3
	if(write(conn->sock, buf, 1536) < 0) {
		ERR_PRINT("failed to send c3.");
		return false;
	}
	// s3
	b = (uint8_t *)buf;
	left_size = 1536;
	read_size = 0;
	do {
		size = read(conn->sock, b + read_size, 1536 - read_size);
		if(size < 0) {
			ERR_PRINT("failed to recv s2");
			return false;
		}
		if(size == 0) {
			ERR_PRINT("disconnected during s2");
			return false;
		}
		left_size -= size;
		read_size += size;
	}while(left_size > 0);
	return true;
}

static bool RtmpConnection_sendConnect(ttLibC_RtmpConnection_ *conn) {
	ttLibC_RtmpMessage *msg = ttLibC_RtmpMessage_connect(
			(ttLibC_RtmpConnection *)conn,
			NULL); // to use custom message, I will use here.

	bool result = ttLibC_RtmpMessage_write(
			(ttLibC_RtmpConnection *)conn,
			msg,
			RtmpConnection_writeCallback,
			conn);

	// rtmp message is no more need. close.
	ttLibC_RtmpMessage_close(&msg);
	return result;
}

// move?
static bool RtmpConnection_sendCreateStream(ttLibC_Net_RtmpConnection_ *conn) {
	return true;
}

ttLibC_RtmpConnection *ttLibC_RtmpConnection_make() {
	ttLibC_RtmpConnection_ *conn = (ttLibC_RtmpConnection_ *)ttLibC_malloc(sizeof(ttLibC_RtmpConnection_));
	if(conn == NULL) {
		ERR_PRINT("failed to allocate rtmpConnection");
		return NULL;
	}
	conn->sock                 = 0;
	conn->send_size            = 0;
	conn->chunk_size           = 128;
	conn->command_id           = 1;
	conn->inherit_super.app    = NULL;
	conn->inherit_super.server = NULL;
	for(int i = 0;i < 64;++ i) {
		conn->headers[i] = NULL;
	}
	conn->ex_headers = NULL;
	conn->tmp_buffer = NULL;
	conn->tmp_buffer_size = 0;
	conn->tmp_buffer_next_pos = 0;

	conn->tmp_header_buffer_next_pos = 0;
	conn->tmp_header_buffer_size = 18;
	conn->tmp_header_buffer_target_size = 0;

	conn->read_type = readType_header; // headerの読み込みから始める。
	conn->event_callback = NULL;
	conn->event_ptr = NULL;
	return (ttLibC_RtmpConnection *)conn;
}

bool ttLibC_RtmpConnection_connect(
		ttLibC_RtmpConnection *conn,
		const char *address,
		ttLibC_RtmpEventFunc callback,
		void *ptr) {
	ttLibC_RtmpConnection_ *conn_ = (ttLibC_RtmpConnection_ *)conn;
	char server[256];
	char app[256];
	int32_t port = 1935;
	char server_path[256];
	conn_->event_callback = callback;
	conn_->event_ptr = ptr;

	if(strstr(address, "rtmp://")    // start with [rtmp://]
	&& sscanf(address, "rtmp://%s", server_path)  // there is data after [rtmp://]
	&& strcmp(address, "rtmp://")) { // not complete [rtmp://]
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
	size_t size = strlen(app);
	conn_->inherit_super.app = ttLibC_malloc(size + 1);
	if(conn_->inherit_super.app == NULL) {
		ERR_PRINT("failed to allocate memory for app string.");
		return false;
	}
	strcpy(conn_->inherit_super.app, app);
	size = strlen(server);
	conn_->inherit_super.server = ttLibC_malloc(size + 1);
	if(conn_->inherit_super.server == NULL) {
		ERR_PRINT("failed to allocate memory for server string.");
		return false;
	}
	strcpy(conn_->inherit_super.server, server);
	conn_->inherit_super.port = port;
	// setup is ok.
	// try connect.
	if(!RtmpConnection_connect(conn_)) {
		return false;
	}
	// do handshake
	if(!RtmpConnection_handshake(conn_)) {
		return false;
	}
	// send amf0 connect message.
	if(!RtmpConnection_sendConnect(conn_)) {
		return false;
	}
	// check response.
	if(!RtmpConnection_checkServerResponse(conn_, callback, ptr)) {
		return false;
	}
	return true;
}

bool ttLibC_RtmpConnection_read(ttLibC_RtmpConnection *conn) {
	ttLibC_RtmpConnection_ *conn_ = (ttLibC_RtmpConnection_ *)conn;
	if(conn_->event_callback != NULL) {
		return RtmpConnection_checkServerResponse(conn_, conn_->event_callback, conn_->event_ptr);
	}
	return false;
}

void ttLibC_RtmpConnection_close(ttLibC_RtmpConnection **conn) {
	ttLibC_RtmpConnection_ *target = (ttLibC_RtmpConnection_ *)*conn;
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.app != NULL) {
		ttLibC_free(target->inherit_super.app);
		target->inherit_super.app = NULL;
	}
	if(target->inherit_super.server != NULL) {
		ttLibC_free(target->inherit_super.server);
		target->inherit_super.server = NULL;
	}
	if(target->sock != 0) {
		close(target->sock);
		target->sock = 0;
	}
	for(int i = 0;i < 64;++ i) {
		ttLibC_RtmpHeader_close(&target->headers[i]);
	}
	if(target->ex_headers != NULL) {
		for(int i = 0;i < 65536;++ i) {
			ttLibC_RtmpHeader_close(&target->ex_headers[i]);
		}
		ttLibC_free(target->ex_headers);
	}
	if(target->tmp_buffer) {
		ttLibC_free(target->tmp_buffer);
		target->tmp_buffer = NULL;
	}
	ttLibC_free(target);
	*conn = NULL;
}

#endif
