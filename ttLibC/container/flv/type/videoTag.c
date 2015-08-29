/*
 * @file   videoTag.c
 * @brief  
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/12
 */

#include "videoTag.h"
#include "../flvTag.h"
#include "../../../log.h"
#include "../../../frame/video/h264.h"
#include "../../../frame/video/flv1.h"
#include "../../../util/hexUtil.h"
#include "../../../util/ioUtil.h"

ttLibC_FlvVideoTag *ttLibC_FlvVideoTag_make(
		ttLibC_FlvTag *prev_tag,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		uint32_t track_id,
		uint8_t codec_id) {
	ttLibC_FlvVideoTag *video_tag = (ttLibC_FlvVideoTag *)ttLibC_FlvTag_make(
			prev_tag,
			data,
			data_size,
			non_copy_mode,
			pts,
			timebase,
			FlvType_video,
			track_id);
	if(video_tag != NULL) {
		video_tag->codec_id = codec_id;
		switch(codec_id) {
		case 1: // jpeg
			video_tag->frame_type = frameType_unknown;
			break;
		case 2:
			video_tag->frame_type = frameType_flv1;
			break;
		case 3:
			// screen video
			video_tag->frame_type = frameType_unknown;
			break;
		case 4:
			// on2vp6
			video_tag->frame_type = frameType_vp6;
			break;
		case 5:
			// on2vp6 alpha
			video_tag->frame_type = frameType_vp6; // 本当はvp6 alpha(あとで判定する。)
			break;
		case 6:
			// screen video 2
			video_tag->frame_type = frameType_unknown;
			break;
		case 7:
			video_tag->frame_type = frameType_h264;
			break;
		default:
			video_tag->frame_type = frameType_unknown;
			break;
		}
	}
	return video_tag;
}

ttLibC_FlvVideoTag *ttLibC_FlvVideoTag_getTag(
		ttLibC_FlvTag *prev_tag,
		uint8_t *data,
		size_t data_size) {
	// データを読み取って解析していく。
	// dataの部分には、フレームデータをそのままいれておく形にしておく。
	/*
	 * 内容メモ
	 * 1byte フラグ
	 * 3byte size
	 * 3byte timestamp
	 * 1byte timestamp-ext
	 * 3byte track_id
	 * 4bit frame type
	 * 4bit codec type
	 *
	 * vp6
	 *  4bit horizontal adjustment
	 *  4bit vertical adjustment
	 *
	 * vp6a
	 *  4byte offset to alpha
	 *
	 * h264
	 *  1byte packet type 0:msg 1:frame 2:end
	 *  3byte dts
	 *
	 * ...
	 *
	 * 4byte post_size
	 */
	size_t size = ((data[1] << 16) & 0xFF0000) | ((data[2] << 8) & 0xFF00) | (data[3] & 0xFF);
	uint32_t timestamp = ((data[4] << 16) & 0xFF0000) | ((data[5] << 8) & 0xFF00) | (data[6] & 0xFF) | ((data[7] << 24) & 0xFF000000);
	uint32_t track_id = ((data[8] << 16) & 0xFF0000) | ((data[9] << 8) & 0xFF00) | (data[10] & 0xFF);
	uint8_t frame_type = ((data[11] >> 4) & 0x0F);
	uint8_t codec_id = data[11] & 0x0F;
	uint32_t dts = 0;
	data += 12;
	data_size -= 12;

	size_t post_size = ((*(data + data_size - 4)) << 24) | ((*(data + data_size - 3)) << 16) | ((*(data + data_size - 2)) << 8) | (*(data + data_size - 1));
	if(size + 11 != post_size) {
		ERR_PRINT("size data is crazy. out of flv format.");
		return NULL;
	}
	return ttLibC_FlvVideoTag_make(
			prev_tag,
			data,
			data_size - 4,
			true,
			timestamp,
			1000,
			track_id,
			codec_id);
}

