/*
 * @file   pes.c
 * @brief  mpegts pes.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/13
 */

#include "pes.h"

#include "../mpegtsPacket.h"
#include "../../../log.h"
#include "../../../allocator.h"
#include "../../../util/byteUtil.h"
#include "../../../util/ioUtil.h"
#include "../../../util/hexUtil.h"

#include "../../../frame/audio/aac.h"
#include "../../../frame/audio/mp3.h"
#include "../../../frame/video/h264.h"

#include <stdlib.h>
#include <string.h>

ttLibC_Pes *ttLibC_Pes_make(
		ttLibC_Pes *prev_pes,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		uint16_t pid,
		uint8_t continuity_counter,
		uint8_t stream_type) {
	ttLibC_Frame *prev_frame = NULL;
	// hold prev frame.
	if(prev_pes != NULL) {
		prev_frame = prev_pes->frame;
	}
	ttLibC_Pes *pes = (ttLibC_Pes *)ttLibC_MpegtsPacket_make(
			(ttLibC_MpegtsPacket *)prev_pes,
			data,
			data_size,
			non_copy_mode,
			pts,
			timebase,
			MpegtsType_pes,
			pid,
			continuity_counter);
	if(pes != NULL) {
		pes->frame = prev_frame;
		switch(stream_type) {
		case 0x01: // mpeg1 video
			pes->frame_type = frameType_unknown;
			break;
		case 0x02: // mpeg2 video
			pes->frame_type = frameType_unknown;
			break;
		case 0x03: // mpeg1 audio(mp3)
			pes->frame_type = frameType_mp3;
			break;
		case 0x04: // mpeg2 audio(mp3?)
			LOG_PRINT("maybe mp3?");
			pes->frame_type = frameType_unknown;
			break;
		case 0x05: // private section
			pes->frame_type = frameType_unknown;
			break;
		case 0x06: // private data (binary data?)
			pes->frame_type = frameType_unknown;
			break;
		case 0x0F: // aac
			pes->frame_type = frameType_aac;
			break;
		case 0x10: // mpeg4 video(h263? wmv?)
			pes->frame_type = frameType_unknown;
			break;
		case 0x11: // aac latm
			pes->frame_type = frameType_unknown;
			break;
		case 0x12: // system mpeg4 pes
			pes->frame_type = frameType_unknown;
			break;
		case 0x13: // system mpeg4 sections
			pes->frame_type = frameType_unknown;
			break;
		case 0x1B: // h264
			pes->frame_type = frameType_h264;
			break;
		case 0x81: // ac3
			pes->frame_type = frameType_unknown;
			break;
		case 0x8A: // dts
			pes->frame_type = frameType_unknown;
			break;
		default:
//		case 0x100: // <-
			pes->frame_type = frameType_unknown;
			break;
		}
	}
	return pes;
}

