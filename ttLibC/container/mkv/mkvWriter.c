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
#include "../../frame/audio/opus.h"
#include "../../frame/video/h264.h"
#include "../../frame/video/vp8.h"
#include "../../util/byteUtil.h"
#include "../../util/ioUtil.h"

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
	for(uint32_t i = 0;i < types_num;++ i) {
		ttLibC_MkvWriteTrack *track = ttLibC_malloc(sizeof(ttLibC_MkvWriteTrack));
		track->frame_queue = ttLibC_FrameQueue_make(i + 1, 255);
		track->h264_configData = NULL;
		track->frame_type = target_frame_types[i];
		// これだけでよさそう。
		ttLibC_StlMap_put(writer->track_list, (void *)(i + 1), (void *)track);
	}
	// とりあえず後で
	writer->inherit_super.is_webm = false; // とりあえずmkvとして処理はじめることにする。
	writer->inherit_super.inherit_super.timebase = 1000;
	writer->inherit_super.inherit_super.pts = 0;
	writer->max_unit_duration = max_unit_duration;
	writer->is_first = true;
	writer->target_pos = 0;
	writer->status = status_init_check;
	return (ttLibC_MkvWriter *)writer;
}

static bool MkvWriter_initCheckTrack(void *ptr, void *key, void *item) {
	if(item == NULL) {
		return false;
	}
	ttLibC_MkvWriteTrack *track = (ttLibC_MkvWriteTrack *)item;
	switch(track->frame_type) {
	case frameType_h264:
		{
			return track->h264_configData != NULL;
		}
	default:
		{
			// vp8とかの動画フレームだったらkeyFrameの情報があるかの確認の方がふさわしいと思われる・・・
			return ttLibC_FrameQueue_ref_first(track->frame_queue) != NULL;
		}
	}
	return true;
}

