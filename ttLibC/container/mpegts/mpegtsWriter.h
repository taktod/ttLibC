/**
 * @file   mpegtsWriter.h
 * @brief  mpegts container writer.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2016/11/18
 */

#ifndef TTLIBC_CONTAINER_MPEGTS_MPEGTSWRITER_H_
#define TTLIBC_CONTAINER_MPEGTS_MPEGTSWRITER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../mpegts.h"
#include "../misc.h"
#include "../containerCommon.h"
#include "../../util/dynamicBufferUtil.h"

typedef struct ttLibC_MpegtsWriteTrack {
	ttLibC_ContainerWriter_WriteTrack inherit_super;
	uint8_t cc; // continuity counter.
	ttLibC_DynamicBuffer *tmp_buffer;
	ttLibC_DynamicBuffer *tmp_frame_buffer;
} ttLibC_MpegtsWriteTrack;

typedef struct ttLibC_ContainerWriter_MpegtsWriter_ {
	ttLibC_ContainerWriter_ inherit_super;
	uint8_t cc_sdt;
	uint8_t cc_pat;
	uint8_t cc_pmt;
	uint8_t sdt_buf[188];
	uint8_t pat_buf[188];
	uint8_t pmt_buf[188];
	bool is_reduce_mode;
	ttLibC_DynamicBuffer *data_buffer;
	uint64_t dts_margin;
} ttLibC_ContainerWriter_MpegtsWriter_;

typedef ttLibC_ContainerWriter_MpegtsWriter_ ttLibC_MpegtsWriter_;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MPEGTS_MPEGTSWRITER_H_ */
