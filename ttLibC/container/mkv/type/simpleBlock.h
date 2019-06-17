/**
 * @file   simpleBlock.h
 * @brief  simple block for mkv container.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/06/30
 */

#ifndef TTLIBC_CONTAINER_MKV_TYPE_SIMPLEBLOCK_H_
#define TTLIBC_CONTAINER_MKV_TYPE_SIMPLEBLOCK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../../../ttLibC_predef.h"
#include "../mkvTag.h"

/**
 * get frame from simple block ebml object.
 */
bool TT_ATTRIBUTE_INNER ttLibC_SimpleBlock_getFrame(
		ttLibC_MkvTag *tag,
		ttLibC_getFrameFunc callback,
		void *ptr);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MKV_TYPE_SIMPLEBLOCK_H_ */
