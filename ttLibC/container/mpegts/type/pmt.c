/*
 * @file   pmt.c
 * @brief  
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/13
 */

#include "pmt.h"
#include "../mpegtsPacket.h"

#include "../../../log.h"
#include "../../../util/bitUtil.h"
#include "../../../util/ioUtil.h"
#include "../../../util/crc32Util.h"

ttLibC_Pmt *ttLibC_Pmt_make(
		ttLibC_MpegtsPacket *prev_packet,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		uint16_t pid,
		uint8_t continuity_counter) {
	ttLibC_Pmt *pmt = (ttLibC_Pmt *)ttLibC_MpegtsPacket_make(
			prev_packet,
			data,
			data_size,
			non_copy_mode,
			pts,
			timebase,
			MpegtsType_pmt,
			pid,
			continuity_counter);
	if(pmt != NULL) {
		for(int i = 0;i < MaxPesTracks;++ i) {
			pmt->pmtElementaryField[i].stream_type = 0;
			pmt->pmtElementaryField[i].pid = 0;
		}
	}
	return pmt;
}

ttLibC_Pmt *ttLibC_Pmt_getPacket(
		ttLibC_MpegtsPacket *prev_packet,
		uint8_t *data,
		size_t data_size,
		uint16_t pmt_pid) {
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
	if(ttLibC_BitReader_bit(reader, 1) != 1) {
		// patは1つでpayload完結するので、常にONになってないとおかしい。
	}
	ttLibC_BitReader_bit(reader, 1);
	if(ttLibC_BitReader_bit(reader, 13) != pmt_pid) {
		// pidはpatと同じでないとだめ。 = 0
		LOG_PRINT("pmtpid is invalid.");
	}
	ttLibC_BitReader_bit(reader, 2);
	uint8_t adaptationFieldExist = ttLibC_BitReader_bit(reader, 1);
	ttLibC_BitReader_bit(reader, 1); // payloadはあるはずなので、このbitも立っているはず。
	uint8_t continuity_counter = ttLibC_BitReader_bit(reader, 4); // continuity counter.
	if(adaptationFieldExist == 1) {
//		LOG_PRINT("rarely comming, do later. I know this happen with mpegts from VLC.");
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
		// 読み飛ばす場合は処理が面倒なので、readerを一旦解放して、元のbufferを進めて、サイドreader作り直した方がよさそう。
		uint8_t adaptationField_size = ttLibC_BitReader_bit(reader, 8);
		ttLibC_BitReader_close(&reader);
		reader = ttLibC_BitReader_make(
				data + adaptationField_size + 5,
				data_size - adaptationField_size - 5,
				BitReaderType_default);
	}
	/*
	 * programPacket共通
	 * 8bit pointerField
	 * 8bit tableId
	 * 1bit sectionSyntaxIndicator
	 * 1bit reservedFutureUse1
	 * 2bit reserved1
	 * 12bit sectionLength(ここから先の長さ)
	 * 16bit programNumber
	 * 2bit reserved
	 * 5bit versionNUmber
	 * 1bit currentNextOrder
	 * 8bit sectionNumber
	 * 8bit lastSectionNumber
	 */
	ttLibC_BitReader_bit(reader, 8);
	ttLibC_BitReader_bit(reader, 8);
	ttLibC_BitReader_bit(reader, 1);
	ttLibC_BitReader_bit(reader, 1);
	ttLibC_BitReader_bit(reader, 2);
	uint32_t section_length = ttLibC_BitReader_bit(reader, 12); // sectionLength // patの場合はcrcまでの長さ
//	LOG_PRINT("section_length:%x", section_length);
	ttLibC_BitReader_bit(reader, 16);
	ttLibC_BitReader_bit(reader, 2);
	ttLibC_BitReader_bit(reader, 5);
	ttLibC_BitReader_bit(reader, 1);
	ttLibC_BitReader_bit(reader, 8);
	ttLibC_BitReader_bit(reader, 8);
	section_length -= 5;
	// ここから先がpmtの独自内容
	/*
	 * 3bit reserved
	 * 13bit pcrPid
	 * 4bit reserved2
	 * 12bit program info length(追加データがあるか？)
	 * (たぶん追加データ)
	 *
	 * 以下トラック分繰り返し(繰り返し回数はsection_lengthから求める。)
	 * 8bit streamType
	 * 3bit reserved
	 * 13bit pid
	 * 4bit reserved
	 * 12bit esInfoLength
	 * 繰り返しここまで
	 *
	 * 32bit CRC32
	 * で、終わり
	 */
	ttLibC_BitReader_bit(reader, 3);
	uint16_t pcrPid = ttLibC_BitReader_bit(reader, 13);
//	LOG_PRINT("pcrPid:%x", pcrPid);
	ttLibC_BitReader_bit(reader, 4);
	ttLibC_BitReader_bit(reader, 12); // 本当はこのinfo分削らないとだめ。
	section_length -= 4;
	// ここまででおいといて、あとの部分はオブジェクトをつくってから応答する。
	ttLibC_Pmt *pmt = ttLibC_Pmt_make(
			prev_packet,
			NULL,
			0,
			true,
			0,
			1000,
			pmt_pid,
			continuity_counter);
	if(pmt == NULL) {
		ERR_PRINT("failed to create pmt object. something is wrong.");
		ttLibC_BitReader_close(&reader);
		return NULL;
	}
	// あとはデータを繰り返す。
	for(int i = 0;i < MaxPesTracks && section_length > 4;++ i) {
		uint8_t stream_type = ttLibC_BitReader_bit(reader, 8);
		ttLibC_BitReader_bit(reader, 3);
		uint16_t pes_pid = ttLibC_BitReader_bit(reader, 13);
		ttLibC_BitReader_bit(reader, 4);
		uint16_t pes_info_length = ttLibC_BitReader_bit(reader, 12);
		if(pes_info_length != 0) {
			for(int i = 0;i < pes_info_length;++ i) {
				ttLibC_BitReader_bit(reader, 8);
			}
			section_length -= pes_info_length;
		}
		pmt->pmtElementaryField[i].stream_type = stream_type;
		pmt->pmtElementaryField[i].pid = pes_pid;
		section_length -= 5;
	}
	ttLibC_BitReader_close(&reader);
	return pmt;
}

