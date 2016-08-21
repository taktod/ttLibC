/**
 * @file   rtmpHeader.h
 * @brief  rtmpHeader
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/07
 */

#ifndef TTLIBC_NET_CLIENT_RTMP_HEADER_RTMPHEADER_H_
#define TTLIBC_NET_CLIENT_RTMP_HEADER_RTMPHEADER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../data/clientObject.h"

/*
 * data size max is 18byte.
 *
 * note
 *
 * 2bit type
 * 6bit cs_id or flag
 * if cs_id = 0
 *  8bit cs_id (64 - 319)
 * if cs_id = 1
 *  16bit cs_id (320 - 65855)
 * else
 *  cs_id (2 - 63)
 *
 * type0
 * 24bit timetsamp
 * 24bit data size
 * 8bit data type
 * 32bit stream_id(little endian)
 * if timestamp = 0xFFFFFF
 *  32bit timestamp
 *
 * type1
 * 24bit deltaTime
 * 24bit size
 * 8bit type
 * stream_id is the same as older one.
 * if deltaTime = 0xFFFFFF
 *  32bit deltaTime
 *
 * type2
 * 24bit deltatime
 * size type stream_id are the same as older one.
 * if deltaTime = 0xFFFFFF
 *  32bit deltaTime
 *
 * type3
 * deltatime size type stream_id are the same as older one.
 *
 *
 * cs_id:
 * system -> 2
 * command -> 3
 * media(?) -> 5
 * video -> 6
 * audio -> 7
 * aggregate -> 8
 */

#include "../../../../util/dynamicBufferUtil.h"
#include "../../../../util/stlMapUtil.h"
#include <stdio.h>
#include <stdint.h>

typedef enum ttLibC_RtmpHeader_Type {
	Type0 = 0,
	Type1 = 1,
	Type2 = 2,
	Type3 = 3
} ttLibC_RtmpHeader_Type;

typedef enum ttLibC_RtmpMessage_Type {
	RtmpMessageType_setChunkSize              = 0x01,
	RtmpMessageType_abortMessage              = 0x02,
	RtmpMessageType_acknowledgement           = 0x03,
	RtmpMessageType_userControlMessage        = 0x04,
	RtmpMessageType_windowAcknowledgementSize = 0x05,
	RtmpMessageType_setPeerBandwidth          = 0x06,
	RtmpMessageType_audioMessage              = 0x08,
	RtmpMessageType_videoMessage              = 0x09,
	RtmpMessageType_amf3DataMessage           = 0x0F,
	RtmpMessageType_amf3SharedObjectMessage   = 0x10,
	RtmpMessageType_amf3Command               = 0x11,
	RtmpMessageType_amf0DataMessage           = 0x12,
	RtmpMessageType_amf0SharedObjectMessage   = 0x13,
	RtmpMessageType_amf0Command               = 0x14,
	RtmpMessageType_aggregateMessage          = 0x16,
} ttLibC_RtmpMessage_Type;

typedef struct ttLibC_Net_Client_Rtmp_Header_RtmpHeader {
	ttLibC_RtmpHeader_Type type;
	uint32_t cs_id;

	uint64_t timestamp;
	uint32_t delta_time;
	uint32_t size;
	ttLibC_RtmpMessage_Type message_type;
	uint32_t stream_id;
} ttLibC_Net_Client_Rtmp_Header_RtmpHeader;

typedef ttLibC_Net_Client_Rtmp_Header_RtmpHeader ttLibC_RtmpHeader;

ttLibC_RtmpHeader *ttLibC_RtmpHeader_make(
		uint32_t cs_id,
		uint64_t timestamp,
		ttLibC_RtmpMessage_Type message_type,
		uint32_t stream_id);

/**
 * source -> target copy.
 * in the case of target = NULL, alloc header.
 */
ttLibC_RtmpHeader *ttLibC_RtmpHeader_copy(
		ttLibC_RtmpHeader *target,
		ttLibC_RtmpHeader *source);

size_t ttLibC_RtmpHeader_getData(
		ttLibC_RtmpHeader *header,
		void *data,
		size_t data_size);

ttLibC_RtmpHeader *ttLibC_RtmpHeader_readBinary(
		ttLibC_DynamicBuffer *buffer,
		ttLibC_ClientObject *client_object);

void ttLibC_RtmpHeader_close(ttLibC_RtmpHeader **header);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_CLIENT_RTMP_HEADER_RTMPHEADER_H_ */
