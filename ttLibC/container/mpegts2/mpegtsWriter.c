/*
 * @file   mpegtsWriter.c
 * @brief  mpegts container writer.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2016/11/18
 */

#include "../mpegts.h"

#include "mpegtsWriter.h"

#include "type/pat.h"
#include "type/pes.h"
#include "type/pmt.h"
#include "type/sdt.h"

#include "../../frame/video/h264.h"
#include "../../frame/audio/mp3.h"
#include "../../frame/audio/aac.h"

#include "../../log.h"
#include "../../allocator.h"
#include "../../util/hexUtil.h"

#include <stdlib.h>

ttLibC_MpegtsWriter *ttLibC_MpegtsWriter_make(
		ttLibC_Frame_Type *target_frame_types,
		uint32_t types_num) {
	return ttLibC_MpegtsWriter_make_ex(
			target_frame_types,
			types_num,
			900000);
}

ttLibC_MpegtsWriter *ttLibC_MpegtsWriter_make_ex(
		ttLibC_Frame_Type *target_frame_types,
		uint32_t types_num,
		uint32_t max_unit_duration) {
	ttLibC_MpegtsWriter_ *writer = (ttLibC_MpegtsWriter_ *)ttLibC_ContainerWriter_make(
			containerType_mpegts,
			sizeof(ttLibC_MpegtsWriter_),
			90000);
	if(writer == NULL) {
		ERR_PRINT("failed to allocate writer.");
		return NULL;
	}
	// prepare for track.
	writer->track_list = ttLibC_StlMap_make();
	for(uint32_t i = 0;i < types_num;++ i) {
		ttLibC_MpegtsWriteTrack *track = ttLibC_malloc(sizeof(ttLibC_MpegtsWriteTrack));;
		track->frame_queue      = ttLibC_FrameQueue_make(0x0100 + i, 255);
		track->h26x_configData  = NULL;
		track->frame_type       = target_frame_types[i];
		track->is_appending     = false;
		track->cc               = 0;
		track->enable_dts       = false;
		track->use_dts          = false;
		track->tmp_buffer       = NULL;
		track->tmp_frame_buffer = NULL;
		ttLibC_StlMap_put(writer->track_list, (void *)(long)(0x0100 + i), (void *)track);
	}
	writer->inherit_super.inherit_super.timebase = 90000;
	writer->inherit_super.inherit_super.pts = 0;
	writer->is_first          = true;
	writer->is_reduce_mode    = false;
	writer->cc_sdt            = 0;
	writer->cc_pat            = 0;
	writer->cc_pmt            = 0;
	writer->max_unit_duration = max_unit_duration;
	writer->status            = status_target_check;
	writer->data_buffer       = NULL;

	ttLibC_Sdt_makePacket((const char *)"ttLibC", (const char *)"mpegtsMuxer", writer->sdt_buf, 188);
	ttLibC_Pat_makePacket(writer->pat_buf, 188);
	ttLibC_Pmt_makePacket(writer, writer->pmt_buf, 188);
	return (ttLibC_MpegtsWriter *)writer;
}