bool ttLibC_Pmt_makePacket(
		ttLibC_MpegtsWriter *writer,
		uint8_t *data,
		size_t data_size) {
	uint8_t *buf_crc = data + 5;
	uint32_t buf_length = 0x0C; // この長さは変化する。// track数に応じて変わる感じ。
	// 作ります。
	// pidは0x1000固定
	// その他のデータはtrackをいれておきたいだけ。

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
	data[0] = 0x47;
	data[1] = 0x50;
	data[2] = 0x00;
	data[3] = 0x10;
	/*
	 * programPacket共通
	 * 8bit pointerField
	 * 8bit tableId
	 * 1bit sectionSyntaxIndicator
	 * 1bit reservedFutureUse1
	 * 2bit reserved1
	 * 12bit sectionLength(ここから先の長さ)
	 * 16bit programNumber
	 * 2bit reserved
	 * 5bit versionNUmber
	 * 1bit currentNextOrder
	 * 8bit sectionNumber
	 * 8bit lastSectionNumber
	 */
	// 00 02 B0 17 00 01 C1 00 00
	data[4]  = 0x00;
	data[5]  = 0x02;
	data[6]  = 0xB0;
	data[7]  = 0x17; // この長さは変化する。*
	data[8]  = 0x00; ///
	data[9]  = 0x01; /// program number
	data[10] = 0xC1;
	data[11] = 0x00;
	data[12] = 0x00;
	/*
	 * 3bit reserved
	 * 13bit pcrPid
	 * 4bit reserved2
	 * 12bit program info length(追加データがあるか？)
	 * (たぶん追加データ)
	 *
	 * 以下トラック分繰り返し(繰り返し回数はsection_lengthから求める。)
	 * 8bit streamType
	 * 3bit reserved
	 * 13bit pid
	 * 4bit reserved
	 * 12bit esInfoLength
	 * 繰り返しここまで
	 *
	 * 32bit CRC32
	 * で、終わり
	 */
	// E1 00 F0 00 1B E1 00 F0 00 0F E1 01 F0 00 2F44B99B
	// これをつくる。
	data[13] = 0xE1;
	data[14] = 0x00; // pcrPIDは0x100にしておく。
	data[15] = 0xF0;
	data[16] = 0x00; // extraデータはなしにするので、ここは固定
	// ここから先は繰り返しデータ
	data += 17;
	data_size -= 17;
	for(int i = 0;i < MaxPesTracks;++ i) {
		if(writer->trackInfo[i].pid == 0) {
			// もうない。
			break;
		}
		// まだある。
		switch(writer->trackInfo[i].frame_type) {
		case frameType_h264:
			buf_length += 5;
			// streamtypeの定義が２箇所 pes.cとpmt.cに別れるのはよろしくないですね。
			data[0] = 0x1B;
			break;
		case frameType_aac:
			buf_length += 5;
			data[0] = 0x0F;
			break;
		case frameType_mp3:
			buf_length += 5;
			data[0] = 0x03;
			break;
		default:
			ERR_PRINT("unexpected frame data.");
			return false;
		}
		++ data;
		-- data_size;
		*((uint16_t *)data) = be_uint16_t((0xE000 | writer->trackInfo[i].pid));
		data[2] = 0xF0;
		data[3] = 0x00;
		data += 4;
		data_size -= 4;
	}
	// あとはcrc32を計算する。
	ttLibC_Crc32 *crc32 = ttLibC_Crc32_make(0xFFFFFFFF);
	for(int i = 0;i < buf_length;++ i) {
		ttLibC_Crc32_update(crc32, *buf_crc);
		++ buf_crc;
	}
	*((uint32_t *)data) = be_uint32_t(ttLibC_Crc32_getValue(crc32));
	data += 4;
	data_size -= 4;
	ttLibC_Crc32_close(&crc32);
	// あとはff埋め
	for(int i = 0;i < data_size;++ i) {
		*data = 0xFF;
		++ data;
	}
	return true;
}
