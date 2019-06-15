/*
 * userControlMessage.c
 *
 *  Created on: 2017/07/22
 *      Author: taktod
 */

#ifdef __ENABLE_SOCKET__

#include "userControlMessage.h"
#include "../../../../ttLibC_predef.h"
#include "../../../../_log.h"
#include "../../../../allocator.h"
#include <string.h>
#include "../../../../util/ioUtil.h"

ttLibC_UserControlMessage TT_VISIBILITY_HIDDEN *ttLibC_UserControlMessage_make(
		ttLibC_UserControlMessage_Type type,
		uint32_t stream_id,
		uint32_t buffer_length,
		uint32_t time) {
	ttLibC_UserControlMessage *message = ttLibC_malloc(sizeof(ttLibC_UserControlMessage));
	if(message == NULL) {
		return NULL;
	}
	ttLibC_RtmpHeader *header = ttLibC_RtmpHeader_make(2, 0, RtmpMessageType_userControlMessage, 0);
	if(header == NULL) {
		ttLibC_free(message);
		return NULL;
	}
	message->inherit_super.header = header;
	message->type = type;
	message->stream_id = stream_id;
	message->buffer_length = buffer_length;
	message->time = time;
	return message;
}

ttLibC_UserControlMessage TT_VISIBILITY_HIDDEN *ttLibC_UserControlMessage_readBinary(
		uint8_t *data,
		size_t data_size) {
	if(data_size < 2) {
		return NULL;
	}
	// make type from data.
	ttLibC_UserControlMessage_Type type = be_uint16_t(*((uint16_t *)data));
	data += 2;
	data_size -= 2;
	switch(type) {
	case Type_StreamBegin:
	case Type_StreamEof:
	case Type_StreamDry:
	case Type_RecordedStream:
	case Type_BufferEmpty:
	case Type_BufferFull:
		{
			if(data_size != 4) {
				return NULL;
			}
			uint32_t stream_id = be_uint32_t(*((uint32_t *)data));
			ttLibC_UserControlMessage *message = ttLibC_UserControlMessage_make(type, stream_id, 0, 0);
			return message;
		}
		break;

	case Type_ClientBufferLength:
		{
			if(data_size != 8) {
				return NULL;
			}
			uint32_t *data32 = (uint32_t *)data;
			uint32_t stream_id = be_uint32_t(*((uint32_t *)data));
			++ data32;
			uint32_t buffer_length = be_uint32_t(*((uint32_t *)data));
			ttLibC_UserControlMessage *message = ttLibC_UserControlMessage_make(type, stream_id, buffer_length, 0);
			return message;
		}
		break;

	case Type_Ping:
	case Type_Pong:
		{
			if(data_size != 4) {
				return NULL;
			}
			uint32_t time = be_uint32_t(*((uint32_t *)data));
			ttLibC_UserControlMessage *message = ttLibC_UserControlMessage_make(type, 0, 0, time);
			return message;
		}
		break;
	default:
		break;
	}
	return NULL;
}

ttLibC_UserControlMessage TT_VISIBILITY_HIDDEN *ttLibC_UserControlMessage_ping(uint32_t time) {
	return ttLibC_UserControlMessage_make(Type_Ping, 0, 0, time);
}

ttLibC_UserControlMessage TT_VISIBILITY_HIDDEN *ttLibC_UserControlMessage_pong(uint32_t time) {
	return ttLibC_UserControlMessage_make(Type_Pong, 0, 0, time);
}

bool TT_VISIBILITY_HIDDEN ttLibC_UserControlMessage_getData(
		ttLibC_UserControlMessage *user_control_message,
		ttLibC_DynamicBuffer *buffer) {
	uint16_t be_type = be_uint16_t(user_control_message->type);
	ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_type, 2);
	switch(user_control_message->type) {
	case Type_StreamBegin:
	case Type_StreamEof:
	case Type_StreamDry:
	case Type_RecordedStream:
	case Type_BufferEmpty:
	case Type_BufferFull:
		return false;

	case Type_ClientBufferLength:
		{
			uint32_t be_stream_id = be_uint32_t(user_control_message->stream_id);
			uint32_t be_buffer_length = be_uint32_t(user_control_message->buffer_length);
			ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_stream_id, 4);
			ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_buffer_length, 4);
		}
		return true;
	case Type_Ping:
	case Type_Pong:
		{
			uint32_t be_time = be_uint32_t(user_control_message->time);
			ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_time, 4);
		}
		return true;

	case Type_Unknown5:
	case Type_Unknown8:
	case Type_PingSwfVerification:
	case Type_PongSwfVerification:
		{
			ERR_PRINT("unknown user control message type:%d", user_control_message->type);
		}
		return false;
	}
	return true;
}

void TT_VISIBILITY_HIDDEN ttLibC_UserControlMessage_close(ttLibC_UserControlMessage **message) {
	ttLibC_UserControlMessage *target = (ttLibC_UserControlMessage *)*message;
	if(target == NULL) {
		return;
	}
	ttLibC_RtmpHeader_close(&target->inherit_super.header);
	ttLibC_free(target);
	*message = NULL;
}

#endif