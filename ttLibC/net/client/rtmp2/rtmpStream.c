/*
 * rtmpStream.c
 *
 *  Created on: 2017/07/21
 *      Author: taktod
 */

#ifdef __ENABLE_SOCKET__

#include "rtmpStream.h"
#include "rtmpConnection.h"
#include "message/amf0Command.h"
#include "message/videoMessage.h"
#include "message/audioMessage.h"
#include "message/userControlMessage.h"
#include "data/clientObject.h"
#include "../../../ttLibC_predef.h"
#include "../../../_log.h"
#include "../../../allocator.h"
#include "../../tetty2/tcpBootstrap.h"
#include <string.h>

#include "../../../frame/frame.h"
#include "../../../frame/video/h264.h"
#include "../../../frame/audio/aac.h"
#include "../../../frame/audio/mp3.h"
#include "../../../util/hexUtil.h"

/**
 * callback for createStream.
 */
static void RtmpStream_createStreamCallback(void *ptr, ttLibC_Tetty2Promise *promise) {
	if(!promise->is_success) {
		return;
	}
	ttLibC_RtmpStream_ *stream = (ttLibC_RtmpStream_ *)ptr;
	ttLibC_ClientObject_PassingObject *passData = (ttLibC_ClientObject_PassingObject *)promise->return_val;
	// save stream_id. for publish
	stream->stream_id = passData->stream_id;
	// update streamId -> rtmpStream map. for play or streamEvent
	ttLibC_StlMap_put(passData->client_object->streamId_rtmpStream_map, (void *)((long)passData->stream_id), stream);
}

/**
 * callback for other event.
 */
static void RtmpStream_promiseCallback(void *ptr, ttLibC_Tetty2Promise *promise) {
	ttLibC_RtmpStream_ *stream = (ttLibC_RtmpStream_ *)ptr;
	ttLibC_Amf0Command *command = (ttLibC_Amf0Command *)promise->return_val;
	if(command->obj2 != NULL) {
		if(stream->event_callback != NULL) {
			stream->event_callback(stream->event_ptr, command->obj2);
		}
	}
}

ttLibC_RtmpStream TT_VISIBILITY_DEFAULT *ttLibC_RtmpStream_make(ttLibC_RtmpConnection *conn) {
	ttLibC_RtmpStream_ *stream = ttLibC_malloc(sizeof(ttLibC_RtmpStream_));
	if(stream == NULL) {
		return NULL;
	}
	// TODO do I need something in the case of deny from server?
	stream->conn = (ttLibC_RtmpConnection_ *)conn;
	stream->event_callback = NULL;
	stream->event_ptr = NULL;
	stream->frame_callback = NULL;
	stream->frame_ptr = NULL;
	stream->promise = NULL;
	stream->pts = 0;
	stream->stream_id = 0;
	stream->frame_manager = ttLibC_FlvFrameManager_make();
	stream->sent_dsi_info = false;
	stream->video_type = frameType_unknown;
	stream->video_queue = ttLibC_FrameQueue_make(9, 1024);
	stream->video_queue->isBframe_fixed = true;
	stream->audio_type = frameType_unknown;
	stream->audio_queue = ttLibC_FrameQueue_make(8, 1024);
	// make createStream and send it to server.
	ttLibC_Amf0Command *createStream = ttLibC_Amf0Command_createStream();
	stream->promise = ttLibC_Tetty2Bootstrap_makePromise(stream->conn->bootstrap);
	ttLibC_Tetty2Promise_addEventListener(stream->promise, RtmpStream_createStreamCallback, stream);
	createStream->promise = stream->promise;
	// send
	ttLibC_Tetty2Bootstrap_write(stream->conn->bootstrap, createStream, sizeof(ttLibC_Amf0Command));
	ttLibC_Tetty2Bootstrap_flush(stream->conn->bootstrap);
	ttLibC_Amf0Command_close(&createStream);
	while(true) {
		ttLibC_TcpBootstrap_update(stream->conn->bootstrap, 10000);
		if(stream->promise->is_done) {
			break;
		}
	}
	// need to check bootstrap->error_number.
//	ttLibC_Tetty2Promise_await(stream->promise); // wait until done.
	ttLibC_Tetty2Promise_addEventListener(stream->promise, RtmpStream_promiseCallback, stream);
	return (ttLibC_RtmpStream *)stream;
}

