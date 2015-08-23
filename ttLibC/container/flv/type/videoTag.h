/**
 * @file   videoTag.h
 * @brief  
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/12
 */

#ifndef TTLIBC_CONTAINER_FLV_TYPE_VIDEOTAG_H_
#define TTLIBC_CONTAINER_FLV_TYPE_VIDEOTAG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../flvTag.h"
#include "../flvWriter.h"

typedef struct {
	ttLibC_FlvTag inherit_super;
	ttLibC_Frame_Type frame_type;
	uint8_t codec_id;
	uint32_t h264_length_size;
} ttLibC_Container_Flv_FlvVideoTag;

typedef ttLibC_Container_Flv_FlvVideoTag ttLibC_FlvVideoTag;

ttLibC_FlvVideoTag *ttLibC_FlvVideoTag_make(
		ttLibC_FlvTag *prev_tag,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		uint32_t track_id,
		uint8_t codec_id);

ttLibC_FlvVideoTag *ttLibC_FlvVideoTag_getTag(
		ttLibC_FlvTag *prev_tag,
		uint8_t *data,
		size_t data_size);

bool ttLibC_FlvVideoTag_getFrame(
		ttLibC_FlvVideoTag *video_tag,
		ttLibC_getFrameFunc callback,
		void *ptr);

bool ttLibC_FlvVideoTag_writeTag(
		ttLibC_FlvWriter_ *writer,
		ttLibC_Frame *frame,
		ttLibC_ContainerWriterFunc callback,
		void *ptr);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_FLV_TYPE_VIDEOTAG_H_ */
