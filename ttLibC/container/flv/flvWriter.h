/**
 * @file   flvWriter.h
 * @brief  flvTag writer to make binary data.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/12
 */

#ifndef TTLIBC_CONTAINER_FLV_FLVWRITER_H_
#define TTLIBC_CONTAINER_FLV_FLVWRITER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../misc.h"

#include "flvTag.h"

/**
 * definition of flv track.
 */
typedef struct ttLibC_FlvTrack {
	ttLibC_FrameQueue *frame_queue;
	ttLibC_Frame_Type frame_type;
	uint32_t crc32; // crc32 value for dsi for aac or configData for h264.
	ttLibC_Frame *configData; // keep sps pps for h264.
} ttLibC_FlvTrack;

/**
 * detail definition of flv writer
 */
typedef struct ttLibC_ContainerWriter_FlvWriter_ {
	ttLibC_FlvWriter inherit_super;
	// flv has at most only 2 track.
	ttLibC_FlvTrack video_track;
	ttLibC_FlvTrack audio_track;
	bool is_first; // for first write, need to write flvHeader.
} ttLibC_ContainerWriter_FlvWriter_;

typedef ttLibC_ContainerWriter_FlvWriter_ ttLibC_FlvWriter_;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_FLV_FLVWRITER_H_ */
