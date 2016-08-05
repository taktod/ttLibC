/**
 * @file   mp4Atom.c
 * @brief  mp4 container support.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/07/03
 */

#include "mp4Atom.h"
#include "mp4Reader.h"
#include "../../log.h"
#include "../../allocator.h"
#include "type/stts.h"
#include "type/stco.h"
#include "type/stsc.h"
#include "type/stsz.h"
#include "type/trun.h"
#include "../../util/hexUtil.h"
#include "../../util/byteUtil.h"
#include "../../frame/video/h264.h"
#include "../../frame/audio/aac.h"

ttLibC_Mp4Atom *ttLibC_Mp4Atom_make(
		ttLibC_Mp4Atom *prev_atom,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		ttLibC_Mp4_Type type) {
	ttLibC_Mp4Atom *atom = (ttLibC_Mp4Atom *)ttLibC_Container_make(
			(ttLibC_Container *)prev_atom,
			sizeof(union{
				ttLibC_Mp4Atom atom;
				ttLibC_Stts stts;
				ttLibC_Stco stco;
				ttLibC_Stsc stsc;
				ttLibC_Stsz stsz;
				ttLibC_Trun trun;
			}),
			containerType_mp4,
			data,
			data_size,
			non_copy_mode,
			pts,
			timebase);
	if(atom != NULL) {
		atom->inherit_super.type = type;
	}
	return atom;
}

// in the case of data is available.
static bool Mp4Atom_getFrame(
		ttLibC_Mp4Track *track,
		uint8_t *data,
		size_t data_size,
		uint64_t pts,
		uint32_t timebase,
		uint32_t duration,
		ttLibC_getFrameFunc callback,
		void *ptr) {
	switch(track->frame_type) {
	case frameType_h264:
		{
			uint8_t *buf = data;
			size_t buf_size = data_size;
			do {
				uint32_t size = 0;
				for(int i = 1;i <= track->size_length;++ i) {
					size = (size << 8) | *buf;
					if(i != track->size_length) {
						*buf = 0x00;
					}
					else {
						*buf = 0x01;
					}
					++ buf;
					-- buf_size;
				}
				buf += size;
				buf_size -= size;
			} while(buf_size > 0);
			ttLibC_H264 *h264 = ttLibC_H264_getFrame(
					(ttLibC_H264 *)track->frame,
					data,
					data_size,
					true,
					pts,
					timebase);
			if(h264 == NULL) {
				ERR_PRINT("failed to make h264 data.");
				return false;
			}
			track->frame = (ttLibC_Frame *)h264;
			if(callback != NULL) {
				if(!callback(ptr, track->frame)) {
					return false;
				}
			}
		}
		break;
	case frameType_aac:
		{
			ttLibC_Aac *aac = ttLibC_Aac_make(
					(ttLibC_Aac *)track->frame,
					AacType_raw,
					track->sample_rate,
					duration,
					track->channel_num,
					data,
					data_size,
					true,
					pts,
					timebase,
					track->dsi_info);
			if(aac == NULL) {
				ERR_PRINT("failed to make aac data.");
				return false;
			}
			track->frame = (ttLibC_Frame *)aac;
			if(callback != NULL) {
				if(!callback(ptr, track->frame)) {
					return false;
				}
			}
		}
		break;
	default:
		break;
	}
	return true;
}

static bool Mp4Atom_getTrackFrame(
		uint8_t *mdat_data,
		size_t mdat_data_size,
		ttLibC_Mp4Reader_ *reader,
		ttLibC_Mp4Track *track,
		ttLibC_getFrameFunc callback,
		void *ptr) {
	while(true) {
		uint32_t currentPos = ttLibC_Stco_refOffset(track->stco);
		uint32_t nextPos = ttLibC_Stco_refNextOffset(track->stco);
		if(currentPos == 0) {
			// stco data is done, no more chunk.
			break;
		}
		// ... should I change the way?
		if(nextPos == 0) {
			// current chunk is the last one.
		}
		else {
			// have enough data for start of next chunk = have enough data for this chunk.
			if(mdat_data_size < nextPos - reader->mdat_start_pos) {
				// maybe need more data. do next time.
				return true;
			}
		}
		uint32_t sample_count = ttLibC_Stsc_refChunkSampleNum(track->stsc);
		for(int i = 0;i < sample_count;++ i) {
			uint32_t sample_size = ttLibC_Stsz_refCurrentSampleSize(track->stsz);
			uint64_t pts = ttLibC_Stts_refCurrentPts(track->stts);
			uint32_t duration = ttLibC_Stts_refCurrentDelta(track->stts);

			if(!Mp4Atom_getFrame(track, mdat_data + currentPos - reader->mdat_start_pos, sample_size, pts, track->timebase, duration, callback, ptr)) {
				reader->error_number = 5;
				// quit the loop.
				return false;
			}
			ttLibC_Stts_moveNext(track->stts); // prepare next sample time information
			ttLibC_Stsz_moveNext(track->stsz); // prepare next sample size
			currentPos += sample_size;
		}
		// go next chunk.
		ttLibC_Stsc_moveNext(track->stsc); // prepare next chunk sample count.
		ttLibC_Stco_moveNext(track->stco); // prepare next chunk
	}
	return true;
}

