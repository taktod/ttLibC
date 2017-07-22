/*
 * rtmpClientHandler.c
 *
 *  Created on: 2017/07/21
 *      Author: taktod
 */

#include "rtmpClientHandler.h"
#include "../../../tetty2/tcpBootstrap.h"
#include "../../../../ttLibC_predef.h"
#include "../../../../_log.h"
#include "../../../../allocator.h"
#include "../../../../util/stlMapUtil.h"
#include "../data/clientObject.h"
#include "../message/acknowledgement.h"
#include "../message/aggregateMessage.h"
#include "../message/audioMessage.h"
#include "../message/rtmpMessage.h"
#include "../message/setChunkSize.h"
#include "../message/setPeerBandwidth.h"
#include "../message/userControlMessage.h"
#include "../message/videoMessage.h"
#include "../message/windowAcknowledgementSize.h"
#include "../rtmpStream.h"
#include <string.h>

static tetty2_errornum RtmpClientHandler_channelRead(
		ttLibC_Tetty2Context *ctx,
		void *data,
		size_t data_size) {
	(void)data_size;
	ttLibC_RtmpMessage *rtmp_message = (ttLibC_RtmpMessage *)data;
	ttLibC_ClientObject *client_object = (ttLibC_ClientObject *)ctx->tetty_info->ptr;
	ttLibC_RtmpClientHandler *handler = (ttLibC_RtmpClientHandler *)ctx->channel_handler;
	// update bytesRead
	handler->bytesRead += rtmp_message->header->size;
/*	if(handler->bytesRead - handler->bytesReadAcked >= handler->bytesReadWindow) {
		// send ack.
		ttLibC_Acknowledgement *acknowledgement = ttLibC_Acknowledgement_make((uint32_t)handler->bytesRead);
		ttLibC_TettyContext_channel_write(ctx, acknowledgement, sizeof(ttLibC_Acknowledgement));
		ttLibC_TettyContext_channel_flush(ctx);
		ttLibC_Acknowledgement_close(&acknowledgement);
		handler->bytesReadAcked = handler->bytesRead;
	}// */
	switch(rtmp_message->header->message_type) {
	case RtmpMessageType_videoMessage:
		{
			ttLibC_RtmpStream_ *stream_ = ttLibC_StlMap_get(client_object->streamId_rtmpStream_map, (void *)((long)rtmp_message->header->stream_id));
			if(stream_ != NULL) {
				tetty2_errornum error_num = ttLibC_VideoMessage_getFrame(
						(ttLibC_VideoMessage *)rtmp_message,
						stream_->frame_manager,
						stream_->frame_callback,
						stream_->frame_ptr);
				if(error_num != 0) {
					LOG_PRINT("something happen during videoMessage_getFrame.:%d", error_num);
					ctx->bootstrap->error_number = error_num;
				}
			}
		}
		break;
	case RtmpMessageType_audioMessage:
		{
			ttLibC_RtmpStream_ *stream_ = ttLibC_StlMap_get(client_object->streamId_rtmpStream_map, (void *)((long)rtmp_message->header->stream_id));
			if(stream_ != NULL) {
				tetty2_errornum error_num = ttLibC_AudioMessage_getFrame(
						(ttLibC_AudioMessage *)rtmp_message,
						stream_->frame_manager,
						stream_->frame_callback,
						stream_->frame_ptr);
				if(error_num != 0) {
					LOG_PRINT("something happen during audioMessage_getFrame.:%d", error_num);
					ctx->bootstrap->error_number = error_num;
				}
			}
		}
		break;
	case RtmpMessageType_aggregateMessage:
		{
			ttLibC_RtmpStream_ *stream_ = ttLibC_StlMap_get(client_object->streamId_rtmpStream_map, (void *)((long)rtmp_message->header->stream_id));
			if(stream_ != NULL) {
				// get frame from aggregate message.
				tetty2_errornum error_num = ttLibC_AggregateMessage_getFrame(
						(ttLibC_AggregateMessage *)rtmp_message,
						stream_->frame_manager,
						stream_->frame_callback,
						stream_->frame_ptr);
				if(error_num != 0) {
					LOG_PRINT("something happen during aggregateMessage_getFrame.:%d", error_num);
					ctx->bootstrap->error_number = error_num;
				}
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
			ttLibC_Tetty2Context_channel_write(ctx, win_ack, sizeof(ttLibC_WindowAcknowledgementSize));
			ttLibC_Tetty2Context_channel_flush(ctx);
			ttLibC_WindowAcknowledgementSize *windowAcknowledgementSize = (ttLibC_WindowAcknowledgementSize *)rtmp_message;
			handler->bytesReadWindow = windowAcknowledgementSize->size;
		}
		break;
	case RtmpMessageType_setPeerBandwidth:
		{
			ttLibC_SetPeerBandwidth *setPeerBandwidth = (ttLibC_SetPeerBandwidth *)rtmp_message;
			handler->bytesWrittenWindow = setPeerBandwidth->size;
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
					ttLibC_UserControlMessage *pong = ttLibC_UserControlMessage_pong(user_control_message->time);
					ttLibC_Tetty2Context_channel_write(ctx, pong, sizeof(ttLibC_UserControlMessage));
					ttLibC_Tetty2Context_channel_flush(ctx);
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

static tetty2_errornum RtmpClientHandler_write(
		ttLibC_Tetty2Context *ctx,
		void *data,
		size_t data_size) {
	if(ttLibC_TcpBootstrap_isServerContext(ctx)) {
//	if(ctx->tetty_info->bootstrap_ptr == NULL) {
		return ttLibC_TcpBootstrap_Context_writeAllClients(ctx);
	}
	else {
		return ttLibC_Tetty2Context_super_write(ctx, data, data_size);
	}
}

ttLibC_RtmpClientHandler TT_VISIBILITY_HIDDEN *ttLibC_RtmpClientHandler_make() {
	ttLibC_RtmpClientHandler *handler = ttLibC_malloc(sizeof(ttLibC_RtmpClientHandler));
	if(handler == NULL) {
		return NULL;
	}
	memset(handler, 0, sizeof(ttLibC_RtmpClientHandler));
	handler->bytesReadWindow = 2500000;
	handler->bytesWrittenWindow = 2500000;
	handler->channel_handler.channelRead = RtmpClientHandler_channelRead;
	handler->channel_handler.write = RtmpClientHandler_write;
	return handler;
}

void TT_VISIBILITY_HIDDEN ttLibC_RtmpClientHandler_close(ttLibC_RtmpClientHandler **handler) {
	ttLibC_RtmpClientHandler *target = (ttLibC_RtmpClientHandler *)*handler;
	if(target == NULL) {
		return;
	}
	ttLibC_free(target);
	*handler = NULL;
}

