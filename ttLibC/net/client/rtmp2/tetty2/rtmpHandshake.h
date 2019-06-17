/*
 * rtmpHandshake.h
 *
 *  Created on: 2017/07/21
 *      Author: taktod
 */

#ifndef TTLIBC_NET_CLIENT_RTMP2_TETTY2_RTMPHANDSHAKE_H_
#define TTLIBC_NET_CLIENT_RTMP2_TETTY2_RTMPHANDSHAKE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../../../../ttLibC_predef.h"
#include "../../rtmp.h"
#include "../../../../util/tetty2.h"

typedef struct ttLibC_Net_Client_Rtmp2_Tetty2_RtmpHandshake{
	ttLibC_Tetty2ChannelHandler channel_handler;
	ttLibC_Tetty2Promise *handshake_promise;
} ttLibC_Net_Client_Rtmp2_Tetty2_RtmpHandshake;

typedef ttLibC_Net_Client_Rtmp2_Tetty2_RtmpHandshake ttLibC_RtmpHandshake;

ttLibC_RtmpHandshake TT_ATTRIBUTE_INNER *ttLibC_RtmpHandshake_make();
ttLibC_Tetty2Promise TT_ATTRIBUTE_INNER *ttLibC_RtmpHandshake_getHandshakePromise(
		ttLibC_Tetty2Bootstrap *bootstrap,
		ttLibC_RtmpHandshake *handshake);
void TT_ATTRIBUTE_INNER ttLibC_RtmpHandshake_close(ttLibC_RtmpHandshake **handshake);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_CLIENT_RTMP2_TETTY2_RTMPHANDSHAKE_H_ */
