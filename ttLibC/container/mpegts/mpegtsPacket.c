/*
 * @file   mpegtsPacket.c
 * @brief  
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
	if(prev_packet != NULL) {
		prev_frame = prev_packet->frame;
	}
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
		mpegts_packet->frame = prev_frame;
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
		// 取得するフレームはない。
		return true;
	case MpegtsType_pes:
		return ttLibC_Pes_getFrame((ttLibC_Pes *)mpegts, callback, ptr);
	}
}

void ttLibC_Mpegts_close(ttLibC_Mpegts **mpegts) {
	ttLibC_MpegtsPacket_close((ttLibC_MpegtsPacket **)mpegts);
}

void ttLibC_MpegtsPacket_close(ttLibC_MpegtsPacket **packet) {
	ttLibC_MpegtsPacket *target = *packet;
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.inherit_super.type != containerType_mpegts) {
		ERR_PRINT("container type is not mpegts.");
		return;
	}
	ttLibC_Frame_close(&target->frame);
	if(!target->inherit_super.inherit_super.is_non_copy) {
		free(target->inherit_super.inherit_super.data);
	}
	free(target);
	*packet = NULL;
}
