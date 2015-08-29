/*
 * @file   pes.c
 * @brief  
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/13
 */

#include "pes.h"

#include "../mpegtsPacket.h"
#include "../../../log.h"
#include "../../../util/bitUtil.h"
#include "../../../util/ioUtil.h"
#include "../../../util/hexUtil.h"

#include "../../../frame/audio/aac.h"
#include "../../../frame/audio/mp3.h"
#include "../../../frame/video/h264.h"

#include <stdlib.h>
#include <string.h>

ttLibC_Pes *ttLibC_Pes_make(
		ttLibC_MpegtsPacket *prev_packet,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		uint16_t pid,
		uint8_t continuity_counter,
		uint8_t stream_type) {
	ttLibC_Pes *pes = (ttLibC_Pes *)ttLibC_MpegtsPacket_make(
			prev_packet,
			data,
			data_size,
			non_copy_mode,
			pts,
			timebase,
			MpegtsType_pes,
			pid,
			continuity_counter);
	if(pes != NULL) {
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
		case 0x06: // private data
			pes->frame_type = frameType_unknown;
			break;
		case 0x0F: // aac
			pes->frame_type = frameType_aac;
			break;
		case 0x10: // mpeg4 video(h263とか？ wmvもここくるか？)
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
//		case 0x100: // <-ありえなくない？
			pes->frame_type = frameType_unknown;
			break;
		}
	}
	return pes;
}

