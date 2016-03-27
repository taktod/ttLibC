/**
 * @file   aggregateMessage.c
 * @brief  rtmp message aggregateMessage (frame information.)
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/22
 */

#include "aggregateMessage.h"
#include "../../../../log.h"
#include "../../../../allocator.h"
#include <string.h>
#include "../../../../util/hexUtil.h"
#include "../../../../util/flvFrameUtil.h"
#include "../rtmpStream.h"

ttLibC_AggregateMessage *ttLibC_AggregateMessage_make() {
	ttLibC_AggregateMessage *message = ttLibC_malloc(sizeof(ttLibC_AggregateMessage));
	if(message == NULL) {
		return NULL;
	}
	ttLibC_RtmpHeader *header = ttLibC_RtmpHeader_make(3, 0, RtmpMessageType_aggregateMessage, 0);
	if(header == NULL) {
		ttLibC_free(message);
		return NULL;
	}
	message->inherit_super.header = header;
	message->data = NULL;
	return message;
}

ttLibC_AggregateMessage *ttLibC_AggregateMessage_readBinary(
		uint8_t *data,
		size_t data_size) {
	ttLibC_AggregateMessage *message = ttLibC_AggregateMessage_make();
	if(message == NULL) {
		return NULL;
	}
	message->data = data;
	return message;
}
tetty_errornum ttLibC_AggregateMessage_getFrame(
		ttLibC_AggregateMessage *message,
		ttLibC_RtmpStream *stream,
		ttLibC_AggregateMessage_getFrameFunc callback,
		void *ptr) {
	ttLibC_RtmpStream_ *stream_ = (ttLibC_RtmpStream_ *)stream;
	if(stream_ == NULL) {
		return 0;
	}
	uint8_t *data = message->data;
	if(data == NULL) {
		return 0;
	}
	int32_t timestamp_diff = message->inherit_super.header->timestamp;
	bool is_first_media = true;
	size_t data_size = message->inherit_super.header->size;
	ttLibC_FlvFrameManager *manager = stream_->frame_manager;
	while(data_size > 0) {
		if(data_size < 11) {
			ERR_PRINT("data size is too small for aggregate message.");
			return -1;
		}
		ttLibC_RtmpMessage_Type type = data[0];
		uint32_t size = (data[1] << 16) | (data[2] << 8) | data[3];
		uint32_t timestamp = (data[4] << 16) | (data[5] << 8) | (data[6]) | (data[7] << 24);
		if(is_first_media) {
			timestamp_diff -= timestamp;
			is_first_media = false;
		}
		timestamp += timestamp_diff;
		uint32_t stream_id = (data[8] << 16) | (data[9] << 8) | data[10];
		data += 11;
		data_size -= 11;
		if(data_size < size + 4) {
			ERR_PRINT("data size is too small for aggregate message._");
		}
		switch(type) {
		case RtmpMessageType_videoMessage:
			{
				ttLibC_Video *video_frame = ttLibC_FlvFrameManager_readVideoBinary(
						manager,
						data,
						size,
						timestamp);
				if(video_frame != NULL) {
					callback(ptr, (ttLibC_Frame *)video_frame);
				}
				data += (size + 4);
				data_size -= (size + 4);
			}
			break;
		case RtmpMessageType_audioMessage:
			{
				ttLibC_Audio *audio_frame = ttLibC_FlvFrameManager_readAudioBinary(
						manager,
						data,
						size,
						timestamp);
				if(audio_frame != NULL) {
					callback(ptr, (ttLibC_Frame *)audio_frame);
				}
				data += (size + 4);
				data_size -= (size + 4);
			}
			break;
		default:
			ERR_PRINT("unknown message found.");
		}
	}
	return 0;
}

void ttLibC_AggregateMessage_close(ttLibC_AggregateMessage **message) {
	ttLibC_AggregateMessage *target = (ttLibC_AggregateMessage *)*message;
	if(target == NULL) {
		return;
	}
	ttLibC_RtmpHeader_close(&target->inherit_super.header);
	ttLibC_free(target);
	*message = NULL;
}

