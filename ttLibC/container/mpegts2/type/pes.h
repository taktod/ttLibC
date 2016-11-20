/**
 * @file   pes.h
 * @brief  mpegts pes.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2016/11/18
 */

#ifndef TTLIBC_CONTAINER_MPEGTS2_TYPE_PES_H_
#define TTLIBC_CONTAINER_MPEGTS2_TYPE_PES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../mpegtsPacket.h"
#include "../mpegtsWriter.h"
#include "../../../util/dynamicBufferUtil.h"

typedef struct ttLibC_Container_Mpegts_Pes {
	ttLibC_MpegtsPacket inherit_super;
	ttLibC_Frame_Type frame_type;
	ttLibC_Frame *frame;
	size_t frame_size;
	bool is_used;
} ttLibC_Container_Mpegts_Pes;

typedef ttLibC_Container_Mpegts_Pes ttLibC_Pes;

ttLibC_Pes *ttLibC_Pes_make(
		ttLibC_Pes *prev_pes,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		uint16_t pid,
		uint8_t continuity_counter,
		uint8_t stream_type);

ttLibC_Pes *ttLibC_Pes_getPacket(
		ttLibC_Pes *prev_pes,
		uint8_t *data,
		size_t data_size,
		uint8_t stream_type,
		uint16_t pid);

bool ttLibC_Pes_getFrame(
		ttLibC_Pes *pes,
		ttLibC_getFrameFunc callback,
		void *ptr);

bool ttLibC_Pes_writePacket(
		ttLibC_MpegtsWriteTrack *track,
		bool has_randomAccess,
		bool has_size,
		uint8_t trackBaseId,
		uint32_t pid,
		uint64_t pts,
		uint64_t dts,
		ttLibC_DynamicBuffer *frame_buffer,
		ttLibC_DynamicBuffer *output_buffer);

void ttLibC_Pes_close(ttLibC_Pes **pes);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MPEGTS2_TYPE_PES_H_ */
