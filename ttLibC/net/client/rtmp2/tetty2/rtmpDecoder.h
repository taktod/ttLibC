/*
 * rtmpDecoder.h
 *
 *  Created on: 2017/07/21
 *      Author: taktod
 */

#ifndef TTLIBC_NET_CLIENT_RTMP2_TETTY2_RTMPDECODER_H_
#define TTLIBC_NET_CLIENT_RTMP2_TETTY2_RTMPDECODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../../../../ttLibC_predef.h"
#include "../../rtmp.h"
#include "../../../../util/tetty2.h"

typedef struct ttLibC_Net_Client_Rtmp2_Tetty2_RtmpDecoder{
	ttLibC_Tetty2ChannelHandler channel_handler;
} ttLibC_Net_Client_Rtmp2_Tetty2_RtmpDecoder;

typedef ttLibC_Net_Client_Rtmp2_Tetty2_RtmpDecoder ttLibC_RtmpDecoder;

ttLibC_RtmpDecoder TT_ATTRIBUTE_INNER *ttLibC_RtmpDecoder_make();
void TT_ATTRIBUTE_INNER ttLibC_RtmpDecoder_close(ttLibC_RtmpDecoder **decoder);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_CLIENT_RTMP2_TETTY2_RTMPDECODER_H_ */
