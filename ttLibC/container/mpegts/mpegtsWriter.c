/*
 * @file   mpegtsWriter.c
 * @brief  
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
#include "../../util/hexUtil.h"

#include <stdlib.h>

ttLibC_MpegtsWriter *ttLibC_MpegtsWriter_make(
		ttLibC_Frame_Type *target_frame_types,
		uint32_t types_num) {
	return ttLibC_MpegtsWriter_make_ex(target_frame_types, types_num, 90000);
}

/*
 * とりあえずmakeの時点で必要となるsdt pat pmtは作成されるものとする。
 * sdtの内容はttProjectの名前がはいっている適当な名前にしておく。
 * patは普通につくる。
 * pmtは0x1000でつくる。
 * トラックはframeTypeで指定されているものにする。pesIDを指定して設定とかできるとなおよいか？
 *
 * sdtの内容は変更させるとちょっと面倒なので(ttLibCでつくったと宣言させる方がこっちとしては都合がよい。)
 * なので、わざとmake_exは作らない。
 */
/**
 * make mpegtsWriter object.
 * @param target_frame_types コンテナに保持させるフレーム指定
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
	for(int i = 0;i < MaxPesTracks; ++ i) {
		writer->track[i].cc = 0;
		if(i >= types_num) {
			writer->inherit_super.trackInfo[i].pid = 0;
			writer->inherit_super.trackInfo[i].frame_type = frameType_unknown;
			writer->track[i].h264_configData = NULL;
			writer->track[i].frame_queue = NULL;
			continue;
		}
		writer->track[i].h264_configData = NULL;
		writer->track[i].frame_queue = ttLibC_FrameQueue_make(pid, 255);
		switch(target_frame_types[i]) {
		case frameType_aac:
			writer->inherit_super.trackInfo[i].pid = pid;
			writer->inherit_super.trackInfo[i].frame_type = frameType_aac;
			pid ++;
			break;
		case frameType_mp3:
			writer->inherit_super.trackInfo[i].pid = pid;
			writer->inherit_super.trackInfo[i].frame_type = frameType_mp3;
			pid ++;
			break;
		case frameType_h264:
			writer->inherit_super.trackInfo[i].pid = pid;
			writer->inherit_super.trackInfo[i].frame_type = frameType_h264;
			pid ++;
			break;
		default:
			writer->inherit_super.trackInfo[i].pid = 0;
			writer->inherit_super.trackInfo[i].frame_type = frameType_unknown;
			break;
		}
	}
	writer->inherit_super.inherit_super.timebase = 90000;
	writer->is_first = true;
	writer->cc_pat = 0;
	writer->cc_pmt = 0;
	writer->cc_sdt = 0;
	writer->max_unit_duration = max_unit_duration;
	writer->inherit_super.inherit_super.type = containerType_mpegts;
	// このタイミングでsdtやpat、pmtをつくってしまう。
	// とりあえずsdtをつくる。
	ttLibC_Sdt_makePacket((const char *)"ttLibC", (const char *)"mpegtsMuxer", writer->sdt_buf, 188);
	ttLibC_Pat_makePacket(writer->pat_buf, 188);
	ttLibC_Pmt_makePacket((ttLibC_MpegtsWriter *)writer, writer->pmt_buf, 188);
	// これで3つのデータはできた。
	return (ttLibC_MpegtsWriter *)writer;
}

// TODO rename from h264_test to sth.
typedef struct {
	ttLibC_MpegtsWriter_ *writer;
	ttLibC_MpegtsTrack *track;
	ttLibC_ContainerWriterFunc callback;
	void *ptr;
	bool error_flg;
} test_t;

bool MpegtsWriter_h264_test(void *ptr, ttLibC_Frame *frame) {
	test_t *testData = (test_t *)ptr;
	if(frame->type != frameType_h264) {
		testData->error_flg = true;
		return false;
	}
	ttLibC_H264 *h264 = (ttLibC_H264 *)frame;
	switch(h264->type) {
	case H264Type_slice:
		// sliceはそのまま足せばOK
		if((uint32_t)(h264->inherit_super.inherit_super.pts - testData->writer->current_pts_pos) > testData->writer->max_unit_duration) {
			// ptsがmax_durationより進んだので、次のフェーズに進める。
			testData->writer->target_pos = h264->inherit_super.inherit_super.pts;
			return false;
		}
		break;
	case H264Type_sliceIDR:
		if(testData->writer->current_pts_pos != h264->inherit_super.inherit_super.pts) {
			// ptsが現在値と違う部分に当たった場合はmpegtsの次のchunkに進むべき
			testData->writer->target_pos = h264->inherit_super.inherit_super.pts;
			return false;
		}
		break;
	default:
		LOG_PRINT("unexpected data.");
		testData->error_flg = true;
		return false;
	}
	// 問題ないデータなので、frameを書き出す。
	if(!ttLibC_Pes_writeH264Packet(testData->track, frame, testData->callback, testData->ptr)) {
		testData->error_flg = true;
		return false;
	}
	return true;
}

// この処理はマルチトラックを考慮していません。
static bool MpegtsWriter_writeFromQueue(
		ttLibC_MpegtsWriter_ *writer,
		ttLibC_ContainerWriterFunc callback,
		void *ptr) {
	// h264はsliceIDRが見つかるまで取得後に即書き出す。
	// とりあえずqueueと取り出す
	ttLibC_MpegtsTrack *h264_track = NULL;
	ttLibC_MpegtsTrack *audio_track = NULL;
	for(int i = 0;i < MaxPesTracks; ++ i) {
		switch(writer->inherit_super.trackInfo[i].frame_type) {
 		case frameType_aac:
		case frameType_mp3:
			if(audio_track == NULL) {
				audio_track = &writer->track[i];
			}
			break;
		case frameType_h264:
			if(h264_track == NULL) {
				h264_track = &writer->track[i];
			}
			break;
		default:
			continue;
		}
	}
	writer->inherit_super.inherit_super.pts = writer->current_pts_pos;
	// aacはh264が書き出したサイズ分後追いで書き出す。
	// queueデータが取得できたので、そのqueueについて書き出しを実施します。
	// とりあえず面倒なので、両方あるものとしたプログラムにします。
	if(writer->current_pts_pos == writer->target_pos) {
		test_t testData;
		testData.writer    = writer;
		testData.callback  = callback;
		testData.ptr       = ptr;
		testData.error_flg = false;
		testData.track     = h264_track;
		// cppとtpが一致する場合は、映像を追加するフェーズ
		// sliceIDRデータに突き当たるまでデータを取り出す。
		// queueに登録されているものを全部確認しなければならない。
		ttLibC_FrameQueue_dequeue(h264_track->frame_queue, MpegtsWriter_h264_test, &testData);
		if(testData.error_flg) {
			return false;
		}
	}
	else {
		// cppとtpが一致しない場合は、音声を追加するフェーズとします。
		if(audio_track->frame_queue->pts > writer->target_pos) {
			// 必要なデータがたまっているので、処理可能。
			// 特定のptsまでのデータを取り出して書き出すという処理が必要。
			bool result = ttLibC_Pes_writeAudioPacket(writer, audio_track, callback, ptr);
			return result;
		}
	}
	return true;
}

/**
 * mpegtsデータを書き出します。
 * @param writer           mpegtsWriter object
 * @param update_info_flag true:write info data(sdt pat pmt)
 * @param pid              pid for target frame.
 * @param frame,           target frame.
 * @param callback         callback for write process.
 * @param ptr              user def data pointer for callback.
 */