ttLibC_Pes *ttLibC_Pes_getPacket(
		ttLibC_Pes *prev_pes,
		uint8_t *data,
		size_t data_size,
		uint8_t stream_type,
		uint16_t pid) {
	// just now, possible to handle only 65536 byte of data.
	// TODO to use dynamicBuffer to use more than 65536 byte.
	// hold the buffer for frame.
	uint8_t *frame_buffer = NULL; // target buffer.
	size_t frame_buffer_size = 0; // total buffer size.
	size_t frame_buffer_next_pos = 0; // current update position

	// read header.
	ttLibC_ByteReader *reader = ttLibC_ByteReader_make(data, data_size, ByteUtilType_default);
	ttLibC_MpegtsPacket_Header header_info;
	if(!ttLibC_MpegtsPacket_loadMpegtsPacketHeader(reader, &header_info)) {
		return NULL;
	}
	if(header_info.payloadUnitStartIndicator == 1) {
		// unit start. there is some data.
		/*
		 * pes
		 * 24bit prefix 00000001 fix?
		 * 8bit streamId audio 0x0C - 0xDF? video 0xE0 - 0xEF?(data from vlc player is out of this rule.)
		 * 16bit pesPacketLength
		 * 2bit markerBit 10 fixed?
		 * 2bit scramblingControl
		 * 1bit priority
		 * 1bit dataAlignmentIndicator
		 * 1bit copyright
		 * 1bit originalFlg 0:original 1:copy
		 * 2bit ptsDtsIndicator 11: for both 10 for pts only
		 * 1bit escrFlag
		 * 1bit esRateFlag
		 * 1bit DSMTrickModeFlag
		 * 1bit additionalCopyInfoFlag
		 * 1bit CRCFlag
		 * 1bit extensionFlag
		 *
		 * 8bit PesHeaderLength
		 * (in case of pts:)
		 * 40bit pts field 0010 XXX1 XXXX XXXX XXXX XXX1 XXXX XXXX XXXX XXX1
		 *
		 * then frame buffer.
		 */
		if(ttLibC_ByteReader_bit(reader, 24) != 1) {
			LOG_PRINT("prefix is not 1, broken mpegts?");
		}
		ttLibC_ByteReader_bit(reader, 8); // streamId (ignore, use pid as trackId)
		uint32_t frame_size = ttLibC_ByteReader_bit(reader, 16); // if this value is 0, frame size is unknown.
		ttLibC_ByteReader_bit(reader, 2);
		ttLibC_ByteReader_bit(reader, 2);
		ttLibC_ByteReader_bit(reader, 1);
		ttLibC_ByteReader_bit(reader, 1);
		ttLibC_ByteReader_bit(reader, 1);
		ttLibC_ByteReader_bit(reader, 1);

		bool has_pts = ttLibC_ByteReader_bit(reader, 1) != 0; // pts flag
		bool has_dts = ttLibC_ByteReader_bit(reader, 1) != 0; // dts flag
		ttLibC_ByteReader_bit(reader, 1);
		ttLibC_ByteReader_bit(reader, 1);
		ttLibC_ByteReader_bit(reader, 1);
		ttLibC_ByteReader_bit(reader, 1);
		ttLibC_ByteReader_bit(reader, 1);
		ttLibC_ByteReader_bit(reader, 1);

		uint8_t pes_length = ttLibC_ByteReader_bit(reader, 8);
		uint64_t pts = 0;
		if(has_pts) {
			ttLibC_ByteReader_bit(reader, 4);
			pts = ((ttLibC_ByteReader_bit(reader, 4) << 29) & 0xC0000000)
					| ((ttLibC_ByteReader_bit(reader, 16) << 14)     & 0x3FFF8000)
					| ((ttLibC_ByteReader_bit(reader, 16) >> 1)      & 0x00007FFF);
		}
		if(has_dts) {
			ttLibC_ByteReader_bit(reader, 4);
			uint64_t dts = ((ttLibC_ByteReader_bit(reader, 4) << 29) & 0xC0000000)
					| ((ttLibC_ByteReader_bit(reader, 16) << 14)     & 0x3FFF8000)
					| ((ttLibC_ByteReader_bit(reader, 16) >> 1)      & 0x00007FFF);
//			LOG_PRINT("pts:%llu, dts:%llu", pts, dts);
//			ERR_PRINT("ignore dts.");
		}
		data += reader->read_size;
		data_size -= reader->read_size;
		ttLibC_ByteReader_close(&reader);
		if(prev_pes != NULL) {
			// get prev frame data, and use it.
			if(!prev_pes->inherit_super.inherit_super.inherit_super.is_non_copy) {
				frame_buffer = prev_pes->inherit_super.inherit_super.inherit_super.data;
				frame_buffer_size = prev_pes->inherit_super.inherit_super.inherit_super.data_size;
				frame_buffer_next_pos = 0;
			}
			// assume as non copy.
			prev_pes->inherit_super.inherit_super.inherit_super.is_non_copy = true;
		}
		bool alloc_flg = false;
		// if no data, alloc.
		if(frame_buffer == NULL) {
			frame_buffer_size = 65536;
			frame_buffer = ttLibC_malloc(frame_buffer_size);
			if(frame_buffer == NULL) {
				ERR_PRINT("failed to allocate frame buffer for pes.");
				return NULL;
			}
			alloc_flg = true;
			frame_buffer_next_pos = 0;
		}
		ttLibC_Pes *pes = ttLibC_Pes_make(
				prev_pes,
				frame_buffer,
				frame_buffer_size,
				true,
				pts,
				90000,
				pid,
				header_info.continuityCounter,
				stream_type);
		if(pes == NULL) {
			LOG_PRINT("failed to make pes.");
			if(alloc_flg) {
				ttLibC_free(frame_buffer);
			}
			return NULL;
		}
		pes->inherit_super.inherit_super.inherit_super.is_non_copy = false;
		memcpy(frame_buffer, data, data_size);
		pes->inherit_super.inherit_super.inherit_super.buffer_size = data_size; // write the copyed size.
		return pes;
	}
	else {
		data += reader->read_size;
		data_size -= reader->read_size;
		ttLibC_ByteReader_close(&reader);

		frame_buffer = prev_pes->inherit_super.inherit_super.inherit_super.data;
		frame_buffer_size = prev_pes->inherit_super.inherit_super.inherit_super.data_size;
		frame_buffer_next_pos = prev_pes->inherit_super.inherit_super.inherit_super.buffer_size;
		if(frame_buffer_size < frame_buffer_next_pos + data_size) {
			ERR_PRINT("data is overflowed.");
			return NULL;
		}
	 	memcpy(frame_buffer + frame_buffer_next_pos, data, data_size);
		prev_pes->inherit_super.inherit_super.inherit_super.buffer_size += data_size; // update copyed size.
		return prev_pes;
	}
}

