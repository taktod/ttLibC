/*
 * @file   mpegtsWriter.c
 * @brief  mpegts container writer.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/13
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

/**
 * passing data for frameQueue callback.
 */
typedef struct {
	ttLibC_MpegtsWriter_ *writer;
	ttLibC_MpegtsTrack *track;
	/** error_number, 0 is no error */
	bool error_number;
} MpegtsWriter_CallbackPtr_t;

/**
 * make mpegtsWriter
 * with default duration.(1sec)
 */
ttLibC_MpegtsWriter *ttLibC_MpegtsWriter_make(
		ttLibC_Frame_Type *target_frame_types,
		uint32_t types_num) {
	return ttLibC_MpegtsWriter_make_ex(target_frame_types, types_num, 90000);
}

/*
 * make mpegtsWriter object.
 * @param target_frame_types
 * @param types_num
 * @param max_unit_duration
 * @return writer object.
 */
ttLibC_MpegtsWriter *ttLibC_MpegtsWriter_make_ex(
		ttLibC_Frame_Type* target_frame_types,
		uint32_t types_num,
		uint32_t max_unit_duration) {
	ttLibC_MpegtsWriter_ *writer = (ttLibC_MpegtsWriter_ *)ttLibC_ContainerWriter_make(containerType_mpegts, sizeof(ttLibC_MpegtsWriter_), 90000);
	if(writer == NULL) {
		ERR_PRINT("failed to allocate writer.");
		return NULL;
	}
	uint16_t pid = 0x100;
	writer->track_list = ttLibC_malloc(sizeof(ttLibC_MpegtsTrack) * types_num);
	if(writer->track_list == NULL) {
		ERR_PRINT("failed to allocate track_list.");
		ttLibC_free(writer);
		return NULL;
	}
	writer->pes_track_num = types_num;
	for(int i = 0;i < writer->pes_track_num; ++ i) {
		writer->track_list[i].cc = 0;
		if(i >= types_num) {
			writer->track_list[i].frame_type = frameType_unknown;
			writer->track_list[i].h264_configData = NULL;
			writer->track_list[i].frame_queue = NULL;
			continue;
		}
		writer->track_list[i].h264_configData = NULL;
		writer->track_list[i].frame_queue = ttLibC_FrameQueue_make(pid, 255);
		switch(target_frame_types[i]) {
		case frameType_aac:
			writer->track_list[i].frame_type = frameType_aac;
			pid ++;
			break;
		case frameType_mp3:
			writer->track_list[i].frame_type = frameType_mp3;
			pid ++;
			break;
		case frameType_h264:
			writer->track_list[i].frame_type = frameType_h264;
			pid ++;
			break;
		default:
			writer->track_list[i].frame_type = frameType_unknown;
			break;
		}
	}
	writer->inherit_super.inherit_super.timebase = 90000;
	writer->inherit_super.inherit_super.pts = 0;
	writer->is_first = true;
	writer->is_reduce_mode = false;
	writer->cc_pat = 0;
	writer->cc_pmt = 0;
	writer->cc_sdt = 0;
	writer->max_unit_duration = max_unit_duration;
	writer->inherit_super.inherit_super.type = containerType_mpegts;
	writer->status = status_target_check; // setup for first status.
	// try to make sdt pat pmt for tracks.
	ttLibC_Sdt_makePacket((const char *)"ttLibC", (const char *)"mpegtsMuxer", writer->sdt_buf, 188);
	ttLibC_Pat_makePacket(writer->pat_buf, 188);
	ttLibC_Pmt_makePacket(writer, writer->pmt_buf, 188);
	// done.
	return (ttLibC_MpegtsWriter *)writer;
}

static bool MpegtsWriter_H264TrackAdd(void *ptr, ttLibC_Frame *frame) {
	MpegtsWriter_CallbackPtr_t *callbackData = (MpegtsWriter_CallbackPtr_t *)ptr;
	ttLibC_H264 *h264 = (ttLibC_H264 *)frame;
	switch(h264->type) {
	case H264Type_slice:
	case H264Type_sliceIDR:
		break;
	default:
		return true; // only slice or sliceIDR
	}
	if(callbackData->writer->target_pos < h264->inherit_super.inherit_super.pts) {
		return false;
	}
	if(!ttLibC_Pes_writeH264Packet(callbackData->writer, callbackData->track, frame)) {
		callbackData->error_number = 1;
		return false;
	}
	if(callbackData->writer->inherit_super.inherit_super.pts > frame->pts) {
		callbackData->writer->inherit_super.inherit_super.pts = frame->pts;
	}
	return true;
}

