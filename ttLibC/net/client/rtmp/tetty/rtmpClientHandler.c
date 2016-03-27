/**
 * @file   rtmpClientHandler.c
 * @brief  tetty handler for client work.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/07
 */

#include "rtmpClientHandler.h"
#include "../../../../log.h"
#include "../../../../allocator.h"
#include "../../../../util/stlMapUtil.h"
#include "../data/clientObject.h"
#include "../message/aggregateMessage.h"
#include "../message/audioMessage.h"
#include "../message/rtmpMessage.h"
#include "../message/setChunkSize.h"
#include "../message/userControlMessage.h"
#include "../message/videoMessage.h"
#include "../message/windowAcknowledgementSize.h"
#include "../rtmpStream.h"
#include <string.h>

static bool RtmpClientHandler_getAggregateFrames(void *ptr, ttLibC_Frame *frame) {
	ttLibC_RtmpStream_ *stream = (ttLibC_RtmpStream_ *)ptr;
	if(stream != NULL && stream->frame_callback != NULL) {
		if(!stream->frame_callback(stream->frame_ptr, frame)) {
			return false;
		}
	}
	return true;
}

static tetty_errornum RtmpClientHandler_channelRead(
		ttLibC_TettyContext *ctx,
		void *data,
		size_t data_size) {
	ttLibC_RtmpMessage *rtmp_message = (ttLibC_RtmpMessage *)data;
	ttLibC_ClientObject *client_object = (ttLibC_ClientObject *)ctx->socket_info->ptr;
	switch(rtmp_message->header->message_type) {
	case RtmpMessageType_videoMessage:
		{
			ttLibC_VideoMessage *video_message = (ttLibC_VideoMessage *)rtmp_message;
			ttLibC_RtmpStream_ *stream_ = ttLibC_StlMap_get(client_object->streamId_rtmpStream_map, (void *)((long)video_message->inherit_super.header->stream_id));
			if(video_message->video_frame == NULL) {
				ERR_PRINT("frame is missing for videoMessage.");
				ctx->bootstrap->error_number = 9;
				return 0;
			}
			if(stream_ != NULL && stream_->frame_callback != NULL) {
				if(!stream_->frame_callback(stream_->frame_ptr, (ttLibC_Frame *)video_message->video_frame)) {
					LOG_PRINT("frame callback returns false.");
					ctx->bootstrap->error_number = 10;
				}
			}
		}
		break;
	case RtmpMessageType_audioMessage:
		{
			ttLibC_AudioMessage *audio_message = (ttLibC_AudioMessage *)rtmp_message;
			ttLibC_RtmpStream_ *stream_ = ttLibC_StlMap_get(client_object->streamId_rtmpStream_map, (void *)((long)audio_message->inherit_super.header->stream_id));
			if(audio_message->audio_frame == NULL) {
				ERR_PRINT("frame is missing for audioMessage.");
				ctx->bootstrap->error_number = 8;
				return 0;
			}
			if(stream_ != NULL && stream_->frame_callback != NULL) {
				if(!stream_->frame_callback(stream_->frame_ptr, (ttLibC_Frame *)audio_message->audio_frame)) {
					LOG_PRINT("frame callback returns false.");
					ctx->bootstrap->error_number = 10;
				}
			}
		}
		break;
	case RtmpMessageType_aggregateMessage:
		{
			ttLibC_RtmpStream *stream = ttLibC_StlMap_get(client_object->streamId_rtmpStream_map, (void *)((long)rtmp_message->header->stream_id));
			// get frame from aggregate message.
			tetty_errornum error_num = ttLibC_AggregateMessage_getFrame(
					(ttLibC_AggregateMessage *)rtmp_message,
					stream,
					RtmpClientHandler_getAggregateFrames,
					stream);
			if(error_num != 0) {
				LOG_PRINT("something happen during aggregateMessage_getFrame.:%d", error_num);
				ctx->bootstrap->error_number = error_num;
			}
		}
		break;
	case RtmpMessageType_setChunkSize:
		{
			ttLibC_SetChunkSize *chunk_size = (ttLibC_SetChunkSize *)rtmp_message;
			client_object->recv_chunk_size = chunk_size->size;
		}
		return 0;
	case RtmpMessageType_windowAcknowledgementSize:
		{
			// for win ack, send it back to server.
			ttLibC_WindowAcknowledgementSize *win_ack = (ttLibC_WindowAcknowledgementSize *)rtmp_message;
			ttLibC_TettyContext_channel_write(ctx, win_ack, sizeof(ttLibC_WindowAcknowledgementSize));
			ttLibC_TettyContext_channel_flush(ctx);
		}
		break;
		// need to add more.(like userControlMessage for ping/pong.)
	case RtmpMessageType_userControlMessage:
		{
			ttLibC_UserControlMessage *user_control_message = (ttLibC_UserControlMessage *)rtmp_message;
			switch(user_control_message->type) {
			case Type_StreamBegin:
				// nothing to do. leave.
				break;
			case Type_StreamEof:
				// nothing to do. leave.
				break;
/*			case Type_StreamDry:
			case Type_RecordedStream:*/
			case Type_BufferEmpty:
				break;
			case Type_BufferFull:
				break;
/*			case Type_ClientBufferLength:
			case Type_Unknown5:*/
			case Type_Ping:
				{
					// reply pong.
					ttLibC_UserControlMessage *pong = ttLibC_UserControlMessage_make(Type_Pong, 0, 0, user_control_message->time);
					ttLibC_TettyContext_channel_write(ctx, pong, sizeof(ttLibC_UserControlMessage));
					ttLibC_TettyContext_channel_flush(ctx);
					ttLibC_UserControlMessage_close(&pong);
				}
				break;
/*			case Type_Pong:
			case Type_Unknown8:
			case Type_PingSwfVerification:
			case Type_PongSwfVerification:*/
			default:
				ERR_PRINT("unknown userControlMessage:%d", user_control_message->type);
				return 0;
			}
		}
		break;
	default:
		break;
	}
	return 0;
}

static tetty_errornum RtmpClientHandler_write(
		ttLibC_TettyContext *ctx,
		void *data,
		size_t data_size) {
	if(ctx->socket_info == NULL) {
		return ttLibC_TettyContext_super_writeEach(ctx, data, data_size);
	}
	else {
		return ttLibC_TettyContext_super_write(ctx, data, data_size);
	}
}

ttLibC_RtmpClientHandler *ttLibC_RtmpClientHandler_make() {
	ttLibC_RtmpClientHandler *handler = ttLibC_malloc(sizeof(ttLibC_RtmpClientHandler));
	if(handler == NULL) {
		return NULL;
	}
	memset(handler, 0, sizeof(ttLibC_RtmpClientHandler));
	handler->channel_handler.channelRead = RtmpClientHandler_channelRead;
	handler->channel_handler.write = RtmpClientHandler_write;
	return handler;
}

void ttLibC_RtmpClientHandler_close(ttLibC_RtmpClientHandler **handler) {
	ttLibC_RtmpClientHandler *target = (ttLibC_RtmpClientHandler *)*handler;
	if(target == NULL) {
		return;
	}
	ttLibC_free(target);
	*handler = NULL;
}
