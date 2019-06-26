/**
 * @file   setPeerBandwidth.h
 * @brief  rtmp message setPeerBandwidth
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/07
 */

#ifndef TTLIBC_NET_CLIENT_RTMP_MESSAGE_SETPEERBANDWIDTH_H_
#define TTLIBC_NET_CLIENT_RTMP_MESSAGE_SETPEERBANDWIDTH_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../../../../ttLibC_predef.h"
#include <stdint.h>
#include "rtmpMessage.h"

typedef enum ttLibC_SetPeerBandwidth_LimitType{
	LimitType_Hard = 0,
	LimitType_Soft = 1,
	LimitType_Dynamic = 2
} ttLibC_SetPeerBandwidth_LimitType;

/*
 * 32bit ack size
 * 8bit limit type. 0:hard 1:soft 2:dynamic
 */
typedef struct ttLibC_Net_Client_Rtmp_Message_SetPeerBandwidth {
	ttLibC_RtmpMessage inherit_super;
	uint32_t size;
	ttLibC_SetPeerBandwidth_LimitType limit_type;
} ttLibC_Net_Client_Rtmp_Message_SetPeerBandwidth;

typedef ttLibC_Net_Client_Rtmp_Message_SetPeerBandwidth ttLibC_SetPeerBandwidth;

ttLibC_SetPeerBandwidth TT_ATTRIBUTE_INNER *ttLibC_SetPeerBandwidth_make(
		uint32_t size,
		ttLibC_SetPeerBandwidth_LimitType limit_type);

void TT_ATTRIBUTE_INNER ttLibC_SetPeerBandwidth_close(ttLibC_SetPeerBandwidth **bandwidth);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_CLIENT_RTMP_MESSAGE_SETPEERBANDWIDTH_H_ */
