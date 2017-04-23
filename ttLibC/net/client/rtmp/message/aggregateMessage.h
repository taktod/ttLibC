/**
 * @file   aggregateMessage.h
 * @brief  rtmp message aggregateMessage (frame information.)
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/07
 */

#ifndef TTLIBC_NET_CLIENT_RTMP_MESSAGE_AGGREGATEMESSAGE_H_
#define TTLIBC_NET_CLIENT_RTMP_MESSAGE_AGGREGATEMESSAGE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "rtmpMessage.h"
#include "../../../../container/container.h"
#include "../rtmpStream.h"

/*
 * header
 *
 * 8bit type
 * 24bit datasize
 * 24bit timestamp
 * 8bit timestamp ext
 * frame binary.
 *  8bit tag type
 *  24bit size
 *  24bit timestamp
 *  8bit timestamp ext
 *  24bit stream_id
 *  data
 *  24bit prev_size
 *
 * @see myLib/myLib.LGPLv3/myLib.flazr/src/main/java/com/ttProject/flazr/unit/MessageManager.java
 */
typedef struct ttLibC_Net_Client_Rtmp_Message_AggregateMessage {
	ttLibC_RtmpMessage inherit_super;
	uint8_t *data;
} ttLibC_Net_Client_Rtmp_Message_AggregateMessage;

typedef ttLibC_Net_Client_Rtmp_Message_AggregateMessage ttLibC_AggregateMessage;

ttLibC_AggregateMessage *ttLibC_AggregateMessage_make();
ttLibC_AggregateMessage *ttLibC_AggregateMessage_readBinary(uint8_t *data);
tetty_errornum ttLibC_AggregateMessage_getFrame(
		ttLibC_AggregateMessage *message,
		ttLibC_FlvFrameManager *manager,
		ttLibC_RtmpStream_getFrameFunc callback,
		void *ptr);
void ttLibC_AggregateMessage_close(ttLibC_AggregateMessage **message);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_CLIENT_RTMP_MESSAGE_AGGREGATEMESSAGE_H_ */