bool ttLibC_FlvVideoTag_getFrame(
		ttLibC_FlvVideoTag *video_tag,
		ttLibC_getFrameFunc callback,
		void *ptr) {
	uint8_t *buffer = video_tag->inherit_super.inherit_super.inherit_super.data;
	size_t left_size = video_tag->inherit_super.inherit_super.inherit_super.buffer_size;
	switch(video_tag->frame_type) {
	case frameType_flv1:
		{
			ttLibC_Flv1 *flv1 = ttLibC_Flv1_getFrame(NULL, buffer, left_size, true, video_tag->inherit_super.inherit_super.inherit_super.pts, video_tag->inherit_super.inherit_super.inherit_super.timebase);
			if(flv1 == NULL) {
				return false;
			}
			video_tag->inherit_super.frame = (ttLibC_Frame *)flv1;
			return callback(ptr, video_tag->inherit_super.frame);
		}
		break;
	case frameType_h264:
		{
			// 始めのbyteをみて、なんであるか判定しないといけない。
			// frameを取り出すことができたら、callbackで呼び出してやる。
			// frameができたら、video_tag->frameにくくっておく。
			// この２点で十分と思われる。
			// ttLibC_h264は基本annexBでできているので、flvのデータをそのまま使うことはできない。
			// よってデータを改良しなければならない。
			// 書き直しが生じるとおもってつくっていく必要がある。
			// これはavcc形式のデータの場合はどこでも発生する話なので、ttLibC_H264側で処理やらせよう。
			uint32_t dts = (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
			if(dts != 0) {
				LOG_PRINT("found dts is non zero h264 data.");
			}
			if(buffer[0] == 0x00) {
				// avcC データ
				buffer += 4;
				left_size -= 4;
				uint32_t length_size = 0;
				ttLibC_H264 *h264 = ttLibC_H264_analyzeAvccTag((ttLibC_H264 *)video_tag->inherit_super.frame, buffer, left_size, &length_size);
				if(h264 == NULL) {
					ERR_PRINT("failed to make h264 configdata frame.");
					return false;
				}
				if(length_size < 3) {
					ERR_PRINT("avcc size length is too small.");
					return false;
				}
				// video_tagのlength_sizeはコピーして保持しないとだめ。
				video_tag->h264_length_size = length_size;
				video_tag->inherit_super.frame = (ttLibC_Frame *)h264;
				return callback(ptr, video_tag->inherit_super.frame);
			}
			else if(buffer[0] == 0x01) {
				// 通常のh264データ
				buffer += 4;
				left_size -= 4;
				uint8_t *buf = buffer;
				size_t buf_size = left_size;
				// サイズも取得する。
				do {
					uint32_t size = 0;
					// まずsize部を00 00 01に変更する。
					for(int i = 1;i <= video_tag->h264_length_size;++ i) {
						size = (size << 8) | *buf;
						if(i != video_tag->h264_length_size) {
							*buf = 0x00;
						}
						else {
							*buf = 0x01;
						}
						++ buf;
						-- buf_size;
					}
					buf += size;
					buf_size -= size;
				} while(buf_size > 0);
				// ここでbufferの中身が完成した
				ttLibC_H264 *h264 = ttLibC_H264_getFrame(
						(ttLibC_H264 *)video_tag->inherit_super.frame,
						buffer,
						left_size,
						true,
						video_tag->inherit_super.inherit_super.inherit_super.pts,
						video_tag->inherit_super.inherit_super.inherit_super.timebase);
				if(h264 == NULL) {
					ERR_PRINT("failed to make h264 data.");
					return false;
				}
				video_tag->inherit_super.frame = (ttLibC_Frame *)h264;
				return callback(ptr, video_tag->inherit_super.frame);
			}
			else if(buffer[0] == 0x02) {
				// h264の終端フラグ
				return true;
			}
			else {
				// それ以外
				LOG_PRINT("found unknown data. h264 end sign?");
				LOG_DUMP(buffer, left_size, true);
				return false;
			}
		}
		break;
	case frameType_vp6:
		// 始めのbyteをみて・・・以下略
		break;
	default:
		break;
	}
	return false;
}

bool ttLibC_FlvVideoTag_writeTag(
		ttLibC_FlvWriter_ *writer,
		ttLibC_Frame *frame,
		ttLibC_ContainerWriterFunc callback,
		void *ptr) {
	uint8_t buf[256]; // buffer for writing.
	switch(frame->type) {
	case frameType_flv1:
		{
			ttLibC_Video *video = (ttLibC_Video *)frame;
			uint8_t *buffer = video->inherit_super.data;
			uint32_t left_size = video->inherit_super.buffer_size;
			buf[0]  = 0x09;
			uint32_t pre_size = 1 + left_size;
			buf[1]  = (pre_size >> 16) & 0xFF;
			buf[2]  = (pre_size >> 8) & 0xFF;
			buf[3]  = pre_size & 0xFF;
			// pts
			buf[4]  = (video->inherit_super.pts >> 16) & 0xFF;
			buf[5]  = (video->inherit_super.pts >> 8) & 0xFF;
			buf[6]  = video->inherit_super.pts & 0xFF;
			buf[7]  = (video->inherit_super.pts >> 24) & 0xFF;
			// track
			buf[8]  = 0x00;
			buf[9]  = 0x00;
			buf[10] = 0x00;
			// tag
			ttLibC_Flv1 *flv1 = (ttLibC_Flv1 *)video;
			switch(flv1->type) {
			case Flv1Type_disposableInner:
				buf[11] = 0x32;
				break;
			case Flv1Type_inner:
				buf[11] = 0x22;
				break;
			case Flv1Type_intra:
				buf[11] = 0x12;
				break;
			}
			// header
			if(!callback(ptr, buf, 12)) {
				return false;
			}
			// data_body
			if(!callback(ptr, buffer, left_size)) {
				return false;
			}
			// post size
			uint32_t post_size = pre_size + 11;
			buf[0] = (post_size >> 24) & 0xFF;
			buf[1] = (post_size >> 16) & 0xFF;
			buf[2] = (post_size >> 8) & 0xFF;
			buf[3] = post_size & 0xFF;
			if(!callback(ptr, buf, 4)) {
				return false;
			}
			// done.
			return true;
		}
		break;
	case frameType_h264:
		{
			// h264の場合はmshである可能性がある。
			// h264のconfigDataのデータはcrc32を取得して保存しておく。
			// このcrcが変わったらmshを再度書き出す必要がでてくる。(まぁ普通ないけど)
			ttLibC_H264 *h264 = (ttLibC_H264*)frame;
			switch(h264->type) {
			case H264Type_configData:
				// writerが保持している現在のconfigDataのcrc32を計算し、writerが保持しているものと比較。
				// 一致していればすでに書き込みずみなので放置
				// 一致しなければ書き込みを実施する、
				// なおcrc32が0の場合は初回書き込みなので、必ず書き込む
				// h264のconfigDataのcrc32計算は他でも必要になりそうですね。
				{
					uint32_t crc32_value = ttLibC_H264_getConfigCrc32(h264);
					if(writer->video_track.crc32 == 0 || writer->video_track.crc32 != crc32_value) {
						/*
						 * 09 s  i  ze ti me st mp tr ck id 17 00 00 00 00 avccTag
						 */
						uint8_t avcc[256];
						size_t size = ttLibC_H264_readAvccTag(h264, avcc, sizeof(avcc));
						// 中身がきまったので、flvタグをつくって応答します。
						// bufの内容を書いていきます。
						buf[0]  = 0x09;
						uint32_t pre_size = size + 4 + 1;
						buf[1]  = (pre_size >> 16) & 0xFF;
						buf[2]  = (pre_size >> 8) & 0xFF;
						buf[3]  = pre_size & 0xFF;
						// pts値
						buf[4]  = (h264->inherit_super.inherit_super.pts >> 16) & 0xFF;
						buf[5]  = (h264->inherit_super.inherit_super.pts >> 8) & 0xFF;
						buf[6]  = h264->inherit_super.inherit_super.pts & 0xFF;
						buf[7]  = (h264->inherit_super.inherit_super.pts >> 24) & 0xFF;
						// track
						buf[8]  = 0x00;
						buf[9]  = 0x00;
						buf[10] = 0x00;
						// tag
						buf[11] = 0x17;
						// type
						buf[12] = 0x00;
						// dts
						buf[13] = 0x00;
						buf[14] = 0x00;
						buf[15] = 0x00;
						if(!callback(ptr, buf, 16)) {
							return false;
						}
						// data
						if(!callback(ptr, avcc, size)) {
							return false;
						}
						// post size
						uint32_t post_size = pre_size + 11;
						buf[0] = (post_size >> 24) & 0xFF;
						buf[1] = (post_size >> 16) & 0xFF;
						buf[2] = (post_size >> 8) & 0xFF;
						buf[3] = post_size & 0xFF;
						if(!callback(ptr, buf, 4)) {
							return false;
						}
						// all ok.
						writer->video_track.crc32 = crc32_value;
					}
					return true;
				}
				break;
			case H264Type_slice:
			case H264Type_sliceIDR:
				{
					// まず全体の長さを知る必要がある。
					uint32_t pre_size = 5; // codec + type + dts(3byte)
					ttLibC_H264_NalInfo nal_info;
					uint8_t *data = h264->inherit_super.inherit_super.data;
					size_t data_size = h264->inherit_super.inherit_super.buffer_size;
					while(ttLibC_H264_getNalInfo(&nal_info, data, data_size)) {
						pre_size += 4; // size;
						pre_size += nal_info.nal_size - nal_info.data_pos;
						data += nal_info.nal_size;
						data_size -= nal_info.nal_size;
					}
					buf[0]  = 0x09;
					buf[1]  = (pre_size >> 16) & 0xFF;
					buf[2]  = (pre_size >> 8) & 0xFF;
					buf[3]  = pre_size & 0xFF;
					// pts値
					buf[4]  = (h264->inherit_super.inherit_super.pts >> 16) & 0xFF;
					buf[5]  = (h264->inherit_super.inherit_super.pts >> 8) & 0xFF;
					buf[6]  = h264->inherit_super.inherit_super.pts & 0xFF;
					buf[7]  = (h264->inherit_super.inherit_super.pts >> 24) & 0xFF;
					// track
					buf[8]  = 0x00;
					buf[9]  = 0x00;
					buf[10] = 0x00;
					// tag
					if(h264->type == H264Type_slice) {
						buf[11] = 0x27;
					}
					else {
						buf[11] = 0x17;
					}
					// type
					buf[12] = 0x01;
					// dts
					buf[13] = 0x00;
					buf[14] = 0x00;
					buf[15] = 0x00;
					if(!callback(ptr, buf, 16)) {
						return false;
					}
					// 実態を書き込んでいく。
					data = h264->inherit_super.inherit_super.data;
					data_size = h264->inherit_super.inherit_super.buffer_size;
					while(ttLibC_H264_getNalInfo(&nal_info, data, data_size)) {
						uint32_t size = nal_info.nal_size - nal_info.data_pos;
						buf[0] = (size >> 24) & 0xFF;
						buf[1] = (size >> 16) & 0xFF;
						buf[2] = (size >> 8) & 0xFF;
						buf[3] = size & 0xFF;
						if(!callback(ptr, buf, 4)) {
							return false;
						}
						if(!callback(ptr, data + nal_info.data_pos, size)) {
							return false;
						}
						data += nal_info.nal_size;
						data_size -= nal_info.nal_size;
					}
					// post size
					uint32_t post_size = pre_size + 11;
					buf[0] = (post_size >> 24) & 0xFF;
					buf[1] = (post_size >> 16) & 0xFF;
					buf[2] = (post_size >> 8) & 0xFF;
					buf[3] = post_size & 0xFF;
					if(!callback(ptr, buf, 4)) {
						return false;
					}
					return true;
				}
				break;
			default:
				ERR_PRINT("unexpected h264 type.:%d", h264->type);
				return false;
			}
		}
		break;
	case frameType_vp6:
		break;
	default:
		ERR_PRINT("unexpected frame type of flv.");
		return false;
	}
	return false;
}