bool ttLibC_MpegtsWriter_write(
		ttLibC_MpegtsWriter *writer,
		bool update_info_flag,
		uint16_t pid,
		ttLibC_Frame *frame,
		ttLibC_ContainerWriterFunc callback,
		void *ptr) {
	ttLibC_MpegtsWriter_ *writer_ = (ttLibC_MpegtsWriter_ *)writer;
	if(writer_->is_first || update_info_flag) {
		// sdt
		writer_->sdt_buf[3] = (writer_->sdt_buf[3] & 0xF0) | writer_->cc_sdt;
		writer_->cc_sdt = (writer_->cc_sdt + 1) & 0x0F;
		callback(ptr, writer_->sdt_buf, 188);
		// pat
		writer_->pat_buf[3] = (writer_->pat_buf[3] & 0xF0) | writer_->cc_pat;
		writer_->cc_pat = (writer_->cc_pat + 1) & 0x0F;
		callback(ptr, writer_->pat_buf, 188);
		// pmtを応答して、その後巡回カウンターを変更しておく。
		// pat
		writer_->pmt_buf[3] = (writer_->pmt_buf[3] & 0xF0) | writer_->cc_pmt;
		writer_->cc_pmt = (writer_->cc_pmt + 1) & 0x0F;
		callback(ptr, writer_->pmt_buf, 188);
	}
	if(frame == NULL) {
		writer_->is_first = false;
		// フレームがない場合は、ここで処理おわり。
		return true;
	}
	uint64_t pts = (uint64_t)(1.0 * frame->pts * 90000 / frame->timebase);
	// 強制的にtimebaseを90000に書き換える。
	frame->pts = pts;
	frame->timebase = 90000;
	if(writer_->is_first) {
		writer_->current_pts_pos = pts; // はじめの処理位置を最初に取得したpts値に変更しておく。
		writer_->target_pos = pts; // 一致させておく。
		writer_->inherit_super.inherit_super.pts = pts;
	}
	writer_->is_first = false;
	// pidに対応するpqueueを見つけないとだめ。
	ttLibC_MpegtsTrack *track = NULL;
	for(int i = 0;i < MaxPesTracks;++ i) {
		if(writer_->track[i].frame_queue != NULL && writer_->track[i].frame_queue->track_id == pid) {
			track = &writer_->track[i];
		}
	}
	// pidを確認しないとだめですが、とりあえずやらずにほっときます。
	if(track == NULL) {
		ERR_PRINT("cannot get track information. invalid pid?:%d", pid);
		return false;
	}
	// 想定では次のようにする。
	// めんどくさいな・・・
	// h264のkeyFrame間ごとにきりわけようか・・・その方が楽かも・・・
	// h264のないデータ(音声のみのhls)でこまったことになるので、いれておく。
	// なお、flvやmp4ではこの情報は必要ない。(音声も１つのデータとして登録する必要があるため。)
	// max_unit_duration(以下mud)のごとに処理をする。例として90000にする。(timebaseが90000なので1秒)
	// 各処理はcurrent_pts_pos(以下cpp)によって管理されます。
	// ここからmud分が單位データとなります。
	// データは映像データmud分保持後、音声のデータをmud分保持するという形にします。
	// 映像データはそれぞれのフレームごとにpesを作成します。
	// 音声データはmud分を１つのpesとします。
	// はじまりは0からはじまります。cpp = 0
	// cpp = 0 〜 cpp + mud = 90000までの映像データは取得したら、即mpegtsとして出力します。
	// 音声データがきた場合は、pqueueに保持しておきます。

	// cpp + mud以降の映像データがきた場合は、queueにcacheしておきます。

	// 音声データに必要量があるのに、映像データが進んでいない場合は、処理待ちとなります。
	// なので、すでに書き込み済みの映像データのptsも保持する必要あります。

	// 保持しておくフレームデータとしては、次のようにします。
	// aacやmp3の場合は、普通にframeをためておき、書き込み実施寺に、結合しておきます。
	// rawデータのaacの場合はdecoderSpecificInfoからadts化する必要があります。

	// h264のデータについては、configDataは先頭にコピーしておきます。

	// とりあえず、h264とaacがくるものとします。
	// aacはadtsでくるものとします。

	// では以下処理
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
				// それ以外のデータ
				if(!ttLibC_FrameQueue_queue(track->frame_queue, frame)) {
					return false;
				}
			}
		}
		break;
	case frameType_aac:
	case frameType_mp3:
		{
			// そのままcacheしておけばそれでよい。
			if(!ttLibC_FrameQueue_queue(track->frame_queue, frame)) {
				return false;
			}
		}
		break;
	default:
		ERR_PRINT("unexpected frame:%d", frame->type);
		return false;
 	}
	return MpegtsWriter_writeFromQueue(writer_, callback, ptr);
}

/**
 * mpegtswriterを閉じます。
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
	for(int i = 0;i < MaxPesTracks;++ i) {
		ttLibC_FrameQueue_close(&target->track[i].frame_queue);
		ttLibC_Frame_close(&target->track[i].h264_configData);
	}
	free(target);
	*writer = NULL;
}
