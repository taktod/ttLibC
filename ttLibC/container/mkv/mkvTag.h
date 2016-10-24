/**
 * @file   mkvTag.h
 * @brief  mkv container support.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/09/04
 */

#ifndef TTLIBC_CONTAINER_MKV_MKVTAG_H_
#define TTLIBC_CONTAINER_MKV_MKVTAG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../mkv.h"
#include "../containerCommon.h"

/**
 * track information.
 * move to mkvTag, cause we need to use mkvTrack for reader and writer.
 */
typedef struct ttLibC_MkvTrack {
	bool is_video;
	int32_t  track_number;
	ttLibC_Frame_Type type;
	uint32_t size; // size of tracks. = 0 means done.
	uint8_t *private_data;
	size_t private_data_size;
	bool is_lacing;

	uint32_t width;
	uint32_t height;

	uint32_t sample_rate;
	uint32_t channel_num;

	uint64_t dsi_info; // dsi_info for aac

	uint32_t size_length; // for h264 / h265 size nal.

	ttLibC_Frame *frame;
} ttLibC_MkvTrack;

typedef struct ttLibC_Container_MkvTag {
	ttLibC_Mkv inherit_super;
	ttLibC_MkvReader *reader;
} ttLibC_Container_MkvTag;

typedef ttLibC_Container_MkvTag ttLibC_MkvTag;

ttLibC_MkvTag *ttLibC_MkvTag_make(
		ttLibC_MkvTag *prev_tag,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		ttLibC_Mkv_Type type);

/**
 * analyze frames in private data.
 * @param reader
 * @param track
 * @param callback
 * @param ptr
 * @note in the case of first reply of simple block, we will return private data information.
 * this code will be move to mkvTag.c
 */
void ttLibC_MkvTag_getPrivateDataFrame(
		ttLibC_MkvReader *reader,
		ttLibC_MkvTrack *track,
		ttLibC_getFrameFunc callback,
		void *ptr);

void ttLibC_MkvTag_close(ttLibC_MkvTag **tag);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MKV_MKVTAG_H_ */
