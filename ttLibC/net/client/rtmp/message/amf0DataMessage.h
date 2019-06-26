/**
 * @file   amf0DataMessage.h
 * @brief  rtmp message amf0Data message.(meta data?)
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/07
 */

#ifndef TTLIBC_NET_CLIENT_RTMP_MESSAGE_AMF0DATAMESSAGE_H_
#define TTLIBC_NET_CLIENT_RTMP_MESSAGE_AMF0DATAMESSAGE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../../../../ttLibC_predef.h"
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

typedef struct ttLibC_Net_Client_Rtmp_Message_Amf0DataMessage {
	ttLibC_RtmpMessage inherit_super;
	char *message_name[256];
	ttLibC_Amf0Object *obj1;
	ttLibC_Amf0Object *obj2;
} ttLibC_Net_Client_Rtmp_Message_Amf0DataMessage;

typedef ttLibC_Net_Client_Rtmp_Message_Amf0DataMessage ttLibC_Amf0DataMessage;

ttLibC_Amf0DataMessage TT_ATTRIBUTE_INNER *ttLibC_Amf0DataMessage_make(const char *message_name);

ttLibC_Amf0DataMessage TT_ATTRIBUTE_INNER *ttLibC_Amf0DataMessage_readBinary(
		uint8_t *data,
		size_t data_size);

void TT_ATTRIBUTE_INNER ttLibC_Amf0DataMessage_close(ttLibC_Amf0DataMessage **message);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_CLIENT_RTMP_MESSAGE_AMF0DATAMESSAGE_H_ */