// for mp4(MP4Box) ftyp moov -> mdat.
static bool Mp4Atom_getMdatFrameCallback(
		void *ptr,
		void *key,
		void *item) {
	ttLibC_Mp4Track *track = (ttLibC_Mp4Track *)item;
	ttLibC_Mp4Atom *mp4Atom = (ttLibC_Mp4Atom *)ptr;
	if(track == NULL || mp4Atom == NULL || mp4Atom->inherit_super.type != Mp4Type_Mdat) {
		return false;
	}
	ttLibC_Mp4Reader_ *reader = (ttLibC_Mp4Reader_ *)mp4Atom->reader;
	if(!Mp4Atom_getTrackFrame(
			mp4Atom->inherit_super.inherit_super.data,
			mp4Atom->inherit_super.inherit_super.buffer_size,
			reader,
			track,
			reader->callback,
			reader->ptr)) {
		reader->error_number = 51;
	}
	return true;
}

// for fmp4 ftyp moov moof mdat moof mdat....
static bool Mp4Atom_getFmp4FrameCallback(
		void *ptr,
		void *key,
		void *item) {
	ttLibC_Mp4Track *track = (ttLibC_Mp4Track *)item;
	ttLibC_Mp4Atom *mp4Atom = (ttLibC_Mp4Atom *)ptr;
	if(track == NULL || mp4Atom == NULL || mp4Atom->inherit_super.type != Mp4Type_Mdat) {
		return false;
	}
	ttLibC_Mp4Reader_ *reader = (ttLibC_Mp4Reader_ *)mp4Atom->reader;
	if(reader->error_number != 0) {
		return false;
	}
	uint8_t *mdat_buffer = mp4Atom->inherit_super.inherit_super.data;
	uint64_t pos, pts;
	uint32_t size, duration;
	do {
		pts = ttLibC_Trun_refCurrentPts(track->trun);
		duration = ttLibC_Trun_refCurrentDelta(track->trun);
		pos = ttLibC_Trun_refCurrentPos(track->trun);
		size = ttLibC_Trun_refCurrentSize(track->trun);
		if(!Mp4Atom_getFrame(
				track,
				mdat_buffer + pos - reader->mdat_start_pos,
				size,
				pts,
				track->timebase,
				duration,
				reader->callback,
				reader->ptr)) {
			reader->error_number = 5;
			return false;
		}
	} while(ttLibC_Trun_moveNext(track->trun));
	return true;
}

