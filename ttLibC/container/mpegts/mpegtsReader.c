/*
 * @file   mpegtsReader.c
 * @brief  
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/13
 */


#include "../mpegts.h"

#include "mpegtsReader.h"

#include "../../log.h"
#include "../../util/hexUtil.h"
#include "../../util/ioUtil.h"
#include "../../util/bitUtil.h"

#include <stdlib.h>
#include <string.h>

ttLibC_MpegtsReader *ttLibC_MpegtsReader_make() {
	ttLibC_MpegtsReader_ *reader = (ttLibC_MpegtsReader_ *)ttLibC_ContainerReader_make(containerType_mpegts, sizeof(ttLibC_MpegtsReader_));

	// なお、sizeはpat、pmt、sdtはsizeのデータを読み込んだら取得すべきデータ量が確定しているので、そのまま解析
	// -> 次もsizeとなる。
	// pesはsize取得したら、取得すべきデータ量が確定するので、次がbodyになる。
	// よって、body時はpesの読み込みのみありえる。
	// size時はsize、pat pme sdtの読み込み時となる。
	reader->pat = NULL;
	reader->pmt = NULL;
	reader->sdt = NULL;
	for(int i = 0;i < MaxPesTracks;++ i) {
		reader->pes[i] = NULL;
	}
	reader->target_size = 188;

	reader->tmp_buffer_size = 188;
	reader->tmp_buffer = malloc(reader->tmp_buffer_size);
	reader->tmp_buffer_next_pos = 0;
	return (ttLibC_MpegtsReader *)reader;
}

static bool MpegtsReader_read(
		ttLibC_MpegtsReader_ *reader,
		uint8_t *buffer,
		size_t left_size,
		ttLibC_MpegtsReaderFunc callback,
		void *ptr) {
//	ttLibC_HexUtil_dump(buffer, reader->target_size, true);
	bool result = true;
	if(buffer[0] != 0x47) {
		ERR_PRINT("malformed mpegts packet, not start with 0x47");
		return false;
	}
	uint32_t pid = ((buffer[1] & 0x1F) << 8) | buffer[2];
	if(pid == MpegtsType_sdt) {
//		LOG_PRINT("sdt");
		// sdtは特になにかしなければいけないということはない
		ttLibC_Sdt *sdt = ttLibC_Sdt_getPacket((ttLibC_MpegtsPacket *)reader->sdt, buffer, reader->target_size);
		if(sdt == NULL) {
			return false;
		}
		reader->sdt = sdt;
		result = callback(ptr, (ttLibC_Mpegts *)reader->sdt);
	}
	else if(pid == MpegtsType_pat){
		// patを読み込んでpmt情報を取得しないといけない。
//		LOG_PRINT("pat");
		ttLibC_Pat *pat = ttLibC_Pat_getPacket((ttLibC_MpegtsPacket *)reader->pat, buffer, reader->target_size);
		if(pat == NULL) {
			LOG_PRINT("failed to get pat.");
			return false;
		}
		reader->pat = pat;
		reader->pmt_pid = pat->pmt_pid;
		result = callback(ptr, (ttLibC_Mpegts *)reader->pat);
		// このpatからpmt_pidがわかる
	}
	else if(pid == reader->pmt_pid) {
//		LOG_PRINT("たぶんpmt");
		ttLibC_Pmt *pmt = ttLibC_Pmt_getPacket((ttLibC_MpegtsPacket *)reader->pmt, buffer, reader->target_size, reader->pmt_pid);
		if(pmt == NULL) {
			LOG_PRINT("failed to get pmt.");
			return false;
		}
		reader->pmt = pmt;
		result = callback(ptr, (ttLibC_Mpegts *)reader->pmt);
		// ここからpesのpidがわかるので、更新しておく。
	}
	else {
//		LOG_PRINT("たぶんpes");
		// pmtの内容から、合致するpesをみつける。
		bool find = false;
		for(int i = 0;i < MaxPesTracks;++ i) {
			if(pid == reader->pmt->pmtElementaryField[i].pid) {
				find = true;
				// unit start pesを見つけてきて、unit startがはいっている場合は前のデータがおわったことを意味するので、前のデータをcallbackにおくってやる必要がある。
				if((buffer[1] & 0x40) != 0) {
//					LOG_PRINT("buffer[1]:%x", buffer[1]);
					// unit startフラグが立っている。
					if(reader->pes[i] != NULL) {
//						LOG_PRINT("pid:%x: %d", pid, left_size);
//						LOG_DUMP(reader->pes[i]->inherit_super.inherit_super.inherit_super.data, 100, true);
						result = callback(ptr, (ttLibC_Mpegts *)reader->pes[i]);
					}
				}
//				LOG_PRINT("合致するpidをみつけた。%x", reader->pmt->pmtElementaryField[i].stream_type);
				// あとはpesの読み込みを実施すればよい。
				// pesオブジェクトをつくって・・・更新する。
				ttLibC_Pes *pes = ttLibC_Pes_getPacket(
						(ttLibC_MpegtsPacket *)reader->pes[i],
						buffer,
						reader->target_size,
						reader->pmt->pmtElementaryField[i].stream_type,
						reader->pmt->pmtElementaryField[i].pid);
				if(pes == NULL) {
					return false;
				}
				reader->pes[i] = pes;
				break;
			}
		}
		if(!find) {
			// 処理ぬけてなにもなければ、どこかがおかしい。
			return false;
		}
	}
	return result;
}

