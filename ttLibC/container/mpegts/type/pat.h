/**
 * @file   pat.h
 * @brief  
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/13
 */

#ifndef TTLIBC_CONTAINER_MPEGTS_TYPE_PAT_H_
#define TTLIBC_CONTAINER_MPEGTS_TYPE_PAT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../mpegtsPacket.h"

typedef struct {
	ttLibC_MpegtsPacket inherit_super;
	uint16_t pmt_pid;
} ttLibC_Container_Mpegts_Pat;

typedef ttLibC_Container_Mpegts_Pat ttLibC_Pat;

ttLibC_Pat *ttLibC_Pat_make(
		ttLibC_MpegtsPacket *prev_packet,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		uint16_t pid,
		uint8_t continuity_counter,
		uint16_t pmt_pid);

ttLibC_Pat *ttLibC_Pat_getPacket(
		ttLibC_MpegtsPacket *prev_packet,
		uint8_t *data,
		size_t data_size);

bool ttLibC_Pat_makePacket(
		uint8_t *data,
		size_t data_size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MPEGTS_TYPE_PAT_H_ */
