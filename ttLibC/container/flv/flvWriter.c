/*
 * @file   flvWriter.c
 * @brief  flvTag writer to make binary data.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/12
 */

#include "flvWriter.h"
#include "type/headerTag.h"
#include "type/audioTag.h"
#include "type/videoTag.h"
#include "type/metaTag.h"
#include "../../_log.h"
#include "../../allocator.h"
#include "../../util/hexUtil.h"
#include "../../frame/video/h264.h"
#include <stdlib.h>

ttLibC_FlvWriter TT_ATTRIBUTE_API *ttLibC_FlvWriter_make(
		ttLibC_Frame_Type video_type,
		ttLibC_Frame_Type audio_type) {
	ttLibC_FlvWriter_ *writer = (ttLibC_FlvWriter_ *)ttLibC_ContainerWriter_make(containerType_flv, sizeof(ttLibC_FlvWriter_), 1000);
	if(writer == NULL) {
		ERR_PRINT("failed to allocate memory for writer.");
		return NULL;
	}
	writer->is_first = true;
	switch(video_type) {
	case frameType_flv1:
	case frameType_h264:
	case frameType_vp6:
		writer->video_track.crc32       = 0;
		writer->video_track.frame_type  = video_type;
		// we need to have enough size of queue. or drop some frame.
		writer->video_track.frame_queue = ttLibC_FrameQueue_make(9, 255);
		writer->video_track.frame_queue->isBframe_fixed = true;
		writer->video_track.configData = NULL;
		break;
	default:
		writer->video_track.crc32       = 0;
		writer->video_track.frame_type  = frameType_unknown;
		writer->video_track.frame_queue = NULL;
		writer->video_track.configData = NULL;
		break;
	}
	switch(audio_type) {
	case frameType_aac:
	case frameType_mp3:
	case frameType_nellymoser:
	case frameType_pcm_alaw:
	case frameType_pcm_mulaw:
	case frameType_pcmS16:
	case frameType_speex:
		writer->audio_track.crc32       = 0;
		writer->audio_track.frame_type  = audio_type;
		writer->audio_track.frame_queue = ttLibC_FrameQueue_make(8, 255);
		break;
	default:
		writer->audio_track.crc32       = 0;
		writer->audio_track.frame_type  = frameType_unknown;
		writer->audio_track.frame_queue = NULL;
		break;
	}
	if(writer->audio_track.frame_type == frameType_unknown && writer->video_track.frame_type == frameType_unknown) {
		ERR_PRINT("target track is invalid for both audio and video.");
		ttLibC_free(writer);
		return NULL;
	}
	return (ttLibC_FlvWriter *)writer;
}

/*
 * add frame on queue.
 * @param writer target flv writer object.
 * @param frame  frame
 * @return true:success false:error
 */
static bool FlvWriter_queueFrame(
		ttLibC_FlvWriter_ *writer,
		ttLibC_Frame *frame) {
	// change the timebase to 1000.(mili sec.)
	frame->pts = (uint64_t)(1.0 * frame->pts * 1000 / frame->timebase);
	frame->timebase = 1000;
	switch(frame->type) {
	case frameType_h264:
		{
			ttLibC_H264 *h264 = (ttLibC_H264 *)frame;
			switch(h264->type) {
			case H264Type_unknown:
				return true;
			case H264Type_configData:
				writer->video_track.configData = ttLibC_Frame_clone(writer->video_track.configData, frame);
				return true;
			default:
				break;
			}
		}
		/* no break */
	case frameType_flv1:
	case frameType_vp6:
		// if the frame type is different from track. return false.
		if(writer->video_track.frame_type != frame->type) {
			ERR_PRINT("invalid video frame is detected.");
			return false;
		}
 		if(!ttLibC_FrameQueue_queue(writer->video_track.frame_queue, frame)) {
 			return false;
 		}
		break;
	case frameType_aac:
	case frameType_mp3:
	case frameType_nellymoser:
	case frameType_pcm_alaw:
	case frameType_pcm_mulaw:
	case frameType_pcmS16:
	case frameType_speex:
		// if the frame type is different from track. return false.
		if(writer->audio_track.frame_type != frame->type) {
			ERR_PRINT("invalid audio frame is detected.");
			return false;
		}
		if(!ttLibC_FrameQueue_queue(writer->audio_track.frame_queue, frame)) {
			return false;
		}
		break;
	default:
		ERR_PRINT("unexpected frame is found:%d", frame->type);
		return false;
	}
	return true;
}

