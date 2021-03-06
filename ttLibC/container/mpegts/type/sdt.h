/**
 * @file   sdt.h
 * @brief  mpegts sdt.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2016/11/18
 */

#ifndef TTLIBC_CONTAINER_MPEGTS_TYPE_SDT_H_
#define TTLIBC_CONTAINER_MPEGTS_TYPE_SDT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../mpegtsPacket.h"

typedef struct ttLibC_Container_Mpegts_Sdt {
	ttLibC_MpegtsPacket inherit_super;
} ttLibC_Container_Mpegts_Sdt;

typedef ttLibC_Container_Mpegts_Sdt ttLibC_Sdt;

ttLibC_Sdt *ttLibC_Sdt_make(
		ttLibC_Sdt *prev_packet,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		uint16_t pid,
		uint8_t continuity_counter);

ttLibC_Sdt *ttLibC_Sdt_getPacket(
		ttLibC_Sdt *prev_Sdt,
		uint8_t *data,
		size_t data_size);

bool ttLibC_Sdt_makePacket(
		const char *provider,
		const char *name,
		uint8_t *data,
		size_t data_size);

void ttLibC_Sdt_close(ttLibC_Sdt **sdt);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MPEGTS_TYPE_SDT_H_ */