void TT_VISIBILITY_DEFAULT ttLibC_RtmpStream_addEventListener(
		ttLibC_RtmpStream *stream,
		ttLibC_RtmpEventFunc callback,
		void *ptr) {
	ttLibC_RtmpStream_ *stream_ = (ttLibC_RtmpStream_ *)stream;
	if(stream_ == NULL) {
		return;
	}
	stream_->event_callback = callback;
	stream_->event_ptr = ptr;
}

/**
 * add frame listener.
 * @param stream
 * @param callback
 * @@aram ptr
 */
void TT_VISIBILITY_DEFAULT ttLibC_RtmpStream_addFrameListener(
		ttLibC_RtmpStream *stream,
		ttLibC_RtmpStream_getFrameFunc callback,
		void *ptr) {
	ttLibC_RtmpStream_ *stream_ = (ttLibC_RtmpStream_ *)stream;
	if(stream_ == NULL) {
		return;
	}
	stream_->frame_callback = callback;
	stream_->frame_ptr = ptr;
}

// do publish.
void TT_VISIBILITY_DEFAULT ttLibC_RtmpStream_publish(
		ttLibC_RtmpStream *stream,
		const char *name) {
	ttLibC_RtmpStream_ *stream_ = (ttLibC_RtmpStream_ *)stream;
	if(stream_ == NULL) {
		return;
	}
	ttLibC_Amf0Command *publish = ttLibC_Amf0Command_publish(stream_->stream_id, name);
	ttLibC_Tetty2Bootstrap_write(stream_->conn->bootstrap, publish, sizeof(ttLibC_Amf0Command));
	ttLibC_Tetty2Bootstrap_flush(stream_->conn->bootstrap);
	ttLibC_Amf0Command_close(&publish);
	return;
}

