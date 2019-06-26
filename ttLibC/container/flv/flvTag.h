/**
 * @file   flvTag.h
 * @brief  flvTag container.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/12
 */

#ifndef TTLIBC_CONTAINER_FLV_FLVTAG_H_
#define TTLIBC_CONTAINER_FLV_FLVTAG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../../ttLibC_predef.h"
#include "../flv.h"
#include "../containerCommon.h"
#include "../../util/flvFrameUtil.h"

/**
 * detail definition of flvTag
 */
typedef struct ttLibC_Container_FlvTag {
	ttLibC_Flv inherit_super;
	uint32_t track_id; // 1
	ttLibC_FlvFrameManager *frameManager; // reference of frameManager from flvReader.
} ttLibC_Container_FlvTag;

typedef ttLibC_Container_FlvTag ttLibC_FlvTag;

/**
 * make flvTag container
 * @param prev_frame
 * @param data
 * @param data_size
 * @param non_copy_mode
 * @param pts
 * @param timebase
 * @param type
 * @param track_id
 */
ttLibC_FlvTag TT_ATTRIBUTE_INNER *ttLibC_FlvTag_make(
		ttLibC_FlvTag *prev_tag,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		ttLibC_Flv_Type type,
		uint32_t track_id);

void TT_ATTRIBUTE_INNER ttLibC_FlvTag_close(ttLibC_FlvTag **tag);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_FLV_FLVTAG_H_ */
