/*
 * aggregateMessage.h
 *
 *  Created on: 2017/07/22
 *      Author: taktod
 */

#ifndef TTLIBC_NET_CLIENT_RTMP2_MESSAGE_AGGREGATEMESSAGE_H_
#define TTLIBC_NET_CLIENT_RTMP2_MESSAGE_AGGREGATEMESSAGE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../../../../ttLibC_predef.h"
#include <stdint.h>
#include "rtmpMessage.h"
#include "../../../../container/container.h"
#include "../../../../util/tetty2.h"
#include "../../../../util/flvFrameUtil.h"

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
typedef struct ttLibC_Net_Client_Rtmp2_Message_AggregateMessage {
	ttLibC_RtmpMessage inherit_super;
	uint8_t *data;
} ttLibC_Net_Client_Rtmp2_Message_AggregateMessage;

typedef ttLibC_Net_Client_Rtmp2_Message_AggregateMessage ttLibC_AggregateMessage;

ttLibC_AggregateMessage TT_ATTRIBUTE_INNER *ttLibC_AggregateMessage_make();
ttLibC_AggregateMessage TT_ATTRIBUTE_INNER *ttLibC_AggregateMessage_readBinary(uint8_t *data);
tetty2_errornum TT_ATTRIBUTE_INNER ttLibC_AggregateMessage_getFrame(
		ttLibC_AggregateMessage *message,
		ttLibC_FlvFrameManager *manager,
		ttLibC_RtmpStream_getFrameFunc callback,
		void *ptr);
void TT_ATTRIBUTE_INNER ttLibC_AggregateMessage_close(ttLibC_AggregateMessage **message);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_CLIENT_RTMP2_MESSAGE_AGGREGATEMESSAGE_H_ */