bool ttLibC_Pes_getFrame(
		ttLibC_Pes *pes,
		ttLibC_getFrameFunc callback,
		void *ptr) {
	uint8_t *buffer = NULL;
	size_t left_size = 0;
	switch(pes->frame_type) {
	case frameType_aac:
		{
			// data should be adts.
			buffer = pes->inherit_super.inherit_super.inherit_super.data;
			left_size = pes->inherit_super.inherit_super.inherit_super.buffer_size;
			uint64_t sample_num_count = 0;
			uint32_t sample_rate = 0;
			do {
				uint64_t pts = pes->inherit_super.inherit_super.inherit_super.pts;
				if(sample_rate != 0) {
					pts = pts + (sample_num_count * 90000 / sample_rate);
				}
				ttLibC_Aac *aac = ttLibC_Aac_getFrame(
						(ttLibC_Aac *)pes->frame,
						buffer,
						left_size,
						true,
						pts,
						pes->inherit_super.inherit_super.inherit_super.timebase);
				if(aac == NULL) {
					ERR_PRINT("failed to get aac frame.");
					return false;
				}
				sample_rate = aac->inherit_super.sample_rate;
				sample_num_count += aac->inherit_super.sample_num;
				pes->frame = (ttLibC_Frame *)aac;
				if(!callback(ptr, pes->frame)) {
					return false;
				}
				buffer += aac->inherit_super.inherit_super.buffer_size;
				left_size -= aac->inherit_super.inherit_super.buffer_size;
			}while(left_size > 0);
		}
		return true;
	case frameType_h264:
		{
			// data should be h264 nal.
			buffer = pes->inherit_super.inherit_super.inherit_super.data;
			left_size = pes->inherit_super.inherit_super.inherit_super.buffer_size;
			do {
				ttLibC_H264 *h264 = ttLibC_H264_getFrame(
						(ttLibC_H264 *)pes->frame,
						buffer,
						left_size,
						true,
						pes->inherit_super.inherit_super.inherit_super.pts,
						pes->inherit_super.inherit_super.inherit_super.timebase);
				if(h264 == NULL) {
					return false;
				}
				pes->frame = (ttLibC_Frame *)h264;
				if(!callback(ptr, pes->frame)) {
					return false;
				}
				buffer += h264->inherit_super.inherit_super.buffer_size;
				left_size -= h264->inherit_super.inherit_super.buffer_size;
			} while(left_size > 0);
			return true;
		}
		break;
	case frameType_mp3:
		{
			buffer = pes->inherit_super.inherit_super.inherit_super.data;
			left_size = pes->inherit_super.inherit_super.inherit_super.buffer_size;
			uint64_t sample_num_count = 0;
			uint32_t sample_rate = 0;
			do {
				uint64_t pts = pes->inherit_super.inherit_super.inherit_super.pts;
				if(sample_rate != 0) {
					pts = pts + (sample_num_count * 90000 / sample_rate);
				}
				ttLibC_Mp3 *mp3 = ttLibC_Mp3_getFrame(
						(ttLibC_Mp3 *)pes->frame,
						buffer,
						left_size,
						pts,
						pes->inherit_super.inherit_super.inherit_super.timebase);
				if(mp3 == NULL) {
					ERR_PRINT("failed to get mp3 frame.");
					return false;
				}
				sample_rate = mp3->inherit_super.sample_rate;
				sample_num_count += mp3->inherit_super.sample_num;
				pes->frame = (ttLibC_Frame *)mp3;
				if(!callback(ptr, pes->frame)) {
					return false;
				}
				buffer += mp3->inherit_super.inherit_super.buffer_size;
				left_size -= mp3->inherit_super.inherit_super.buffer_size;
			} while(left_size > 0);
			return true;
		}
		break;
	default:
		LOG_PRINT("unexpected frame type is found.:%d", pes->frame_type);
	}
	return false;
}