bool ttLibC_MpegtsReader_read(ttLibC_MpegtsReader *reader, void *data, size_t data_size, ttLibC_MpegtsReaderFunc callback, void *ptr) {
//	LOG_PRINT("データをreadする。");
	ttLibC_MpegtsReader_ *reader_ = (ttLibC_MpegtsReader_ *)reader;
	if(reader_ == NULL) {
		ERR_PRINT("reader is null");
		return false;
	}
	/**
	 * 188byte読み込む
	 * pat sdt pmtはそのまま解析可能になる。
	 * pesについては、はじめの188byteを読み込めば全体の長さがわかるので、次回その長さ読み込む形にする。
	 * で・いいかな。
	 */
	uint8_t *buffer = data;
	size_t left_size = data_size;
	if(reader_->tmp_buffer_next_pos != 0) {
		// TODO 追記しても足りない場合の処理が抜けている。
		size_t copy_size = reader_->tmp_buffer_size - reader_->tmp_buffer_next_pos;
		memcpy(reader_->tmp_buffer + reader_->tmp_buffer_next_pos, data, copy_size);
		buffer += copy_size;
		left_size -= copy_size;
		if(!MpegtsReader_read(reader_, reader_->tmp_buffer, reader_->tmp_buffer_size, callback, ptr)) {
			return false;
		}
		reader_->tmp_buffer_next_pos = 0;
	}
//	LOG_PRINT("処理します。");
	do {
//		LOG_PRINT("target_size:%d", reader_->target_size);
		if(reader_->target_size > left_size) {
			// 十分なサイズではない。
//			LOG_PRINT("もう十分なサイズがない:%d", left_size);
			memcpy(reader_->tmp_buffer, buffer, left_size);
			reader_->tmp_buffer_next_pos = left_size;
			return true;
		}
		if(!MpegtsReader_read(reader_, buffer, left_size, callback, ptr)) {
			return false;
		}
		buffer += 188;
		left_size -= 188;
	} while(true);
//	return false;
}

void ttLibC_MpegtsReader_close(ttLibC_MpegtsReader **reader) {
	ttLibC_MpegtsReader_ *target = (ttLibC_MpegtsReader_ *)*reader;
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.inherit_super.type != containerType_mpegts) {
		ERR_PRINT("this reader is not mpegts reader.");
		return;
	}
	ttLibC_MpegtsPacket_close((ttLibC_MpegtsPacket **)&target->pat);
	ttLibC_MpegtsPacket_close((ttLibC_MpegtsPacket **)&target->pmt);
	ttLibC_MpegtsPacket_close((ttLibC_MpegtsPacket **)&target->sdt);
	for(int i = 0;i < MaxPesTracks;++ i) {
		ttLibC_MpegtsPacket_close((ttLibC_MpegtsPacket **)&target->pes[0]);
	}
	if(target->tmp_buffer != NULL) {
		free(target->tmp_buffer);
	}
	free(target);
	*reader = NULL;
}