ttLibC_Pes *ttLibC_Pes_getPacket(
		ttLibC_MpegtsPacket *prev_packet,
		uint8_t *data,
		size_t data_size,
		uint8_t stream_type,
		uint16_t pid) {
	// 処理で利用するframeを保持しておくbuffer
	uint8_t *frame_buffer = NULL;
	// 処理で保持しているframe_bufferのサイズ
	size_t frame_buffer_size = 0;
	// 処理で保持しているframe_bufferの追記位置
	size_t frame_buffer_next_pos = 0;
	// packetのデータを読み込んでどういうデータであるか知る必要がある。
	// とりあえず内容を読み込もう・・・
	ttLibC_BitReader *reader = ttLibC_BitReader_make(data, data_size, BitReaderType_default);
	/*
	 * 内容メモ
	 * 先頭4byte
	 * 8bit syncByte
	 * 1bit transportErrorIndicator
	 * 1bit payloadUnitStartIndicator
	 * 1bit transportPriority
	 * 13bit pid
	 * 2bit scramblingControl
	 * 1bit adaptationFieldExist
	 * 1bit payloadFieldExist
	 * 4bit continuityCounter
	 */
	if(ttLibC_BitReader_bit(reader, 8) != 0x47) {
		// sync bitがおかしい。
	}
	ttLibC_BitReader_bit(reader, 1);
	bool is_unit_start = ttLibC_BitReader_bit(reader, 1) != 0;
	ttLibC_BitReader_bit(reader, 1);
	if(ttLibC_BitReader_bit(reader, 13) != pid) {
		// pidが一致しない、なんかがおかしい。
	}
	ttLibC_BitReader_bit(reader, 2);
	bool has_adaptation_field = ttLibC_BitReader_bit(reader, 1) != 0;
	ttLibC_BitReader_bit(reader, 1);
	uint8_t continuity_counter = ttLibC_BitReader_bit(reader, 4);
	data += 4;
	data_size -= 4;
	if(has_adaptation_field) {
//		LOG_PRINT("adaptation field読み込むよ？");
		/*
		 * adaptationFieldExistがある場合
		 * 8bit size
		 * 1bit discontinuityIndicator
		 * 1bit randomAccessIndicator(ランダムにアクセス可能なデータか示す。 keyFrameならここに無理やりアクセスできるので、このフラグが立っている。)
		 * 1bit elementaryStreamPriorityIndicator
		 * 1bit pcrFlag
		 * 1bit opcrFlag
		 * 1bit splicingPointFlag
		 * 1bit transportPrivateDataFlag
		 * 1bit adaptationFieldExtensionFlag
		 *
		 * pcrFlagが立っている場合
		 * 33bit pcrBase
		 * 6bit pcrPadding
		 * 9bit pcrExtension
		 *
		 * opcrFlagが立っている場合
		 * 33bit pcrBase
		 * 6bit pcrPadding
		 * 9bit pcrExtension
		 */
		uint8_t adaptation_field_size = ttLibC_BitReader_bit(reader, 8);
		data += (adaptation_field_size + 1);
		data_size -= (adaptation_field_size + 1);
		if(adaptation_field_size > 0) {
			// データ読み込みできる。
			ttLibC_BitReader_bit(reader, 1);
			ttLibC_BitReader_bit(reader, 1);
			ttLibC_BitReader_bit(reader, 1);
			bool has_pcr = ttLibC_BitReader_bit(reader, 1) != 0;
			bool has_opcr = ttLibC_BitReader_bit(reader, 1) != 0;
			ttLibC_BitReader_bit(reader, 1);
			ttLibC_BitReader_bit(reader, 1);
 			ttLibC_BitReader_bit(reader, 1);
 			-- adaptation_field_size;
			if(has_pcr) {
//				LOG_PRINT("pcrあるみたいです。");
				// pcrの読み込みする。
				// データ的に33bit読み込めないけど・・・ま、いいや。
				uint32_t pcrbase = ttLibC_BitReader_bit(reader, 33);
				ttLibC_BitReader_bit(reader, 6);
				ttLibC_BitReader_bit(reader, 9);
//				LOG_PRINT("pcrbase:%d %f", pcrbase, pcrbase / 90000.0f);
				adaptation_field_size -= 6;
			}
			if(adaptation_field_size > 0) {
				// 該当field分だけデータをずらすしかない。
				ttLibC_BitReader_close(&reader);
				// 新しいところから再開すれば、adaptationFieldを飛ばしたところからはじまる。
				reader = ttLibC_BitReader_make(data, data_size, BitReaderType_default);
			}
		}
	}
	/*
	 * pesデータのdataの部分には、frameデータをそのままいれておく。
	 * サイズはどうなるかわからないので、とりあえず、65536にして、足りなくなったら65536byteずつ増やしていく感じにしようか・・・
	 * 音声側はそんなに大きなサイズ必要ないはず・・・
	 */
	if(is_unit_start) {
		/*
		 * pes独自
		 * 24bit prefix 00000001fixっぽい
		 * 8bit streamId audio 0x0C - 0xDF? video 0xE0 - 0xEF?(vlcにはこのくくりがないので、特にきまっているわけではなさそう。)
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
		 * Xの部分を結合するとpts値になる。
		 * timebaseは90000になる。
		 * あとはデータ実体がはいっている・・・はず。
		 */
//		LOG_PRINT("unit startなので、情報を読み込みます。");
		if(ttLibC_BitReader_bit(reader, 24) != 1) {
			LOG_PRINT("prefix is not 1, broken mpegts?");
		}
		ttLibC_BitReader_bit(reader, 8); // streamId、まぁどうでもいい。
		uint32_t frame_size = ttLibC_BitReader_bit(reader, 16); // 0の場合もある、その場合は、サイズ不明・・・
//		LOG_PRINT("pes frame_size:%x", frame_size);
		ttLibC_BitReader_bit(reader, 2);
		ttLibC_BitReader_bit(reader, 2);
		ttLibC_BitReader_bit(reader, 1);
		ttLibC_BitReader_bit(reader, 1);
		ttLibC_BitReader_bit(reader, 1);
		ttLibC_BitReader_bit(reader, 1);

		bool has_pts = ttLibC_BitReader_bit(reader, 1) != 0; // pts flag
		bool has_dts = ttLibC_BitReader_bit(reader, 1) != 0; // dts flag
		ttLibC_BitReader_bit(reader, 1);
		ttLibC_BitReader_bit(reader, 1);
		ttLibC_BitReader_bit(reader, 1);
		ttLibC_BitReader_bit(reader, 1);
		ttLibC_BitReader_bit(reader, 1);
		ttLibC_BitReader_bit(reader, 1);
		data += 9;
		data_size -= 9;

		uint8_t pes_length = ttLibC_BitReader_bit(reader, 8);
		data +=  pes_length;
		data_size -= pes_length;
//		LOG_PRINT("pes_length:%d", pes_length);
		uint64_t pts = 0;
		if(has_pts) {
			ttLibC_BitReader_bit(reader, 4); // 2である必要あり。
			pts = ((ttLibC_BitReader_bit(reader, 4) << 29) & 0xC0000000)
					| ((ttLibC_BitReader_bit(reader, 16) << 14)     & 0x3FFF8000)
					| ((ttLibC_BitReader_bit(reader, 16) >> 1)      & 0x00007FFF);
//			LOG_PRINT("pts:%d, %f", pts, pts / 90000.0f);
		}
		if(has_dts) {
			ttLibC_BitReader_bit(reader, 4); // 3である必要あり。
		}
		// 新しいデータなので、pesを作成する必要がある。
		if(prev_packet != NULL) {
//			LOG_PRINT("前のデータが完成したみたいなので、一旦dumpをとっておわります。");

//			LOG_DUMP(prev_packet->inherit_super.inherit_super.data, prev_packet->inherit_super.inherit_super.buffer_size, true);
//			if(firstflag && pid != 0x100) {
//				return NULL;
//			}
			// 前のデータがある場合は、そこからbufferを取り出して、利用する必要がある。
			if(!prev_packet->inherit_super.inherit_super.is_non_copy) {
				// 元のデータがnon_copyなら使い回しが可能です。
				frame_buffer = prev_packet->inherit_super.inherit_super.data;
				frame_buffer_size = prev_packet->inherit_super.inherit_super.data_size;
				frame_buffer_next_pos = 0; // 0byte目から利用します。
			}
//			if(pid != 0x100) {
//				firstflag = true;
//			}
			// non_copyであると仮定して動作させます。
			prev_packet->inherit_super.inherit_super.is_non_copy = true;
		}
		if(frame_buffer == NULL) {
			frame_buffer_size = 65536;
			frame_buffer = malloc(frame_buffer_size);
			if(frame_buffer == NULL) {
				ERR_PRINT("failed to allocate frame buffer for pes.");
				// ここで処理を抜ける必要あり。
				ttLibC_BitReader_close(&reader);
				return NULL;
			}
			frame_buffer_next_pos = 0;
		}
		ttLibC_Pes *pes = ttLibC_Pes_make(
				prev_packet,
				frame_buffer,
				frame_buffer_size,
				true,
				pts,
				90000,
				pid,
				continuity_counter,
				stream_type);
		if(pes == NULL) {
			LOG_PRINT("failed to make pes.");
			// 処理を抜けます。
			ttLibC_BitReader_close(&reader);
			if(prev_packet == NULL) {
				// prev_packetがnullの場合はframe_bufferをallocateしたので、解放しておきます。
				free(frame_buffer);
			}
			return NULL;
		}
		// データが実体であるとして保持しておきます。
		pes->inherit_super.inherit_super.inherit_super.is_non_copy = false;
		// 残りのデータをコピーしておく。
		// TODO 本来ならここでデータが入る余地がきちんとあるか確認しておきたい。
		memcpy(frame_buffer, data, data_size);
		pes->inherit_super.inherit_super.inherit_super.buffer_size = data_size; // コピーしたサイズ分だけ、実データができたとする。
		ttLibC_BitReader_close(&reader);
		return pes;
	}
	else {
		ttLibC_BitReader_close(&reader);

		frame_buffer = prev_packet->inherit_super.inherit_super.data;
		frame_buffer_size = prev_packet->inherit_super.inherit_super.data_size;
		frame_buffer_next_pos = prev_packet->inherit_super.inherit_super.buffer_size;
//		LOG_PRINT("unit startじゃないデータにきました。");
		if(frame_buffer_size < frame_buffer_next_pos + data_size) {
			ERR_PRINT("data is overflowed.");
			return NULL;
		}
	 	memcpy(frame_buffer + frame_buffer_next_pos, data, data_size);
		prev_packet->inherit_super.inherit_super.buffer_size += data_size;
		return (ttLibC_Pes *)prev_packet;
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
		// 内容はadtsであるはず
		{
			// aacのptsはフレームごとにちょっとずつ移動しないといけない。(同じ位置で再生すればよいというものではない。)
			buffer = pes->inherit_super.inherit_super.inherit_super.data;
			left_size = pes->inherit_super.inherit_super.inherit_super.buffer_size;
			uint64_t sample_num_count = 0;
			uint32_t sample_rate = 0;
			do {
				// 読み込んだら前のaacのsample_num分ptsを進めないといけない。
				uint64_t pts = pes->inherit_super.inherit_super.inherit_super.pts;
				if(sample_rate != 0) {
					pts = pts + (sample_num_count * 90000 / sample_rate);
				}
				ttLibC_Aac *aac = ttLibC_Aac_getFrame(
						(ttLibC_Aac *)pes->inherit_super.frame,
						buffer,
						left_size,
						pts,
						pes->inherit_super.inherit_super.inherit_super.timebase);
				if(aac == NULL) {
					ERR_PRINT("failed to get aac frame.");
					return false;
				}
				sample_rate = aac->inherit_super.sample_rate;
				sample_num_count += aac->inherit_super.sample_num;
				pes->inherit_super.frame = (ttLibC_Frame *)aac;
				if(!callback(ptr, pes->inherit_super.frame)) {
					return false;
				}
				buffer += aac->inherit_super.inherit_super.buffer_size;
				left_size -= aac->inherit_super.inherit_super.buffer_size;
			}while(left_size > 0);
		}
		return true;
	case frameType_h264:
		{
			buffer = pes->inherit_super.inherit_super.inherit_super.data;
			left_size = pes->inherit_super.inherit_super.inherit_super.buffer_size;
			// あとはこの中身をみて、h264のフレームとして取り出せばOK
			do {
				// 内容はnal構造のh264のはず
				ttLibC_H264 *h264 = ttLibC_H264_getFrame(
						(ttLibC_H264 *)pes->inherit_super.frame,
						buffer,
						left_size,
						true,
						pes->inherit_super.inherit_super.inherit_super.pts,
						pes->inherit_super.inherit_super.inherit_super.timebase);
				if(h264 == NULL) {
					return false;
				}
				pes->inherit_super.frame = (ttLibC_Frame *)h264;
				if(!callback(ptr, pes->inherit_super.frame)) {
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
						(ttLibC_Mp3 *)pes->inherit_super.frame,
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
				pes->inherit_super.frame = (ttLibC_Frame *)mp3;
				if(!callback(ptr, pes->inherit_super.frame)) {
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
		ttLibC_MpegtsTrack *track, // ccの更新で必要
		ttLibC_Frame *frame, // 書き込み対象のデータ
		ttLibC_ContainerWriterFunc callback,
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
		config = NULL; // 使わないのでconfigをnullにしておく。
		total_size = 6 + h264->inherit_super.inherit_super.buffer_size;
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
		total_size = 6 + track->h264_configData->buffer_size + h264->inherit_super.inherit_super.buffer_size;
		p_buf[3] = 0x30 | (track->cc & 0x0F);
		if(total_size < 162) {
			// このサイズの場合は、adaptation Fieldで埋めが必要。
			LOG_PRINT("data size is too small, need to have padding with adaptation field. :make later.");
			return false;
		}
		else {
			// adaptation fieldをつくる。
			p_buf[4] = 0x07; // adaptation sizeは7にする。
			p_buf[5] = 0x50; // random Access + pcr
			p_buf[6] = (pts >> 25) & 0xFF; // ここからpcr base 33bitにはデータをいれるが、extensionにはデータをいれないことにする。
			p_buf[7] = (pts >> 17) & 0xFF; // ここからpcr base 33bitにはデータをいれるが、extensionにはデータをいれないことにする。
			p_buf[8] = (pts >> 9) & 0xFF; // ここからpcr base 33bitにはデータをいれるが、extensionにはデータをいれないことにする。
			p_buf[9] = (pts >> 1) & 0xFF; // ここからpcr base 33bitにはデータをいれるが、extensionにはデータをいれないことにする。
			p_buf[10] = ((pts << 7) & 0x80) | 0x7E;
			p_buf[11] = 0x00;
			p_buf += 12;
			p_buf_left_size -= 12;
		}
		break;
	}
	track->cc ++;
	p_buf[0] = 0x00;
	p_buf[1] = 0x00;
	p_buf[2] = 0x01;
	p_buf[3] = 0xE0; // ID固定
	uint32_t pes_size = total_size + 8;
	if(pes_size < 0x10000) {
		p_buf[4] = (pes_size >> 8) & 0xFF;
		p_buf[5] = pes_size & 0xFF;
	}
	else {
		p_buf[4] = 0x00; // pesPacketLengthは0にしておきます。
		p_buf[5] = 0x00; // pesPacketLengthは0にしておきます。
	}
	p_buf[6] = 0x80; // markerつけて
	p_buf[7] = 0x80; // ptsいれて
	// ここから5byte + ptsデータ
	p_buf[8] = 0x05; // pes拡張データ とりあえずptsのみ
	p_buf[9] = 0x21 | ((pts >> 30) & 0x0E);
	p_buf[10] = (pts >> 22) & 0xFF;
	p_buf[11] = ((pts >> 14) & 0xFE) | 0x01;
	p_buf[12] = (pts >> 7) & 0xFF;
	p_buf[13] = ((pts << 1) & 0xFE) | 0x01;
	p_buf += 14;
	p_buf_left_size -= 14;
	// あとはデータをいれていく。
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
				callback(ptr, buf, 188);
				break;
			}
		}
		if(p_buf_left_size == 0) {
			// pesの先頭を書き込む処理が必要。
			callback(ptr, buf, 188);
			// 次のデータを準備する。
			if(nal_size >= 184) {
				// 4byteデータをつくって、残りのデータをつくっていきます。
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

// TODO rename from aac_data to audio_data.
typedef struct {
	uint32_t total_size;
	void    *buf;
	uint8_t *p_buf;
	uint32_t p_buf_left_size;
	uint64_t target_pts;
	uint64_t start_pts;
	uint8_t *data;
	size_t   data_size;
	ttLibC_ContainerWriterFunc callback;
	ttLibC_MpegtsTrack *track;
	void *ptr;
	bool error_flg;
} aac_data_t;

bool Pes_checkAudioTotalSize(void *ptr, ttLibC_Frame *frame) {
	aac_data_t *aacData = (aac_data_t *)ptr;
	if(aacData->target_pts < frame->pts) {
		// すぎたので、読み込みやめる。
		return false;
	}
	if(aacData->start_pts > frame->pts) {
		aacData->start_pts = frame->pts;
	}
	switch(frame->type) {
	case frameType_aac:
		{
			// ここでAACにキャストしているので、mp3の場合はおかしくなるな。
			ttLibC_Aac *aac = (ttLibC_Aac *)frame;
			if(aac->type == AacType_raw) {
				aacData->total_size += frame->buffer_size + 7;
			}
			else {
				aacData->total_size += frame->buffer_size;
			}
		}
		break;
	case frameType_mp3:
		{
			ttLibC_Mp3 *mp3 = (ttLibC_Mp3 *)frame;
			aacData->total_size += frame->buffer_size;
		}
		break;
	default:
		ERR_PRINT("unexpected frame.:%d", frame->type);
		aacData->error_flg = true;
		return false;
	}
	return true;
}

bool Pes_writeAudioData(void *ptr, ttLibC_Frame *frame) {
	aac_data_t *aacData = (aac_data_t *)ptr;
	if(aacData->target_pts < frame->pts) {
		// 完了したので、終了
		return false;
	}
	uint8_t aac_header_buf[7];
	bool is_body_flag = true;
	if(frame->type == frameType_aac && ((ttLibC_Aac *)frame)->type == AacType_raw) {
		if(ttLibC_Aac_readAdtsHeader((ttLibC_Aac *)frame, aac_header_buf, 7) == 0) {
			LOG_PRINT("failed to get adts header information.");
			aacData->error_flg = true;
			return false;
		}
		aacData->data = aac_header_buf;
		aacData->data_size = 7;
		is_body_flag = false; // 処理中のデータはbodyではない
	}
	else {
		aacData->data = frame->data;
		aacData->data_size = frame->buffer_size;
	}
	while(aacData->total_size > 0) {
		// まだデータが残っている場合は処理していく。
		*aacData->p_buf = *aacData->data;
		++ aacData->data;
		-- aacData->data_size;
		++ aacData->p_buf;
		-- aacData->p_buf_left_size;
		-- aacData->total_size;
		if(aacData->p_buf_left_size == 0) {
			// 書き込みバッファが満了した場合
			aacData->callback(aacData->ptr, aacData->buf, 188);

			if(aacData->total_size > 0) {
				aacData->p_buf = aacData->buf;
				aacData->p_buf_left_size = 188;

				aacData->p_buf[0] = 0x47;
				aacData->p_buf[1] = ((aacData->track->frame_queue->track_id >> 8) & 0xFF);
				aacData->p_buf[2] = aacData->track->frame_queue->track_id & 0xFF;
				if(aacData->total_size >= 184) {
					aacData->p_buf[3] = 0x10 | (aacData->track->cc & 0x0F);
					aacData->p_buf += 4;
					aacData->p_buf_left_size -= 4;
				}
				else {
					uint8_t adaptation_field_size = 183 - aacData->total_size;
					aacData->p_buf[3] = 0x30 | (aacData->track->cc & 0x0F);
					aacData->p_buf += 4;
					aacData->p_buf_left_size -= 4;
					// size of adaptation field.
					*aacData->p_buf = adaptation_field_size;
					++ aacData->p_buf;
					-- aacData->p_buf_left_size;
					for(int i = 0;i < adaptation_field_size;i ++) {
						if(i == 0) {
							*aacData->p_buf = 0x00;
						}
						else {
							*aacData->p_buf = 0xFF;
						}
						++ aacData->p_buf;
						-- aacData->p_buf_left_size;
					}
				}
				aacData->track->cc ++;
			}
		}
		if(aacData->data_size == 0) {
			if(!is_body_flag) {
				// まだbodyデータを読み込む必要がある。
				aacData->data = frame->data;
				aacData->data_size = frame->buffer_size;
			}
			break;
		}
	}
	return true;
}

bool ttLibC_Pes_writeAudioPacket(
		ttLibC_MpegtsWriter_ *writer,
		ttLibC_MpegtsTrack *track,
		ttLibC_ContainerWriterFunc callback,
		void *ptr) {
	// ptsを超えないデータについて、まず、全体長を調べる。
	// 続いて書き込みを実施する。
	uint8_t buf[188];
	aac_data_t aacData;
	aacData.track = track;
	aacData.total_size = 0;
	aacData.buf = buf;
	aacData.p_buf = buf;
	aacData.p_buf_left_size = 188;
	aacData.data = NULL;
	aacData.data_size = 0;
	aacData.start_pts = 0xFFFFFFFFFFFFFFFFL;
	aacData.target_pts = writer->target_pos;
	aacData.callback = callback;
	aacData.ptr = ptr;
	aacData.error_flg = false;

	// まずqueueの中身を確認して、total_sizeを出さないといけない。
	ttLibC_FrameQueue_ref(track->frame_queue, Pes_checkAudioTotalSize, &aacData);

	// あとはデータをかいていく。
	aacData.p_buf[0] = 0x47;
	aacData.p_buf[1] = 0x40 | ((track->frame_queue->track_id >> 8) & 0xFF);
	aacData.p_buf[2] = track->frame_queue->track_id & 0xFF;
	aacData.p_buf[3] = 0x30 | (track->cc & 0x0F);
	track->cc ++;
	if(aacData.total_size < 168) {
		uint32_t fill_size = 168 - aacData.total_size + 1;
		aacData.p_buf[4] = fill_size;
		aacData.p_buf[5] = 0x40;
		aacData.p_buf += 6;
		aacData.p_buf_left_size -= 6;
		for(uint32_t i = 0;i < fill_size - 1;i ++) {
			*aacData.p_buf = 0xFF;
			++ aacData.p_buf;
			-- aacData.p_buf_left_size;
		}
	}
	else {
		// adaptation fieldを書き込む。
		aacData.p_buf[4] = 0x01;
		aacData.p_buf[5] = 0x40; // random accessのみON
		aacData.p_buf += 6;
		aacData.p_buf_left_size -= 6;
	}
	aacData.p_buf[0] = 0x00;
	aacData.p_buf[1] = 0x00;
	aacData.p_buf[2] = 0x01;
	aacData.p_buf[3] = 0xC0;
	// データサイズをいれます。total_size + 8;
	uint32_t pes_size = aacData.total_size + 8;
	if(pes_size < 0x10000) {
		aacData.p_buf[4] = (pes_size >> 8) & 0xFF;
		aacData.p_buf[5] = pes_size & 0xFF;
	}
	else {
		aacData.p_buf[4] = 0x00; // pesPacketLengthは0にしておきます。
		aacData.p_buf[5] = 0x00; // pesPacketLengthは0にしておきます。
	}
	aacData.p_buf[6] = 0x80;
	aacData.p_buf[7] = 0x80;
	aacData.p_buf[8] = 0x05;
	aacData.p_buf[9] = 0x21 | ((aacData.start_pts >> 30) & 0x0E);
	aacData.p_buf[10] = (aacData.start_pts >> 22) & 0xFF;
	aacData.p_buf[11] = ((aacData.start_pts >> 14) & 0xFE) | 0x01;
	aacData.p_buf[12] = (aacData.start_pts >> 7) & 0xFF;
	aacData.p_buf[13] = ((aacData.start_pts << 1) & 0xFE) | 0x01;
	aacData.p_buf += 14;
	aacData.p_buf_left_size -= 14;
	// あとはデータを書いていけばOK
	ttLibC_FrameQueue_dequeue(track->frame_queue, Pes_writeAudioData, &aacData);
	if(aacData.error_flg) {
		return false;
	}
	// 処理がおわったら、ptsを更新しなければならない。
	writer->current_pts_pos = writer->target_pos;
	return true;
}