static bool MpegtsWriter_PcrH264TrackCheck(void *ptr, ttLibC_Frame *frame) {
	ttLibC_MpegtsWriter_ *writer = (ttLibC_MpegtsWriter_ *)ptr;
	ttLibC_H264 *h264 = (ttLibC_H264 *)frame;
	switch(h264->type) {
	case H264Type_slice:
		// exceed max_unit_duration.
		if(h264->inherit_super.inherit_super.pts - writer->current_pts_pos > writer->max_unit_duration) {
			writer->target_pos = writer->current_pts_pos + writer->max_unit_duration;
			return false;
		}
		return true;
	case H264Type_sliceIDR:
		// find next sliceIDR.
		if(writer->current_pts_pos != h264->inherit_super.inherit_super.pts) {
			writer->target_pos = h264->inherit_super.inherit_super.pts;
			return false;
		}
		return true;
	default:
		return true;
	}
}

static bool MpegtsWriter_writeFromQueue(
		ttLibC_MpegtsWriter_ *writer) {
	switch(writer->status) {
	case status_target_check:
		{
			switch(writer->track_list[0].frame_type) {
			case frameType_h264:
				// check frames.
				ttLibC_FrameQueue_ref(writer->track_list[0].frame_queue, MpegtsWriter_PcrH264TrackCheck, writer);
				break;
			case frameType_aac:
			case frameType_mp3:
				// just to use max_unit_duration.
				writer->target_pos = writer->current_pts_pos + writer->max_unit_duration;
				break;
			default:
				ERR_PRINT("unexpected frame is found.");
				return false;
			}
			// if target_pos is updated, go next.
			if(writer->target_pos != writer->current_pts_pos) {
				writer->status = status_video_check;
				return MpegtsWriter_writeFromQueue(writer);
			}
			return true;
		}
		break;
	case status_video_check:
		{
			for(int i = 0;i < writer->pes_track_num;++ i) {
				switch(writer->track_list[i].frame_type) {
				case frameType_h264:
					// if some track doesn't have enough data. return and do next time.
					if(writer->track_list[i].frame_queue->pts < writer->target_pos) {
						return true;
					}
					break;
				default:
					break;
				}
			}
			// all track has enough data, go next.
			writer->status = status_video_add;
			return MpegtsWriter_writeFromQueue(writer);
		}
		break;
	case status_video_add:
		{
			MpegtsWriter_CallbackPtr_t callbackData;
			callbackData.error_number = 0;
			callbackData.writer = writer;
			for(int i = 0;i < writer->pes_track_num;++ i) {
				switch(writer->track_list[i].frame_type) {
				case frameType_h264:
					callbackData.track = &writer->track_list[i];
					ttLibC_FrameQueue_dequeue(writer->track_list[i].frame_queue, MpegtsWriter_H264TrackAdd, &callbackData);
					// if error occured.
					if(callbackData.error_number != 0) {
						ERR_PRINT("error happen during video frame writing.");
						return false;
					}
					break;
				default:
					break;
				}
			}
			// done, go next.
			writer->status = status_audio_check;
			return MpegtsWriter_writeFromQueue(writer);
		}
		break;
	case status_audio_check:
		{
			for(int i = 0;i < writer->pes_track_num;++ i) {
				switch(writer->track_list[i].frame_type) {
				case frameType_aac:
				case frameType_mp3:
					// check the data.
					if(writer->track_list[i].frame_queue->pts < writer->target_pos) {
						return true;
					}
					break;
				default:
					break;
				}
			}
			// ok, go next.
			writer->status = status_audio_add;
			return MpegtsWriter_writeFromQueue(writer);
		}
		break;
	case status_audio_add:
		{
			for(int i = 0;i < writer->pes_track_num;++ i) {
				switch(writer->track_list[i].frame_type) {
				case frameType_aac:
				case frameType_mp3:
					if(!ttLibC_Pes_writeAudioPacket(writer, &writer->track_list[i])) {
						ERR_PRINT("error happen during audio frame writing.");
						return false;
					}
					break;
				default:
					break;
				}
			}
			// go next.
			writer->status = status_current_update;
			return MpegtsWriter_writeFromQueue(writer);
		}
		break;
	case status_current_update:
		{
			writer->current_pts_pos = writer->target_pos;
			if(writer->inherit_super.inherit_super.pts < writer->current_pts_pos) {
				writer->inherit_super.inherit_super.pts = writer->current_pts_pos;
			}
			// go back to first step.
			writer->status = status_target_check;
			return MpegtsWriter_writeFromQueue(writer);
		}
		break;
	}
	return true;
}