static bool MkvWriter_makeTrackEntry(void *ptr, void *key, void *item) {
	if(ptr != NULL && item != NULL) {
		ttLibC_DynamicBuffer *buffer = (ttLibC_DynamicBuffer *)ptr;
		ttLibC_MkvWriteTrack *track = (ttLibC_MkvWriteTrack *)item;
		switch(track->frame_type) {
		case frameType_h264:
		case frameType_aac:
		case frameType_vp8:
		case frameType_opus:
			break;
		default:
			ERR_PRINT("想定外のframeTypeでした。");
			return true;
		}
		// あとはトラックの中身をつくっていけばよい。
		// trackEntryを頭につける必要があるわけだが・・・codecPrivateがどのようなサイズになるかわからない以上、どうしようもないか・・・
		// というわけで、ここでもDynamiBufferをつくって、あとで解放という処理で攻めてみるか・・・
		ttLibC_DynamicBuffer *trackEntryBuffer = ttLibC_DynamicBuffer_make();
		// このtrackEntryBufferにデータをいれていく。
		uint8_t buf[256];
		size_t in_size = 0;
		ttLibC_ByteConnector *connector = ttLibC_ByteConnector_make(buf, 256, ByteUtilType_default);
		// trackEntryはあとで書き込む。
//		ttLibC_ByteConnector_ebml2(connector, MkvType_TrackEntry, true);
		// trackNumber
		ttLibC_ByteConnector_ebml2(connector, MkvType_TrackNumber, true);
		ttLibC_ByteConnector_ebml2(connector, 1, false);
		ttLibC_ByteConnector_bit(connector, track->frame_queue->track_id, 8);
		// trackUID
		ttLibC_ByteConnector_ebml2(connector, MkvType_TrackUID, true);
		ttLibC_ByteConnector_ebml2(connector, 1, false);
		ttLibC_ByteConnector_bit(connector, track->frame_queue->track_id, 8);
		// ここまでは共通あとは、別々と思われ。
		// video要素
		uint8_t inner[256];
		ttLibC_ByteConnector *innerConnector = ttLibC_ByteConnector_make(inner, 256, ByteUtilType_default);
		switch(track->frame_type) {
		case frameType_h264:
			{
				// codecID
				ttLibC_ByteConnector_ebml2(connector, MkvType_CodecID, true);
				ttLibC_ByteConnector_ebml2(connector, 15, false);
				ttLibC_ByteConnector_string(connector, "V_MPEG4/ISO/AVC", 15);
				// trackType
				ttLibC_ByteConnector_ebml2(connector, MkvType_TrackType, true);
				ttLibC_ByteConnector_ebml2(connector, 1, false);
				ttLibC_ByteConnector_bit(connector, 1, 8);
				ttLibC_H264 *configData = (ttLibC_H264 *)track->h264_configData;
				// video要素の中身をつくっていく。
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_PixelWidth, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 2, false);
				ttLibC_ByteConnector_bit(innerConnector, configData->inherit_super.width, 16);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_PixelHeight, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 2, false);
				ttLibC_ByteConnector_bit(innerConnector, configData->inherit_super.height, 16);

				ttLibC_ByteConnector_ebml2(connector, MkvType_Video, true);
				ttLibC_ByteConnector_ebml2(connector, innerConnector->write_size, false);
				ttLibC_ByteConnector_string(connector, (const char *)inner, innerConnector->write_size);
				// あとはcodecPrivateを書き出せばOK
				in_size = ttLibC_H264_readAvccTag(configData, inner, 256);
				ttLibC_ByteConnector_ebml2(connector, MkvType_CodecPrivate, true);
				ttLibC_ByteConnector_ebml2(connector, in_size, false);
				ttLibC_DynamicBuffer_append(trackEntryBuffer, buf, connector->write_size);
				ttLibC_DynamicBuffer_append(trackEntryBuffer, inner, in_size);
			}
			break;
		case frameType_aac:
			{
				// codecID
				ttLibC_ByteConnector_ebml2(connector, MkvType_CodecID, true);
				ttLibC_ByteConnector_ebml2(connector, 5, false);
				ttLibC_ByteConnector_string(connector, "A_AAC", 5);
				// trackType
				ttLibC_ByteConnector_ebml2(connector, MkvType_TrackType, true);
				ttLibC_ByteConnector_ebml2(connector, 1, false);
				ttLibC_ByteConnector_bit(connector, 2, 8);
				// audioの子要素をつくっていく。
				ttLibC_Aac *aac = (ttLibC_Aac *)ttLibC_FrameQueue_ref_first(track->frame_queue);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_SamplingFrequency, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 4, false);
				float sr = aac->inherit_super.sample_rate;
				ttLibC_ByteConnector_bit(innerConnector, *(uint32_t *)&sr, 32);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_Channels, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 1, false);
				ttLibC_ByteConnector_bit(innerConnector, aac->inherit_super.channel_num, 8);
				// bitはいるのかな？とりあえず放置しとくか・・・

				ttLibC_ByteConnector_ebml2(connector, MkvType_Audio, true);
				ttLibC_ByteConnector_ebml2(connector, innerConnector->write_size, false);
				ttLibC_ByteConnector_string(connector, (const char *)inner, innerConnector->write_size);
				// dsi情報をつくらなければいけないけどさ・・・
				in_size = ttLibC_Aac_readDsiInfo(aac, inner, 256);
				ttLibC_ByteConnector_ebml2(connector, MkvType_CodecPrivate, true);
				ttLibC_ByteConnector_ebml2(connector, in_size, false);
				ttLibC_DynamicBuffer_append(trackEntryBuffer, buf, connector->write_size);
				ttLibC_DynamicBuffer_append(trackEntryBuffer, inner, in_size);
			}
			break;
		case frameType_vp8:
			{
				// codecID
				ttLibC_ByteConnector_ebml2(connector, MkvType_CodecID, true);
				ttLibC_ByteConnector_ebml2(connector, 5, false);
				ttLibC_ByteConnector_string(connector, "V_VP8", 5);
				// trackType
				ttLibC_ByteConnector_ebml2(connector, MkvType_TrackType, true);
				ttLibC_ByteConnector_ebml2(connector, 1, false);
				ttLibC_ByteConnector_bit(connector, 1, 8);
				// video要素の中身をつくっていく。
				ttLibC_Vp8 *vp8 = (ttLibC_Vp8 *)ttLibC_FrameQueue_ref_first(track->frame_queue);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_PixelWidth, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 2, false);
				ttLibC_ByteConnector_bit(innerConnector, vp8->inherit_super.width, 16);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_PixelHeight, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 2, false);
				ttLibC_ByteConnector_bit(innerConnector, vp8->inherit_super.height, 16);

				ttLibC_ByteConnector_ebml2(connector, MkvType_Video, true);
				ttLibC_ByteConnector_ebml2(connector, innerConnector->write_size, false);
				ttLibC_ByteConnector_string(connector, (const char *)inner, innerConnector->write_size);
				ttLibC_DynamicBuffer_append(trackEntryBuffer, buf, connector->write_size);
