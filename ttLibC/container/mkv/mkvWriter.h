/**
 * @file   mkvWriter.h
 * @brief  mkv container writer.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/09/04
 */

#ifndef TTLIBC_CONTAINER_MKV_MKVWRITER_H_
#define TTLIBC_CONTAINER_MKV_MKVWRITER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../mkv.h"
#include "mkvTag.h"

#include "../misc.h"

typedef struct ttLibC_ContainerWriter_MkvWriter_ {
	ttLibC_MkvWriter inherit_super;
} ttLibC_ContainerWriter_MkvWriter_;

typedef ttLibC_ContainerWriter_MkvWriter_ ttLibC_MkvWriter_;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MKV_MKVWRITER_H_ */