static bool MpegtsWriter_makeH264Data(
		ttLibC_MpegtsWriter_ *writer,
		ttLibC_MpegtsWriteTrack *track,
		ttLibC_DynamicBuffer *buffer) {
	// とりあえずこれが一番面倒か・・・
	// pesにbufferを渡してデータをいれてもらうことにする。
	// sliceの場合はaud + slice
	// sliceIDRの場合はaud + sps + pps + sliceIDR
	// というbufferを作らなければならない。
	// reducemodeの場合は、nalの分解を00 00 00 01ではなく00 00 01にしなければならないわけだが・・・
	// データ実体はこんなところ。

	// sliceはadaptationFieldはいらないっぽい。
	// sliceIDRはadaptationFieldでpcrPIDの場合はptsの書き込みも必要・・・
	// これは渡すframe情報でなんとかできるからいいか。frame->idが0x0100である場合に判定できる。
	// あとはrandom accessが可能であるかのフラグも必要。これもadaptationFieldの情報

	// pesデータの登録としては、まずtrackマスクになる値が必要。映像なら0xE0、音声なら0xC0が対象になる。

	if(track->tmp_frame_buffer == NULL) {
		track->tmp_frame_buffer = ttLibC_DynamicBuffer_make();
	}
	ttLibC_DynamicBuffer *dataBuffer = track->tmp_frame_buffer;
	bool result = true;
	// 必要になる情報はこんなものかねぇ・・・
	// とりあえずframeを取り出して処理にまわさないとだめだな。
	// とりあえず使うべきframeをpop upして減らしておこうか・・・
	while(true) {
		ttLibC_H264 *h264 = (ttLibC_H264 *)ttLibC_FrameQueue_ref_first(track->frame_queue);
		if(h264 != NULL && h264->inherit_super.inherit_super.pts < writer->target_pos) {
			ttLibC_H264 *h = (ttLibC_H264 *)ttLibC_FrameQueue_dequeue_first(track->frame_queue);
			if(h264 != h) {
				ERR_PRINT("data is corrupted. broken?");
				result = false;
				break;
			}
			if(h264->inherit_super.inherit_super.pts < writer->current_pts_pos) {
				continue;
			}
			// とりあえずデータchunkを作らなければならない。
			// dynamicBufferにつくるのはいいけど・・・メモリーのreallocがもったいないか・・・
			// メモリーのallocもできた。
			// さて・・・
			uint32_t aud_size = 6;
			uint8_t aud[6] = {0x00, 0x00, 0x00, 0x01, 0x09, 0xF0};
			// とりあえずこういうややこしいのはおいといてつくるだけつくってみよう。
/*			if(writer->is_reduce_mode) {
				aud[2] = 0x01;
				aud[3] = 0x09;
				aud[4] = 0xF0;
				aud_size = 5;
			}*/
			ttLibC_DynamicBuffer_append(dataBuffer, aud, aud_size);
			switch(h264->type) {
			case H264Type_slice:
				// sliceの場合はaud + slice
				ttLibC_DynamicBuffer_append(dataBuffer, h264->inherit_super.inherit_super.data, h264->inherit_super.inherit_super.buffer_size);
				ttLibC_Pes_writePacket(track, false, false, 0xE0, h264->inherit_super.inherit_super.id, h264->inherit_super.inherit_super.pts, h264->inherit_super.inherit_super.dts, dataBuffer, buffer);
				break;
			case H264Type_sliceIDR:
				// sliceIDRの場合はaud + sps + pps + sliceIDRという形にしておく。
				{
					ttLibC_H264 *configData = (ttLibC_H264 *)track->h26x_configData;
					if(configData == NULL) {
						ERR_PRINT("no sps pps information for this track. something wrong is happened.");
					}
					else {
						ttLibC_DynamicBuffer_append(dataBuffer, configData->inherit_super.inherit_super.data, configData->inherit_super.inherit_super.buffer_size);
					}
					ttLibC_DynamicBuffer_append(dataBuffer, h264->inherit_super.inherit_super.data, h264->inherit_super.inherit_super.buffer_size);
					ttLibC_Pes_writePacket(track, true, false, 0xE0, h264->inherit_super.inherit_super.id, h264->inherit_super.inherit_super.pts, h264->inherit_super.inherit_super.dts, dataBuffer, buffer);
				}
				break;
			default:
				break;
			}
			ttLibC_DynamicBuffer_empty(dataBuffer);
		}
		else {
			break;
		}
	}
	ttLibC_DynamicBuffer_empty(dataBuffer);
	return result;
}

static bool MpegtsWriter_makeAacData(
		ttLibC_MpegtsWriter_ *writer,
		ttLibC_MpegtsWriteTrack *track,
		ttLibC_DynamicBuffer *buffer) {
	if(track->tmp_frame_buffer == NULL) {
		track->tmp_frame_buffer = ttLibC_DynamicBuffer_make();
	}
	ttLibC_DynamicBuffer *dataBuffer = track->tmp_frame_buffer;
	bool result = true;
	uint32_t pid = 0;
	uint64_t pts = 0;
	bool is_info_update = false;
	while(true) {
		ttLibC_Aac *aac = (ttLibC_Aac *)ttLibC_FrameQueue_ref_first(track->frame_queue);
		if(aac != NULL && aac->inherit_super.inherit_super.pts < writer->target_pos) {
			ttLibC_Aac *a = (ttLibC_Aac *)ttLibC_FrameQueue_dequeue_first(track->frame_queue);
			if(aac != a) {
				ERR_PRINT("data is corrupted. broken?");
				result = false;
				break;
			}
			if(aac->inherit_super.inherit_super.pts < writer->current_pts_pos) {
				continue;
			}
			if(!is_info_update) {
				pid = aac->inherit_super.inherit_super.id;
				pts = aac->inherit_super.inherit_super.pts;
				is_info_update = true;
			}
			switch(aac->type) {
			case AacType_raw:
				{
					// need to convert to adts format.
					uint8_t aac_header_buf[7];
					if(ttLibC_Aac_readAdtsHeader(aac, aac_header_buf, 7) == 0) {
						ERR_PRINT("failed to get adts header information.");
					}
					ttLibC_DynamicBuffer_append(dataBuffer, aac_header_buf, 7);
					ttLibC_DynamicBuffer_append(dataBuffer, aac->inherit_super.inherit_super.data, aac->inherit_super.inherit_super.buffer_size);
				}
				break;
			case AacType_adts:
				ttLibC_DynamicBuffer_append(dataBuffer, aac->inherit_super.inherit_super.data, aac->inherit_super.inherit_super.buffer_size);
				break;
			case AacType_dsi:
			default:
				break;
			}
		}
		else {
			break;
		}
	}
	ttLibC_Pes_writePacket(track, true, true, 0xC0, pid, pts, 0, dataBuffer, buffer);
	ttLibC_DynamicBuffer_empty(dataBuffer);
	return result;
}

