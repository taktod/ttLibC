/**
 * @file   rtmpDecoder.h
 * @brief  tetty handler for rtmpDecoder.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/07
 */

#ifndef TTLIBC_NET_CLIENT_RTMP_TETTY_RTMPDECODER_H_
#define TTLIBC_NET_CLIENT_RTMP_TETTY_RTMPDECODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../../rtmp.h"
#include "../../../tetty.h"

typedef struct ttLibC_Net_Client_Rtmp_Tetty_RtmpDecoder{
	ttLibC_TettyChannelHandler channel_handler;
} ttLibC_Net_Client_Rtmp_Tetty_RtmpDecoder;

typedef ttLibC_Net_Client_Rtmp_Tetty_RtmpDecoder ttLibC_RtmpDecoder;

ttLibC_RtmpDecoder *ttLibC_RtmpDecoder_make();
void ttLibC_RtmpDecoder_close(ttLibC_RtmpDecoder **decoder);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_CLIENT_RTMP_TETTY_RTMPDECODER_H_ */
