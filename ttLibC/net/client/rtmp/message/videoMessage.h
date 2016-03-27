/**
 * @file   videoMessage.h
 * @brief  rtmp message videoMessage
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/13
 */

#ifndef TTLIBC_NET_CLIENT_RTMP_MESSAGE_VIDEOMESSAGE_H_
#define TTLIBC_NET_CLIENT_RTMP_MESSAGE_VIDEOMESSAGE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "rtmpMessage.h"
#include "../header/rtmpHeader.h"
#include "../../../../frame/video/video.h"

/*
 * header
 * 4bit keyFrame flag 1:keyFrame 2:innerFrame 3:disposable inner(for flv1 only)
 * 4bit video Codec
 * for avc
 *  8bit flag 0:avcc header 1:frame 2:end of stream
 *  24bit dts
 *  avc
 */
typedef struct ttLibC_Net_Client_Rtmp_Message_VideoMessage {
	ttLibC_RtmpMessage inherit_super;
	ttLibC_Video *video_frame;
} ttLibC_Net_Client_Rtmp_Message_VideoMessage;

typedef ttLibC_Net_Client_Rtmp_Message_VideoMessage ttLibC_VideoMessage;

ttLibC_VideoMessage *ttLibC_VideoMessage_make();
ttLibC_VideoMessage *ttLibC_VideoMessage_addFrame(
		ttLibC_RtmpStream *stream,
		ttLibC_Video *frame);

ttLibC_VideoMessage *ttLibC_VideoMessage_readBinary(
		ttLibC_RtmpHeader *header,
		ttLibC_RtmpStream *stream,
		uint8_t *data,
		size_t data_size);

bool ttLibC_VideoMessage_getData(
		ttLibC_VideoMessage *message,
		ttLibC_DynamicBuffer *buffer);

void ttLibC_VideoMessage_close(ttLibC_VideoMessage **message);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_CLIENT_RTMP_MESSAGE_VIDEOMESSAGE_H_ */
