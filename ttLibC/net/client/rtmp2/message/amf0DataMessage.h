/*
 * amf0DataMessage.h
 *
 *  Created on: 2017/07/22
 *      Author: taktod
 */

#ifndef TTLIBC_NET_CLIENT_RTMP2_MESSAGE_AMF0DATAMESSAGE_H_
#define TTLIBC_NET_CLIENT_RTMP2_MESSAGE_AMF0DATAMESSAGE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "../../../../util/amfUtil.h"
#include "rtmpMessage.h"

/*
 * unknown...
 * String
 * amf0Object
 * amf0Object
 * ?
 */

typedef struct ttLibC_Net_Client_Rtmp2_Message_Amf0DataMessage {
	ttLibC_RtmpMessage inherit_super;
	char *message_name[256];
	ttLibC_Amf0Object *obj1;
	ttLibC_Amf0Object *obj2;
} ttLibC_Net_Client_Rtmp2_Message_Amf0DataMessage;

typedef ttLibC_Net_Client_Rtmp2_Message_Amf0DataMessage ttLibC_Amf0DataMessage;

ttLibC_Amf0DataMessage *ttLibC_Amf0DataMessage_make(const char *message_name);

ttLibC_Amf0DataMessage *ttLibC_Amf0DataMessage_readBinary(
		uint8_t *data,
		size_t data_size);

void ttLibC_Amf0DataMessage_close(ttLibC_Amf0DataMessage **message);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_CLIENT_RTMP2_MESSAGE_AMF0DATAMESSAGE_H_ */
