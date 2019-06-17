/*
 * rtmpMessage.h
 *
 *  Created on: 2017/07/22
 *      Author: taktod
 */

#ifndef TTLIBC_NET_CLIENT_RTMP2_MESSAGE_RTMPMESSAGE_H_
#define TTLIBC_NET_CLIENT_RTMP2_MESSAGE_RTMPMESSAGE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../../../../ttLibC_predef.h"
#include "../header/rtmpHeader.h"
#include "../../../../util/dynamicBufferUtil.h"
#include "../../../../util/stlMapUtil.h"

/**
 * definition of rtmpMessage
 */
typedef struct ttLibC_Net_Client_Rtmp2_Message_RtmpMessage {
	ttLibC_RtmpHeader *header;
} ttLibC_Net_Client_Rtmp2_Message_RtmpMessage;

typedef ttLibC_Net_Client_Rtmp2_Message_RtmpMessage ttLibC_RtmpMessage;

bool TT_ATTRIBUTE_INNER ttLibC_RtmpMessage_getData(
		ttLibC_ClientObject *client_object,
		ttLibC_RtmpMessage *message,
		ttLibC_DynamicBuffer *buffer);

ttLibC_RtmpMessage TT_ATTRIBUTE_INNER *ttLibC_RtmpMessage_readBinary(
		ttLibC_DynamicBuffer *buffer,
		ttLibC_ClientObject *client_object);

void TT_ATTRIBUTE_INNER ttLibC_RtmpMessage_close(ttLibC_RtmpMessage **message);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_CLIENT_RTMP2_MESSAGE_RTMPMESSAGE_H_ */