static bool FlvWriter_writeFrameAudioOnly(
		ttLibC_FlvWriter_ *writer,
		ttLibC_ContainerWriteFunc callback,
		void *ptr) {
	while(true) {
		ttLibC_Frame *audio = ttLibC_FrameQueue_ref_first(writer->audio_track.frame_queue);
		if(audio == NULL) {
			break;
		}
		audio = ttLibC_FrameQueue_dequeue_first(writer->audio_track.frame_queue);
		writer->inherit_super.inherit_super.pts = audio->pts;
		if(!ttLibC_FlvAudioTag_writeTag(writer, audio, callback, ptr)) {
			return false;
		}
	}
	return true;
}

static bool FlvWriter_writeFrameVideoOnly(
		ttLibC_FlvWriter_ *writer,
		ttLibC_ContainerWriteFunc callback,
		void *ptr) {
	uint32_t count = ttLibC_FrameQueue_getReadyFrameCount(writer->video_track.frame_queue);
	while(true) {
		if(count == 0) { // no more frame. 
			break;
		}
		ttLibC_Frame *video = ttLibC_FrameQueue_ref_first(writer->video_track.frame_queue);
		if(video == NULL) {
			break;
		}
		count --;
		video = ttLibC_FrameQueue_dequeue_first(writer->video_track.frame_queue);
		writer->inherit_super.inherit_super.pts = video->dts;
		if(!ttLibC_FlvVideoTag_writeTag(writer, video, callback, ptr)) {
			return false;
		}
	}
	return true;
}

static int FlvWriter_writeFrame(
		ttLibC_FlvWriter_ *writer,
		ttLibC_ContainerWriteFunc callback,
		void *ptr) {
	uint32_t count = ttLibC_FrameQueue_getReadyFrameCount(writer->video_track.frame_queue);
	while(true) {
		if(count == 0) { // no more frame.
			break;
		}
		ttLibC_Frame *video = ttLibC_FrameQueue_ref_first(writer->video_track.frame_queue);
		ttLibC_Frame *audio = ttLibC_FrameQueue_ref_first(writer->audio_track.frame_queue);
		if(video == NULL || audio == NULL) {
			break;
		}
		if(video->dts > audio->pts) {
			audio = ttLibC_FrameQueue_dequeue_first(writer->audio_track.frame_queue);
			writer->inherit_super.inherit_super.pts = audio->pts;
			if(!ttLibC_FlvAudioTag_writeTag(writer, audio, callback, ptr)) {
				return false;
			}
		}
		else {
			count --;
			video = ttLibC_FrameQueue_dequeue_first(writer->video_track.frame_queue);
			writer->inherit_super.inherit_super.pts = video->dts;
			if(!ttLibC_FlvVideoTag_writeTag(writer, video, callback, ptr)) {
				return false;
			}
		}
	}
	return true;
}

bool TT_ATTRIBUTE_API ttLibC_FlvWriter_write(
		ttLibC_FlvWriter *writer,
		ttLibC_Frame *frame,
		ttLibC_ContainerWriteFunc callback,
		void *ptr) {
	ttLibC_FlvWriter_ *writer_ = (ttLibC_FlvWriter_ *)writer;
	if(writer_->is_first) {
		// try to write header information.
		if(!ttLibC_FlvHeaderTag_writeTag(writer_, callback, ptr)) {
			return false;
		}
		// TODO need to write meta frame?
		writer_->is_first = false;
	}
	// add queue for input frame.
	if(!FlvWriter_queueFrame(writer_, frame)) {
		return false;
	}
	// try to write frames.
	if(writer_->video_track.frame_type == frameType_unknown) {
		if(!FlvWriter_writeFrameAudioOnly(
				writer_,
				callback,
				ptr)) {
			return false;
		}
	}
	else if(writer_->audio_track.frame_type == frameType_unknown) {
		if(!FlvWriter_writeFrameVideoOnly(
				writer_,
				callback,
				ptr)) {
			return false;
		}
	}
	else {
		if(!FlvWriter_writeFrame(
				writer_,
				callback,
				ptr)) {
			return false;
		}
	}
	return true;
}

void TT_ATTRIBUTE_API ttLibC_FlvWriter_close(ttLibC_FlvWriter **writer) {
	ttLibC_FlvWriter_ *target = (ttLibC_FlvWriter_ *)*writer;
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.inherit_super.type != containerType_flv) {
		ERR_PRINT("try to close non flvWriter.");
		return;
	}
	ttLibC_FrameQueue_close(&target->video_track.frame_queue);
	ttLibC_FrameQueue_close(&target->audio_track.frame_queue);
	ttLibC_Frame_close(&target->video_track.configData);
	ttLibC_free(target);
	*writer = NULL;
}
