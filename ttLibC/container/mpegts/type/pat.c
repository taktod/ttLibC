/*
 * @file   pat.c
 * @brief  mpegts pat.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2016/11/18
 */

#include "pat.h"
#include "../mpegtsPacket.h"

#include "../../../ttLibC_predef.h"
#include "../../../_log.h"
#include "../../../allocator.h"
#include "../../../util/byteUtil.h"
#include "../../../util/hexUtil.h"

ttLibC_Pat *ttLibC_Pat_make(
		ttLibC_Pat *prev_packet,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		uint16_t pid,
		uint8_t continuity_counter,
		uint16_t pmt_pid) {
	ttLibC_Pat *pat = (ttLibC_Pat *)ttLibC_MpegtsPacket_make(
			(ttLibC_MpegtsPacket *)prev_packet,
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
		ttLibC_Pat *prev_pat,
		uint8_t *data,
		size_t data_size) {
	ttLibC_ByteReader *reader = ttLibC_ByteReader_make(data, data_size, ByteUtilType_default);
	ttLibC_ProgramPacket_Header header_info;
	if(!ttLibC_MpegtsPacket_loadProgramPacketHeader(reader, &header_info)) {
		return NULL;
	}
	/*
	 * pat original data.
	 * 16bit programNum
	 * 3bit reserved
	 * 13bit pmtPid(0x1000 is target.)
	 * 32bit crc32
	 */
	ttLibC_ByteReader_bit(reader, 16);
	ttLibC_ByteReader_bit(reader, 3);
	int pmt_pid = ttLibC_ByteReader_bit(reader, 13);
	ttLibC_ByteReader_bit(reader, 32); // TODO check crc value.
	ttLibC_ByteReader_close(&reader);
	return ttLibC_Pat_make(
			prev_pat,
			NULL,
			0,
			true,
			0,
			1000,
			MpegtsType_pat,
			header_info.header.continuityCounter,
			pmt_pid);
}

bool ttLibC_Pat_makePacket(
		uint8_t *data,
		size_t data_size) {
	// use fixed value.
	uint32_t size = ttLibC_HexUtil_makeBuffer("474000100000B00D0001C100000001F0002AB104B2", data, data_size);
	data += size;
	data_size -= size;
	for(uint32_t i = 0;i < data_size;++ i) {
		*data = 0xFF;
		++ data;
	}
	return true;
}

void ttLibC_Pat_close(ttLibC_Pat **pat) {
	ttLibC_Pat *target = *pat;
	if(target == NULL) {
		return;
	}
	if(!target->inherit_super.inherit_super.inherit_super.is_non_copy) {
		if(target->inherit_super.inherit_super.inherit_super.data) {
			ttLibC_free(target->inherit_super.inherit_super.inherit_super.data);
		}
	}
	ttLibC_free(target);
	*pat = NULL;
}
