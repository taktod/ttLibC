/**
 * @file   pmt.h
 * @brief  
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/13
 */

#ifndef TTLIBC_CONTAINER_MPEGTS_TYPE_PMT_H_
#define TTLIBC_CONTAINER_MPEGTS_TYPE_PMT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../mpegtsPacket.h"

typedef struct {
	uint8_t stream_type;
	uint16_t pid;
} PmtElementaryField;

typedef struct {
	ttLibC_MpegtsPacket inherit_super;
	PmtElementaryField pmtElementaryField[MaxPesTracks];
} ttLibC_Container_Mpegts_Pmt;

typedef ttLibC_Container_Mpegts_Pmt ttLibC_Pmt;

ttLibC_Pmt *ttLibC_Pmt_make(
		ttLibC_MpegtsPacket *prev_packet,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		uint16_t pid,
		uint8_t continuity_counter);

ttLibC_Pmt *ttLibC_Pmt_getPacket(
		ttLibC_MpegtsPacket *prev_packet,
		uint8_t *data,
		size_t data_size,
		uint16_t pmt_pid);

bool ttLibC_Pmt_makePacket(
		ttLibC_MpegtsWriter *writer,
		uint8_t *data,
		size_t data_size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MPEGTS_TYPE_PMT_H_ */
