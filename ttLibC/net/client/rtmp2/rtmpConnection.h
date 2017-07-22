/*
 * rtmpConnection.h
 *
 *  Created on: 2017/07/21
 *      Author: taktod
 */

#ifndef TTLIBC_NET_CLIENT_RTMP2_RTMPCONNECTION_H_
#define TTLIBC_NET_CLIENT_RTMP2_RTMPCONNECTION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../rtmp.h"
#include "../../../util/tetty2.h"
#include "tetty2/rtmpClientHandler.h"
#include "tetty2/rtmpCommandHandler.h"
#include "tetty2/rtmpDecoder.h"
#include "tetty2/rtmpEncoder.h"
#include "tetty2/rtmpHandshake.h"

/**
 * detail definition of rtmpConnection
 */
typedef struct ttLibC_Net_Client_Rtmp2_RtmpConnection_ {
	ttLibC_RtmpConnection inherit_super;

	// bootstrap
	ttLibC_Tetty2Bootstrap *bootstrap;

	// handlers
	ttLibC_RtmpHandshake *handshake;
	ttLibC_RtmpDecoder *decoder;
	ttLibC_RtmpEncoder *encoder;
	ttLibC_RtmpCommandHandler *command_handler;
	ttLibC_RtmpClientHandler *client_handler;

	// event listener callback.
	ttLibC_RtmpEventFunc callback;
	void *ptr;
} ttLibC_Net_Client_Rtmp2_RtmpConnection_;

typedef ttLibC_Net_Client_Rtmp2_RtmpConnection_ ttLibC_RtmpConnection_;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_CLIENT_RTMP2_RTMPCONNECTION_H_ */