static bool MpegtsWriter_makeMp3Data(
		ttLibC_MpegtsWriter_ *writer,
		ttLibC_MpegtsWriteTrack *track,
		ttLibC_DynamicBuffer *buffer) {
	if(track->tmp_frame_buffer == NULL) {
		track->tmp_frame_buffer = ttLibC_DynamicBuffer_make();
	}
	ttLibC_DynamicBuffer *dataBuffer = track->tmp_frame_buffer;
	bool result = true;
	uint32_t pid = 0;
	uint64_t pts = 0;
	bool is_info_update = false;
	while(true) {
		ttLibC_Mp3 *mp3 = (ttLibC_Mp3 *)ttLibC_FrameQueue_ref_first(track->frame_queue);
		if(mp3 != NULL && mp3->inherit_super.inherit_super.pts < writer->target_pos) {
			ttLibC_Mp3 *m = (ttLibC_Mp3 *)ttLibC_FrameQueue_dequeue_first(track->frame_queue);
			if(mp3 != m) {
				ERR_PRINT("data is corrupted. broken?");
				result = false;
				break;
			}
			if(mp3->inherit_super.inherit_super.pts < writer->current_pts_pos) {
				continue;
			}
			if(!is_info_update) {
				pid = mp3->inherit_super.inherit_super.id;
				pts = mp3->inherit_super.inherit_super.pts;
				is_info_update = true;
			}
			switch(mp3->type) {
			case Mp3Type_frame:
				ttLibC_DynamicBuffer_append(dataBuffer, mp3->inherit_super.inherit_super.data, mp3->inherit_super.inherit_super.buffer_size);
				break;
			default:
				break;
			}
		}
		else {
			break;
		}
	}
	ttLibC_Pes_writePacket(track, true, true, 0xC0, pid, pts, 0, dataBuffer, buffer);
	ttLibC_DynamicBuffer_empty(dataBuffer);
	return true;
}

static bool MpegtsWriter_makeData(ttLibC_MpegtsWriter_ *writer) {
	if(writer->data_buffer == NULL) {
		writer->data_buffer = ttLibC_DynamicBuffer_make();
	}
	ttLibC_DynamicBuffer *buffer = writer->data_buffer;
	uint32_t pid = 0x0100;
	bool result = true;
	while(true) {
		ttLibC_MpegtsWriteTrack *track = (ttLibC_MpegtsWriteTrack *)ttLibC_StlMap_get(writer->track_list, (void *)(long)pid);
		if(track == NULL) {
			break;
		}
		switch(track->frame_type) {
		case frameType_h264:
			if(!MpegtsWriter_makeH264Data(
					writer,
					track,
					buffer)) {
				result = false;
			}
			break;
		case frameType_aac:
			if(!MpegtsWriter_makeAacData(
					writer,
					track,
					buffer)) {
				result = false;
			}
			break;
		case frameType_mp3:
			if(!MpegtsWriter_makeMp3Data(
					writer,
					track,
					buffer)) {
				result = false;
			}
			break;
		default:
			break;
		}
		if(!result) {
			break;
		}
		++ pid;
	}
	// output is complete, call callback.
	if(writer->callback != NULL) {
		if(ttLibC_DynamicBuffer_refSize(buffer) != 0) {
			result = writer->callback(writer->ptr, ttLibC_DynamicBuffer_refData(buffer), ttLibC_DynamicBuffer_refSize(buffer));
		}
	}
	ttLibC_DynamicBuffer_empty(buffer);
	return result;
}

