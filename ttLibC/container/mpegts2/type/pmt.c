/*
 * @file   pmt.c
 * @brief  mpegts pmt.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2016/11/18
 */

#include "pmt.h"
#include "../mpegtsPacket.h"

#include "../../../log.h"
#include "../../../allocator.h"
#include "../../../util/byteUtil.h"
#include "../../../util/ioUtil.h"
#include "../../../util/crc32Util.h"
#include "../../../util/dynamicBufferUtil.h"

ttLibC_Pmt *ttLibC_Pmt_make(
		ttLibC_Pmt *prev_pmt,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		uint16_t pid,
		uint8_t continuity_counter,
		int pes_track_num) {
	// copy prev field list.
	ttLibC_PmtElementaryField *pmtElementaryField_list = NULL;
	if(prev_pmt != NULL) {
		if(prev_pmt->pes_track_num == pes_track_num) {
			pmtElementaryField_list = prev_pmt->pmtElementaryField_list;
		}
		else {
			if(prev_pmt->pmtElementaryField_list != NULL) {
				ttLibC_free(prev_pmt->pmtElementaryField_list);
			}
		}
	}
	ttLibC_Pmt *pmt = (ttLibC_Pmt *)ttLibC_MpegtsPacket_make(
			(ttLibC_MpegtsPacket *)prev_pmt,
			data,
			data_size,
			non_copy_mode,
			pts,
			timebase,
			MpegtsType_pmt,
			pid,
			continuity_counter);
	if(pmt != NULL) {
		pmt->pmtElementaryField_list = pmtElementaryField_list;
		if(pmt->pmtElementaryField_list == NULL) {
			pmt->pmtElementaryField_list = ttLibC_malloc(sizeof(ttLibC_PmtElementaryField) * pes_track_num);
		}
		if(pmt->pmtElementaryField_list == NULL) {
			ttLibC_Pmt_close(&pmt);
			return NULL;
		}
		for(int i = 0;i < pes_track_num;++ i) {
			pmt->pmtElementaryField_list[i].stream_type = 0;
			pmt->pmtElementaryField_list[i].pid = 0;
		}
	}
	return pmt;
}

ttLibC_Pmt *ttLibC_Pmt_getPacket(
		ttLibC_Pmt *prev_pmt,
		uint8_t *data,
		size_t data_size,
		uint16_t pmt_pid) {
	ttLibC_ByteReader *reader = ttLibC_ByteReader_make(data, data_size, ByteUtilType_default);
	ttLibC_ProgramPacket_Header header_info;
	if(!ttLibC_MpegtsPacket_loadProgramPacketHeader(reader, &header_info)) {
		return NULL;
	}
	uint32_t section_length = header_info.sectionLength - 5;
	/*
	 * 3bit reserved
	 * 13bit pcrPid
	 * 4bit reserved2
	 * 12bit program info length (expect 0)
	 *
	 * loop
	 * 8bit streamType
	 * 3bit reserved
	 * 13bit pid
	 * 4bit reserved
	 * 12bit esInfoLength
	 * loop end.
	 *
	 * 32bit CRC32
	 */
	ttLibC_ByteReader_bit(reader, 3);
	uint16_t pcrPid = ttLibC_ByteReader_bit(reader, 13);
	ttLibC_ByteReader_bit(reader, 4);
	ttLibC_ByteReader_bit(reader, 12); // if this value is not 0, maybe we have some extra information later.
	section_length -= 4;
	uint32_t pes_track_num = 0;
	// check the number of pes.
	for(int i = 0;i < section_length - 4;++ i) {
		++ pes_track_num;
		uint8_t stream_type = ttLibC_ByteReader_bit(reader, 8);
		ttLibC_ByteReader_bit(reader, 3);
		uint16_t pes_pid = ttLibC_ByteReader_bit(reader, 13);
		ttLibC_ByteReader_bit(reader, 4);
		uint16_t pes_info_length = ttLibC_ByteReader_bit(reader, 12);
		for(int j = 0;j < pes_info_length;++ j) {
			ttLibC_ByteReader_bit(reader, 8);
		}
		i += pes_info_length;
		i += 5;
	}
	ttLibC_Pmt *pmt = ttLibC_Pmt_make(
			prev_pmt,
			NULL,
			0,
			true,
			0,
			1000,
			pmt_pid,
			header_info.header.continuityCounter,
			pes_track_num);
	if(pmt == NULL) {
		ERR_PRINT("failed to create pmt object. something is wrong.");
		ttLibC_ByteReader_close(&reader);
		return NULL;
	}
	ttLibC_ByteReader_rewindByte(reader, section_length - 4);
	for(int i = 0, k = 0;i < section_length - 4;++ i) {
		uint8_t stream_type = ttLibC_ByteReader_bit(reader, 8);
		ttLibC_ByteReader_bit(reader, 3);
		uint16_t pes_pid = ttLibC_ByteReader_bit(reader, 13);
		ttLibC_ByteReader_bit(reader, 4);
		uint16_t pes_info_length = ttLibC_ByteReader_bit(reader, 12);
		for(int j = 0;j < pes_info_length;++ j) {
			ttLibC_ByteReader_bit(reader, 8);
		}
		i += pes_info_length;
		i += 5;
		pmt->pmtElementaryField_list[k].stream_type = stream_type;
		pmt->pmtElementaryField_list[k].pid = pes_pid;
		++ k;
	}
	ttLibC_ByteReader_close(&reader);
	pmt->pes_track_num = pes_track_num;
	return pmt;
}

