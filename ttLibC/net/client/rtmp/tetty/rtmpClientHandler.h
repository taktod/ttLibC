/**
 * @file   rtmpClientHandler.h
 * @brief  tetty handler for client work.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/07
 */

#ifndef TTLIBC_NET_CLIENT_RTMP_TETTY_RTMPCLIENTHANDLER_H_
#define TTLIBC_NET_CLIENT_RTMP_TETTY_RTMPCLIENTHANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../../rtmp.h"
#include "../../../tetty.h"
#include "../../../../util/stlMapUtil.h"

typedef struct ttLibC_Net_Client_Rtmp_Tetty_RtmpClientHandler{
	ttLibC_TettyChannelHandler channel_handler;
	uint32_t bytesReadWindow;
	uint32_t bytesWrittenWindow;
	uint64_t bytesRead;
	uint64_t bytesReadAcked;
} ttLibC_Net_Client_Rtmp_Tetty_RtmpClientHandler;

typedef ttLibC_Net_Client_Rtmp_Tetty_RtmpClientHandler ttLibC_RtmpClientHandler;

ttLibC_RtmpClientHandler *ttLibC_RtmpClientHandler_make();

void ttLibC_RtmpClientHandler_close(ttLibC_RtmpClientHandler **handler);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_CLIENT_RTMP_TETTY_RTMPCLIENTHANDLER_H_ */
