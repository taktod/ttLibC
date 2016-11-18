/**
 * @file   mpegtsWriter.h
 * @brief  mpegts container writer.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/13
 */

#ifndef TTLIBC_CONTAINER_MPEGTS_MPEGTSWRITER_H_
#define TTLIBC_CONTAINER_MPEGTS_MPEGTSWRITER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../mpegts.h"
#include "../containerCommon.h"

#include "../misc.h"

/**
 * definition of mpegts tracks
 */
typedef struct ttLibC_MpegtsTrack {
	ttLibC_FrameQueue *frame_queue;
	ttLibC_Frame *h264_configData;
	uint8_t cc; // continuity counter.
	ttLibC_Frame_Type frame_type;
	bool enable_dts;
	bool use_dts;
} ttLibC_MpegtsTrack;

/**
 * detail definition of mpegts writer.
 */
typedef struct {
	ttLibC_MpegtsWriter inherit_super;

	uint8_t cc_sdt;
	uint8_t cc_pat;
	uint8_t cc_pmt;

	uint8_t sdt_buf[188];
	uint8_t pat_buf[188];
	uint8_t pmt_buf[188];

	ttLibC_MpegtsTrack *track_list;
	uint32_t pes_track_num;

	bool is_first;
	uint64_t current_pts_pos;
	uint64_t target_pos;
	uint32_t max_unit_duration; // max for audio frame chunk.

	ttLibC_ContainerWriter_Status status;

	ttLibC_ContainerWriteFunc callback;
	void *ptr;

	bool is_reduce_mode;
} ttLibC_ContainerWriter_MpegtsWriter_;

typedef ttLibC_ContainerWriter_MpegtsWriter_ ttLibC_MpegtsWriter_;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MPEGTS_MPEGTSWRITER_H_ */
