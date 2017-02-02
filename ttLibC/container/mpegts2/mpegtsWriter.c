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
			180000);
}

ttLibC_MpegtsWriter *ttLibC_MpegtsWriter_make_ex(
		ttLibC_Frame_Type *target_frame_types,
		uint32_t types_num,
		uint32_t unit_duration) {
	ttLibC_MpegtsWriter_ *writer = (ttLibC_MpegtsWriter_ *)ttLibC_ContainerWriter_make_(
			containerType_mpegts,
			sizeof(ttLibC_MpegtsWriter_),
			90000,
			sizeof(ttLibC_MpegtsWriteTrack),
			0x0100,
			target_frame_types,
			types_num,
			unit_duration);
	if(writer == NULL) {
		return NULL;
	}
	writer->is_reduce_mode = false;
	writer->cc_sdt = 0;
	writer->cc_pat = 0;
	writer->cc_pmt = 0;
	writer->data_buffer = NULL;
	writer->dts_margin = 20000;

	ttLibC_Sdt_makePacket((const char *)"ttLibC", (const char *)"mpegtsMuxer", writer->sdt_buf, 188);
	ttLibC_Pat_makePacket(writer->pat_buf, 188);
	ttLibC_Pmt_makePacket(writer, writer->pmt_buf, 188);
	return (ttLibC_MpegtsWriter *)writer;
}

/**
 * update margin of dts calcuration.
 * @param writer
 * @param margin
 * @node default is 20000
 */
bool ttLibC_MpegtsWriter_updateDtsMargin(
		ttLibC_MpegtsWriter *writer,
		uint64_t margin) {
	ttLibC_MpegtsWriter_ *writer_ = (ttLibC_MpegtsWriter_ *)writer;
	if(writer->type != containerType_mpegts) {
		return false;
	}
	writer_->dts_margin = margin;
	return true;
}

