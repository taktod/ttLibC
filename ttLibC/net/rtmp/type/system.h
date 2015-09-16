/**
 * @file   system.h
 * @brief  
 *
 * This code is under 3-Cause BSD license
 *
 * @author taktod
 * @date   2015/09/06
 */

#ifndef TTLIBC_NET_RTMP_MESSAGE_SYSTEM_H_
#define TTLIBC_NET_RTMP_MESSAGE_SYSTEM_H_

#ifdef __cplusplus
extern "C" {
#endif

// まずはsystemからつくる必要があるか・・・
#include "../message.h"

typedef ttLibC_Rtmp4ByteMessage ttLibC_RtmpWindowAcknowledgementSize;
typedef ttLibC_Rtmp4ByteMessage ttLibC_RtmpSetChunkSize;
typedef ttLibC_Rtmp4ByteMessage ttLibC_RtmpAcknowledgement;

typedef struct {
	ttLibC_RtmpMessage inherit_super;
	uint32_t window_acknowledge_size;
	uint8_t type;
} ttLibC_RtmpSetPeerBandwidth;

typedef enum {
	RtmpEventType_StreamBegin = 0,
	RtmpEventType_StreamEof = 1,
	RtmpEventType_StreamDry = 2,
	RtmpEventType_ClientBufferLength = 3,
	RtmpEventType_RecordedStreamBegin = 4,
	RtmpEventType_Unknown5 = 5,
	RtmpEventType_Ping = 6,
	RtmpEventType_Pong = 7,
	RtmpEventType_Unknown8 = 8,
	RtmpEventType_PingSwfVerification = 26,
	RtmpEventType_PongSwfVerification = 27,
	RtmpEventType_BufferEmpty = 31,
	RtmpEventType_BufferFull = 32,
} ttLibC_RtmpUserControlMessage_EventType;

typedef struct {
	ttLibC_RtmpMessage inheerit_super;
	ttLibC_RtmpUserControlMessage_EventType event_type;
	uint32_t value;
	uint32_t buffer_length;
} ttLibC_RtmpUserControlMessage;

// ping用のpongを応答するのにつかう予定。
ttLibC_RtmpMessage *ttLibC_RtmpSystemMessage_userControlMessage(
		ttLibC_RtmpConnection *conn,
		ttLibC_RtmpUserControlMessage_EventType event_type);

bool ttLibC_RtmpSystemMessage_sendPong(
		ttLibC_RtmpConnection *conn,
		uint32_t time,
		ttLibC_RtmpDataWriteFunc callback,
		void *ptr);

bool ttLibC_RtmpSystemMessage_sendAcknowledgement(
		ttLibC_RtmpConnection *conn,
		ttLibC_RtmpDataWriteFunc callback,
		void *ptr);

void ttLibC_RtmpSystemMessage_close(ttLibC_RtmpMessage **message);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_RTMP_MESSAGE_SYSTEM_H_ */
