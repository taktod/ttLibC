/**
 * @file   mp4Reader.h
 * @brief  mp4 container reader.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/07/03
 */

#ifndef TTLIBC_CONTAINER_MP4_MP4READER_H_
#define TTLIBC_CONTAINER_MP4_MP4READER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../mp4.h"
#include "../../util/dynamicBufferUtil.h"
#include "../../util/stlMapUtil.h"

#include "mp4Atom.h"

// memo
//  mp4(ffmpeg) ftyp mdat moov
//  mp4(MP4Box) ftyp moov mdat
//  fmp4 ftyp moov sidx moof mdat moof mdat...
//  fmp4 [ftyp moov] [styp moof mdat] [styp moof mdat]...

/**
 * detail definition of mp4Reader
 */
typedef struct ttLibC_ContainerReader_Mp4Reader_ {
	ttLibC_Mp4Reader inherit_super;
	uint32_t error_number;
	ttLibC_StlMap *tracks;
	ttLibC_Mp4Track *track; // tmp pointer for track object.
	bool in_reading;
	ttLibC_DynamicBuffer *tmp_buffer;

	ttLibC_Mp4Atom *atom;
	uint32_t timebase;
	uint64_t duration;
	ttLibC_DynamicBuffer *mdat_buffer; // buffer for mdat. use for mp4 type ffmpeg.
	size_t mdat_start_pos; // start position for mdat data, need for stco co64, and trun.

	size_t moof_position; // position for moofAtom.
	uint64_t position; // position for next mp4Atom.

	// callback and ptr data for getFrame func.
	ttLibC_getFrameFunc callback;
	void *ptr;

	// for fmp4.
	ttLibC_Mp4 *mvex;
	bool is_fmp4;
} ttLibC_ContainerReader_Mp4Reader_;

typedef ttLibC_ContainerReader_Mp4Reader_ ttLibC_Mp4Reader_;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MP4_MP4READER_H_ */