static bool MpegtsWriter_primaryH26xTrackCheck(void *ptr, ttLibC_Frame *frame) {
	ttLibC_MpegtsWriter_ *writer = (ttLibC_MpegtsWriter_ *)ptr;
	ttLibC_Video *video = (ttLibC_Video *)frame;
	switch(video->type) {
	case videoType_inner:
		{
			if(video->inherit_super.pts - writer->current_pts_pos > writer->max_unit_duration) {
				writer->target_pos = writer->current_pts_pos + writer->max_unit_duration;
				return false;
			}
		}
		break;
	case videoType_key:
		{
			if(writer->current_pts_pos + writer->max_unit_duration < video->inherit_super.pts) {
				writer->target_pos = writer->current_pts_pos + writer->max_unit_duration;
				return false;
			}
			if(writer->current_pts_pos != video->inherit_super.pts) {
				writer->target_pos = video->inherit_super.pts;
				return false;
			}
		}
		break;
	default:
		return true;
	}
	return true;
}

static bool MpegtsWriter_dataCheckTrack(void *ptr, void *key, void *item) {
	(void)key;
	if(ptr != NULL && item != NULL) {
		ttLibC_MpegtsWriter_ *writer = (ttLibC_MpegtsWriter_ *)ptr;
		ttLibC_MpegtsWriteTrack *track = (ttLibC_MpegtsWriteTrack *)item;
		if(writer->target_pos > track->frame_queue->pts) {
			return false;
		}
		return true;
	}
	return false;
}

static bool MpegtsWriter_writeFromQueue(
		ttLibC_MpegtsWriter_ *writer) {
	switch(writer->status) {
	case status_target_check: // pcrのデータから書き込む範囲を決定する
		{
			ttLibC_MpegtsWriteTrack *track = (ttLibC_MpegtsWriteTrack *)ttLibC_StlMap_get(writer->track_list, (void *)(long)0x100);
			switch(track->frame_type) {
			case frameType_h264:
				ttLibC_FrameQueue_ref(track->frame_queue, MpegtsWriter_primaryH26xTrackCheck, writer);
				break;
			case frameType_aac:
			case frameType_mp3:
				writer->target_pos = writer->current_pts_pos + writer->max_unit_duration;
				break;
			default:
				ERR_PRINT("unexpected frame is found.");
				return false;
			}
			if(writer->target_pos != writer->current_pts_pos) {
				writer->status = status_data_check;
				return MpegtsWriter_writeFromQueue(writer);
			}
		}
		break;
	case status_data_check:   // 書き込む範囲にきちんとしたデータがあることを確認する。
		{
			if(ttLibC_StlMap_forEach(writer->track_list, MpegtsWriter_dataCheckTrack, writer)) {
				// all track is ok.
				writer->status = status_make_data;
				return MpegtsWriter_writeFromQueue(writer);
			}
		}
		break;
	case status_make_data:    // 実際の書き込むデータを作る。
		{
			// データを作る。
			if(MpegtsWriter_makeData(writer)) {
				// trueが応答された書き出し完了とする。
				writer->status = status_update;
				return MpegtsWriter_writeFromQueue(writer);
			}
			else {
				ERR_PRINT("we already check the range of data, however, some error happen to make data. so, there is some serious error.");
			}
		}
		break;
	case status_update:       // 次のフェーズに移動する。
		{
			writer->current_pts_pos = writer->target_pos;
			writer->status = status_target_check;
		}
		break;
	default:
		break;
	}
	return true;
}

static bool MpegtsWriter_appendQueue(
		ttLibC_MpegtsWriteTrack *track,
		ttLibC_Frame *frame,
		uint64_t pts) {
	uint64_t original_pts = frame->pts;
	uint32_t original_timebase = frame->timebase;
	frame->pts = pts;
	frame->timebase = 1000;
	bool result = ttLibC_FrameQueue_queue(track->frame_queue, frame);
	frame->pts = original_pts;
	frame->timebase = original_timebase;
	return result;
}

