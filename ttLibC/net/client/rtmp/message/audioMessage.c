/**
 * @file   audioMessage.c
 * @brief  rtmp message AudioMessage
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/13
 */

#include "audioMessage.h"
#include "../../../../log.h"
#include "../../../../allocator.h"
#include "../rtmpStream.h"
#include <string.h>
#include "../../../../util/hexUtil.h"
#include "../../../../frame/audio/aac.h"
#include "../../../../frame/audio/mp3.h"
#include "../../../../util/ioUtil.h"
#include "../../../../util/flvFrameUtil.h"

ttLibC_AudioMessage *ttLibC_AudioMessage_make() {
	ttLibC_AudioMessage *message = ttLibC_malloc(sizeof(ttLibC_AudioMessage));
	if(message == NULL) {
		return NULL;
	}
	ttLibC_RtmpHeader *header = ttLibC_RtmpHeader_make(7, 0, RtmpMessageType_audioMessage, 0);
	if(header == NULL) {
		ttLibC_free(message);
		return NULL;
	}
	message->inherit_super.header = header;
	message->audio_frame = NULL;
	message->is_dsi_info = false;
	return message;
}
ttLibC_AudioMessage *ttLibC_AudioMessage_addFrame(
		ttLibC_RtmpStream *stream,
		ttLibC_Audio *frame) {
	ttLibC_RtmpStream_ *stream_ = (ttLibC_RtmpStream_ *)stream;
	if(stream == NULL) {
		ERR_PRINT("need rtmpStream object.");
		return NULL;
	}
	if(frame == NULL) {
		return NULL;
	}
	ttLibC_AudioMessage *message = ttLibC_AudioMessage_make();
	if(message == NULL) {
		ERR_PRINT("failed to allocate message object.");
		return NULL;
	}
	message->audio_frame = frame;
	// update pts and stream_id
	message->inherit_super.header->timestamp = message->audio_frame->inherit_super.pts * 1000 / message->audio_frame->inherit_super.timebase;
	message->inherit_super.header->stream_id = stream_->stream_id;
	return message;
}

ttLibC_AudioMessage *ttLibC_AudioMessage_readBinary(
		ttLibC_RtmpHeader *header,
		ttLibC_RtmpStream *stream,
		uint8_t *data,
		size_t data_size) {
	ttLibC_RtmpStream_ *stream_ = (ttLibC_RtmpStream_ *)stream;
	if(stream_ == NULL) {
		return NULL;
	}
	ttLibC_FlvFrameManager *manager = stream_->frame_manager;
	ttLibC_Audio *audio_frame = ttLibC_FlvFrameManager_readAudioBinary(
			manager,
			data,
			data_size,
			header->timestamp);
	if(audio_frame == NULL) {
		return NULL;
	}
	ttLibC_AudioMessage *message = ttLibC_AudioMessage_make();
	if(message == NULL) {
		return NULL;
	}
	message->audio_frame = audio_frame;
	return message;
}

bool ttLibC_AudioMessage_getData(
		ttLibC_AudioMessage *message,
		ttLibC_DynamicBuffer *buffer) {
	if(message->audio_frame->inherit_super.type == frameType_aac) {
		if(message->is_dsi_info) {
			return ttLibC_FlvFrameManager_getAacDsiData(
					(ttLibC_Frame *)message->audio_frame,
					buffer);
		}
	}
	return ttLibC_FlvFrameManager_getData(
			(ttLibC_Frame *)message->audio_frame,
			buffer);
}

void ttLibC_AudioMessage_close(ttLibC_AudioMessage **message) {
	ttLibC_AudioMessage *target = (ttLibC_AudioMessage *)*message;
	if(target == NULL) {
		return;
	}
	ttLibC_RtmpHeader_close(&target->inherit_super.header);
	ttLibC_free(target);
	*message = NULL;
}
