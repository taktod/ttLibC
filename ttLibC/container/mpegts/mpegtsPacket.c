/*
 * @file   mpegtsPacket.c
 * @brief  mpegts container support.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/13
 */

#include "mpegtsPacket.h"
#include "type/pat.h"
#include "type/pes.h"
#include "type/pmt.h"
#include "type/sdt.h"
#include <stdlib.h>
#include "../../log.h"
#include "../../allocator.h"
#include "../../frame/frame.h"

ttLibC_MpegtsPacket *ttLibC_MpegtsPacket_make(
		ttLibC_MpegtsPacket *prev_packet,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		ttLibC_Mpegts_Type type,
		uint16_t pid,
		uint8_t continuity_counter) {
	ttLibC_Frame *prev_frame = NULL;
	ttLibC_MpegtsPacket *mpegts_packet = (ttLibC_MpegtsPacket *)ttLibC_Container_make(
			(ttLibC_Container *)prev_packet,
			sizeof(union {
				ttLibC_Pat pat;
				ttLibC_Pes pes;
				ttLibC_Pmt pmt;
				ttLibC_Sdt sdt;
			}),
			containerType_mpegts,
			data,
			data_size,
			non_copy_mode,
			pts,
			timebase);
	if(mpegts_packet != NULL) {
		mpegts_packet->inherit_super.type = type;
		mpegts_packet->inherit_super.pid = pid;
		mpegts_packet->continuity_counter = continuity_counter;
	}
	return mpegts_packet;
}

bool ttLibC_Mpegts_getFrame(ttLibC_Mpegts *mpegts, ttLibC_getFrameFunc callback, void *ptr) {
	switch(mpegts->type) {
	case MpegtsType_pat:
	case MpegtsType_pmt:
	case MpegtsType_sdt:
	default:
		return true;
	case MpegtsType_pes: // only pes has frame.
		return ttLibC_Pes_getFrame((ttLibC_Pes *)mpegts, callback, ptr);
	}
}

/*
 * load mpegts packet header (4byte and adaptation field.)
 * @param reader
 * @param header_info
 * @return true:ok false:error.
 */
bool ttLibC_MpegtsPacket_loadMpegtsPacketHeader(
		ttLibC_ByteReader *reader,
		ttLibC_MpegtsPacket_Header *header_info) {
	if(ttLibC_ByteReader_bit(reader, 8) != 0x47) {
		ERR_PRINT("sync bit is invalid.(not 0x47)");
		return false;
	}
	ttLibC_ByteReader_bit(reader, 1);
	header_info->payloadUnitStartIndicator = ttLibC_ByteReader_bit(reader, 1);
	ttLibC_ByteReader_bit(reader, 1);
	header_info->pid = ttLibC_ByteReader_bit(reader, 13);
	ttLibC_ByteReader_bit(reader, 2);
	header_info->adaptationFieldExist = ttLibC_ByteReader_bit(reader, 1);
	header_info->payloadFieldExist = ttLibC_ByteReader_bit(reader, 1);
	header_info->continuityCounter = ttLibC_ByteReader_bit(reader, 4);

	if(header_info->adaptationFieldExist == 1) {
		uint32_t left_size = 0;
		header_info->adaptationField.discontinuityIndicator = 0;
		header_info->adaptationField.randomAccessIndicator = 0;
		header_info->adaptationField.pcrFlag = 0;
		header_info->adaptationField.pcrBase = 0;
		header_info->adaptationField.pcrExtension = 0;

		header_info->adaptationField.size = ttLibC_ByteReader_bit(reader, 8);
		left_size = header_info->adaptationField.size;
		if(left_size == 0) {
			return !reader->error_flag;
		}
		header_info->adaptationField.discontinuityIndicator = ttLibC_ByteReader_bit(reader, 1);
		header_info->adaptationField.randomAccessIndicator = ttLibC_ByteReader_bit(reader, 1);
		ttLibC_ByteReader_bit(reader, 1);
		header_info->adaptationField.pcrFlag = ttLibC_ByteReader_bit(reader, 1);
		ttLibC_ByteReader_bit(reader, 1);
		ttLibC_ByteReader_bit(reader, 1);
		ttLibC_ByteReader_bit(reader, 1);
		ttLibC_ByteReader_bit(reader, 1);
		-- left_size;
		if(left_size == 0) {
			return !reader->error_flag;
		}
		if(header_info->adaptationField.pcrFlag == 1) {
			if(left_size < 6) {
				ERR_PRINT("no space for pcrInfo on adaptationField.");
				return false;
			}
			header_info->adaptationField.pcrBase = ttLibC_ByteReader_bit(reader, 33);
			ttLibC_ByteReader_bit(reader, 6);
			header_info->adaptationField.pcrExtension = ttLibC_ByteReader_bit(reader, 9);
			left_size -= 6;
		}
		if(left_size > 0) {
			ttLibC_ByteReader_skipByte(reader, left_size);
		}
	}
	return !reader->error_flag;
}

/*
 * load program packet header information.
 * @param reader
 * @param header_info
 * @return true:ok false:error
 */
bool ttLibC_MpegtsPacket_loadProgramPacketHeader(
		ttLibC_ByteReader *reader,
		ttLibC_ProgramPacket_Header *header_info) {
	if(!ttLibC_MpegtsPacket_loadMpegtsPacketHeader(reader, &header_info->header)) {
		return false;
	}
	ttLibC_ByteReader_bit(reader, 8);
	ttLibC_ByteReader_bit(reader, 8);
	ttLibC_ByteReader_bit(reader, 1);
	ttLibC_ByteReader_bit(reader, 1);
	ttLibC_ByteReader_bit(reader, 2);
	header_info->sectionLength = ttLibC_ByteReader_bit(reader, 12);
	ttLibC_ByteReader_bit(reader, 16);
	ttLibC_ByteReader_bit(reader, 2);
	ttLibC_ByteReader_bit(reader, 5);
	ttLibC_ByteReader_bit(reader, 1);
	ttLibC_ByteReader_bit(reader, 8);
	ttLibC_ByteReader_bit(reader, 8);
	return !reader->error_flag;
}

/*
 * close
 * @param mpegts
 */
void ttLibC_Mpegts_close(ttLibC_Mpegts **mpegts) {
	ttLibC_MpegtsPacket_close((ttLibC_MpegtsPacket **)mpegts);
}

/**
 * close
 * @param packet
 */
void ttLibC_MpegtsPacket_close(ttLibC_MpegtsPacket **packet) {
	ttLibC_MpegtsPacket *target = *packet;
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.inherit_super.type != containerType_mpegts) {
		ERR_PRINT("container type is not mpegts.");
		return;
	}
	switch(target->inherit_super.type) {
	case MpegtsType_pat:
		ttLibC_Pat_close((ttLibC_Pat **)packet);
		return;
	case MpegtsType_pes:
		ttLibC_Pes_close((ttLibC_Pes **)packet);
		return;
	case MpegtsType_pmt:
		ttLibC_Pmt_close((ttLibC_Pmt **)packet);
		return;
	case MpegtsType_sdt:
		ttLibC_Sdt_close((ttLibC_Sdt **)packet);
		return;
	default:
		break;
	}
	if(!target->inherit_super.inherit_super.is_non_copy) {
		ttLibC_free(target->inherit_super.inherit_super.data);
	}
	ttLibC_free(target);
	*packet = NULL;
}
