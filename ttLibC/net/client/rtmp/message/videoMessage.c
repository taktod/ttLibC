/**
 * @file   videoMessage.c
 * @brief  rtmp message videoMessage
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/13
 */
#ifdef __ENABLE_SOCKET__

#include "videoMessage.h"
#include "../../../../_log.h"
#include "../../../../allocator.h"
#include "../rtmpStream.h"
#include <string.h>
#include "../../../../util/hexUtil.h"
#include "../../../../frame/video/video.h"
#include "../../../../frame/video/flv1.h"
#include "../../../../frame/video/h264.h"
#include "../../../../util/ioUtil.h"
#include "../../../../util/flvFrameUtil.h"

ttLibC_VideoMessage *ttLibC_VideoMessage_make() {
	ttLibC_VideoMessage *message = ttLibC_malloc(sizeof(ttLibC_VideoMessage));
	if(message == NULL) {
		return NULL;
	}
	ttLibC_RtmpHeader *header = ttLibC_RtmpHeader_make(6, 0, RtmpMessageType_videoMessage, 0);
	if(header == NULL) {
		ttLibC_free(message);
		return NULL;
	}
	message->inherit_super.header = header;
	message->video_frame = NULL;
	return message;
}

ttLibC_VideoMessage *ttLibC_VideoMessage_readBinary(
		uint8_t *data,
		size_t data_size) {
	if(data_size <= 1) {
		return NULL;
	}
	ttLibC_VideoMessage *message = ttLibC_VideoMessage_make();
	if(message == NULL) {
		return NULL;
	}
	message->data = data;
	return message;
}

tetty_errornum ttLibC_VideoMessage_getFrame(
		ttLibC_VideoMessage *message,
		ttLibC_FlvFrameManager *manager,
		ttLibC_RtmpStream_getFrameFunc callback,
		void *ptr) {
	if(manager == NULL) {
		return 0;
	}
	uint8_t *data = message->data;
	size_t data_size = message->inherit_super.header->size;
	if(data == NULL) {
		return 0;
	}
	switch(manager->video_type) {
	case frameType_h264:
		if(data_size <= 4) {
			return 0;
		}
		break;
	default:
		if(data_size <= 1) {
			return 0;
		}
		break;
	}
	if(!ttLibC_FlvFrameManager_readVideoBinary(
			manager,
			data,
			message->inherit_super.header->size,
			message->inherit_super.header->timestamp,
			callback,
			ptr)) {
		return 98;
	}
	return 0;
}

ttLibC_VideoMessage *ttLibC_VideoMessage_addFrame(
		uint32_t stream_id,
		ttLibC_Video *frame) {
	if(frame == NULL) {
		return NULL;
	}
	ttLibC_VideoMessage *message = ttLibC_VideoMessage_make();
	if(message == NULL) {
		ERR_PRINT("failed to allocate message object.");
		return NULL;
	}
	// hold the ref of frame.
	message->video_frame = frame;
	// update pts and stream_id
	message->inherit_super.header->timestamp = message->video_frame->inherit_super.pts * 1000 / message->video_frame->inherit_super.timebase;
	message->inherit_super.header->stream_id = stream_id;
	return message;
}

bool ttLibC_VideoMessage_getData(
		ttLibC_VideoMessage *message,
		ttLibC_DynamicBuffer *buffer) {
	return ttLibC_FlvFrameManager_getData(
			(ttLibC_Frame *)message->video_frame,
			buffer);
}

void ttLibC_VideoMessage_close(ttLibC_VideoMessage **message) {
	ttLibC_VideoMessage *target = (ttLibC_VideoMessage *)*message;
	if(target == NULL) {
		return;
	}
	ttLibC_RtmpHeader_close(&target->inherit_super.header);
	ttLibC_free(target);
	*message = NULL;
}

#endif
