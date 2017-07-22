/**
 * @file   rtmpHandshake.h
 * @brief  tetty handler for rtmpHandshake.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/07
 */

#ifndef TTLIBC_NET_CLIENT_RTMP_TETTY_RTMPHANDSHAKE_H_
#define TTLIBC_NET_CLIENT_RTMP_TETTY_RTMPHANDSHAKE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../../rtmp.h"
#include "../../../tetty.h"

typedef struct ttLibC_Net_Client_Rtmp_Tetty_RtmpHandshake{
	ttLibC_TettyChannelHandler channel_handler;
	ttLibC_TettyPromise *handshake_promise;
} ttLibC_Net_Client_Rtmp_Tetty_RtmpHandshake;

typedef ttLibC_Net_Client_Rtmp_Tetty_RtmpHandshake ttLibC_RtmpHandshake;

ttLibC_RtmpHandshake *ttLibC_RtmpHandshake_make();
ttLibC_TettyPromise *ttLibC_RtmpHandshake_getHandshakePromise(
		ttLibC_TettyBootstrap *bootstrap,
		ttLibC_RtmpHandshake *handshake);
void ttLibC_RtmpHandshake_close(ttLibC_RtmpHandshake **handshake);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_CLIENT_RTMP_TETTY_RTMPHANDSHAKE_H_ */
