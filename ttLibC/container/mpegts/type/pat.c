/*
 * @file   pat.c
 * @brief  
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/13
 */

#include "pat.h"
#include "../mpegtsPacket.h"

#include "../../../log.h"
#include "../../../util/bitUtil.h"
#include "../../../util/hexUtil.h"

ttLibC_Pat *ttLibC_Pat_make(
		ttLibC_MpegtsPacket *prev_packet,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		uint16_t pid,
		uint8_t continuity_counter,
		uint16_t pmt_pid) {
	ttLibC_Pat *pat = (ttLibC_Pat *)ttLibC_MpegtsPacket_make(
			prev_packet,
			data,
			data_size,
			non_copy_mode,
			pts,
			timebase,
			MpegtsType_pat,
			pid,
			continuity_counter);
	if(pat != NULL) {
		pat->pmt_pid = pmt_pid;
	}
	return pat;
}

ttLibC_Pat *ttLibC_Pat_getPacket(
		ttLibC_MpegtsPacket *prev_packet,
		uint8_t *data,
		size_t data_size) {
//	LOG_PRINT("getPaketがよばれた for pat");
//	LOG_PRINT("ここからpmtのpidを取得しなければいけない");
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
	if(ttLibC_BitReader_bit(reader, 13) != MpegtsType_pat) {
		// pidはpatと同じでないとだめ。 = 0
	}
	ttLibC_BitReader_bit(reader, 2);
	uint8_t adaptationFieldExist = ttLibC_BitReader_bit(reader, 1);
	ttLibC_BitReader_bit(reader, 1); // payloadはあるはずなので、このbitも立っているはず。
	uint8_t continuity_counter = ttLibC_BitReader_bit(reader, 4); // continuity counter.
	if(adaptationFieldExist == 1) {
		LOG_PRINT("rarely come here. I know mpegts from vlc can. make this later.");
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
	ttLibC_BitReader_bit(reader, 12); // sectionLength // patの場合はcrcまでの長さ
	ttLibC_BitReader_bit(reader, 16);
	ttLibC_BitReader_bit(reader, 2);
	ttLibC_BitReader_bit(reader, 5);
	ttLibC_BitReader_bit(reader, 1);
	ttLibC_BitReader_bit(reader, 8);
	ttLibC_BitReader_bit(reader, 8);
	/*
	 * patの中身
	 * 16bit programNum
	 * 3bit reserved
	 * 13bit pmtPid(0x1000が候補)
	 * 32bit crc32
	 */
	ttLibC_BitReader_bit(reader, 16);
	ttLibC_BitReader_bit(reader, 3);
	int pmt_pid = ttLibC_BitReader_bit(reader, 13);
	ttLibC_BitReader_bit(reader, 32); // crcもできたら確認しておきたい。
	ttLibC_BitReader_close(&reader);
	return ttLibC_Pat_make(
			prev_packet, NULL, 0, true, 0, 1000, MpegtsType_pat, continuity_counter, pmt_pid);
}

bool ttLibC_Pat_makePacket(
		uint8_t *data,
		size_t data_size) {
	// めんどくさいので決め打ち
	uint32_t size = ttLibC_HexUtil_makeBuffer("474000100000B00D0001C100000001F0002AB104B2", data, data_size);
	data += size;
	data_size -= size;
	for(int i = 0;i < data_size;++ i) {
		*data = 0xFF;
		++ data;
	}
	return true;
}
