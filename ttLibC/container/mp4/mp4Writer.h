/**
 * @file   mp4Writer.h
 * @brief  
 * @author taktod
 * @date   2016/07/03
 */

#ifndef TTLIBC_CONTAINER_MP4_MP4WRITER_H_
#define TTLIBC_CONTAINER_MP4_MP4WRITER_H_

#ifdef __cplusplus
extern"C" {
#endif

#include "../mp4.h"
#include "../misc.h"
#include "../containerCommon.h"

#include "../../util/dynamicBufferUtil.h"

/**
 * definition of mp4 tracks
 */
typedef struct ttLibC_Mp4WriteTrack {
	ttLibC_ContainerWriter_WriteTrack inherit_super;
	// use chunk writing.
	ttLibC_DynamicBuffer *mdat_buffer; // buffer for mdat.
	uint32_t dataOffsetPosForTrun; // data off set using for trun atom.
} ttLibC_Mp4WriteTrack;

typedef struct ttLibC_ContainerWriter_Mp4Writer_ {
	ttLibC_ContainerWriter_ inherit_super;
	ttLibC_DynamicBuffer   *currentWritingBuffer;
	uint32_t                currentMoofSizePos;
	uint32_t                chunk_counter;
	uint32_t                current_sap_diff; // sap = stream access point 
} ttLibC_ContainerWriter_Mp4Writer_;

typedef ttLibC_ContainerWriter_Mp4Writer_ ttLibC_Mp4Writer_;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MP4_MP4WRITER_H_ */