bool ttLibC_Pes_writeH264Packet(
		ttLibC_MpegtsTrack *track,
		ttLibC_Frame *frame,
		ttLibC_ContainerWriteFunc callback,
		void *ptr) {
	ttLibC_H264 *h264 = (ttLibC_H264 *)frame;
	ttLibC_H264 *config = (ttLibC_H264 *)track->h264_configData;
	uint32_t total_size = 0;
	uint64_t pts = h264->inherit_super.inherit_super.pts;

	uint8_t buf[188];
	uint8_t *p_buf = buf;
	uint32_t p_buf_left_size = 188;

	uint8_t aud[] = {0x00, 0x00, 0x00, 0x01, 0x09, 0xF0};
	uint8_t *nal    = NULL;
	size_t nal_size = 0;

	p_buf[0] = 0x47;
	p_buf[1] = 0x40 | ((track->frame_queue->track_id >> 8) & 0x1F);
	p_buf[2] = track->frame_queue->track_id & 0xFF;
	switch(h264->type) {
	default:
		return false;
	case H264Type_slice:
		// aud slice slice...
		config = NULL; // no use
		total_size = 6 + h264->inherit_super.inherit_super.buffer_size;
		// with adaptation field(pcr), should be better.
		if(total_size < 170) {
			p_buf[3] = 0x30 | (track->cc & 0x0F);
			uint8_t adaptation_field_size = 170 - total_size - 1;
			p_buf[4] = adaptation_field_size;
			p_buf += 5;
			p_buf_left_size -= 5;
			for(int i = 0;i < adaptation_field_size;++ i) {
				if(i == 0) {
					*p_buf = 0x00;
				}
				else {
					*p_buf = 0xFF;
				}
				++ p_buf;
				-- p_buf_left_size;
			}
		}
		else {
			p_buf[3] = 0x10 | (track->cc & 0x0F);
			p_buf += 4;
			p_buf_left_size -= 4;
		}
		break;
	case H264Type_sliceIDR:
		// aud sps pps sliceIDR sliceIDR ...
		total_size = 6 + track->h264_configData->buffer_size + h264->inherit_super.inherit_super.buffer_size;
		p_buf[3] = 0x30 | (track->cc & 0x0F);
		if(track->frame_queue->track_id == 0x100 // pcr
				&& total_size < 162) { // less than 162byte
			// need to fill with adaptation field.
			LOG_PRINT("data size is too small, need to have padding with adaptation field. :make later.");
			return false;
		}
		else if(track->frame_queue->track_id != 0x100 // non pcr
				&& total_size < 168) { // less than 168byte
			// need to fill with adaptation field.
			LOG_PRINT("data size is too small, need to have padding with adaptation field. :make later.");
			return false;
		}
		else if(track->frame_queue->track_id == 0x100) {
			p_buf[4] = 0x07; // adaptation size = 7
			p_buf[5] = 0x50; // random Access + pcr
			// pcrbase
			p_buf[6] = (pts >> 25) & 0xFF;
			p_buf[7] = (pts >> 17) & 0xFF;
			p_buf[8] = (pts >> 9) & 0xFF;
			p_buf[9] = (pts >> 1) & 0xFF;
			p_buf[10] = ((pts << 7) & 0x80) | 0x7E;
			// pcr extend
			p_buf[11] = 0x00;
			p_buf += 12;
			p_buf_left_size -= 12;
		}
		else {
			p_buf[4] = 0x01; // adaptation size = 1
			p_buf[5] = 0x40; // random Access
			p_buf += 6;
			p_buf_left_size -= 6;
		}
		break;
	}
	track->cc ++;
	p_buf[0] = 0x00;
	p_buf[1] = 0x00;
	p_buf[2] = 0x01;
	p_buf[3] = 0xE0 | (track->frame_queue->track_id & 0xFF);
	uint32_t pes_size = total_size + 8;
	if(pes_size < 0x10000) {
		p_buf[4] = (pes_size >> 8) & 0xFF;
		p_buf[5] = pes_size & 0xFF;
	}
	else {
		p_buf[4] = 0x00; // for h264 pesPacketLength is 0
		p_buf[5] = 0x00;
	}
	p_buf[6] = 0x80; // marker
	p_buf[7] = 0x80; // pts
	// 5byte for pts
	p_buf[8] = 0x05;
	p_buf[9] = 0x21 | ((pts >> 30) & 0x0E);
	p_buf[10] = (pts >> 22) & 0xFF;
	p_buf[11] = ((pts >> 14) & 0xFE) | 0x01;
	p_buf[12] = (pts >> 7) & 0xFF;
	p_buf[13] = ((pts << 1) & 0xFE) | 0x01;
	p_buf += 14;
	p_buf_left_size -= 14;
	// then put the h264 data.
	nal = aud;
	nal_size = 6;
	for(int i = 0;i < total_size;++ i) {
		*p_buf = *nal;
		++ nal;
		-- nal_size;
		++ p_buf;
		-- p_buf_left_size;
		if(nal_size == 0) {
			if(config != NULL) {
				nal = config->inherit_super.inherit_super.data;
				nal_size = config->inherit_super.inherit_super.buffer_size;
				config = NULL;
			}
			else if(h264 != NULL) {
				nal = h264->inherit_super.inherit_super.data;
				nal_size = h264->inherit_super.inherit_super.buffer_size;
				h264 = NULL;
			}
			else {
				if(!callback(ptr, buf, 188)) {
					return false;
				}
				break;
			}
		}
		if(p_buf_left_size == 0) {
			// packet is complete, need to go next.
			if(!callback(ptr, buf, 188)) {
				return false;
			}
			// prepare data.
			if(nal_size >= 184) {
				// 4byte header and frame data.
				p_buf = buf;
				p_buf_left_size = 188;
				p_buf[0] = 0x47;
				p_buf[1] = ((track->frame_queue->track_id >> 8) & 0xFF);
				p_buf[2] = track->frame_queue->track_id & 0xFF;
				p_buf[3] = 0x10 | (track->cc & 0x0F);
				p_buf += 4;
				p_buf_left_size -= 4;
				track->cc ++;
			}
			else {
				// need to have adaptation field to fill.
				uint8_t adaptation_field_size = 184 - nal_size - 1;
				p_buf = buf;
				p_buf_left_size = 188;
				p_buf[0] = 0x47;
				p_buf[1] = ((track->frame_queue->track_id >> 8) & 0xFF);
				p_buf[2] = track->frame_queue->track_id & 0xFF;
				p_buf[3] = 0x30 | (track->cc & 0x0F);
				p_buf += 4;
				p_buf_left_size -= 4;
				track->cc ++;
				// size of adaptation field.
				*p_buf = adaptation_field_size;
				++ p_buf;
				-- p_buf_left_size;
				for(int i = 0;i < adaptation_field_size;i ++) {
					if(i == 0) {
						*p_buf = 0x00;
					}
					else {
						*p_buf = 0xFF;
					}
					++ p_buf;
					-- p_buf_left_size;
				}
			}
		}
	}
	return true;
}

