/**
 * @file   pmt.h
 * @brief  mpegts pmt.
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
#include "../mpegtsWriter.h"

typedef struct ttLibC_PmtElementaryField {
	uint8_t stream_type;
	uint16_t pid;
} ttLibC_PmtElementaryField;

typedef struct ttLibC_Container_Mpegts_Pmt {
	ttLibC_MpegtsPacket inherit_super;
	ttLibC_PmtElementaryField *pmtElementaryField_list;
	uint32_t pes_track_num;
} ttLibC_Container_Mpegts_Pmt;

typedef ttLibC_Container_Mpegts_Pmt ttLibC_Pmt;

ttLibC_Pmt *ttLibC_Pmt_make(
		ttLibC_Pmt *prev_pmt,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		uint16_t pid,
		uint8_t continuity_counter,
		int pes_track_num);

ttLibC_Pmt *ttLibC_Pmt_getPacket(
		ttLibC_Pmt *prev_pmt,
		uint8_t *data,
		size_t data_size,
		uint16_t pmt_pid);

bool ttLibC_Pmt_makePacket(
		ttLibC_MpegtsWriter_ *writer,
		uint8_t *data,
		size_t data_size);

void ttLibC_Pmt_close(ttLibC_Pmt **pmt);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MPEGTS_TYPE_PMT_H_ */
