/**
 * @file   rtmpMessage.h
 * @brief  rtmp message
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/08
 */

#ifndef TTLIBC_NET_CLIENT_RTMP_MESSAGE_RTMPMESSAGE_H_
#define TTLIBC_NET_CLIENT_RTMP_MESSAGE_RTMPMESSAGE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../header/rtmpHeader.h"
#include "../../../../util/dynamicBufferUtil.h"
#include "../../../../util/stlMapUtil.h"

/**
 * definition of rtmpMessage
 */
typedef struct ttLibC_Net_Client_Rtmp_Message_RtmpMessage {
	ttLibC_RtmpHeader *header;
} ttLibC_Net_Client_Rtmp_Message_RtmpMessage;

typedef ttLibC_Net_Client_Rtmp_Message_RtmpMessage ttLibC_RtmpMessage;

bool ttLibC_RtmpMessage_getData(
		ttLibC_ClientObject *client_object,
		ttLibC_RtmpMessage *message,
		ttLibC_DynamicBuffer *buffer);

ttLibC_RtmpMessage *ttLibC_RtmpMessage_readBinary(
		ttLibC_DynamicBuffer *buffer,
		ttLibC_ClientObject *client_object);

void ttLibC_RtmpMessage_close(ttLibC_RtmpMessage **message);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_CLIENT_RTMP_MESSAGE_RTMPMESSAGE_H_ */
