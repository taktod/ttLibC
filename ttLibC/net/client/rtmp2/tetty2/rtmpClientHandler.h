/*
 * rtmpClientHandler.h
 *
 *  Created on: 2017/07/21
 *      Author: taktod
 */

#ifndef TTLIBC_NET_CLIENT_RTMP2_TETTY2_RTMPCLIENTHANDLER_H_
#define TTLIBC_NET_CLIENT_RTMP2_TETTY2_RTMPCLIENTHANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../../rtmp.h"
#include "../../../../util/tetty2.h"
#include "../../../../util/stlMapUtil.h"

typedef struct ttLibC_Net_Client_Rtmp2_Tetty2_RtmpClientHandler{
	ttLibC_Tetty2ChannelHandler channel_handler;
	uint32_t bytesReadWindow;
	uint32_t bytesWrittenWindow;
	uint64_t bytesRead;
	uint64_t bytesReadAcked;
} ttLibC_Net_Client_Rtmp2_Tetty2_RtmpClientHandler;

typedef ttLibC_Net_Client_Rtmp2_Tetty2_RtmpClientHandler ttLibC_RtmpClientHandler;

ttLibC_RtmpClientHandler *ttLibC_RtmpClientHandler_make();

void ttLibC_RtmpClientHandler_close(ttLibC_RtmpClientHandler **handler);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_CLIENT_RTMP2_TETTY2_RTMPCLIENTHANDLER_H_ */
