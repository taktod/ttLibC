/**
 * @file   mp4Atom.h
 * @brief  mp4 container support.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/07/03
 */

#ifndef TTLIBC_CONTAINER_MP4_MP4ATOM_H_
#define TTLIBC_CONTAINER_MP4_MP4ATOM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../mp4.h"
#include "../containerCommon.h"
#include "../../util/dynamicBufferUtil.h"

/**
 * definition of track object.
 */
typedef struct ttLibC_Mp4Track {
	// TODO use enum.
	bool is_video;
	bool is_audio;
	ttLibC_Frame_Type frame_type; // from stsd
	ttLibC_Frame *frame;
	uint32_t size; // size of trak tag. = 0 means done.
	int32_t track_number;

	uint32_t width;
	uint32_t height;
	uint32_t sample_rate;
	uint32_t channel_num;
	uint32_t timebase;
	uint64_t duration;
	uint64_t pts;

	uint64_t dsi_info; // dsi info for aac.
	uint32_t size_length; // size for h264 size nal
	// for vorbis private data.
	ttLibC_DynamicBuffer *private_data;

	// for now, just deal with 1 entry only and rate = 1 only elst information.
	uint32_t elst_mediatime;
//	float elst_mediarate;

	ttLibC_Mp4 *stts;
	ttLibC_Mp4 *stsc;
	ttLibC_Mp4 *stsz;
	ttLibC_Mp4 *stco;
	ttLibC_Mp4 *ctts;
	// co64...

	uint32_t trex_sample_desription_index;
	uint32_t trex_sample_duration;
	uint32_t trex_sample_size;
	uint32_t trex_sample_flags;

	uint64_t tfhd_base_data_offset;
	uint32_t tfhd_sample_desription_index;
	uint32_t tfhd_sample_duration;
	uint32_t tfhd_sample_size;
	uint32_t tfhd_sample_flags;

	uint64_t decode_time_duration;

	ttLibC_Mp4 *trun;
} ttLibC_Mp4Track;

typedef struct ttLibC_Container_Mp4Atom {
	ttLibC_Mp4 inherit_super;
	ttLibC_Mp4Reader *reader;
} ttLibC_Container_Mp4Atom;

typedef ttLibC_Container_Mp4Atom ttLibC_Mp4Atom;

ttLibC_Mp4Atom *ttLibC_Mp4Atom_make(
		ttLibC_Mp4Atom *prev_atom,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		ttLibC_Mp4_Type type);

void ttLibC_Mp4Atom_close(ttLibC_Mp4Atom **atom);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MP4_MP4ATOM_H_ */
