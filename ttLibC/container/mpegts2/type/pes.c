/*
 * @file   pes.c
 * @brief  mpegts pes.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2016/11/18
 */

#include "pes.h"

#include "../mpegtsPacket.h"
#include "../../../log.h"
#include "../../../allocator.h"
#include "../../../util/byteUtil.h"
#include "../../../util/ioUtil.h"
#include "../../../util/hexUtil.h"
#include "../mpegtsWriter.h"

#include "../../../frame/frame.h"
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
		pes->is_used = false;
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
		ttLibC_ByteReader_close(&reader);
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
		 * 1bit originalFlag 0:original 1:copy
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
		if(frame_size != 0) {
			frame_size -= (pes_length + 3);
		}
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
		bool alloc_flag = false;
		// if no data, alloc.
		if(frame_buffer == NULL) {
			frame_buffer_size = 65536;
			frame_buffer = ttLibC_malloc(frame_buffer_size);
			if(frame_buffer == NULL) {
				ERR_PRINT("failed to allocate frame buffer for pes.");
				return NULL;
			}
			alloc_flag = true;
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
			ERR_PRINT("failed to make pes.");
			if(alloc_flag) {
				ttLibC_free(frame_buffer);
			}
			return NULL;
		}
		pes->frame_size = frame_size;
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
				aac->inherit_super.inherit_super.id = pes->inherit_super.inherit_super.pid;
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
				h264->inherit_super.inherit_super.id = pes->inherit_super.inherit_super.pid;
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
						true,
						pts,
						pes->inherit_super.inherit_super.inherit_super.timebase);
				if(mp3 == NULL) {
					ERR_PRINT("failed to get mp3 frame.");
					return false;
				}
				sample_rate = mp3->inherit_super.sample_rate;
				sample_num_count += mp3->inherit_super.sample_num;
				mp3->inherit_super.inherit_super.id = pes->inherit_super.inherit_super.pid;
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

bool ttLibC_Pes_writePacket(
		ttLibC_MpegtsWriteTrack *track, // for continuity counter
		bool has_randomAccess, // for random access flag
		bool has_pcr,
		bool has_size, // for size data flag
		uint8_t trackBaseId, // trackBaseId
		uint32_t pid, // pid
		uint64_t pts, // pts
		uint64_t dts, // dts
		ttLibC_DynamicBuffer *frame_buffer, // frame buffer
		ttLibC_DynamicBuffer *output_buffer) { // buffer for result.
	// このbufferにデータをいれていこうと思う。
	// そっか巡回カウンターの値もなんとかしないとだめなのね・・・うーん。
	// 書き出しを実施する。
	// データから書き出しを実施する。
	// とりあえずbinaryをつくって、dump出しておくか・・・
	// とりあえずpidが0x100の場合 or has_randomAccessがtrueの場合、データサイズがおもったより小さい場合(188byte以下になる場合)
	// これらの条件に当てはまる場合はadaptation fieldが必要になる。
	// サンプル
	// 47 41 00 30 07 50 00 00 7B 0C 7E 00 00 00 01 E0 00 00 80 C0 0A 31 00 09 07 4D 11 00 07 D8 61 00 00 00 01 09
	//          10 adaptation fieldがなければ10
	//             07 adaptation size
	//                40 10でランダムアクセス + pcr
	//                   pcrのデータ
	//                                  ここからフレームデータ
	//                                             トラックIDとかだったと思う。
	//                                                          C0:ptsとdts 80pts 0Aのあとで5byte 5byteとなってる。
	//                                                 [    ]sizeだっけ
	// 残りはh264のnal

	// サンプル2
	// 47 41 01 30 01 40 00 00 01 C0 0B 75 80 80 05 21 00 07 F5 AF FF FB 90 64
	//          30でadaptationfieldあり
	//             1byte 40 ランダムアクセス only
	//                               データサイズ 0xB75っぽい。
	//                                        80でptsのみあり。      [ここから普通のmp3
	// とりあえず普通のデータを一旦つくって、必要となるデータサイズがどうなるか確認後、adaptation fieldについて、調整すればよさそう。

	// とりあえずマクロでごまかしておく。
#define Buf_t_init(a)			{a.ptr = a.buf;a.length = 0;a.ptr_length = 188;}
#define Buf_t_byte(a, b)		{*a.ptr = b;++ a.ptr;--a.ptr_length;++ a.length;}
#define Buf_t_hex(a, b)			{int c = ttLibC_HexUtil_makeBuffer(b, a.ptr, a.ptr_length); \
									a.ptr += c;a.ptr_length -= c;a.length += c; \
								}