/*
 * data structure for audio frame queue.
 */
typedef struct {
	uint32_t total_size;
	void    *buf;
	uint8_t *p_buf;
	uint32_t p_buf_left_size;
	uint64_t target_pts;
	uint64_t start_pts;
	uint8_t *data;
	size_t   data_size;
	ttLibC_ContainerWriteFunc callback;
	ttLibC_MpegtsTrack *track;
	void *ptr;
	bool error_flg;
} audio_data_t;

/*
 * callback for check audio total size before write.
 * @param ptr
 * @param frame
 */
bool Pes_checkAudioTotalSize(
		void *ptr,
		ttLibC_Frame *frame) {
	audio_data_t *audioData = (audio_data_t *)ptr;
	if(audioData->target_pts < frame->pts) {
		return false;
	}
	if(audioData->start_pts > frame->pts) {
		audioData->start_pts = frame->pts;
	}
	switch(frame->type) {
	case frameType_aac:
		{
			ttLibC_Aac *aac = (ttLibC_Aac *)frame;
			if(aac->type == AacType_raw) {
				audioData->total_size += frame->buffer_size + 7;
			}
			else {
				audioData->total_size += frame->buffer_size;
			}
		}
		break;
	case frameType_mp3:
		{
			ttLibC_Mp3 *mp3 = (ttLibC_Mp3 *)frame;
			audioData->total_size += frame->buffer_size;
		}
		break;
	default:
		ERR_PRINT("unexpected frame.:%d", frame->type);
		audioData->error_flg = true;
		return false;
	}
	return true;
}