//				ttLibC_DynamicBuffer_append(trackEntryBuffer, inner, in_size);
			}
			break;
		case frameType_opus:
			{
				// codecID
				ttLibC_ByteConnector_ebml2(connector, MkvType_CodecID, true);
				ttLibC_ByteConnector_ebml2(connector, 6, false);
				ttLibC_ByteConnector_string(connector, "A_OPUS", 6);
				// trackType
				ttLibC_ByteConnector_ebml2(connector, MkvType_TrackType, true);
				ttLibC_ByteConnector_ebml2(connector, 1, false);
				ttLibC_ByteConnector_bit(connector, 2, 8);
				// audioの子要素をつくっていく。
				ttLibC_Opus *opus = (ttLibC_Opus *)ttLibC_FrameQueue_ref_first(track->frame_queue);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_SamplingFrequency, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 4, false);
				float sr = opus->inherit_super.sample_rate;
				ttLibC_ByteConnector_bit(innerConnector, *(uint32_t *)&sr, 32);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_Channels, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 1, false);
				ttLibC_ByteConnector_bit(innerConnector, opus->inherit_super.channel_num, 8);

				ttLibC_ByteConnector_ebml2(connector, MkvType_Audio, true);
				ttLibC_ByteConnector_ebml2(connector, innerConnector->write_size, false);
				ttLibC_ByteConnector_string(connector, (const char *)inner, innerConnector->write_size);
				// やっぱここcodecPrivateいれるべき
				in_size = ttLibC_HexUtil_makeBuffer("4F 70 75 73 48 65 61 64 01 02 00 00 BB 80 00 00 00 00 00", inner, 256);
				ttLibC_ByteConnector_ebml2(connector, MkvType_CodecPrivate, true);
				ttLibC_ByteConnector_ebml2(connector, in_size, false);
				// codecPrivateいるのかな？とりあえず放置しとく。
				ttLibC_DynamicBuffer_append(trackEntryBuffer, buf, connector->write_size);
				ttLibC_DynamicBuffer_append(trackEntryBuffer, inner, in_size);
			}
			break;
		default:
			ERR_PRINT("ここにくることはありえません。");
			return true;
		}
		ttLibC_ByteConnector_close(&innerConnector);
		ttLibC_ByteConnector_close(&connector); // 追記済みなので、これもいらない。
		// trackEntryが作成完了したので、trackのbufferに追加しなければならない。
		connector = ttLibC_ByteConnector_make(buf, 256, ByteUtilType_default);
		ttLibC_ByteConnector_ebml2(connector, MkvType_TrackEntry, true);
		ttLibC_ByteConnector_ebml2(connector, ttLibC_DynamicBuffer_refSize(trackEntryBuffer), false);
		ttLibC_DynamicBuffer_append(buffer, buf, connector->write_size);
		ttLibC_DynamicBuffer_append(buffer,
				ttLibC_DynamicBuffer_refData(trackEntryBuffer),
				ttLibC_DynamicBuffer_refSize(trackEntryBuffer));
		ttLibC_ByteConnector_close(&connector);
		// 追加してみる。
		// trackNumber
		// trackUID(trackNumberと同じにしておく。)
		// ここまでは共通。
		// 以下はframeTypeで別れる。
		// CodecID
		// TrackType
		// Video子要素 or Audio子要素
		// CodecPrivate
		// ここまで書き出せばOKだと思われ。
		// ここでconnectしてデータをつくらなければならない。
		ttLibC_DynamicBuffer_close(&trackEntryBuffer);
		// 解析おわり。
		// h264の場合
		//  trackNumber
		//  trackUID
		//  frameLacing 00:未設定 (なくてもいいと思う)
		//  Language undでOK なくてよさげ
		//  CodecID:V_MPEG4/ISO/AVC
		//  TrackType 01:video
		//  DefaultDuration:よくわからん。なくてもいいんじゃない？33000000になってる。
		//  videoの子要素
		//   PixelWidth
		//   PixelHeight
		//   DisplayWidth
		//   DisplayHeight
		//  CodecPrivate:(h264のavcCがはいってる。)
		// aacの場合
		//  trackNumber
		//  trackUID
		//  frameLacing 00:未設定 (なくてもいいと思う)
		//  Language undでOK
		//  CodecID:A_AAC
		//  TrackType 02:Audioたと思われ
		//  Audioの子要素
		//   Channel
		//   SampleFrequency:
		//   bitDepth 16bitになってる。
		//  CodecPrivate:(aacのdsi情報がはいってる。)
		// vp8の場合
		//  TrackNumber:01
		//  TrackUID:ちゃんとはいってる。なにかは不明
		//  trackType:01 video
		//  defaultDurationがはいってる。なにこれ？33333333になってる。
		//  CodecType:V_VP8
		//  Video子要素
		//   PixelWidth
		//   PixelHeight
		//   Videoの子要素としてframeRateがはいってた。ほぅ・・・
		// opusの場合
		//  TrackNumber:02
		//  TrackUID:いろいろ
		//  TrackType:02 audio
		//  CodecType:A_OPUS
		//  CodecPrivate:Opusのhead情報がはいってる。
		//   これだけちょっと気になるOpusHead 01 [チャンネル数1byte] 00 00 [周波数 2byte] 00 00 00 00 00
		//   でOKっぽい。なるへそ
		//  Audioの子要素
		//   Hz:48000(ただしfloatではいってた。)
		//   channel:01モノラルなるほどね。
	}
	return true;
}