bool ttLibC_MpegtsWriter_write(
		ttLibC_MpegtsWriter *writer,
		ttLibC_Frame *frame,
		ttLibC_ContainerWriteFunc callback,
		void *ptr) {
	ttLibC_MpegtsWriter_ *writer_ = (ttLibC_MpegtsWriter_ *)writer;
	if(writer_ == NULL) {
		ERR_PRINT("writer is null.");
		return false;
	}
	if(frame == NULL) {
		return true;
	}
	ttLibC_MpegtsWriteTrack *track = (ttLibC_MpegtsWriteTrack *)ttLibC_StlMap_get(writer_->track_list, (void *)(long)frame->id);
	if(track == NULL) {
		ERR_PRINT("failed to get correspond track. %x", frame->id);
		return false;
	}
	uint64_t pts = (uint64_t)(1.0 * frame->pts * 90000 / frame->timebase);
	track->enable_dts = writer->enable_dts;
	// trackにframeを追加する。
	switch(frame->type) {
	case frameType_h264:
		{
			ttLibC_H264 *h264 = (ttLibC_H264 *)frame;
			if(h264->type == H264Type_unknown) {
				return true;
			}
			if(h264->type == H264Type_configData) {
				ttLibC_H264 *h = ttLibC_H264_clone(
						(ttLibC_H264 *)track->h26x_configData,
						h264);
				if(h == NULL) {
					ERR_PRINT("failed to make clone data.");
					return false;
				}
				h->inherit_super.inherit_super.pts = 0;
				h->inherit_super.inherit_super.timebase = 90000;
				track->h26x_configData = (ttLibC_Frame *)h;
			}
			if(writer_->is_first) {
				track->use_dts = track->enable_dts;
			}
			if(!track->is_appending && h264->type != H264Type_sliceIDR) {
				// queueにデータがなく、sliceIDRでない場合はまだ始める時期ではない。
				return true;
			}
		}
		break;
	default:
		break;
	}
	track->is_appending = true;
	if(writer_->is_first) {
		writer_->current_pts_pos = pts;
		writer_->target_pos = pts;
		writer_->inherit_super.inherit_super.pts = pts;
		writer_->is_first = false;
		if(!ttLibC_MpegtsWriter_writeInfo(
				writer,
				callback,
				ptr)) {
			return false;
		}
	}
	if(!MpegtsWriter_appendQueue(track, frame, pts)) {
		return false;
	}
	writer_->callback = callback;
	writer_->ptr = ptr;
	return MpegtsWriter_writeFromQueue(writer_);
}

bool ttLibC_MpegtsWriter_writeInfo(
		ttLibC_MpegtsWriter *writer,
		ttLibC_ContainerWriteFunc callback,
		void *ptr) {
	ttLibC_MpegtsWriter_ *writer_ = (ttLibC_MpegtsWriter_ *)writer;
	if(writer_ == NULL) {
		return false;
	}
	if(writer_->is_first) {
		return true;
	}
	// sdt
	writer_->sdt_buf[3] = (writer_->sdt_buf[3] & 0xF0) | writer_->cc_sdt;
	writer_->cc_sdt = (writer_->cc_sdt + 1) & 0x0F;
	if(!callback(ptr, writer_->sdt_buf, 188)) {
		return false;
	}
	// pat
	writer_->pat_buf[3] = (writer_->pat_buf[3] & 0xF0) | writer_->cc_pat;
	writer_->cc_pat = (writer_->cc_pat + 1) & 0x0F;
	if(!callback(ptr, writer_->pat_buf, 188)) {
		return false;
	}
	// pmt
	writer_->pmt_buf[3] = (writer_->pmt_buf[3] & 0xF0) | writer_->cc_pmt;
	writer_->cc_pmt = (writer_->cc_pmt + 1) & 0x0F;
	if(!callback(ptr, writer_->pmt_buf, 188)) {
		return false;
	}
	return true;
}

bool ttLibC_MpegtsWriter_setReduceMode(
		ttLibC_MpegtsWriter *writer,
		bool reduce_mode_flag) {
	ttLibC_MpegtsWriter_ *writer_ = (ttLibC_MpegtsWriter_ *)writer;
	if(writer_ == NULL) {
		return false;
	}
	writer_->is_reduce_mode = reduce_mode_flag;
	return true;
}

static bool MpegtsWriter_closeTracks(void *ptr, void *key, void *item) {
	(void)ptr;
	(void)key;
	if(item != NULL) {
		ttLibC_MpegtsWriteTrack *track = (ttLibC_MpegtsWriteTrack *)item;
		ttLibC_FrameQueue_close(&track->frame_queue);
		ttLibC_Frame_close(&track->h26x_configData);
		ttLibC_DynamicBuffer_close(&track->tmp_buffer);
		ttLibC_DynamicBuffer_close(&track->tmp_frame_buffer);
		ttLibC_free(track);
	}
	return true;
}

void ttLibC_MpegtsWriter_close(ttLibC_MpegtsWriter **writer) {
	ttLibC_MpegtsWriter_ *target = (ttLibC_MpegtsWriter_ *)*writer;
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.inherit_super.type != containerType_mpegts) {
		ERR_PRINT("try to close non mpegtsWriter.");
		return;
	}
	ttLibC_DynamicBuffer_close(&target->data_buffer);
	ttLibC_StlMap_forEach(target->track_list, MpegtsWriter_closeTracks, NULL);
	ttLibC_StlMap_close(&target->track_list);
	ttLibC_free(target);
	*writer = NULL;
}