/*
 * callback for audio data write.
 * @param ptr
 * @param frame
 */
bool Pes_writeAudioData(void *ptr, ttLibC_Frame *frame) {
	audio_data_t *audioData = (audio_data_t *)ptr;
	if(audioData->target_pts < frame->pts) {
		// done.
		return false;
	}
	uint8_t aac_header_buf[7];
	bool is_body_flag = true;
	if(frame->type == frameType_aac && ((ttLibC_Aac *)frame)->type == AacType_raw) {
		if(ttLibC_Aac_readAdtsHeader((ttLibC_Aac *)frame, aac_header_buf, 7) == 0) {
			LOG_PRINT("failed to get adts header information.");
			audioData->error_flg = true;
			return false;
		}
		audioData->data = aac_header_buf;
		audioData->data_size = 7;
		is_body_flag = false;
	}
	else {
		audioData->data = frame->data;
		audioData->data_size = frame->buffer_size;
	}
	while(audioData->total_size > 0) {
		*audioData->p_buf = *audioData->data;
		++ audioData->data;
		-- audioData->data_size;
		++ audioData->p_buf;
		-- audioData->p_buf_left_size;
		-- audioData->total_size;
		if(audioData->p_buf_left_size == 0) {
			// 188 byte packet is complete.
			if(!audioData->callback(audioData->ptr, audioData->buf, 188)) {
				audioData->error_flg = true;
				return false;
			}

			if(audioData->total_size > 0) {
				audioData->p_buf = audioData->buf;
				audioData->p_buf_left_size = 188;

				audioData->p_buf[0] = 0x47;
				audioData->p_buf[1] = ((audioData->track->frame_queue->track_id >> 8) & 0xFF);
				audioData->p_buf[2] = audioData->track->frame_queue->track_id & 0xFF;
				if(audioData->total_size >= 184) {
					audioData->p_buf[3] = 0x10 | (audioData->track->cc & 0x0F);
					audioData->p_buf += 4;
					audioData->p_buf_left_size -= 4;
				}
				else {
					uint8_t adaptation_field_size = 183 - audioData->total_size;
					audioData->p_buf[3] = 0x30 | (audioData->track->cc & 0x0F);
					audioData->p_buf += 4;
					audioData->p_buf_left_size -= 4;
					// size of adaptation field.
					*audioData->p_buf = adaptation_field_size;
					++ audioData->p_buf;
					-- audioData->p_buf_left_size;
					for(int i = 0;i < adaptation_field_size;i ++) {
						if(i == 0) {
							*audioData->p_buf = 0x00;
						}
						else {
							*audioData->p_buf = 0xFF;
						}
						++ audioData->p_buf;
						-- audioData->p_buf_left_size;
					}
				}
				audioData->track->cc ++;
			}
		}
		if(audioData->data_size == 0) {
			if(!is_body_flag) {
				audioData->data = frame->data;
				audioData->data_size = frame->buffer_size;
				is_body_flag = true;
			}
			else {
				break;
			}
		}
	}
	return true;
}