static bool MkvWriter_makeInitMkv(ttLibC_MkvWriter_ *writer) {
	// 初期データのmkvを作成します。
	ttLibC_DynamicBuffer *buffer = ttLibC_DynamicBuffer_make();
//	writer->inherit_super.is_webm;
	uint8_t buf[256];
	size_t in_size;
	// mkvの初期データを作らなければならない・・・
	// EBML
	if(writer->inherit_super.is_webm) {
		in_size = ttLibC_HexUtil_makeBuffer("1A 45 DF A3 9F 42 86 81 01 42 F7 81 01 42 F2 81 04 42 F3 81 08 42 82 84 77 65 62 6D 42 87 81 04 42 85 81 02", buf, 256);
	}
	else {
		in_size = ttLibC_HexUtil_makeBuffer("1A 45 DF A3 A3 42 86 81 01 42 F7 81 01 42 F2 81 04 42 F3 81 08 42 82 88 6D 61 74 72 6F 73 6B 61 42 87 81 04 42 85 81 02", buf, 256);
	}
	ttLibC_DynamicBuffer_append(buffer, buf, in_size);
	// segmentの書き込み、長さは無限大にして、liveに対応できるようにしておく。
	// シークはいらね。
	in_size = ttLibC_HexUtil_makeBuffer("18 53 80 67 01 FF FF FF FF FF FF FF", buf, 256);
	ttLibC_DynamicBuffer_append(buffer, buf, in_size);
	// chromeの場合はinfoが先にきたか・・・
	// seek headいるのかな？(chromeが出力するwebmにはないっぽいけど・・・)mandatoryではないな・・・まじですか・・・
	// じゃなしで

	// infoの書き出し。muxerを書き込むくらいか？
	// timecode scale 1000000は固定っぽいな。2AD7B183 0f4240は固定っぽい。
	// WritingAppとMuxingAppの２つがあれば十分なのかな？
	// segmentUIDとdurationはchromeにはなかったな・・・、放置しとくか、とりあえずdurationは計算することできないし(どうなるかわからん)
	in_size = ttLibC_HexUtil_makeBuffer("15 49 A9 66 99 2A D7 B1 83 0F 42 40 4D 80 86 74 74 4C 69 62 43 57 41 86 74 74 4C 69 62 43", buf, 256);
	ttLibC_DynamicBuffer_append(buffer, buf, in_size);

	// 次、トラック情報
	// トラックサイズ用のbufferをつくって、あとで結合するようにしよう。
	ttLibC_DynamicBuffer *trackBuffer = ttLibC_DynamicBuffer_make();
	// tracks 全体の大きさ(すべてのトラックの書き出しが完了するまでデータがわからないわけか・・・)
	//  trackEntry
	//   ここのやつは、vp8、h264、aac、opusでそれぞれ別のものになると思われ。
	//   とりあえず解析していこう。
	//   実際はtrackのforEachで処置関数に飛ばして処理させるというのが、正しいやり方になるだろうね。
	ttLibC_StlMap_forEach(writer->track_list, MkvWriter_makeTrackEntry, trackBuffer);
	// ここまでおわったら、一度データをdumpしてみたい。
	ttLibC_ByteConnector *connector = ttLibC_ByteConnector_make(buf, 256, ByteUtilType_default);
	ttLibC_ByteConnector_ebml2(connector, MkvType_Tracks, true);
	ttLibC_ByteConnector_ebml2(connector, ttLibC_DynamicBuffer_refSize(trackBuffer), false);
	ttLibC_DynamicBuffer_append(buffer, buf, connector->write_size);
	ttLibC_DynamicBuffer_append(
			buffer,
			ttLibC_DynamicBuffer_refData(trackBuffer),
			ttLibC_DynamicBuffer_refSize(trackBuffer));
	ttLibC_ByteConnector_close(&connector);
	ttLibC_DynamicBuffer_close(&trackBuffer);
	bool result = true;
	if(writer->callback != NULL) {
		result = writer->callback(writer->ptr, ttLibC_DynamicBuffer_refData(buffer), ttLibC_DynamicBuffer_refSize(buffer));
	}
	// このデータが書き出しすべきデータと思われる。なので、書き出す。
	ttLibC_DynamicBuffer_close(&buffer);
	// Tagはなし。
	// そのままclusterになってるっぽい。
	// あとはclusterが並んでいるだけ。
	return result;
}

