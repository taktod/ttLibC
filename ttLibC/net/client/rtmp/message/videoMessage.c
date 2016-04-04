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
#include "../../../../log.h"
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

ttLibC_VideoMessage *ttLibC_VideoMessage_addFrame(
		ttLibC_RtmpStream *stream,
		ttLibC_Video *frame) {
	ttLibC_RtmpStream_ *stream_ = (ttLibC_RtmpStream_ *)stream;
	if(stream == NULL) {
		ERR_PRINT("need rtmpStream object.");
		return NULL;
	}
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
	message->inherit_super.header->stream_id = stream_->stream_id;
	return message;
}

ttLibC_VideoMessage *ttLibC_VideoMessage_readBinary(
		ttLibC_RtmpHeader *header,
		ttLibC_RtmpStream *stream,
		uint8_t *data,
		size_t data_size) {
	ttLibC_RtmpStream_ *stream_ = (ttLibC_RtmpStream_ *)stream;
	if(stream_ == NULL) {
		return NULL;
	}
	ttLibC_FlvFrameManager *manager = stream_->frame_manager;
	ttLibC_Video *video_frame = ttLibC_FlvFrameManager_readVideoBinary(
			manager,
			data,
			data_size,
			header->timestamp);
	if(video_frame == NULL) {
		return NULL;
	}
	ttLibC_VideoMessage *message = ttLibC_VideoMessage_make();
	if(message == NULL) {
		return NULL;
	}
	message->video_frame = video_frame;
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
