/**
 * @file   mpegtsPacket.h
 * @brief  
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/13
 */

#ifndef TTLIBC_CONTAINER_MPEGTS_MPEGTSPACKET_H_
#define TTLIBC_CONTAINER_MPEGTS_MPEGTSPACKET_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../mpegts.h"

typedef struct {
	ttLibC_Mpegts inherit_super;
	uint8_t continuity_counter; // 巡回カウンター値
	ttLibC_Frame *frame; // 本来ならpesのみにあれば十分だが、pesにいれると、メモリー管理が面倒になる。(個別のpacketのcloseがないand別のpacketとして使いまわせるようにするため。)
} ttLibC_Container_MpegtsPacket;

typedef ttLibC_Container_MpegtsPacket ttLibC_MpegtsPacket;

ttLibC_MpegtsPacket *ttLibC_MpegtsPacket_make(
		ttLibC_MpegtsPacket *prev_packet,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		ttLibC_Mpegts_Type type,
		uint16_t pid,
		uint8_t continuity_counter);

void ttLibC_MpegtsPacket_close(ttLibC_MpegtsPacket **packet);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MPEGTS_MPEGTSPACKET_H_ */
