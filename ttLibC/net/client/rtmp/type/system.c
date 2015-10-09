/*
 * @file   system.c
 * @brief  
 *
 * This code is under 3-Cause BSD license
 *
 * @author taktod
 * @date   2015/09/06
 */

#include "system.h"
#include "../netConnection.h"
#include "../message.h"
#include "../../../../log.h"
#include "../../../../allocator.h"

ttLibC_RtmpMessage *ttLibC_RtmpSystemMessage_userControlMessage(
		ttLibC_RtmpConnection *conn,
		ttLibC_RtmpUserControlMessage_EventType event_type) {
	uint32_t size = 0;
	switch(event_type) {
	case RtmpEventType_StreamBegin:
	case RtmpEventType_StreamEof:
	case RtmpEventType_StreamDry:
		size = 6;
		break;
	case RtmpEventType_ClientBufferLength:
		size = 10;
		break;
	case RtmpEventType_RecordedStreamBegin:
		size = 6;
		break;
	case RtmpEventType_Unknown5:
		ERR_PRINT("unknown 5");
		size = 0;
		break;
	case RtmpEventType_Ping:
	case RtmpEventType_Pong:
		size = 6;
		break;
	case RtmpEventType_Unknown8:
		ERR_PRINT("unknown 8");
		size = 0;
		break;
	case RtmpEventType_PingSwfVerification:
	case RtmpEventType_PongSwfVerification:
		ERR_PRINT("swf verification ping/pong. I need to check.");
		size = 0;
		break;
	case RtmpEventType_BufferEmpty:
	case RtmpEventType_BufferFull:
		size = 6;
		break;
	}
	((ttLibC_RtmpConnection_ *)conn)->cs_id = 2;
	ttLibC_RtmpUserControlMessage *user_control_message = (ttLibC_RtmpUserControlMessage *)ttLibC_RtmpMessage_make(
			conn,
			sizeof(ttLibC_RtmpUserControlMessage),
			0,
			size,
			RtmpMessageType_userControlMessage,
			0);
	user_control_message->event_type = event_type;
	return (ttLibC_RtmpMessage *)user_control_message;
}

bool ttLibC_RtmpSystemMessage_sendPong(
		ttLibC_RtmpConnection *conn,
		uint32_t time,
		ttLibC_RtmpDataWriteFunc callback,
		void *ptr) {
	ttLibC_RtmpUserControlMessage *pong = (ttLibC_RtmpUserControlMessage *)ttLibC_RtmpSystemMessage_userControlMessage(
			conn,
			RtmpEventType_Pong);
	pong->value = time;
	bool result = ttLibC_RtmpMessage_write(
			conn,
			(ttLibC_RtmpMessage *)pong,
			callback,
			ptr);
	ttLibC_RtmpMessage_close((ttLibC_RtmpMessage **)&pong);
	return result;
}

bool ttLibC_RtmpSystemMessage_sendAcknowledgement(
		ttLibC_RtmpConnection *conn,
		ttLibC_RtmpDataWriteFunc callback,
		void *ptr) {
	ttLibC_RtmpConnection_ *conn_ = (ttLibC_RtmpConnection_ *)conn;
	conn_->cs_id = 2;
	ttLibC_RtmpAcknowledgement *acknowledgement = (ttLibC_RtmpAcknowledgement *)ttLibC_RtmpMessage_make(
			conn,
			sizeof(ttLibC_RtmpAcknowledgement),
			0,
			4,
			RtmpMessageType_acknowledgement,
			0);
	acknowledgement->value = conn_->read_size;
	bool result = ttLibC_RtmpMessage_write(
			conn,
			(ttLibC_RtmpMessage *)acknowledgement,
			callback,
			ptr);
	ttLibC_RtmpMessage_close((ttLibC_RtmpMessage **)&acknowledgement);
	conn_->read_size = 0;
	return result;
}

void ttLibC_RtmpSystemMessage_close(ttLibC_RtmpMessage **message) {
	ttLibC_RtmpMessage *target = *message;
	if(target == NULL) {
		return;
	}
	if(target->header != NULL) {
		switch(target->header->message_type) {
		case RtmpMessageType_setChunkSize:
			break; // checked
		case RtmpMessageType_abortMessage:
		case RtmpMessageType_acknowledgement:
		case RtmpMessageType_userControlMessage:
			break;
		case RtmpMessageType_windowAcknowledgementSize:
			break; // checked
		case RtmpMessageType_setPeerBandwidth:
			break; // checked
		case RtmpMessageType_audioMessage:
		case RtmpMessageType_videoMessage:
		case RtmpMessageType_amf3DataMessage:
		case RtmpMessageType_amf3SharedObjectMessage:
		case RtmpMessageType_amf3Command:
		case RtmpMessageType_amf0DataMessage:
		case RtmpMessageType_amf0SharedObjectMessage:
			break;
		case RtmpMessageType_amf0Command:
			break;
		case RtmpMessageType_aggregateMessage:
			break;
		}
	}
	// header will be close during netConnection close.
//	ttLibC_RtmpHeader_close(&target->header);
	ttLibC_free(target);
	*message = NULL;
}

