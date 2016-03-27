/**
 * @file   setChunkSize.h
 * @brief  rtmp message setChunkSize
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/07
 */

#ifndef TTLIBC_NET_CLIENT_RTMP_MESSAGE_SETCHUNKSIZE_H_
#define TTLIBC_NET_CLIENT_RTMP_MESSAGE_SETCHUNKSIZE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "rtmpMessage.h"

/**
 * header
 * 32bit chunkSize
 */

typedef struct ttLibC_Net_Client_Rtmp_Message_SetChunkSize {
	ttLibC_RtmpMessage inherit_super;
	uint32_t size;
} ttLibC_Net_Client_Rtmp_Message_SetChunkSize;

typedef ttLibC_Net_Client_Rtmp_Message_SetChunkSize ttLibC_SetChunkSize;

ttLibC_SetChunkSize *ttLibC_SetChunkSize_make(uint32_t size);

void ttLibC_SetChunkSize_close(ttLibC_SetChunkSize **chunk_size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_CLIENT_RTMP_MESSAGE_SETCHUNKSIZE_H_ */
