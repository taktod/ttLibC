/**
 * @file   pes.h
 * @brief  mpegts pes.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/13
 */

#ifndef TTLIBC_CONTAINER_MPEGTS_TYPE_PES_H_
#define TTLIBC_CONTAINER_MPEGTS_TYPE_PES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../mpegtsPacket.h"
#include "../mpegtsWriter.h"

typedef struct {
	ttLibC_MpegtsPacket inherit_super;
	ttLibC_Frame_Type frame_type;
	ttLibC_Frame *frame;
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

/**
 * write h264 packet.
 * @param track
 * @param frame
 * @param callback
 * @param ptr
 */
bool ttLibC_Pes_writeH264Packet(
		ttLibC_MpegtsTrack *track,
		ttLibC_Frame *frame,
		ttLibC_ContainerWriteFunc callback,
		void *ptr);

/**
 * write audio packet.
 * @param writer
 * @param track
 * @param callback
 * @param ptr
 */
bool ttLibC_Pes_writeAudioPacket(
		ttLibC_MpegtsWriter_ *writer,
		ttLibC_MpegtsTrack *track,
		ttLibC_ContainerWriteFunc callback,
		void *ptr);

void ttLibC_Pes_close(ttLibC_Pes **pes);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MPEGTS_TYPE_PES_H_ */