/*
 * write mpegts data.
 * @param writer      mpegtsWriter object
 * @param update_info true:write info data(sdt pat pmt)
 * @param pid         pid for target frame.
 * @param frame       target frame.
 * @param callback    callback for write process.
 * @param ptr         user def data pointer for callback.
 */
bool ttLibC_MpegtsWriter_write(
		ttLibC_MpegtsWriter *writer,
		bool update_info,
		uint16_t pid,
		ttLibC_Frame *frame,
		ttLibC_ContainerWriteFunc callback,
		void *ptr) {
	ttLibC_MpegtsWriter_ *writer_ = (ttLibC_MpegtsWriter_ *)writer;
	if(writer_->is_first || update_info) {
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
	}
	// no frame -> just update sdt pat pmt only.
	if(frame == NULL) {
		return true;
	}
	uint64_t pts = (uint64_t)(1.0 * frame->pts * 90000 / frame->timebase);
	// rewrite timebase into 90000.(default of mpegts.)
	frame->pts = pts;
	frame->timebase = 90000;
	if(writer_->is_first) {
		writer_->current_pts_pos = pts;
		writer_->target_pos = pts;
		writer_->inherit_super.inherit_super.pts = pts;
	}
	writer_->is_first = false;
	// find track for current pid
	ttLibC_MpegtsTrack *track = NULL;
	for(int i = 0;i < writer_->pes_track_num;++ i) {
		if(writer_->track_list[i].frame_queue != NULL && writer_->track_list[i].frame_queue->track_id == pid) {
			track = &writer_->track_list[i];
		}
	}
	if(track == NULL) {
		ERR_PRINT("cannot get track information. invalid pid?:%d", pid);
		return false;
	}
	switch(frame->type) {
	case frameType_h264:
		{
			ttLibC_H264 *h264 = (ttLibC_H264 *)frame;
			if(h264->type == H264Type_unknown) {
				return true; // unknown frame is skipping.
			}
			if(h264->type == H264Type_configData) {
				ttLibC_H264 *h = ttLibC_H264_clone(
						(ttLibC_H264 *)track->h264_configData,
						h264);
				if(h == NULL) {
					ERR_PRINT("failed to make copy data.");
					return false;
				}
				h->inherit_super.inherit_super.pts = 0;
				h->inherit_super.inherit_super.timebase = 90000;
				track->h264_configData = (ttLibC_Frame *)h;
				return true;
			}
			else {
				if(!ttLibC_FrameQueue_queue(track->frame_queue, frame)) {
					return false;
				}
			}
		}
		break;
	case frameType_aac:
	case frameType_mp3:
		{
			if(!ttLibC_FrameQueue_queue(track->frame_queue, frame)) {
				return false;
			}
		}
		break;
	default:
		ERR_PRINT("unexpected frame:%d", frame->type);
		return false;
 	}
	writer_->callback = callback;
	writer_->ptr = ptr;
	return MpegtsWriter_writeFromQueue(writer_);
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

/**
 * close mpegtsWriter object.
 * @param writer
 */
void ttLibC_MpegtsWriter_close(ttLibC_MpegtsWriter **writer) {
	ttLibC_MpegtsWriter_ *target = (ttLibC_MpegtsWriter_ *)*writer;
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.inherit_super.type != containerType_mpegts) {
		ERR_PRINT("try to close non MpegtsWriter.");
		return;
	}
	if(target->track_list != NULL) {
		for(int i = 0;i < target->pes_track_num;++ i) {
			ttLibC_FrameQueue_close(&target->track_list[i].frame_queue);
			ttLibC_Frame_close(&target->track_list[i].h264_configData);
		}
		ttLibC_free(target->track_list);
	}
	ttLibC_free(target);
	*writer = NULL;
}