bool ttLibC_Pes_writeAudioPacket(
		ttLibC_MpegtsWriter_ *writer,
		ttLibC_MpegtsTrack *track,
		ttLibC_ContainerWriteFunc callback,
		void *ptr) {
	uint8_t buf[188];
	audio_data_t audioData;
	audioData.track = track;
	audioData.total_size = 0;
	audioData.buf = buf;
	audioData.p_buf = buf;
	audioData.p_buf_left_size = 188;
	audioData.data = NULL;
	audioData.data_size = 0;
	audioData.start_pts = 0xFFFFFFFFFFFFFFFFL;
	audioData.target_pts = writer->target_pos;
	audioData.callback = callback;
	audioData.ptr = ptr;
	audioData.error_flg = false;

	// firstly, check total size of frame.
	ttLibC_FrameQueue_ref(track->frame_queue, Pes_checkAudioTotalSize, &audioData);

	// now write data.
	audioData.p_buf[0] = 0x47;
	audioData.p_buf[1] = 0x40 | ((track->frame_queue->track_id >> 8) & 0xFF);
	audioData.p_buf[2] = track->frame_queue->track_id & 0xFF;
	audioData.p_buf[3] = 0x30 | (track->cc & 0x0F); // must have adaptation field.(random access.)
	track->cc ++;
	if(track->frame_queue->track_id == 0x100 // pcr
			&& audioData.total_size < 162) { // need adaptation field padding
		uint32_t fill_size = 162 - audioData.total_size + 1;
		audioData.p_buf[4] = fill_size;
		audioData.p_buf[5] = 0x50;
		// pcr
		audioData.p_buf[6] = (audioData.start_pts >> 25) & 0xFF;
		audioData.p_buf[7] = (audioData.start_pts >> 17) & 0xFF;
		audioData.p_buf[8] = (audioData.start_pts >> 9) & 0xFF;
		audioData.p_buf[9] = (audioData.start_pts >> 1) & 0xFF;
		audioData.p_buf[10] = ((audioData.start_pts << 7) & 0x80) | 0x7E;
		audioData.p_buf[11] = 0x00;
		audioData.p_buf += 12;
		audioData.p_buf_left_size -= 12;
		for(uint32_t i = 0;i < fill_size - 7;i ++) {
			*audioData.p_buf = 0xFF;
			++ audioData.p_buf;
			-- audioData.p_buf_left_size;
		}
	}
	else if(track->frame_queue->track_id != 0x100 // non pcr
			&& audioData.total_size < 168) { // need adaptation field padding
		uint32_t fill_size = 168 - audioData.total_size + 1;
		audioData.p_buf[4] = fill_size;
		audioData.p_buf[5] = 0x40;
		audioData.p_buf += 6;
		audioData.p_buf_left_size -= 6;
		for(uint32_t i = 0;i < fill_size - 1;i ++) {
			*audioData.p_buf = 0xFF;
			++ audioData.p_buf;
			-- audioData.p_buf_left_size;
		}
	}
	else if(track->frame_queue->track_id == 0x100) { // pcr
		audioData.p_buf[4] = 0x07;
		audioData.p_buf[5] = 0x50; // random access & pcr
		// pcr
		audioData.p_buf[6] = (audioData.start_pts >> 25) & 0xFF;
		audioData.p_buf[7] = (audioData.start_pts >> 17) & 0xFF;
		audioData.p_buf[8] = (audioData.start_pts >> 9) & 0xFF;
		audioData.p_buf[9] = (audioData.start_pts >> 1) & 0xFF;
		audioData.p_buf[10] = ((audioData.start_pts << 7) & 0x80) | 0x7E;
		audioData.p_buf[11] = 0x00;
		audioData.p_buf += 12;
		audioData.p_buf_left_size -= 12;
	}
	else {
		// adaptation field
		audioData.p_buf[4] = 0x01;
		audioData.p_buf[5] = 0x40; // random access
		audioData.p_buf += 6;
		audioData.p_buf_left_size -= 6;
	}
	audioData.p_buf[0] = 0x00;
	audioData.p_buf[1] = 0x00;
	audioData.p_buf[2] = 0x01;
	audioData.p_buf[3] = 0xC0 | (track->frame_queue->track_id & 0xFF);
	// put data size.
	uint32_t pes_size = audioData.total_size + 8;
	if(pes_size < 0x10000) {
		audioData.p_buf[4] = (pes_size >> 8) & 0xFF;
		audioData.p_buf[5] = pes_size & 0xFF;
	}
	else {
		audioData.p_buf[4] = 0x00;
		audioData.p_buf[5] = 0x00;
	}
	audioData.p_buf[6] = 0x80;
	audioData.p_buf[7] = 0x80;
	audioData.p_buf[8] = 0x05;
	// pts
	audioData.p_buf[9] = 0x21 | ((audioData.start_pts >> 30) & 0x0E);
	audioData.p_buf[10] = (audioData.start_pts >> 22) & 0xFF;
	audioData.p_buf[11] = ((audioData.start_pts >> 14) & 0xFE) | 0x01;
	audioData.p_buf[12] = (audioData.start_pts >> 7) & 0xFF;
	audioData.p_buf[13] = ((audioData.start_pts << 1) & 0xFE) | 0x01;
	audioData.p_buf += 14;
	audioData.p_buf_left_size -= 14;
	// then put frame data.
	ttLibC_FrameQueue_dequeue(track->frame_queue, Pes_writeAudioData, &audioData);
	if(audioData.error_flg) {
		return false;
	}
	return true;
}

void ttLibC_Pes_close(ttLibC_Pes **pes) {
	ttLibC_Pes *target = *pes;
	if(!target->inherit_super.inherit_super.inherit_super.is_non_copy) {
		if(target->inherit_super.inherit_super.inherit_super.data) {
			ttLibC_free(target->inherit_super.inherit_super.inherit_super.data);
		}
	}
	ttLibC_Frame_close(&target->frame);
	ttLibC_free(target);
	*pes = NULL;
}
