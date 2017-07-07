/*
 * @file   mpegtsPacket.c
 * @brief  mpegts container support.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2016/11/18
 */

#include "mpegtsPacket.h"
#include "type/pat.h"
#include "type/pes.h"
#include "type/pmt.h"
#include "type/sdt.h"
#include <stdlib.h>
#include "../../ttLibC_predef.h"
#include "../../_log.h"
#include "../../allocator.h"
#include "../../frame/frame.h"
#include <stdio.h>

ttLibC_MpegtsPacket TT_VISIBILITY_HIDDEN *ttLibC_MpegtsPacket_make(
		ttLibC_MpegtsPacket *prev_packet,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		ttLibC_Mpegts_Type type,
		uint16_t pid,
		uint8_t continuity_counter) {
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

bool TT_VISIBILITY_DEFAULT ttLibC_Mpegts_getFrame(
		ttLibC_Mpegts *mpegts,
		ttLibC_getFrameFunc callback,
		void *ptr) {
	switch(mpegts->type) {
	case MpegtsType_pat:
	case MpegtsType_pmt:
	case MpegtsType_sdt:
	default:
		return true;
	case MpegtsType_pes:
		return ttLibC_Pes_getFrame((ttLibC_Pes *)mpegts, callback, ptr);
	}
}

bool TT_VISIBILITY_HIDDEN ttLibC_MpegtsPacket_loadMpegtsPacketHeader(
		ttLibC_ByteReader *reader,
		ttLibC_MpegtsPacket_Header *header_info) {
	if(ttLibC_ByteReader_bit(reader, 8) != 0x47) {
		ERR_PRINT("sync bit is invalid.(not 0x47)");
		return false;
	}
	header_info->syncByte                  = 0x47;
	header_info->transportErrorIndicator   = ttLibC_ByteReader_bit(reader, 1);
	header_info->payloadUnitStartIndicator = ttLibC_ByteReader_bit(reader, 1);
	header_info->transportPriority         = ttLibC_ByteReader_bit(reader, 1);
	header_info->pid                       = ttLibC_ByteReader_bit(reader, 13);
	header_info->transportPriority         = ttLibC_ByteReader_bit(reader, 2);
	header_info->adaptationFieldExist      = ttLibC_ByteReader_bit(reader, 1);
	header_info->payloadFieldExist         = ttLibC_ByteReader_bit(reader, 1);
	header_info->continuityCounter         = ttLibC_ByteReader_bit(reader, 4);

	if(header_info->adaptationFieldExist == 1) {
		uint32_t left_size = 0;
		header_info->adaptationField.discontinuityIndicator            = 0;
		header_info->adaptationField.randomAccessIndicator             = 0;
		header_info->adaptationField.elementaryStreamPriorityIndicator = 0;
		header_info->adaptationField.pcrFlag                           = 0;
		header_info->adaptationField.opcrFlag                          = 0;
		header_info->adaptationField.splicingPointFlag                 = 0;
		header_info->adaptationField.transportPrivateDataFlag          = 0;
		header_info->adaptationField.adaptationFieldExtensionFlag      = 0;

		header_info->adaptationField.pcrBase      = 0;
		header_info->adaptationField.pcrExtension = 0;

		header_info->adaptationField.size = ttLibC_ByteReader_bit(reader, 8);
		left_size = header_info->adaptationField.size;
		if(left_size == 0) {
			if(reader->error_number != 0) {
				return false;
			}
			return true;
		}
		header_info->adaptationField.discontinuityIndicator            = ttLibC_ByteReader_bit(reader, 1);
		header_info->adaptationField.randomAccessIndicator             = ttLibC_ByteReader_bit(reader, 1);
		header_info->adaptationField.elementaryStreamPriorityIndicator = ttLibC_ByteReader_bit(reader, 1);
		header_info->adaptationField.pcrFlag                           = ttLibC_ByteReader_bit(reader, 1);
		header_info->adaptationField.opcrFlag                          = ttLibC_ByteReader_bit(reader, 1);
		header_info->adaptationField.splicingPointFlag                 = ttLibC_ByteReader_bit(reader, 1);
		header_info->adaptationField.transportPrivateDataFlag          = ttLibC_ByteReader_bit(reader, 1);
		header_info->adaptationField.adaptationFieldExtensionFlag      = ttLibC_ByteReader_bit(reader, 1);
		-- left_size;
		if(left_size == 0) {
			if(reader->error != 0) {
				return false;
			}
			return true;
		}
		if(header_info->adaptationField.pcrFlag == 1) {
			if(left_size < 6) {
				ERR_PRINT("no space for pcrInfo on adaptationField.");
				return false;
			}
			header_info->adaptationField.pcrBase      = ttLibC_ByteReader_bit(reader, 33);
			ttLibC_ByteReader_bit(reader, 6);
			header_info->adaptationField.pcrExtension = ttLibC_ByteReader_bit(reader, 9);
			left_size -= 6;
		}
		if(left_size > 0) {
			ttLibC_ByteReader_skipByte(reader, left_size);
		}
	}
	if(reader->error_number != 0) {
		return false;
	}
	return true;
}

bool TT_VISIBILITY_HIDDEN ttLibC_MpegtsPacket_loadProgramPacketHeader(
		ttLibC_ByteReader *reader,
		ttLibC_ProgramPacket_Header *header_info) {
	if(!ttLibC_MpegtsPacket_loadMpegtsPacketHeader(reader, &header_info->header)) {
		return false;
	}
	header_info->pointerField           = ttLibC_ByteReader_bit(reader, 8);
	header_info->tableId                = ttLibC_ByteReader_bit(reader, 8);
	header_info->sectionSyntaxIndicator = ttLibC_ByteReader_bit(reader, 1);
	header_info->reservedFutureUse1     = ttLibC_ByteReader_bit(reader, 1);
	header_info->reserved1              = ttLibC_ByteReader_bit(reader, 2);
	header_info->sectionLength          = ttLibC_ByteReader_bit(reader, 12);
	header_info->programNumber          = ttLibC_ByteReader_bit(reader, 16);
	header_info->reserved               = ttLibC_ByteReader_bit(reader, 2);
	header_info->versionNumber          = ttLibC_ByteReader_bit(reader, 5);
	header_info->currentNextOrder       = ttLibC_ByteReader_bit(reader, 1);
	header_info->sectionNumber          = ttLibC_ByteReader_bit(reader, 8);
	header_info->lastSectionNumber      = ttLibC_ByteReader_bit(reader, 8);
	if(reader->error_number != 0) {
		return false;
	}
	return true;
}

void TT_VISIBILITY_DEFAULT ttLibC_Mpegts_close(ttLibC_Mpegts **mpegts) {
	ttLibC_MpegtsPacket_close((ttLibC_MpegtsPacket **)mpegts);
}

void TT_VISIBILITY_HIDDEN ttLibC_MpegtsPacket_close(ttLibC_MpegtsPacket **packet) {
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
