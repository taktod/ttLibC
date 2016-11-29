/**
 * @file   mpegtsWriter.h
 * @brief  mpegts container writer.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2016/11/18
 */

#ifndef TTLIBC_CONTAINER_MPEGTS2_MPEGTSWRITER_H_
#define TTLIBC_CONTAINER_MPEGTS2_MPEGTSWRITER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../mpegts.h"
#include "../misc.h"
#include "../containerCommon.h"
#include "../../util/stlMapUtil.h"
#include "../../util/dynamicBufferUtil.h"

typedef struct ttLibC_MpegtsWriteTrack {
	ttLibC_ContainerWriter_WriteTrack inherit_super;
	uint8_t cc; // continuity counter.
	ttLibC_DynamicBuffer *tmp_buffer; // mpegtsのデータを構築するときに利用する一時buffer
	ttLibC_DynamicBuffer *tmp_frame_buffer; // mpegtsデータを構築するときに一時的に利用するbuffer.
} ttLibC_MpegtsWriteTrack;

typedef struct ttLibC_ContainerWriter_MpegtsWriter_ {
	ttLibC_MpegtsWriter inherit_super;
	uint8_t cc_sdt;
	uint8_t cc_pat;
	uint8_t cc_pmt;
	uint8_t sdt_buf[188];
	uint8_t pat_buf[188];
	uint8_t pmt_buf[188];

	ttLibC_StlMap *track_list;
//	uint32_t pes_track_num; // トラック数の保持ポインタかな？いらないなたぶん。
	bool is_first;
	uint64_t current_pts_pos;
	uint64_t target_pos;
	uint32_t max_unit_duration;

	ttLibC_ContainerWriter_Status status;
	ttLibC_ContainerWriteFunc callback;
	void *ptr;

	bool is_reduce_mode;
	ttLibC_DynamicBuffer *data_buffer; // mpegtsのデータを構築するときに出来上がったデータを保持するための
} ttLibC_ContainerWriter_MpegtsWriter_;

typedef ttLibC_ContainerWriter_MpegtsWriter_ ttLibC_MpegtsWriter_;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MPEGTS2_MPEGTSWRITER_H_ */
