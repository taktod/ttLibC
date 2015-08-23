/*
 * @file   sdt.c
 * @brief  
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/13
 */

#include "sdt.h"
#include "../mpegtsPacket.h"

#include "../../../log.h"
#include "../../../util/hexUtil.h"
#include "../../../util/crc32Util.h"
#include "../../../util/ioUtil.h"
#include <string.h>

ttLibC_Sdt *ttLibC_Sdt_make(
		ttLibC_MpegtsPacket *prev_packet,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		uint16_t pid,
		uint8_t continuity_counter) {
	ttLibC_Sdt *sdt = (ttLibC_Sdt *)ttLibC_MpegtsPacket_make(
			prev_packet,
			data,
			data_size,
			non_copy_mode,
			pts,
			timebase,
			MpegtsType_sdt,
			pid,
			continuity_counter);
	if(sdt != NULL) {
		// nothing to do...
	}
	return sdt;
}

ttLibC_Sdt *ttLibC_Sdt_getPacket(
		ttLibC_MpegtsPacket *prev_packet,
		uint8_t *data,
		size_t data_size) {
	if(data_size != 188) {
		ERR_PRINT("size is not 188 bytes");
		return NULL;
	}
	// 該当のデータがsdtだったら、sdtの巡回カウンターを取得して、次のデータがその値+1になっているか確認しておきたい。
	return ttLibC_Sdt_make(prev_packet, NULL, 0, true, 0, 1000, MpegtsType_sdt, 0);
}

bool ttLibC_Sdt_makePacket(
		const char *provider,
		const char *name,
		uint8_t *data,
		size_t data_size) {
	uint32_t provider_length = strlen(provider);
	uint32_t name_length = strlen(name);
	uint8_t *buf_crc = data + 5;
	uint32_t buf_length = 0x15 + name_length + provider_length;
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
	data[1] = 0x40;
	data[2] = 0x11;
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
	data[4]  = 0x00;
	data[5]  = 0x42; // fixed id
	data[6]  = 0xF0;
	data[7]  = 0x16 + provider_length + name_length; // section length. changacble.* // これはcrc込みっぽい。s
	data[8]  = 0x00; ///
	data[9]  = 0x01; /// program number.
	data[10] = 0xC1;
	data[11] = 0x00;
	data[12] = 0x00;

	// sdt
	data[13] = 0x00;
	data[14] = 0x01;
	data[15] = 0xFF;

	// sdt service field
	data[16] = 0x00;
	data[17] = 0x01;
	data[18] = 0xFC;
	data[19] = 0x80;
	data[20] = 5 + provider_length + name_length; // length of contents.*

	// descriptor type
	data[21] = 0x48;
	data[22] = 3 + provider_length + name_length; // size of descriptor.*

	// service descriptor
	data[23] = 0x01;
	data[24] = provider_length; // length of provider
	data += 25;
	data_size -= 25;
	memcpy(data, provider, provider_length);
	data += provider_length;
	data_size -= provider_length;
	data[0] = name_length;
	++ data;
	-- data_size;
	memcpy(data, name, name_length);
	data += name_length;
	data_size -= name_length;
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
	// crc32
	// あとはff埋め
	for(int i = 0;i < data_size;++ i) {
		*data = 0xFF;
		++ data;
	}
	return true;
}
