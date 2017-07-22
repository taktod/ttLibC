/**
 * @file   rtmpCommandHandler.h
 * @brief  tetty handler for amf0/amf3command.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/07
 */

#ifndef TTLIBC_NET_CLIENT_RTMP_TETTY_RTMPCOMMANDHANDLER_H_
#define TTLIBC_NET_CLIENT_RTMP_TETTY_RTMPCOMMANDHANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../../rtmp.h"
#include "../../../tetty.h"

typedef struct ttLibC_Net_Client_Rtmp_Tetty_RtmpCommandHandler{
	ttLibC_TettyChannelHandler channel_handler;
} ttLibC_Net_Client_Rtmp_Tetty_RtmpCommandHandler;

typedef ttLibC_Net_Client_Rtmp_Tetty_RtmpCommandHandler ttLibC_RtmpCommandHandler;

ttLibC_RtmpCommandHandler *ttLibC_RtmpCommandHandler_make();
void ttLibC_RtmpCommandHandler_close(ttLibC_RtmpCommandHandler **handler);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_CLIENT_RTMP_TETTY_RTMPCOMMANDHANDLER_H_ */
