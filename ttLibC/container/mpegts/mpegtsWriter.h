/**
 * @file   mpegtsWriter.h
 * @brief  
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

#include "../misc.h"

typedef struct {
	ttLibC_FrameQueue *frame_queue;
	ttLibC_Frame *h264_configData;
	uint8_t cc;
} ttLibC_MpegtsTrack;

typedef struct {
	ttLibC_MpegtsWriter inherit_super;
	uint64_t current_pts_pos;
	uint64_t target_pos;
	uint32_t max_unit_duration;
	uint8_t cc_sdt;
	uint8_t cc_pat;
	uint8_t cc_pmt;
	// bufferを一度作成すれば、あとは巡回カウンターの値だけ変えれば十分なので、ここに実態を保持させておきます。
	uint8_t sdt_buf[188];
	uint8_t pat_buf[188];
	uint8_t pmt_buf[188];
	ttLibC_MpegtsTrack track[MaxPesTracks];
	bool is_first;
} ttLibC_ContainerWriter_MpegtsWriter_;

typedef ttLibC_ContainerWriter_MpegtsWriter_ ttLibC_MpegtsWriter_;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MPEGTS_MPEGTSWRITER_H_ */
