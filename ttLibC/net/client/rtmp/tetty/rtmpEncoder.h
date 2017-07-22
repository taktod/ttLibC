/**
 * @file   rtmpEncoder.h
 * @brief  tetty handler for rtmpEncoder.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/07
 */

#ifndef TTLIBC_NET_CLIENT_RTMP_TETTY_RTMPENCODER_H_
#define TTLIBC_NET_CLIENT_RTMP_TETTY_RTMPENCODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../../rtmp.h"
#include "../../../tetty.h"

typedef struct ttLibC_Net_Client_Rtmp_Tetty_RtmpEncoder{
	ttLibC_TettyChannelHandler channel_handler;
} ttLibC_Net_Client_Rtmp_Tetty_RtmpEncoder;

typedef ttLibC_Net_Client_Rtmp_Tetty_RtmpEncoder ttLibC_RtmpEncoder;

ttLibC_RtmpEncoder *ttLibC_RtmpEncoder_make();
void ttLibC_RtmpEncoder_close(ttLibC_RtmpEncoder **encoder);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_CLIENT_RTMP_TETTY_RTMPENCODER_H_ */
