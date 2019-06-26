/*
 * acknowledgement.h
 *
 *  Created on: 2017/07/22
 *      Author: taktod
 */

#ifndef TTLIBC_NET_CLIENT_RTMP2_MESSAGE_ACKNOWLEDGEMENT_H_
#define TTLIBC_NET_CLIENT_RTMP2_MESSAGE_ACKNOWLEDGEMENT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../../../../ttLibC_predef.h"
#include <stdint.h>
#include "../../../../util/ioUtil.h"
#include "rtmpMessage.h"

/*
 * note:
 * header
 * 32bit size
 */

typedef struct ttLibC_Net_Client_Rtmp2_Message_Acknowledgement {
	ttLibC_RtmpMessage inherit_super;
	uint32_t size;
} ttLibC_Net_Client_Rtmp2_Message_Acknowledgement;

typedef ttLibC_Net_Client_Rtmp2_Message_Acknowledgement ttLibC_Acknowledgement;

ttLibC_Acknowledgement TT_ATTRIBUTE_INNER *ttLibC_Acknowledgement_make(uint32_t size);

bool TT_ATTRIBUTE_INNER ttLibC_Acknowledgement_getData(
		ttLibC_Acknowledgement *acknowledgement,
		ttLibC_DynamicBuffer *buffer);

void TT_ATTRIBUTE_INNER ttLibC_Acknowledgement_close(ttLibC_Acknowledgement **ack);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_CLIENT_RTMP2_MESSAGE_ACKNOWLEDGEMENT_H_ */