bool ttLibC_Pmt_makePacket(
		ttLibC_MpegtsWriter_ *writer,
		uint8_t *data,
		size_t data_size) {
	uint32_t size = ttLibC_HexUtil_makeBuffer("47 50 00 10 00 02 B0 00 00 01 C1 00 00 E1 00 F0 00", data, data_size);
	uint8_t *buf_crc = data + 5;
	data[7] = 13 + 5 * writer->inherit_super.track_list->size;
	data += size;
	data_size -= size;
	uint32_t i = 0;
	while(true) {
		uint32_t pid = 0x0100 + i;
		ttLibC_ContainerWriter_WriteTrack *track = (ttLibC_ContainerWriter_WriteTrack *)ttLibC_StlMap_get(writer->inherit_super.track_list, (void *)(long)pid);
		if(track == NULL) {
			break;
		}
		switch(track->frame_type) {
		case frameType_h264:
			data[0] = 0x1B;
			break;
		case frameType_aac:
			data[0] = 0x0F;
			break;
		case frameType_mp3:
			data[0] = 0x03;
			break;
		default:
			ERR_PRINT("unexppected frame data.");
			return false;
		}
		++ data;
		-- data_size;
		*((uint16_t *)data) = be_uint16_t((0xE000 | pid));
		data[2] = 0xF0;
		data[3] = 0x00;
		data += 4;
		data_size -= 4;
		++ i;
	}
	// ready, make crc32.
	ttLibC_Crc32 *crc32 = ttLibC_Crc32_make(0xFFFFFFFF);
	for(;buf_crc < data;++ buf_crc) {
		ttLibC_Crc32_update(crc32, *buf_crc);
	}
	*((uint32_t *)data) = be_uint32_t(ttLibC_Crc32_getValue(crc32));
	data += 4;
	data_size -= 4;
	ttLibC_Crc32_close(&crc32);
	for(int i = 0;i < data_size;++ i) {
		*data = 0xFF;
		++ data;
	}
	return true;
}

void ttLibC_Pmt_close(ttLibC_Pmt **pmt) {
	ttLibC_Pmt *target = *pmt;
	if(target == NULL) {
		return;
	}
	if(!target->inherit_super.inherit_super.inherit_super.is_non_copy) {
		if(target->inherit_super.inherit_super.inherit_super.data) {
			ttLibC_free(target->inherit_super.inherit_super.inherit_super.data);
		}
	}
	if(target->pmtElementaryField_list != NULL) {
		ttLibC_free(target->pmtElementaryField_list);
	}
	ttLibC_free(target);
	*pmt = NULL;
}
