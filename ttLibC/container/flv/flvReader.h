/**
 * @file   flvReader.h
 * @brief  flvTag reader from binary data.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/12
 */

#ifndef TTLIBC_CONTAINER_FLV_FLVREADER_H_
#define TTLIBC_CONTAINER_FLV_FLVREADER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "flvTag.h"
#include "type/audioTag.h"
#include "type/headerTag.h"
#include "type/metaTag.h"
#include "type/videoTag.h"

#include "../../util/dynamicBufferUtil.h"

/**
 * reading data status
 */
typedef enum ttLibC_FlvReader_Status {
	size,
	body
} ttLibC_FlvReader_Status;

/**
 * flvReader detail definition.
 */
typedef struct ttLibC_ContainerReader_FlvReader_ {
	ttLibC_FlvReader inherit_super;

	// reuse for flvTag audioTag videoTag
	ttLibC_FlvTag *flv_tag;
	ttLibC_FlvAudioTag *audio_tag;
	ttLibC_FlvVideoTag *video_tag;

	// reading data information.(need this, cause header data doesn't have size information.)
	ttLibC_Flv_Type type;
	ttLibC_FlvReader_Status status;
	size_t target_size;

	ttLibC_DynamicBuffer *tmp_buffer;
} ttLibC_ContainerReader_FlvReader_;

typedef ttLibC_ContainerReader_FlvReader_ ttLibC_FlvReader_;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_FLV_FLVREADER_H_ */
