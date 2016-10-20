/*
 * @file   mkvWriter.c
 * @brief  mkv container writer.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/09/04
 */

#include "mkvWriter.h"

#include "../../log.h"
#include "../../allocator.h"
#include "../../util/hexUtil.h"
#include "../container.h"
#include "../containerCommon.h"
#include "../../frame/audio/aac.h"
#include "../../frame/video/h264.h"

#include <stdlib.h>

ttLibC_MkvWriter *ttLibC_MkvWriter_make(
		ttLibC_Frame_Type* target_frame_types,
		uint32_t types_num) {
	return ttLibC_MkvWriter_make_ex(
			target_frame_types,
			types_num,
			5000); // 5sec for target_unit_duration
}

ttLibC_MkvWriter *ttLibC_MkvWriter_make_ex(
		ttLibC_Frame_Type* target_frame_types,
		uint32_t types_num,
		uint32_t max_unit_duration) {
	ttLibC_MkvWriter_ *writer = (ttLibC_MkvWriter_ *)ttLibC_ContainerWriter_make(
			containerType_mkv,
			sizeof(ttLibC_MkvWriter_),
			1000); // work with 1000 only for now.
	if(writer == NULL) {
		ERR_PRINT("failed to allocate writer.");
		return NULL;
	}
	writer->track_list = ttLibC_StlMap_make();
	// trackをつくっておく。
	for(int i = 0;i < types_num;++ i) {
		ttLibC_MkvTrack *track = ttLibC_malloc(sizeof(ttLibC_MkvTrack));
		track->frame_queue = ttLibC_FrameQueue_make(i + 1, 255);
		track->h264_configData = NULL;
		track->frame_type = target_frame_types[i];
		// これだけでよさそう。
		ttLibC_StlMap_put(writer->track_list, (void *)(i + 1), (void *)track);
	}
	// とりあえず後で
	writer->inherit_super.inherit_super.timebase = 1000;
	writer->inherit_super.inherit_super.pts = 0;
	writer->max_unit_duration = max_unit_duration;
	writer->is_first = true;
	writer->target_pos = 0;
	writer->status = status_init_check;
//	writer->chunk_counter = 1; // これ必要かな？
	return (ttLibC_MkvWriter *)writer;
}

static bool MkvWriter_writeFromQueue(
		ttLibC_MkvWriter_ *writer) {
	switch(writer->status) {
	case status_init_check: // 初期データ作成可能か確認
		LOG_PRINT("初期データ作成可能かチェック。");
		return true;
	case status_make_init: // 初期データ作成
	case status_target_check: // cluster書き込み先がどの程度になるか初めのトラックから判断
	case status_data_check: // cluster分書き込むのに必要なデータがあるか確認
	case status_make_data: // clusterの書き込みを実施
	case status_update: // 次の処理へ移動する。(status_target_checkにいく。)
		break;
	default:
		break;
	}
	return true;
}

bool ttLibC_MkvWriter_write(
		ttLibC_MkvWriter *writer,
		ttLibC_Frame *frame,
		ttLibC_ContainerWriteFunc callback,
		void *ptr) {
	ttLibC_MkvWriter_ *writer_ = (ttLibC_MkvWriter_ *)writer;
	if(writer_ == NULL) {
		ERR_PRINT("writer is null.");
		return false;
	}
	if(frame == NULL) {
		return true;
	}
	uint64_t pts = 0;
	switch(frame->type) {
	case frameType_h264:
		{
			pts = (uint64_t)(1.0 * frame->pts * 1000 / frame->timebase);
			frame->pts = pts;
			frame->timebase = 1000;
		}
		break;
	case frameType_aac:
		{
			ttLibC_Audio *audio = (ttLibC_Audio *)frame;
			pts = (uint64_t)(1.0 * frame->pts * audio->sample_rate / frame->timebase);
			frame->pts = pts;
			frame->timebase = audio->sample_rate;
		}
		break;
	default:
		return true;
	}
	// 該当trackを取得
	ttLibC_MkvTrack *track = (ttLibC_MkvTrack *)ttLibC_StlMap_get(writer_->track_list, (void *)frame->id);
	if(track == NULL) {
		ERR_PRINT("failed to get correspond track. %d", frame->id);
		return false;
	}
	// trackにframeを追加する。
	LOG_PRINT("trackにframeを追加しなければ・・・");
	switch(frame->type) {
	case frameType_h264:
		{
			ttLibC_H264 *h264 = (ttLibC_H264 *)frame;
			if(h264->type == H264Type_unknown) {
				return true;
			}
			if(h264->type == H264Type_configData) {
				ttLibC_H264 *h = ttLibC_H264_clone(
						(ttLibC_H264 *)track->h264_configData,
						h264);
				if(h == NULL) {
					ERR_PRINT("failed to make clone data.");
					return false;
				}
				h->inherit_super.inherit_super.pts = 0;
				h->inherit_super.inherit_super.timebase = 1000;
				track->h264_configData = (ttLibC_Frame *)h;
				return true;
			}
			if(writer_->is_first) {
				writer_->current_pts_pos = pts;
				writer_->target_pos = pts;
				writer_->inherit_super.inherit_super.pts = pts;
				writer_->is_first = false;
			}
		}
		/* no break */
	default:
		{
			if(writer_->is_first) {
				return true;
			}
			if(!ttLibC_FrameQueue_queue(track->frame_queue, frame)) {
				return false;
			}
		}
		break;
	}
	writer_->callback = callback;
	writer_->ptr = ptr;
	// Queue処理に進む。
	return MkvWriter_writeFromQueue(writer_);
}

static bool MkvWriter_closeTracks(void *ptr, void *key, void *item) {
	if(item != NULL) {
		// trackの内容データを解放する処理を書いておく。
		ttLibC_MkvTrack *track = (ttLibC_MkvTrack *)item;
		ttLibC_FrameQueue_close(&track->frame_queue);
		ttLibC_Frame_close(&track->h264_configData);
		ttLibC_free(track);
	}
	return true;
}

void ttLibC_MkvWriter_close(ttLibC_MkvWriter **writer) {
	ttLibC_MkvWriter_ *target = (ttLibC_MkvWriter_ *)*writer;
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.inherit_super.type != containerType_mkv) {
		ERR_PRINT("try to close non mkvWriter.");
		return;
	}
	ttLibC_StlMap_forEach(target->track_list, MkvWriter_closeTracks, NULL);
	ttLibC_StlMap_close(&target->track_list);
	ttLibC_free(target);
	*writer = NULL;
}

