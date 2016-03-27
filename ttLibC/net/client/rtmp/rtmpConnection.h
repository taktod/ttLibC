/**
 * @file   rtmpConnection.h
 * @brief  rtmpConnection detail definition.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/06
 */

#ifndef TTLIBC_NET_CLIENT_RTMP_RTMPCONNECTION_H_
#define TTLIBC_NET_CLIENT_RTMP_RTMPCONNECTION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../rtmp.h"
#include "tetty/rtmpClientHandler.h"
#include "tetty/rtmpCommandHandler.h"
#include "tetty/rtmpDecoder.h"
#include "tetty/rtmpEncoder.h"
#include "tetty/rtmpHandshake.h"

/**
 * detail definition of rtmpConnection
 */
typedef struct ttLibC_Net_Client_Rtmp_RtmpConnection_ {
	ttLibC_RtmpConnection inherit_super;

	// bootstrap
	ttLibC_TettyBootstrap *bootstrap;

	// handlers
	ttLibC_RtmpHandshake *handshake;
	ttLibC_RtmpDecoder *decoder;
	ttLibC_RtmpEncoder *encoder;
	ttLibC_RtmpCommandHandler *command_handler;
	ttLibC_RtmpClientHandler *client_handler;

	// event listener callback.
	ttLibC_RtmpEventFunc callback;
	void *ptr;
} ttLibC_Net_Client_Rtmp_RtmpConnection_;

typedef ttLibC_Net_Client_Rtmp_RtmpConnection_ ttLibC_RtmpConnection_;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_CLIENT_RTMP_RTMPCONNECTION_H_ */
