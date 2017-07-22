/*
 * setChunkSize.h
 *
 *  Created on: 2017/07/22
 *      Author: taktod
 */

#ifndef TTLIBC_NET_CLIENT_RTMP2_MESSAGE_SETCHUNKSIZE_H_
#define TTLIBC_NET_CLIENT_RTMP2_MESSAGE_SETCHUNKSIZE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "rtmpMessage.h"

/**
 * header
 * 32bit chunkSize
 */

typedef struct ttLibC_Net_Client_Rtmp2_Message_SetChunkSize {
	ttLibC_RtmpMessage inherit_super;
	uint32_t size;
} ttLibC_Net_Client_Rtmp2_Message_SetChunkSize;

typedef ttLibC_Net_Client_Rtmp2_Message_SetChunkSize ttLibC_SetChunkSize;

ttLibC_SetChunkSize *ttLibC_SetChunkSize_make(uint32_t size);

void ttLibC_SetChunkSize_close(ttLibC_SetChunkSize **chunk_size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_CLIENT_RTMP2_MESSAGE_SETCHUNKSIZE_H_ */
