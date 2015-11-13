/**
 * @file   headerTag.h
 * @brief  
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/12
 */

#ifndef TTLIBC_CONTAINER_FLV_TYPE_HEADERTAG_H_
#define TTLIBC_CONTAINER_FLV_TYPE_HEADERTAG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../flvTag.h"
#include "../flvWriter.h"

typedef struct {
	ttLibC_FlvTag inherit_super;
	bool has_audio;
	bool has_video;
} ttLibC_Container_Flv_FlvHeaderTag;

typedef ttLibC_Container_Flv_FlvHeaderTag ttLibC_FlvHeaderTag;

// いるかこの処理？
ttLibC_FlvHeaderTag *ttLibC_FlvHeaderTag_make(
		ttLibC_FlvTag *prev_tag,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		uint32_t track_id,
		bool has_audio,
		bool has_video);

ttLibC_FlvHeaderTag *ttLibC_FlvHeaderTag_getTag(
		ttLibC_FlvTag *prev_tag,
		uint8_t *data,
		size_t data_size);

bool ttLibC_FlvHeaderTag_writeTag(
		ttLibC_FlvWriter_ *writer,
		ttLibC_ContainerWriteFunc callback,
		void *ptr);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_FLV_TYPE_HEADERTAG_H_ */