#define Buf_t_pcr(a, b)			{ \
									Buf_t_byte(a, (b >> 25) & 0xFF); \
									Buf_t_byte(a, (b >> 17) & 0xFF); \
									Buf_t_byte(a, (b >> 9) & 0xFF); \
									Buf_t_byte(a, (b >> 1) & 0xFF); \
									Buf_t_byte(a, ((b << 7) & 0x80) | 0x7E); \
									Buf_t_byte(a, 0x00); \
								}
#define Buf_t_timestamp(a, b, c){ \
									Buf_t_byte(a, b | ((c >> 30) & 0x0E)); \
									Buf_t_byte(a, (c >> 22) & 0xFF); \
									Buf_t_byte(a, ((c >> 14) & 0xFE) | 0x01); \
									Buf_t_byte(a, (c >> 7) & 0xFF); \
									Buf_t_byte(a, ((c << 1) & 0xFE) | 0x01); \
								}
#define Buf_t_fillFF(a, b)		{for(int i = 0;i < b;++ i) Buf_t_byte(a, 0xFF);}
#define Buf_t_dump(a)			{LOG_DUMP(a.buf, a.length, true);}

	struct buf_t{
		uint8_t buf[188];
		uint32_t length;
		uint8_t *ptr;
		uint32_t ptr_length;
	};
	struct buf_t adapt_buf;
	struct buf_t header_buf;
	struct buf_t sync_buf;
	bool is_first = true;
	while(ttLibC_DynamicBuffer_refSize(frame_buffer) > 0) {
		Buf_t_init(adapt_buf);
		Buf_t_init(header_buf);
		Buf_t_init(sync_buf);
		if(is_first) {
			Buf_t_hex(header_buf, "00 00 01");
			Buf_t_byte(header_buf, trackBaseId | (pid & 0x0F));
			Buf_t_hex(header_buf, "00 00 80 80 05"); // フレームの全長、あとでいれる。
			Buf_t_timestamp(header_buf, 0x21, pts); // pts追加

			// データをつくっておく。
			switch(track->inherit_super.frame_type) {
			case frameType_h264:
//			case frameType_h265:
				if((track->inherit_super.use_mode & containerWriter_enable_dts) != 0 && pts != dts) {
					header_buf.buf[7] |= 0x40; // こっちもdtsあるフラグ追加
					header_buf.buf[8] = 0x0A;
					header_buf.buf[9] |= 0x10; // dtsありにする。
					Buf_t_timestamp(header_buf, 0x11, dts); // dts追加
				}
				break;
			default:
				break;
			}
			if(has_size) {
				uint32_t size = header_buf.length - 6 + ttLibC_DynamicBuffer_refSize(frame_buffer);
				if(size < 0x10000) {
					header_buf.buf[4] = (size >> 8) & 0xFF;
					header_buf.buf[5] = size & 0xFF;
				}
			}
			if(has_pcr) {
				// pcrPIDの場合adaptation fieldは8byte固定 1byte(size) + 1byte(flag) + 6byte(pts)
				Buf_t_byte(adapt_buf, 0x07);
				if(has_randomAccess) {
					Buf_t_byte(adapt_buf, 0x50);
				}
				else {
					Buf_t_byte(adapt_buf, 0x10);
				}
				// これ・・・h264の場合はdtsいれた方が建設的かも
				Buf_t_pcr(adapt_buf, pts);
			}
			else if(has_randomAccess) {
				// ランダムアクセスがonの場合はfragがある
				Buf_t_byte(adapt_buf, 0x01);
				Buf_t_byte(adapt_buf, 0x40);
			}
		}
		// このlengthが184以下の場合はadapatation fieldで埋めが必要
		Buf_t_byte(sync_buf, 0x47);
		if(is_first) {
			Buf_t_byte(sync_buf, 0x40 | ((pid >> 8) & 0xFF));
		}
		else {
			Buf_t_byte(sync_buf, (pid >> 8) & 0xFF);
		}
		Buf_t_byte(sync_buf, pid & 0xFF);
		if(adapt_buf.length + header_buf.length + ttLibC_DynamicBuffer_refSize(frame_buffer) < 184) {
			uint32_t count = 184 - ttLibC_DynamicBuffer_refSize(frame_buffer) - header_buf.length;
			if(count != adapt_buf.length) {
				uint32_t adapt_buf_length = adapt_buf.length;
				for(uint32_t i = adapt_buf_length;i < count;++ i) {
					Buf_t_byte(adapt_buf, 0xFF);
				}
				adapt_buf.buf[0] = count - 1;
				if(adapt_buf_length == 0 && adapt_buf.length > 1) {
					adapt_buf.buf[1] = 0x00;
				}
			}
			if(adapt_buf.length != 0) {
				Buf_t_byte(sync_buf, 0x30 | (track->cc & 0x0F));
			}
			else {
				Buf_t_byte(sync_buf, 0x10 | (track->cc & 0x0F));
			}
			++ track->cc;
			ttLibC_DynamicBuffer_append(output_buffer, sync_buf.buf, sync_buf.length);
			ttLibC_DynamicBuffer_append(output_buffer, adapt_buf.buf, adapt_buf.length);
			ttLibC_DynamicBuffer_append(output_buffer, header_buf.buf, header_buf.length);
			ttLibC_DynamicBuffer_append(output_buffer, ttLibC_DynamicBuffer_refData(frame_buffer), ttLibC_DynamicBuffer_refSize(frame_buffer));
			ttLibC_DynamicBuffer_markAsRead(frame_buffer, ttLibC_DynamicBuffer_refSize(frame_buffer));
		}
		else {
			if(adapt_buf.length != 0) {
				Buf_t_byte(sync_buf, 0x30 | (track->cc & 0x0F));
			}
			else {
				Buf_t_byte(sync_buf, 0x10 | (track->cc & 0x0F));
			}
			++ track->cc;
			ttLibC_DynamicBuffer_append(output_buffer, sync_buf.buf, sync_buf.length);
			ttLibC_DynamicBuffer_append(output_buffer, adapt_buf.buf, adapt_buf.length);
			ttLibC_DynamicBuffer_append(output_buffer, header_buf.buf, header_buf.length);
			ttLibC_DynamicBuffer_append(output_buffer, ttLibC_DynamicBuffer_refData(frame_buffer), 184 - (adapt_buf.length + header_buf.length));
			ttLibC_DynamicBuffer_markAsRead(frame_buffer, 184 - (adapt_buf.length + header_buf.length));
		}
		is_first = false;
	}
	return true;
}

void ttLibC_Pes_close(ttLibC_Pes **pes) {
	ttLibC_Pes *target = *pes;
	if(target == NULL) {
		return;
	}
	if(!target->inherit_super.inherit_super.inherit_super.is_non_copy) {
		if(target->inherit_super.inherit_super.inherit_super.data) {
			ttLibC_free(target->inherit_super.inherit_super.inherit_super.data);
		}
	}
	ttLibC_Frame_close(&target->frame);
	ttLibC_free(target);
	*pes = NULL;
}