static bool MkvWirter_PrimaryVideoTrackCheck(void *ptr, ttLibC_Frame *frame) {
	ttLibC_MkvWriter_ *writer = (ttLibC_MkvWriter_ *)ptr;
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

static bool MkvWriter_dataCheckTrack(void *ptr, void *key, void *item) {
	if(ptr != NULL && item != NULL) {
		ttLibC_MkvWriter_ *writer = (ttLibC_MkvWriter_ *)ptr;
		ttLibC_MkvWriteTrack *track = (ttLibC_MkvWriteTrack *)item;

		uint64_t pts = (uint64_t)(1.0 * track->frame_queue->pts * 1000 / track->frame_queue->timebase);
		if(writer->target_pos > pts) {
			return false;
		}
		return true;
	}
	return false;
}

static bool MkvWriter_makeData(
		ttLibC_MkvWriter_ *writer) {
	// 書き出しを実施する
//	LOG_PRINT("書き出しする。:%d", writer->current_pts_pos);
	// clusterを書き出す。(中身完了してからやらないとだめ)
	// bufferをつくる。ここにデータを書き出す。
	ttLibC_DynamicBuffer *buffer = ttLibC_DynamicBuffer_make();
	uint8_t buf[256]; // 一時データ処理用buffer
	ttLibC_ByteConnector *connector = ttLibC_ByteConnector_make(buf, 256, ByteUtilType_default);
	// timecodeを書き出します。
	ttLibC_ByteConnector_ebml2(connector, MkvType_Timecode, true);
	// とりあえず8byteにしておこうと思う。
	ttLibC_ByteConnector_ebml2(connector, 8, false);
	ttLibC_ByteConnector_bit(connector, (writer->current_pts_pos >> 32), 32); // 64bitで書き出ししておく。
	ttLibC_ByteConnector_bit(connector, (writer->current_pts_pos), 32); // 64bitで書き出ししておく。
	ttLibC_DynamicBuffer_append(buffer, buf, connector->write_size);
	// このデータがtimecodeとして書き込むべきデータ
	ttLibC_ByteConnector_close(&connector);
	// simpleBlockを書き出していく。
	// trackからデータを取り出して、一番若いフレームを書き出すという処理をかかないとだめ。
	// トラックの最大数を知る必要がある。
	uint64_t target_track;
	ttLibC_Frame *frame;
	uint64_t pts;
	bool is_found;
	while(true) {
		target_track = 0;
		frame = NULL;
		pts = 0;
		is_found = false;
		// 全トラックから、一番ptsの低いものを見つけなければならない。
		for(uint32_t i = 0;i < writer->track_list->size;++ i) {
			ttLibC_MkvWriteTrack *track = (ttLibC_MkvWriteTrack *)ttLibC_StlMap_get(writer->track_list, (void *)(1 + i));
			frame = ttLibC_FrameQueue_ref_first(track->frame_queue);
			if(frame != NULL) {
				if(!is_found) {
					// まだ見つかってなければなんであれ、登録する。
					target_track = i + 1;
					pts = (uint64_t)(1.0 * frame->pts * 1000 / frame->timebase);
					is_found = true;
				}
				else {
					if(pts > (uint64_t)(1.0 * frame->pts * 1000 / frame->timebase)) {
						target_track = i + 1;
						pts = (uint64_t)(1.0 * frame->pts * 1000 / frame->timebase);
					}
				}
			}
		}
		if(pts > writer->target_pos) {
			// すでに必要データ以上になった場合は、処理やめる。
			break;
		}
		if(target_track == 0) {
			LOG_PRINT("処理すべきトラックがみつからなかった。");
			break;
		}
//		LOG_PRINT("処理すべきデータみつけた。:%d %d", target_track, pts);
		ttLibC_MkvWriteTrack *track = (ttLibC_MkvWriteTrack *)ttLibC_StlMap_get(writer->track_list, (void *)target_track);
		frame = ttLibC_FrameQueue_dequeue_first(track->frame_queue);
		switch(frame->type) {
		case frameType_aac:
			{
				ttLibC_Aac *aac = (ttLibC_Aac *)frame;
				if(aac->type == AacType_dsi) {
					// dsi情報は処理にまわさない。
					continue; // これできちんとwhileループのcontinueになってる模様。
				}
			}
			break;
		case frameType_h264:
			{
				ttLibC_H264 *h264 = (ttLibC_H264 *)frame;
				switch(h264->type) {
				case H264Type_unknown:
				case H264Type_configData:
					continue;
				default:
					break;
				}
			}
			break;
		default:
			break;
		}
		// このframeの内容が書き出すべきもの。
		// あとはこのframeのデータを書き出すように調整しなければならないわけか・・・
		// さて・・・
		// このデータでsimpleBlockを書き出していく。
		connector = ttLibC_ByteConnector_make(buf, 255, ByteUtilType_default);
		switch(frame->type) {
		case frameType_h264:
			{
				ttLibC_H264 *h264 = (ttLibC_H264 *)frame;
				// まずデータサイズがわからないとどうしようもない。
				// sizeNalに改良する。
				uint8_t *h264_data = frame->data;
				size_t h264_data_size = frame->buffer_size;
				ttLibC_H264_NalInfo nal_info;
				uint32_t h264_size = 0;
				// ちょっと無駄がおおいけど、２周してごまかすか・・・
				while(ttLibC_H264_getNalInfo(&nal_info, h264_data, h264_data_size)) {
					uint32_t nal_size = nal_info.nal_size - nal_info.data_pos;
					h264_size += 4 + nal_size;
					h264_data += nal_info.nal_size;
					h264_data_size -= nal_info.nal_size;
				}
				h264_size += 4;
				// サイズはわかった。あとはこのサイズ + 4byteにするだけ。
				ttLibC_ByteConnector_ebml2(connector, MkvType_SimpleBlock, true);
				ttLibC_ByteConnector_ebml2(connector, h264_size, false);
				// トラックIDがあまり増えないことを期待しておくことにする。
				ttLibC_ByteConnector_ebml2(connector, frame->id, false);
				// 時間情報
				ttLibC_ByteConnector_bit(connector, pts - writer->current_pts_pos, 16);
				// とりあえずデータをつくるわけだが・・・
				switch(h264->type) {
				case H264Type_slice:
					ttLibC_ByteConnector_bit(connector, 0x00, 8);
					break;
				case H264Type_sliceIDR:
					ttLibC_ByteConnector_bit(connector, 0x80, 8);
					break;
				default:
					LOG_PRINT("想定外");
					break;
				}
				// ここまでであとはsizeNalを書き出していけばOKのはず。
				ttLibC_DynamicBuffer_append(buffer, buf, connector->write_size);
				// あとはh264のデータを書き出していく。
				h264_data = frame->data;
				h264_data_size = frame->buffer_size;
				// ちょっと無駄がおおいけど、２周してごまかすか・・・
				while(ttLibC_H264_getNalInfo(&nal_info, h264_data, h264_data_size)) {
					uint32_t nal_size = nal_info.nal_size - nal_info.data_pos;
					h264_size += 4 + nal_size;
					uint32_t be_nal_size = be_uint32_t(nal_size);
					ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_nal_size, 4);
					ttLibC_DynamicBuffer_append(buffer, h264_data + nal_info.data_pos, nal_size);
					h264_data += nal_info.nal_size;
					h264_data_size -= nal_info.nal_size;
				}
				// ここまで終わればきっと大丈夫なデータになってるはず。

				// A3 4304 81 00 00 80 00 00 02 FC 65 ...
				// simpleBlock
				//    データサイズ
				//         トラックID(ebml)
				//            timestamp
				//                  データフラグ
				//                     以降データ実体
				// よって実体をつくっていかなければならない。
			}
			break;
		case frameType_aac:
			{
				ttLibC_Aac *aac = (ttLibC_Aac *)frame;
				// データサイズを知る必要がある。
				uint8_t *aac_data = aac->inherit_super.inherit_super.data;
				uint32_t aac_size = aac->inherit_super.inherit_super.buffer_size;
				if(aac->type == AacType_adts) {
					// dsiが1くることはないと思うが・・・
					aac_data += 7;
					aac_size -= 7;
				}
				aac_size += 4;
				// サイズはわかった。あとはこのサイズ + 4byteにするだけ。
				ttLibC_ByteConnector_ebml2(connector, MkvType_SimpleBlock, true);
				ttLibC_ByteConnector_ebml2(connector, aac_size, false);
				// トラックIDがあまり増えないことを期待しておくことにする。
				ttLibC_ByteConnector_ebml2(connector, frame->id, false);
				// 時間情報
				ttLibC_ByteConnector_bit(connector, pts - writer->current_pts_pos, 16);
				ttLibC_ByteConnector_bit(connector, 0x80, 8);
				// あとはデータをかけばOK
				ttLibC_DynamicBuffer_append(buffer, buf, connector->write_size);

				aac_size -= 4;
				ttLibC_DynamicBuffer_append(buffer, aac_data, aac_size);
				// A3 8D 82 00 00 80 21...
				// simpleBlock
				//    データサイズ
				//       track
				//          timestamp
				//                データフラグ(keyFrameの映像と同じ扱いっぽい。)
				//                   あとはデータ実体
			}
			break;
		case frameType_vp8:
			{
				ttLibC_Vp8 *vp8 = (ttLibC_Vp8 *)frame;
				uint8_t *vp8_data = frame->data;
				size_t vp8_data_size = frame->buffer_size;
				ttLibC_ByteConnector_ebml2(connector, MkvType_SimpleBlock, true);
				ttLibC_ByteConnector_ebml2(connector, vp8_data_size + 4, false);
				ttLibC_ByteConnector_ebml2(connector, frame->id, false);
				ttLibC_ByteConnector_bit(connector, pts - writer->current_pts_pos, 16);
				switch(vp8->inherit_super.type) {
				case videoType_inner:
					ttLibC_ByteConnector_bit(connector, 0x00, 8);
					break;
				case videoType_key:
					ttLibC_ByteConnector_bit(connector, 0x80, 8);
					break;
				default:
					LOG_PRINT("想定外");
					break;
				}
				ttLibC_DynamicBuffer_append(buffer, buf, connector->write_size);
				ttLibC_DynamicBuffer_append(buffer, vp8_data, vp8_data_size);
			}
			break;
		case frameType_opus:
			{
				ttLibC_Opus *opus = (ttLibC_Opus *)frame;
				uint8_t *opus_data = frame->data;
				size_t opus_data_size = frame->buffer_size;
				ttLibC_ByteConnector_ebml2(connector, MkvType_SimpleBlock, true);
				ttLibC_ByteConnector_ebml2(connector, opus_data_size + 4, false);
				ttLibC_ByteConnector_ebml2(connector, frame->id, false);
				ttLibC_ByteConnector_bit(connector, pts - writer->current_pts_pos, 16);
				ttLibC_ByteConnector_bit(connector, 0x80, 8);
				ttLibC_DynamicBuffer_append(buffer, buf, connector->write_size);
				ttLibC_DynamicBuffer_append(buffer, opus_data, opus_data_size);
			}
			break;
		default:
			break;
		}
		ttLibC_ByteConnector_close(&connector);
	}
	ttLibC_DynamicBuffer *clusterBuffer = ttLibC_DynamicBuffer_make();
	// ここにデータがきちんとはいった。
	// clusterタグとかかいていく。
	connector = ttLibC_ByteConnector_make(buf, 256, ByteUtilType_default);
	ttLibC_ByteConnector_ebml2(connector, MkvType_Cluster, true);
	ttLibC_ByteConnector_ebml2(connector, ttLibC_DynamicBuffer_refSize(buffer), false);
	ttLibC_DynamicBuffer_append(clusterBuffer, buf, connector->write_size);
	ttLibC_ByteConnector_close(&connector);
	ttLibC_DynamicBuffer_append(
			clusterBuffer,
			ttLibC_DynamicBuffer_refData(buffer),
			ttLibC_DynamicBuffer_refSize(buffer));
	// とりあえずデータは大丈夫っぽい。
	ttLibC_DynamicBuffer_close(&buffer);
	// ここでcallbackを返せばいいと思う。
	bool result = true;
	if(writer->callback != NULL) {
		result = writer->callback(writer->ptr, ttLibC_DynamicBuffer_refData(clusterBuffer), ttLibC_DynamicBuffer_refSize(clusterBuffer));
	}
	ttLibC_DynamicBuffer_close(&clusterBuffer);
	return result;
}

