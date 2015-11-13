/**
 * @file   metaTag.h
 * @brief  flvTag for meta.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/12
 */

#ifndef TTLIBC_CONTAINER_FLV_TYPE_METATAG_H_
#define TTLIBC_CONTAINER_FLV_TYPE_METATAG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../flvTag.h"

typedef struct {
	ttLibC_FlvTag inherit_super;
	// meta data is just ignore for now.
} ttLibC_Container_Flv_FlvMetaTag;

typedef ttLibC_Container_Flv_FlvMetaTag ttLibC_FlvMetaTag;

ttLibC_FlvMetaTag *ttLibC_FlvMetaTag_make(
		ttLibC_FlvTag *prev_tag,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		uint32_t track_id);

ttLibC_FlvMetaTag *ttLibC_FlvMetaTag_getTag(
		ttLibC_FlvTag *prev_tag,
		uint8_t *data,
		size_t data_size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_FLV_TYPE_METATAG_H_ */