bool TT_VISIBILITY_DEFAULT ttLibC_RtmpStream_addFrame(
		ttLibC_RtmpStream *stream,
		ttLibC_Frame *frame) {
	ttLibC_RtmpStream_ *stream_ = (ttLibC_RtmpStream_ *)stream;
	if(stream_ == NULL) {
		return false;
	}
	if(frame == NULL) {
		return true;
	}
	uint64_t timestamp = (uint64_t)(1.0 * frame->pts * 1000 / frame->timebase);
	if(timestamp > stream_->pts) {
		stream_->pts = timestamp;
	}
	switch(frame->type) {
	case frameType_h264:
		{
			ttLibC_H264 *h264 = (ttLibC_H264 *)frame;
			if(h264->type == H264Type_unknown) {
				return true;
			}
			if(h264->type == H264Type_configData) {
				frame->pts = 0;
				frame->dts = 0;
			}
		}
		/* no break */
	case frameType_flv1:
	case frameType_vp6:
		{
			if(stream_->video_type == frameType_unknown) {
				stream_->video_type = frame->type;
			}
			if(stream_->video_type != frame->type) {
				return false;
			}
			if(!ttLibC_FrameQueue_queue(stream_->video_queue, frame)) {
				return false;
			}
		}
		break;
	case frameType_aac:
	case frameType_mp3:
	case frameType_nellymoser:
	case frameType_pcm_alaw:
	case frameType_pcm_mulaw:
	case frameType_pcmS16:
	case frameType_speex:
		{
			if(stream_->audio_type == frameType_unknown) {
				stream_->audio_type = frame->type;
			}
			if(stream_->audio_type != frame->type) {
				return false;
			}
			if(!ttLibC_FrameQueue_queue(stream_->audio_queue, frame)) {
				return false;
			}
		}
		break;
	default:
		return false;
	}
	if(stream_->audio_type != frameType_unknown) {
		if(stream_->video_type != frameType_unknown) {
				// audio + video
			while(true) {
				ttLibC_Frame *video = ttLibC_FrameQueue_ref_first(stream_->video_queue);
				ttLibC_Frame *audio = ttLibC_FrameQueue_ref_first(stream_->audio_queue);
				if(video == NULL || audio == NULL) {
					break;
				}
				if(video->dts == 0 && video->pts != 0) {
					break;
				}
				if(video->dts > audio->pts) {
					audio = ttLibC_FrameQueue_dequeue_first(stream_->audio_queue);
					if(audio->type == frameType_aac) {
						ttLibC_AudioMessage *audioMessage = ttLibC_AudioMessage_addFrame(stream_->stream_id, (ttLibC_Audio *)audio);
						if(audioMessage != NULL) {
							audioMessage->is_dsi_info = true;
							ttLibC_Tetty2Bootstrap_write(stream_->conn->bootstrap, audioMessage, sizeof(ttLibC_AudioMessage));
							ttLibC_AudioMessage_close(&audioMessage);
						}
					}
					ttLibC_AudioMessage *audioMessage = ttLibC_AudioMessage_addFrame(stream_->stream_id, (ttLibC_Audio *)audio);
					if(audioMessage != NULL) {
						ttLibC_Tetty2Bootstrap_write(stream_->conn->bootstrap, audioMessage, sizeof(ttLibC_AudioMessage));
						ttLibC_Tetty2Bootstrap_flush(stream_->conn->bootstrap);
						ttLibC_AudioMessage_close(&audioMessage);
					}
				}
				else {
					video = ttLibC_FrameQueue_dequeue_first(stream_->video_queue);
					ttLibC_VideoMessage *videoMessage = ttLibC_VideoMessage_addFrame(stream_->stream_id, (ttLibC_Video *)video);
					if(videoMessage != NULL) {
						ttLibC_Tetty2Bootstrap_write(stream_->conn->bootstrap, videoMessage, sizeof(ttLibC_VideoMessage));
						ttLibC_Tetty2Bootstrap_flush(stream_->conn->bootstrap);
						ttLibC_VideoMessage_close(&videoMessage);
					}
				}
			}
		}
		else {
			// audio only
			while(true) {
				ttLibC_Frame *audio = ttLibC_FrameQueue_ref_first(stream_->audio_queue);
				if(audio == NULL) {
					break;
				}
				audio = ttLibC_FrameQueue_dequeue_first(stream_->audio_queue);
				if(audio->type == frameType_aac) {
					ttLibC_AudioMessage *audioMessage = ttLibC_AudioMessage_addFrame(stream_->stream_id, (ttLibC_Audio *)audio);
					if(audioMessage != NULL) {
						audioMessage->is_dsi_info = true;
						ttLibC_Tetty2Bootstrap_write(stream_->conn->bootstrap, audioMessage, sizeof(ttLibC_AudioMessage));
						ttLibC_AudioMessage_close(&audioMessage);
					}
				}
				ttLibC_AudioMessage *audioMessage = ttLibC_AudioMessage_addFrame(stream_->stream_id, (ttLibC_Audio *)audio);
				if(audioMessage != NULL) {
					ttLibC_Tetty2Bootstrap_write(stream_->conn->bootstrap, audioMessage, sizeof(ttLibC_AudioMessage));
					ttLibC_Tetty2Bootstrap_flush(stream_->conn->bootstrap);
					ttLibC_AudioMessage_close(&audioMessage);
				}
			}
		}
	}
	else {
		if(stream_->video_type != frameType_unknown) {
			// video only
			while(true) {
				ttLibC_Frame *video = ttLibC_FrameQueue_ref_first(stream_->video_queue);
				if(video == NULL) {
					break;
				}
				if(video->dts == 0 && video->pts != 0) {
					break;
				}
				video = ttLibC_FrameQueue_dequeue_first(stream_->video_queue);
				ttLibC_VideoMessage *videoMessage = ttLibC_VideoMessage_addFrame(stream_->stream_id, (ttLibC_Video *)video);
				ttLibC_Tetty2Bootstrap_write(stream_->conn->bootstrap, videoMessage, sizeof(ttLibC_VideoMessage));
				ttLibC_Tetty2Bootstrap_flush(stream_->conn->bootstrap);
				ttLibC_VideoMessage_close(&videoMessage);
			}
		}
	}
	return true;
}

/**
 * send buffer length for play.
 */
void TT_VISIBILITY_DEFAULT ttLibC_RtmpStream_setBufferLength(
		ttLibC_RtmpStream *stream,
		uint32_t buffer_length) {
	ttLibC_RtmpStream_ *stream_ = (ttLibC_RtmpStream_ *)stream;
	if(stream_ == NULL) {
		return;
	}
	ttLibC_UserControlMessage *client_buffer_length = ttLibC_UserControlMessage_make(Type_ClientBufferLength, stream_->stream_id, buffer_length, stream_->pts);
	ttLibC_Tetty2Bootstrap_write(stream_->conn->bootstrap, client_buffer_length, sizeof(ttLibC_UserControlMessage));
	ttLibC_Tetty2Bootstrap_flush(stream_->conn->bootstrap);
	ttLibC_UserControlMessage_close(&client_buffer_length);
}

