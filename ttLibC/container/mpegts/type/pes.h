/**
 * @file   pes.h
 * @brief  
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
} ttLibC_Container_Mpegts_Pes;

typedef ttLibC_Container_Mpegts_Pes ttLibC_Pes;

ttLibC_Pes *ttLibC_Pes_make(
		ttLibC_MpegtsPacket *prev_packet,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		uint16_t pid,
		uint8_t continuity_counter,
		uint8_t stream_type);

ttLibC_Pes *ttLibC_Pes_getPacket(
		ttLibC_MpegtsPacket *prev_packet,
		uint8_t *data,
		size_t data_size,
		uint8_t stream_type,
		uint16_t pid);

bool ttLibC_Pes_getFrame(
		ttLibC_Pes *pes,
		ttLibC_getFrameFunc callback,
		void *ptr);

/**
 * この関数だけは、sdt pat pmtと違い、書き込みのcallbackを呼び出します。
 * sdt pat pmtは初期化時にbufferをつくるので、別扱いになる感じ。
 * /
bool ttLibC_Pes_writePacket(
		ttLibC_MpegtsWriter_ *writer,
		ttLibC_Frame_Type frame_type,
		ttLibC_PointerQueue *queue,
		uint32_t start_pos,
		uint32_t end_pos,
		ttLibC_ContainerWriteFunc callback,
		void *ptr);
*/

bool ttLibC_Pes_writeH264Packet(
		ttLibC_MpegtsTrack *track, // ccの更新で必要
		ttLibC_Frame *frame, // 書き込み対象のデータ
		ttLibC_ContainerWriteFunc callback,
		void *ptr);

bool ttLibC_Pes_writeAudioPacket(
		ttLibC_MpegtsWriter_ *writer,
		ttLibC_MpegtsTrack *track,
//		uint64_t pts,
		ttLibC_ContainerWriteFunc callback,
		void *ptr);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MPEGTS_TYPE_PES_H_ */
