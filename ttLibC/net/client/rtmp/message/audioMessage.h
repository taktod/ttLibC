/**
 * @file   audioMessage.h
 * @brief  rtmp message AudioMessage
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/13
 */

#ifndef TTLIBC_NET_CLIENT_RTMP_MESSAGE_AUDIOMESSAGE_H_
#define TTLIBC_NET_CLIENT_RTMP_MESSAGE_AUDIOMESSAGE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "rtmpMessage.h"
#include "../../../../frame/audio/audio.h"
#include "../rtmpStream.h"

/*
 * same as flv.
 * header
 * 4bit codec
 * 2bit sample_rate index
 * 1bit channel
 * 1bit 8bit or 16bit
 * frame
 *
 * for aac
 * 1byte global header flag
 *  in the case of 0
 *  dsi info.
 *  in the case of 1
 *  aac raw frame.
 */
typedef struct ttLibC_Net_Client_Rtmp_Message_AudioMessage {
	ttLibC_RtmpMessage inherit_super;
	ttLibC_Audio *audio_frame;
	uint8_t *data;
	bool is_dsi_info;
} ttLibC_Net_Client_Rtmp_Message_AudioMessage;

typedef ttLibC_Net_Client_Rtmp_Message_AudioMessage ttLibC_AudioMessage;

ttLibC_AudioMessage *ttLibC_AudioMessage_make();

// for receive

ttLibC_AudioMessage *ttLibC_AudioMessage_readBinary(
		uint8_t *data,
		size_t data_size);

tetty_errornum ttLibC_AudioMessage_getFrame(
		ttLibC_AudioMessage *message,
		ttLibC_RtmpStream_ *stream);

// for send

ttLibC_AudioMessage *ttLibC_AudioMessage_addFrame(
		ttLibC_RtmpStream *stream,
		ttLibC_Audio *frame);

bool ttLibC_AudioMessage_getData(
		ttLibC_AudioMessage *message,
		ttLibC_DynamicBuffer *buffer);

void ttLibC_AudioMessage_close(ttLibC_AudioMessage **message);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_CLIENT_RTMP_MESSAGE_AUDIOMESSAGE_H_ */
