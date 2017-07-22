/*
 * rtmpCommandHandler.h
 *
 *  Created on: 2017/07/21
 *      Author: taktod
 */

#ifndef TTLIBC_NET_CLIENT_RTMP2_TETTY2_RTMPCOMMANDHANDLER_H_
#define TTLIBC_NET_CLIENT_RTMP2_TETTY2_RTMPCOMMANDHANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../../rtmp.h"
#include "../../../../util/tetty2.h"

typedef struct ttLibC_Net_Client_Rtmp2_Tetty2_RtmpCommandHandler{
	ttLibC_Tetty2ChannelHandler channel_handler;
} ttLibC_Net_Client_Rtmp2_Tetty2_RtmpCommandHandler;

typedef ttLibC_Net_Client_Rtmp2_Tetty2_RtmpCommandHandler ttLibC_RtmpCommandHandler;

ttLibC_RtmpCommandHandler *ttLibC_RtmpCommandHandler_make();
void ttLibC_RtmpCommandHandler_close(ttLibC_RtmpCommandHandler **handler);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_CLIENT_RTMP2_TETTY2_RTMPCOMMANDHANDLER_H_ */