static bool MkvWriter_writeFromQueue(
		ttLibC_MkvWriter_ *writer) {
	switch(writer->status) {
	case status_init_check: // 初期データ作成可能か確認
		{
			if(ttLibC_StlMap_forEach(writer->track_list, MkvWriter_initCheckTrack, NULL)) {
				writer->status = status_make_init;
				return MkvWriter_writeFromQueue(writer);
			}
		}
		break;
	case status_make_init: // 初期データ作成
		// 初期データ作成動作を実際に発動させる。
		if(MkvWriter_makeInitMkv(writer)) {
			writer->status = status_target_check;
			return MkvWriter_writeFromQueue(writer);
		}
		else {
			// TODO update
			ERR_PRINT("something fatal is happen. update later");
			return false;
		}
		break;
	case status_target_check: // cluster書き込み先がどの程度になるか初めのトラックから判断
		{
			ttLibC_MkvWriteTrack *track = (ttLibC_MkvWriteTrack *)ttLibC_StlMap_get(writer->track_list, (void *)1);
			switch(track->frame_type) {
			case frameType_h264:
			case frameType_vp8:
				ttLibC_FrameQueue_ref(track->frame_queue, MkvWirter_PrimaryVideoTrackCheck, writer);
				break;
			case frameType_aac:
			case frameType_opus:
				writer->target_pos = writer->current_pts_pos + writer->max_unit_duration;
				break;
			default:
				ERR_PRINT("unexpected frame is found.");
				return false;
			}
			// 位置情報が更新されたら次い進む。
			if(writer->target_pos != writer->current_pts_pos) {
				writer->status = status_data_check;
				return MkvWriter_writeFromQueue(writer);
			}
		}
		break;
	case status_data_check: // cluster分書き込むのに必要なデータがあるか確認
		{
			if(ttLibC_StlMap_forEach(writer->track_list, MkvWriter_dataCheckTrack, writer)) {
				writer->status = status_make_data;
				return MkvWriter_writeFromQueue(writer);
			}
		}
		break;
	case status_make_data: // clusterの書き込みを実施
		{
			if(MkvWriter_makeData(writer)) {
				// write done.
				writer->status = status_update;
				return MkvWriter_writeFromQueue(writer);
			}
		}
		break;
	case status_update: // 次の処理へ移動する。(status_target_checkにいく。)
		{
			writer->current_pts_pos = writer->target_pos;
			writer->status = status_target_check;
			// 無限ループが怖いのでここで止めとく。
			// 呼び出しループが大きすぎるとstackエラーになる環境がありえるので、こうしとく。
		}
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
	case frameType_vp8:
		{
			pts = (uint64_t)(1.0 * frame->pts * 1000 / frame->timebase);
			frame->pts = pts;
			frame->timebase = 1000;
		}
		break;
	case frameType_aac:
	case frameType_opus:
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
	ttLibC_MkvWriteTrack *track = (ttLibC_MkvWriteTrack *)ttLibC_StlMap_get(writer_->track_list, (void *)frame->id);
	if(track == NULL) {
		ERR_PRINT("failed to get correspond track. %d", frame->id);
		return false;
	}
	// trackにframeを追加する。
//	LOG_PRINT("trackにframeを追加しなければ・・・");
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
			if(frame->type == frameType_vp8) {
				if(writer_->is_first) {
					writer_->current_pts_pos = pts;
					writer_->target_pos = pts;
					writer_->inherit_super.inherit_super.pts = pts;
					writer_->is_first = false;
				}
			}
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
		ttLibC_MkvWriteTrack *track = (ttLibC_MkvWriteTrack *)item;
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