void TT_VISIBILITY_DEFAULT ttLibC_RtmpStream_play(
		ttLibC_RtmpStream *stream,
		const char *name,
		bool accept_video,
		bool accept_audio) {
	ttLibC_RtmpStream_ *stream_ = (ttLibC_RtmpStream_ *)stream;
	if(stream_ == NULL) {
		return;
	}
	ttLibC_Amf0Command *receiveAudio = ttLibC_Amf0Command_receiveAudio(stream_->stream_id, accept_audio);
	if(receiveAudio == NULL) {
		return;
	}
	ttLibC_Tetty2Bootstrap_write(stream_->conn->bootstrap, receiveAudio, sizeof(ttLibC_Amf0Command));
	ttLibC_Amf0Command_close(&receiveAudio);

	ttLibC_Amf0Command *receiveVideo = ttLibC_Amf0Command_receiveVideo(stream_->stream_id, accept_video);
	if(receiveVideo == NULL) {
		return;
	}
	ttLibC_Tetty2Bootstrap_write(stream_->conn->bootstrap, receiveVideo, sizeof(ttLibC_Amf0Command));
	ttLibC_Amf0Command_close(&receiveVideo);
	ttLibC_Amf0Command *play = ttLibC_Amf0Command_play(stream_->stream_id, name);
	if(play == NULL) {
		return;
	}
	ttLibC_Tetty2Bootstrap_write(stream_->conn->bootstrap, play, sizeof(ttLibC_Amf0Command));
	ttLibC_Tetty2Bootstrap_flush(stream_->conn->bootstrap);
	ttLibC_Amf0Command_close(&play);
}

void TT_VISIBILITY_DEFAULT ttLibC_RtmpStream_pause(ttLibC_RtmpStream *stream) {
	ttLibC_RtmpStream_ *stream_ = (ttLibC_RtmpStream_ *)stream;
	if(stream_ == NULL) {
		return;
	}
	ttLibC_Amf0Command *pause = ttLibC_Amf0Command_pause(stream_->stream_id);
	if(pause == NULL) {
		return;
	}
	ttLibC_Tetty2Bootstrap_write(stream_->conn->bootstrap, pause, sizeof(ttLibC_Amf0Command));
	ttLibC_Tetty2Bootstrap_flush(stream_->conn->bootstrap);
}

void TT_VISIBILITY_DEFAULT ttLibC_RtmpStream_close(ttLibC_RtmpStream **stream) {
	ttLibC_RtmpStream_ *target = (ttLibC_RtmpStream_ *)*stream;
	if(target == NULL) {
		return;
	}
	// dispose old promise.
	ttLibC_Tetty2Promise_close(&target->promise);
	*stream = NULL; // *
	// if we can, do stream.close command execute.
	if(target->conn->bootstrap->error_number == 0) {
		// send close stream.
		ttLibC_Amf0Command *closeStream = ttLibC_Amf0Command_closeStream(target->stream_id);
		closeStream->inherit_super.header->timestamp = target->pts;
		target->promise = ttLibC_Tetty2Bootstrap_makePromise(target->conn->bootstrap);
		ttLibC_Tetty2Promise_addEventListener(target->promise, RtmpStream_promiseCallback, target);
		// send
 		ttLibC_Tetty2Bootstrap_write(target->conn->bootstrap, closeStream, sizeof(ttLibC_Amf0Command));
		ttLibC_Tetty2Bootstrap_flush(target->conn->bootstrap);
		ttLibC_Amf0Command_close(&closeStream);
		while(true) {
			ttLibC_TcpBootstrap_update(target->conn->bootstrap, 10000);
			if(target->promise->is_done) {
				break;
			}
		}
//		ttLibC_Tetty2Promise_await(target->promise);
		ttLibC_Tetty2Promise_close(&target->promise);
	}
	ttLibC_FlvFrameManager_close(&target->frame_manager);
	ttLibC_FrameQueue_close(&target->video_queue);
	ttLibC_FrameQueue_close(&target->audio_queue);
	ttLibC_free(target);
}

#endif
