/*
 * userControlMessage.h
 *
 *  Created on: 2017/07/22
 *      Author: taktod
 */

#ifndef TTLIBC_NET_CLIENT_RTMP2_MESSAGE_USERCONTROLMESSAGE_H_
#define TTLIBC_NET_CLIENT_RTMP2_MESSAGE_USERCONTROLMESSAGE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../../../../ttLibC_predef.h"
#include <stdint.h>
#include <stdbool.h>
#include "rtmpMessage.h"

typedef enum ttLibC_UserControlMessage_Type {
	Type_StreamBegin = 0,
	Type_StreamEof = 1,
	Type_StreamDry = 2,
	Type_RecordedStream = 4,
	Type_BufferEmpty = 31,
	Type_BufferFull = 32,

	Type_ClientBufferLength = 3,
	Type_Unknown5 = 5,
	Type_Ping = 6,
	Type_Pong = 7,
	Type_Unknown8 = 8,
	Type_PingSwfVerification = 26,
	Type_PongSwfVerification = 27
} ttLibC_UserControlMessage_Type;

/*
 * 16bit type
 *  0:StreamBegin
 *  1:StreamEof
 *  2:StreamDry
 *  4:RecordedStream
 *  31:BufferEmpty
 *  32:BufferFull
 *   32bit streamId
 *
 *  3:ClientBufferLength
 *   32bit streamId
 *   32bit bufferLength
 *
 *  5:Unknown
 *
 *  6:Ping
 *  7:Pong
 *   32bit time
 *
 *  8:Unkwown
 *
 *  26:PingSwfVerification
 *  27:PongSwfVerification
 *   ????
 */
typedef struct ttLibC_Net_Client_Rtmp2_Message_UserControlMessage {
	ttLibC_RtmpMessage inherit_super;
	ttLibC_UserControlMessage_Type type;
	uint32_t stream_id;
	uint32_t buffer_length;
	uint32_t time;
} ttLibC_Net_Client_Rtmp2_Message_UserControlMessage;

typedef ttLibC_Net_Client_Rtmp2_Message_UserControlMessage ttLibC_UserControlMessage;

ttLibC_UserControlMessage TT_ATTRIBUTE_INNER *ttLibC_UserControlMessage_make(
		ttLibC_UserControlMessage_Type type,
		uint32_t stream_id,
		uint32_t buffer_length,
		uint32_t time);

ttLibC_UserControlMessage TT_ATTRIBUTE_INNER *ttLibC_UserControlMessage_readBinary(
		uint8_t *data,
		size_t data_size);

ttLibC_UserControlMessage TT_ATTRIBUTE_INNER *ttLibC_UserControlMessage_ping(uint32_t time);

ttLibC_UserControlMessage TT_ATTRIBUTE_INNER *ttLibC_UserControlMessage_pong(uint32_t time);

bool TT_ATTRIBUTE_INNER ttLibC_UserControlMessage_getData(
		ttLibC_UserControlMessage *user_control_message,
		ttLibC_DynamicBuffer *buffer);

void TT_ATTRIBUTE_INNER ttLibC_UserControlMessage_close(ttLibC_UserControlMessage **message);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_CLIENT_RTMP2_MESSAGE_USERCONTROLMESSAGE_H_ */
