/*
 * @file   flvWriter.c
 * @brief  
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
#include "../../log.h"
#include "../../util/hexUtil.h"
#include "../../frame/video/h264.h"
#include <stdlib.h>

ttLibC_FlvWriter *ttLibC_FlvWriter_make(
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
		// そんなにためることもないでしょう。
		// 貯める必要がありました。元のデータがなんであるかによって、stackを大きめにとる必要があるかは変わってくるみたいです。
		// ソースをmpegtsにしたところ、stackが小さすぎて正しい動作にならなかった。
		writer->video_track.frame_queue = ttLibC_FrameQueue_make(9, 255);
		break;
	default:
		writer->video_track.crc32       = 0;
		writer->video_track.frame_type  = frameType_unknown;
		writer->video_track.frame_queue = NULL;
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
	return (ttLibC_FlvWriter *)writer;
}

bool ttLibC_FlvWriter_write(
		ttLibC_FlvWriter *writer,
		ttLibC_Frame *frame,
		ttLibC_ContainerWriterFunc callback,
		void *ptr) {
	ttLibC_FlvWriter_ *writer_ = (ttLibC_FlvWriter_ *)writer;
	if(writer_->is_first) {
		// flvHeader情報を応答しなければいけない。
		if(!ttLibC_FlvHeaderTag_writeTag(writer_, callback, ptr)) {
			return false;
		}
		// 必要ならここでmetaタグも書いておきたい。
		// まぁttLibCのクレジットくらいだけど・・・書き込む情報・・・
		writer_->is_first = false;
	}
	// timebaseを1000に合わせておく。
	frame->pts = (uint64_t)(1.0 * frame->pts * 1000 / frame->timebase);
	frame->timebase = 1000;
	switch(frame->type) {
	case frameType_h264:
		{
			ttLibC_H264 *h264 = (ttLibC_H264 *)frame;
			if(h264->type == H264Type_unknown) {
				// unknownデータはskipしておく。(AUDとか)
				return true;
			}
		}
	case frameType_flv1:
	case frameType_vp6:
		// 以下videoTagの指定と一致するか確認してから、videoTagで処理する。
		if(writer_->video_track.frame_type != frame->type) {
			ERR_PRINT("invalid video frame is detected.");
			return false;
		}
		// まずqueueにためておく。
 		ttLibC_FrameQueue_queue(writer_->video_track.frame_queue, frame);
		break;
	case frameType_aac:
	case frameType_mp3:
	case frameType_nellymoser:
	case frameType_pcm_alaw:
	case frameType_pcm_mulaw:
	case frameType_pcmS16:
	case frameType_speex:
		if(writer_->audio_track.frame_type != frame->type) {
			ERR_PRINT("invalid audio frame is detected.");
			return false;
		}
		ttLibC_FrameQueue_queue(writer_->audio_track.frame_queue, frame);
		// こっちもqueueにためておく。
		break;
	default:
		ERR_PRINT("unexpected frame is found:%d", frame->type);
		return false;
	}
	// audioQueueとVideoQueueを比較して、timestampが若いのを優先して出力するようにする。
	// ただしvideo優先
	while(true) {
		ttLibC_Frame *video = ttLibC_FrameQueue_ref_first(writer_->video_track.frame_queue);
		ttLibC_Frame *audio = ttLibC_FrameQueue_ref_first(writer_->audio_track.frame_queue);
		if(video == NULL || audio == NULL) {
			// 追加すべきデータがない。
			break;
		}
		if(video->pts > audio->pts) {
			audio = ttLibC_FrameQueue_dequeue_first(writer_->audio_track.frame_queue);
			writer_->inherit_super.inherit_super.pts = audio->pts;
			// audioを追加すべき
			if(!ttLibC_FlvAudioTag_writeTag(writer_, audio, callback, ptr)) {
				return false;
			}
		}
		else {
			video = ttLibC_FrameQueue_dequeue_first(writer_->video_track.frame_queue);
			writer_->inherit_super.inherit_super.pts = video->pts;
			// videoを追加すべき
			if(!ttLibC_FlvVideoTag_writeTag(writer_, video, callback, ptr)) {
				return false;
			}
		}
	}
	return true;
}

void ttLibC_FlvWriter_close(ttLibC_FlvWriter **writer) {
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
	free(target);
	*writer = NULL;
}
