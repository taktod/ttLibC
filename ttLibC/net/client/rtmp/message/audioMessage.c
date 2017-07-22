/**
 * @file   audioMessage.c
 * @brief  rtmp message AudioMessage
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/13
 */
#ifdef __ENABLE_SOCKET__

#include "audioMessage.h"
#include "../../../../ttLibC_predef.h"
#include "../../../../_log.h"
#include "../../../../allocator.h"
#include <string.h>
#include "../../../../util/hexUtil.h"
#include "../../../../frame/audio/aac.h"
#include "../../../../frame/audio/mp3.h"
#include "../../../../util/ioUtil.h"
#include "../../../../util/flvFrameUtil.h"

ttLibC_AudioMessage TT_VISIBILITY_HIDDEN *ttLibC_AudioMessage_make() {
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

ttLibC_AudioMessage TT_VISIBILITY_HIDDEN *ttLibC_AudioMessage_readBinary(
		uint8_t *data,
		size_t data_size) {
	if(data_size <= 1) {
		return NULL;
	}
	ttLibC_AudioMessage *message = ttLibC_AudioMessage_make();
	if(message == NULL) {
		return NULL;
	}
	message->data = data;
	return message;
}

tetty_errornum TT_VISIBILITY_HIDDEN ttLibC_AudioMessage_getFrame(
		ttLibC_AudioMessage *message,
		ttLibC_FlvFrameManager *manager,
		ttLibC_RtmpStream_getFrameFunc callback,
		void *ptr) {
	if(manager == NULL) {
		return 0;
	}
	uint8_t *data = message->data;
	if(data == NULL) {
		return 0;
	}
	if(!ttLibC_FlvFrameManager_readAudioBinary(
			manager,
			data,
			message->inherit_super.header->size,
			message->inherit_super.header->timestamp,
			callback,
			ptr)) {
		return 97;
	}
	return 0;
}

ttLibC_AudioMessage TT_VISIBILITY_HIDDEN *ttLibC_AudioMessage_addFrame(
		uint32_t stream_id,
		ttLibC_Audio *frame) {
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
	if(message->audio_frame->inherit_super.timebase == 1000) {
		message->inherit_super.header->timestamp = message->audio_frame->inherit_super.pts;
	}
	else {
		message->inherit_super.header->timestamp = message->audio_frame->inherit_super.pts * 1000 / message->audio_frame->inherit_super.timebase;
	}
	message->inherit_super.header->stream_id = stream_id;
	return message;
}

bool TT_VISIBILITY_HIDDEN ttLibC_AudioMessage_getData(
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

void TT_VISIBILITY_HIDDEN ttLibC_AudioMessage_close(ttLibC_AudioMessage **message) {
	ttLibC_AudioMessage *target = (ttLibC_AudioMessage *)*message;
	if(target == NULL) {
		return;
	}
	ttLibC_RtmpHeader_close(&target->inherit_super.header);
	ttLibC_free(target);
	*message = NULL;
}

#endif