static bool MpegtsWriter_makeH264Data(
		ttLibC_MpegtsWriter_ *writer,
		ttLibC_MpegtsWriteTrack *track,
		ttLibC_DynamicBuffer *buffer) {
	if(track->tmp_frame_buffer == NULL) {
		track->tmp_frame_buffer = ttLibC_DynamicBuffer_make();
	}
	ttLibC_DynamicBuffer *dataBuffer = track->tmp_frame_buffer;
	bool result = true;
	while(true) {
		ttLibC_H264 *h264 = (ttLibC_H264 *)ttLibC_FrameQueue_ref_first(track->inherit_super.frame_queue);
		if(h264 != NULL && h264->inherit_super.inherit_super.pts < writer->inherit_super.target_pos) {
			ttLibC_H264 *h = (ttLibC_H264 *)ttLibC_FrameQueue_dequeue_first(track->inherit_super.frame_queue);
			if(h264 != h) {
				ERR_PRINT("data is corrupted. broken?");
				result = false;
				break;
			}
/*			if(h264->inherit_super.inherit_super.pts < writer->current_pts_pos) {
				continue;
			}*/
			uint32_t aud_size = 6;
			uint8_t aud[6] = {0x00, 0x00, 0x00, 0x01, 0x09, 0xF0};
			// make reduce mode later.
/*			if(writer->is_reduce_mode) {
				aud[2] = 0x01;
				aud[3] = 0x09;
				aud[4] = 0xF0;
				aud_size = 5;
			}*/
			uint64_t dts = h264->inherit_super.inherit_super.dts;
			if(dts < writer->dts_margin) {
				dts = 0;
			}
			else {
				dts -= writer->dts_margin;
			}
			ttLibC_DynamicBuffer_append(dataBuffer, aud, aud_size);
			switch(h264->type) {
			case H264Type_slice:
				// slice => aud + slice
				ttLibC_DynamicBuffer_append(dataBuffer, h264->inherit_super.inherit_super.data, h264->inherit_super.inherit_super.buffer_size);
				ttLibC_Pes_writePacket(track, false, false, true, 0xE0, h264->inherit_super.inherit_super.id, h264->inherit_super.inherit_super.pts, dts, dataBuffer, buffer);
				break;
			case H264Type_sliceIDR:
				// sliceIDR => aud + sps + pps + sliceIDR
				{
					ttLibC_H264 *configData = (ttLibC_H264 *)track->inherit_super.h26x_configData;
					if(configData == NULL) {
						ERR_PRINT("no sps pps information for this track. something wrong is happened.");
					}
					else {
						ttLibC_DynamicBuffer_append(dataBuffer, configData->inherit_super.inherit_super.data, configData->inherit_super.inherit_super.buffer_size);
					}
					ttLibC_DynamicBuffer_append(dataBuffer, h264->inherit_super.inherit_super.data, h264->inherit_super.inherit_super.buffer_size);
					ttLibC_Pes_writePacket(track, true, (h264->inherit_super.inherit_super.id == 0x0100), true, 0xE0, h264->inherit_super.inherit_super.id, h264->inherit_super.inherit_super.pts, dts, dataBuffer, buffer);
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
		ttLibC_ContainerWriter_ *writer,
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
	uint64_t current_pts_pos = writer->current_pts_pos;
	while(true) {
		ttLibC_Aac *aac = (ttLibC_Aac *)ttLibC_FrameQueue_ref_first(track->inherit_super.frame_queue);
		if(aac != NULL && aac->inherit_super.inherit_super.pts < writer->target_pos) {
			ttLibC_Aac *a = (ttLibC_Aac *)ttLibC_FrameQueue_dequeue_first(track->inherit_super.frame_queue);
			if(aac != a) {
				ERR_PRINT("data is corrupted. broken?");
				result = false;
				break;
			}
/*			if(aac->inherit_super.inherit_super.pts < writer->current_pts_pos) {
				continue;
			}*/
			// divide data into small unit.
			if(current_pts_pos + writer->unit_duration <= aac->inherit_super.inherit_super.pts) {
				current_pts_pos = aac->inherit_super.inherit_super.pts;
				ttLibC_Pes_writePacket(track, true, pid == 0x0100, true, 0xC0, pid, pts, 0, dataBuffer, buffer);
				pid = 0;
				pts = 0;
				is_info_update = false;
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
	ttLibC_Pes_writePacket(track, true, pid == 0x0100, true, 0xC0, pid, pts, 0, dataBuffer, buffer);
	ttLibC_DynamicBuffer_empty(dataBuffer);
	return result;
}

static bool MpegtsWriter_makeMp3Data(
		ttLibC_ContainerWriter_ *writer,
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
	uint64_t current_pts_pos = writer->current_pts_pos;
	while(true) {
		ttLibC_Mp3 *mp3 = (ttLibC_Mp3 *)ttLibC_FrameQueue_ref_first(track->inherit_super.frame_queue);
		if(mp3 != NULL && mp3->inherit_super.inherit_super.pts < writer->target_pos) {
			ttLibC_Mp3 *m = (ttLibC_Mp3 *)ttLibC_FrameQueue_dequeue_first(track->inherit_super.frame_queue);
			if(mp3 != m) {
				ERR_PRINT("data is corrupted. broken?");
				result = false;
				break;
			}
/*			if(mp3->inherit_super.inherit_super.pts < writer->current_pts_pos) {
				continue;
			}*/
			if(current_pts_pos + writer->unit_duration <= mp3->inherit_super.inherit_super.pts) {
				current_pts_pos = mp3->inherit_super.inherit_super.pts;
				ttLibC_Pes_writePacket(track, true, pid == 0x0100, true, 0xC0, pid, pts, 0, dataBuffer, buffer);
				pid = 0;
				pts = 0;
				is_info_update = false;
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
	ttLibC_Pes_writePacket(track, true, pid == 0x0100, true, 0xC0, pid, pts, 0, dataBuffer, buffer);
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
		ttLibC_ContainerWriter_WriteTrack *track = (ttLibC_ContainerWriter_WriteTrack *)ttLibC_StlMap_get(writer->inherit_super.track_list, (void *)(long)pid);
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
	if(writer->inherit_super.callback != NULL) {
		if(ttLibC_DynamicBuffer_refSize(buffer) != 0) {
			result = writer->inherit_super.callback(writer->inherit_super.ptr, ttLibC_DynamicBuffer_refData(buffer), ttLibC_DynamicBuffer_refSize(buffer));
		}
	}
	ttLibC_DynamicBuffer_empty(buffer);
	return result;
}

static bool MpegtsWriter_writeFromQueue(
		ttLibC_ContainerWriter_ *writer) {
	switch(writer->status) {
	case status_init_check:
		{
			if(ttLibC_ContainerWriter_isReadyToStart(writer)) {
				writer->status = status_make_init;
				return MpegtsWriter_writeFromQueue(writer);
			}
		}
		break;
	case status_make_init:
		{
			if(ttLibC_MpegtsWriter_writeInfo(
					writer,
					writer->callback,
					writer->ptr)) {
				writer->status = status_target_check;
				return MpegtsWriter_writeFromQueue(writer);
			}
		}
		break;
	case status_target_check:
		{
			ttLibC_ContainerWriter_WriteTrack *track = (ttLibC_ContainerWriter_WriteTrack *)ttLibC_StlMap_get(writer->track_list, (void *)(long)0x0100);
			ttLibC_FrameQueue_ref(track->frame_queue, ttLibC_ContainerWriter_primaryTrackCheck, writer);
			if(writer->target_pos != writer->current_pts_pos) {
				writer->status = status_data_check;
				return MpegtsWriter_writeFromQueue(writer);
			}
		}
		break;
	case status_data_check:
		{
			if(ttLibC_ContainerWriter_isReadyToWrite(writer)) {
				// all track is ok.
				writer->status = status_make_data;
				return MpegtsWriter_writeFromQueue(writer);
			}
		}
		break;
	case status_make_data:
		{
			if(MpegtsWriter_makeData(writer)) {
				writer->status = status_update;
				return MpegtsWriter_writeFromQueue(writer);
			}
			else {
				ERR_PRINT("we already check the range of data, however, some error happen to make data. so, there is some serious error.");
			}
		}
		break;
	case status_update:
		{
			writer->current_pts_pos = writer->target_pos;
			writer->inherit_super.pts = writer->target_pos;
			writer->status = status_target_check;
		}
		break;
	default:
		break;
	}
	return true;
}

bool ttLibC_MpegtsWriter_write(
		ttLibC_MpegtsWriter *writer,
		ttLibC_Frame *frame,
		ttLibC_ContainerWriteFunc callback,
		void *ptr) {
	ttLibC_ContainerWriter_ *writer_ = (ttLibC_ContainerWriter_ *)writer;
	switch(ttLibC_ContainerWriter_write_(
			writer_,
			frame,
			callback,
			ptr)) {
	case -1:
	default:
		return false;
	case 0:
		return true;
	case 1:
		break;
	}
	ttLibC_ContainerWriter_WriteTrack *track = (ttLibC_ContainerWriter_WriteTrack *)ttLibC_StlMap_get(writer_->track_list, (void *)(long)frame->id);
	if(track != NULL) {
		track->use_mode = track->enable_mode;
	}
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
	if(writer_->inherit_super.is_first) {
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
		ttLibC_DynamicBuffer_close(&track->tmp_buffer);
		ttLibC_DynamicBuffer_close(&track->tmp_frame_buffer);
		ttLibC_ContainerWriteTrack_close((ttLibC_ContainerWriter_WriteTrack **)&track);
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
	ttLibC_StlMap_forEach(target->inherit_super.track_list, MpegtsWriter_closeTracks, NULL);
	ttLibC_StlMap_close(&target->inherit_super.track_list);
	ttLibC_ContainerWriter_close_(writer);
}