static bool Mp4Atom_analyzeStsd(
		ttLibC_Mp4Atom *mp4Atom,
		ttLibC_getFrameFunc callback,
		void *ptr) {
	ttLibC_Mp4Reader_ *reader = (ttLibC_Mp4Reader_ *)mp4Atom->reader;
	ttLibC_Mp4Track *track = reader->track;
	if(track == NULL) {
		ERR_PRINT("track object is missing. unexpected.");
		return false;
	}
	switch(track->frame_type) {
	case frameType_h264:
		{
			ttLibC_ByteReader *byte_reader = ttLibC_ByteReader_make(mp4Atom->inherit_super.inherit_super.data, mp4Atom->inherit_super.inherit_super.buffer_size, ByteUtilType_default);
			ttLibC_ByteReader_skipByte(byte_reader, 12);
			if(ttLibC_ByteReader_bit(byte_reader, 32) != 1) {
				ERR_PRINT("only 1 entry count is expected.");
				ttLibC_ByteReader_close(&byte_reader);
				return false;
			}
			ttLibC_ByteReader_skipByte(byte_reader, 4);
			uint32_t in_tag = ttLibC_ByteReader_bit(byte_reader, 32);
			if(in_tag != 'avc1') {
				ERR_PRINT("expected to have avc1 atom for h264.");
				ttLibC_ByteReader_close(&byte_reader);
				return false;
			}
			ttLibC_ByteReader_skipByte(byte_reader, 78);
			uint32_t in_size = ttLibC_ByteReader_bit(byte_reader, 32);
			in_tag = ttLibC_ByteReader_bit(byte_reader, 32);
			if(in_tag != 'avcC') {
				ERR_PRINT("avcC is expected.");
				ttLibC_ByteReader_close(&byte_reader);
				return false;
			}
			uint8_t *data = mp4Atom->inherit_super.inherit_super.data;
			data += byte_reader->read_size;

			// try to make h264->configData.
			uint32_t size_length;
			ttLibC_H264 *h264 = ttLibC_H264_analyzeAvccTag(
					(ttLibC_H264 *)track->frame,
					data,
					in_size - 8,
					&size_length);
			if(h264 != NULL) {
				track->size_length = size_length;
				h264->inherit_super.inherit_super.pts = 0;
				h264->inherit_super.inherit_super.timebase = track->timebase;
				track->frame = (ttLibC_Frame *)h264;
				if(callback != NULL) {
					if(!callback(ptr, track->frame)) {
						ttLibC_ByteReader_close(&byte_reader);
						reader->error_number = 7;
						return false;
					}
				}
			}
			ttLibC_ByteReader_close(&byte_reader);
		}
		break;
	case frameType_aac:
		return true;
	default:
		ERR_PRINT("unknown frame:%d", track->frame_type);
		break;
	}
	return true;
}

bool ttLibC_Mp4_getFrame(
		ttLibC_Mp4 *mp4,
		ttLibC_getFrameFunc callback,
		void *ptr) {
	ttLibC_Mp4Atom *mp4Atom = (ttLibC_Mp4Atom *)mp4;
	ttLibC_Mp4Reader_ *reader = (ttLibC_Mp4Reader_ *)mp4Atom->reader;
	if(mp4Atom->inherit_super.type == Mp4Type_Stsd) {
		// get special frame from stsd data.
		// h264 configData
		// vorbis identification, comment, setup frame.
		//  and so on...
		if(!Mp4Atom_analyzeStsd(mp4Atom, callback, ptr)) {
			reader->error_number = 6;
			return false;
		}
	}
	// fmp4
	if(reader->is_fmp4
	&& mp4->type == Mp4Type_Mdat) {
		if(!mp4->is_complete) {
			return true;
		}
		reader->callback = callback;
		reader->ptr = ptr;
		ttLibC_StlMap_forEach(reader->tracks, Mp4Atom_getFmp4FrameCallback, mp4);
		reader->callback = NULL;
		reader->ptr = NULL;
		return true;
	}
	// mp4(MP4Box)
	if(reader->tracks->size != 0
	&& mp4->type == Mp4Type_Mdat) {
		reader->callback = callback;
		reader->ptr = ptr;
		ttLibC_StlMap_forEach(reader->tracks, Mp4Atom_getMdatFrameCallback, mp4);
		reader->callback = NULL;
		reader->ptr = NULL;
		return reader->error_number == 0;
	}
	// mp4(ffmpeg)
	if(reader->mdat_buffer != NULL
	&& reader->track != NULL
	&& reader->track->size == 0) {
		return Mp4Atom_getTrackFrame(
				ttLibC_DynamicBuffer_refData(reader->mdat_buffer),
				ttLibC_DynamicBuffer_refSize(reader->mdat_buffer),
				reader,
				reader->track,
				callback,
				ptr);
	}
	return true;
}

void ttLibC_Mp4Atom_close(ttLibC_Mp4Atom **atom) {
	ttLibC_Mp4Atom *target = *atom;
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.inherit_super.type != containerType_mp4) {
		ERR_PRINT("container type is not mp4");
		return;
	}
	if(!target->inherit_super.inherit_super.is_non_copy) {
		ttLibC_free(target->inherit_super.inherit_super.data);
	}
	ttLibC_free(target);
	*atom = NULL;
}
