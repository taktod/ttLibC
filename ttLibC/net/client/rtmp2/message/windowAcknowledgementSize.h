/*
 * windowAcknowledgementSize.h
 *
 *  Created on: 2017/07/22
 *      Author: taktod
 */

#ifndef TTLIBC_NET_CLIENT_RTMP2_MESSAGE_WINDOWACKNOWLEDGEMENTSIZE_H_
#define TTLIBC_NET_CLIENT_RTMP2_MESSAGE_WINDOWACKNOWLEDGEMENTSIZE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "rtmpMessage.h"
#include <stdbool.h>
/*
 * 32bit size
 */
typedef struct ttLibC_Net_Client_Rtmp2_Message_WindowAcknowledgementSize {
	ttLibC_RtmpMessage inherit_super;
	uint32_t size;
} ttLibC_Net_Client_Rtmp2_Message_WindowAcknowledgementSize;

typedef ttLibC_Net_Client_Rtmp2_Message_WindowAcknowledgementSize ttLibC_WindowAcknowledgementSize;

ttLibC_WindowAcknowledgementSize *ttLibC_WindowAcknowledgementSize_make(uint32_t size);

bool ttLibC_WindowAcknowledgementSize_getData(
		ttLibC_WindowAcknowledgementSize *win_ack,
		ttLibC_DynamicBuffer *buffer);

void ttLibC_WindowAcknowledgementSize_close(ttLibC_WindowAcknowledgementSize **win_ack);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_CLIENT_RTMP2_MESSAGE_WINDOWACKNOWLEDGEMENTSIZE_H_ */
