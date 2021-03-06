/**
 * @file   mpegtsReader.h
 * @brief  mpegts container reader.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2016/11/18
 */

#ifndef TTLIBC_CONTAINER_MPEGTS_MPEGTSREADER_H_
#define TTLIBC_CONTAINER_MPEGTS_MPEGTSREADER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mpegtsPacket.h"

#include "type/pat.h"
#include "type/pes.h"
#include "type/pmt.h"
#include "type/sdt.h"

#include "../../util/dynamicBufferUtil.h"
#include "../../util/stlMapUtil.h"

/**
 * detail definition of mpegtsReader
 */
typedef struct ttLibC_ContainerReader_MpegtsReader_ {
	ttLibC_MpegtsReader inherit_super;

	// re-use container objects.
	ttLibC_Pat *pat;
	ttLibC_Pmt *pmt;
	ttLibC_Sdt *sdt;

	ttLibC_StlMap *pes_list;

	size_t target_size; // read_size is 188 fixed.
	uint16_t pmt_pid;

	ttLibC_DynamicBuffer *tmp_buffer;
	bool is_reading;
} ttLibC_ContainerReader_MpegtsReader_;

typedef ttLibC_ContainerReader_MpegtsReader_ ttLibC_MpegtsReader_;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MPEGTS_MPEGTSREADER_H_ */
