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

#include "../../util/stlMapUtil.h"
#include "../../util/dynamicBufferUtil.h"

/**
 * definition of mp4 tracks
 */
typedef struct ttLibC_Mp4Track {
	ttLibC_FrameQueue *frame_queue;
	ttLibC_Frame *h264_configData;
	ttLibC_Frame_Type frame_type;

	// use chunk writing.
	ttLibC_DynamicBuffer *mdat_buffer; // buffer for mdat.
	uint32_t dataOffsetPosForTrun; // data off set using for trun atom.
} ttLibC_Mp4Track;

/**
 * detail definition of mp4 writer.
 */
typedef struct ttLibC_ContainerWriter_Mp4Writer_ {
	ttLibC_Mp4Writer inherit_super;

	ttLibC_StlMap *track_list;

	ttLibC_ContainerWriter_Status status;
	ttLibC_ContainerWriteFunc callback;
	void *ptr;
	uint32_t max_unit_duration;

	bool is_first;
	uint64_t current_pts_pos; // written data pts.
	uint64_t target_pos; // chunk target pts.
	uint32_t chunk_counter; // chunk counter for mfhd atom.

	// using for data writing.
	ttLibC_DynamicBuffer *currentWritingBuffer;
	uint32_t currentMoofSizePos;
} ttLibC_ContainerWriter_Mp4Writer_;

typedef ttLibC_ContainerWriter_Mp4Writer_ ttLibC_Mp4Writer_;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MP4_MP4WRITER_H_ */
