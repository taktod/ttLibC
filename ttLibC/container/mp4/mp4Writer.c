/**
 * @file   mp4Writer.c
 * @brief  
 * @author taktod
 * @date   2016/07/03
 */

#include "../mp4.h"
#include "mp4Writer.h"

#include "../../log.h"
#include "../../allocator.h"
#include "../../util/hexUtil.h"
#include "../../util/ioUtil.h"
#include "../container.h"
#include "../containerCommon.h"
#include "../../frame/audio/aac.h"
#include "../../frame/video/h264.h"

#include <stdlib.h>

// just support fmp4 only for now.
ttLibC_Mp4Writer *ttLibC_Mp4Writer_make(
		ttLibC_Frame_Type* target_frame_types,
		uint32_t types_num) {
	return ttLibC_Mp4Writer_make_ex(
			target_frame_types,
			types_num,
			5000); // 5sec for target_unit_duration.
}

ttLibC_Mp4Writer *ttLibC_Mp4Writer_make_ex(
		ttLibC_Frame_Type* target_frame_types,
		uint32_t types_num,
		uint32_t max_unit_duration) {
	ttLibC_Mp4Writer_ *writer = (ttLibC_Mp4Writer_ *)ttLibC_ContainerWriter_make(
			containerType_mp4,
			sizeof(ttLibC_Mp4Writer_),
			1000); // work with 1000 only for now.
	if(writer == NULL) {
		ERR_PRINT("failed to allocate writer.");
		return NULL;
	}
	// setup tracks
	writer->track_list = ttLibC_StlMap_make();
	for(uint32_t i = 0;i < types_num;++ i) {
		ttLibC_Mp4WriteTrack *track = ttLibC_malloc(sizeof(ttLibC_Mp4WriteTrack));
		track->frame_queue     = ttLibC_FrameQueue_make(i + 1, 255);
		track->h26x_configData = NULL;
		track->frame_type      = target_frame_types[i];
		track->mdat_buffer     = NULL;
		track->is_appending    = false;
		track->counter         = 0;
		track->enable_dts      = false;
		track->use_dts         = false;
		ttLibC_StlMap_put(writer->track_list, (void *)(long)(i + 1), (void *)track); // trackId -> track
	}
	writer->inherit_super.inherit_super.timebase = 1000;
	writer->inherit_super.inherit_super.pts = 0;
	writer->inherit_super.enable_dts = false;
	writer->max_unit_duration = max_unit_duration;
	writer->is_first = true;
	writer->current_pts_pos = 0;
	writer->target_pos = 0;
	writer->status = status_init_check;
	writer->chunk_counter = 1;
	return (ttLibC_Mp4Writer *)writer;
}

/**
 * common func for update atom size.
 * @param buffer
 * @param size_pos
 */
static void Mp4Writer_updateSize(
		ttLibC_DynamicBuffer *buffer,
		uint32_t size_pos) {
	uint8_t *b = ttLibC_DynamicBuffer_refData(buffer);
	b += size_pos;
	uint32_t size = ttLibC_DynamicBuffer_refSize(buffer) - size_pos;
	*((uint32_t *)b) = be_uint32_t(size);
}

/**
 * make trex for each track.
 * @param ptr
 * @param key
 * @param item
 */
static bool Mp4Writer_makeTrex(void *ptr, void *key, void *item) {
	(void)key;
	if(item != NULL && ptr != NULL) {
		ttLibC_DynamicBuffer *buffer = (ttLibC_DynamicBuffer *)ptr;
		ttLibC_Mp4WriteTrack *track = (ttLibC_Mp4WriteTrack *)item;
		uint8_t buf[256];
		size_t in_size;
		switch(track->frame_type) {
		case frameType_h264:
			{
				in_size = ttLibC_HexUtil_makeBuffer("00 00 00 20 74 72 65 78 00 00 00 00", buf, 256);
				ttLibC_DynamicBuffer_append(buffer, buf, in_size);
				uint32_t be_track_id = be_uint32_t(track->frame_queue->track_id);
				ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_track_id, 4);
				in_size = ttLibC_HexUtil_makeBuffer("00 00 00 01 00 00 22 1A 00 00 00 00 00 01 00 00", buf, 256);
				ttLibC_DynamicBuffer_append(buffer, buf, in_size);
			}
			break;
		case frameType_aac:
			{
				// aac frame sample num is 1024. = 0x400
				in_size = ttLibC_HexUtil_makeBuffer("00 00 00 20 74 72 65 78 00 00 00 00", buf, 256);
				ttLibC_DynamicBuffer_append(buffer, buf, in_size);
				uint32_t be_track_id = be_uint32_t(track->frame_queue->track_id);
				ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_track_id, 4);
				in_size = ttLibC_HexUtil_makeBuffer("00 00 00 01 00 00 04 00 00 00 00 00 02 00 00 00", buf, 256);
				ttLibC_DynamicBuffer_append(buffer, buf, in_size);
			}
			break;
//		case frameType_mp3:
		default:
			return false;
		}
		return true;
	}
	return false;
}

/**
 * make trak for each track.
 * @param ptr
 * @param key
 * @param item
 */
static bool Mp4Writer_makeTrak(void *ptr, void *key, void *item) {
	(void)key;
	if(ptr != NULL && item != NULL) {
		ttLibC_DynamicBuffer *buffer = (ttLibC_DynamicBuffer *)ptr;
		ttLibC_Mp4WriteTrack *track = (ttLibC_Mp4WriteTrack *)item;
		// setup trak
		size_t in_size;
		uint8_t buf[256];
		uint16_t be_width, be_height;
		ttLibC_Aac *aac = NULL;
		// trak
		uint32_t trakSizePos = ttLibC_DynamicBuffer_refSize(buffer);
		in_size = ttLibC_HexUtil_makeBuffer("00 00 00 00 74 72 61 6B", buf, 256);
		ttLibC_DynamicBuffer_append(buffer, buf, in_size);
			// tkhd
			in_size = ttLibC_HexUtil_makeBuffer("00 00 00 5C 74 6B 68 64 00 00 00 03 00 00 00 00 00 00 00 00", buf, 256);
			ttLibC_DynamicBuffer_append(buffer, buf, in_size);
			uint32_t be_track_id = be_uint32_t(track->frame_queue->track_id);
			ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_track_id, 4);
			switch(track->frame_type) {
			case frameType_h264:
				in_size = ttLibC_HexUtil_makeBuffer("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 40 00 00 00", buf, 256);
				ttLibC_DynamicBuffer_append(buffer, buf, in_size);
				ttLibC_H264 *h264 = (ttLibC_H264 *)track->h26x_configData;
				be_width = be_uint16_t(h264->inherit_super.width);
				ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_width, 2);
				uint16_t zero = 0;
				ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&zero, 2);
				be_height = be_uint16_t(h264->inherit_super.height);
				ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_height, 2);
				ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&zero, 2);
				break;
			case frameType_aac:
				in_size = ttLibC_HexUtil_makeBuffer("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 01 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 40 00 00 00 00 00 00 00 00 00 00 00", buf, 256);
				//                                                                                         [   ] what's alternateGroup?...
				//                                                                                               [   ] this is for volume.
				ttLibC_DynamicBuffer_append(buffer, buf, in_size);
				break;
			default:
				return false;
			}
			switch(track->frame_type) {
			case frameType_h264:
			case frameType_h265:
				// add edts and elst for dts.
				if(track->enable_dts) {
					track->use_dts = true;
					// edtsとelst([duration 0] [mediatime = timebase] [rate 1.0]にする。)
					in_size = ttLibC_HexUtil_makeBuffer("00 00 00 24 65 64 74 73 00 00 00 1C 65 6C 73 74 00 00 00 00 00 00 00 01 00 00 00 00", buf, 256);
					ttLibC_DynamicBuffer_append(buffer, buf, in_size);
					uint32_t be_mediatime = be_uint32_t(track->h26x_configData->timebase);
					ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_mediatime, 4);
					in_size = ttLibC_HexUtil_makeBuffer("00 01 00 00", buf, 256);
					ttLibC_DynamicBuffer_append(buffer, buf, in_size);
				}
				break;
			default:
				break;
			}
			// mdia
			uint32_t mdiaSizePos = ttLibC_DynamicBuffer_refSize(buffer);
			in_size = ttLibC_HexUtil_makeBuffer("00 00 00 00 6D 64 69 61", buf, 256);
			ttLibC_DynamicBuffer_append(buffer, buf, in_size);
				// mdhd
				switch(track->frame_type) {
				case frameType_h264:
					in_size = ttLibC_HexUtil_makeBuffer("00 00 00 20 6D 64 68 64 00 00 00 00 00 00 00 00 00 00 00 00 00 00 03 E8 00 00 00 00 55 C4 00 00", buf, 256);
					ttLibC_DynamicBuffer_append(buffer, buf, in_size);
					break;
				case frameType_aac:
					in_size = ttLibC_HexUtil_makeBuffer("00 00 00 20 6D 64 68 64 00 00 00 00 00 00 00 00 00 00 00 00", buf, 256);
					ttLibC_DynamicBuffer_append(buffer, buf, in_size);
					aac = (ttLibC_Aac *)ttLibC_FrameQueue_ref_first(track->frame_queue);
					uint32_t be_timescale = be_uint32_t(aac->inherit_super.sample_rate);
					ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_timescale, 4);
					in_size = ttLibC_HexUtil_makeBuffer("00 00 00 00 55 C4 00 00", buf, 256);
					ttLibC_DynamicBuffer_append(buffer, buf, in_size);
					break;
				default:
					return false;
				}
				// hdlr
				switch(track->frame_type) {
				case frameType_h264:
					in_size = ttLibC_HexUtil_makeBuffer("00 00 00 21 68 64 6C 72 00 00 00 00 00 00 00 00 76 69 64 65 00 00 00 00 00 00 00 00 00 00 00 00 00", buf, 256);
					break;
				case frameType_aac:
					in_size = ttLibC_HexUtil_makeBuffer("00 00 00 21 68 64 6C 72 00 00 00 00 00 00 00 00 73 6F 75 6E 00 00 00 00 00 00 00 00 00 00 00 00 00", buf, 256);
					break;
				default:
					return false;
				}
				ttLibC_DynamicBuffer_append(buffer, buf, in_size);
				// minf
				uint32_t minfSizePos = ttLibC_DynamicBuffer_refSize(buffer);
				in_size = ttLibC_HexUtil_makeBuffer("00 00 00 00 6D 69 6E 66", buf, 256);
				ttLibC_DynamicBuffer_append(buffer, buf, in_size);
					switch(track->frame_type) {
					case frameType_h264:
						// vmhd
						in_size = ttLibC_HexUtil_makeBuffer("00 00 00 14 76 6D 68 64 00 00 00 01 00 00 00 00 00 00 00 00", buf, 256);
						ttLibC_DynamicBuffer_append(buffer, buf, in_size);
						break;
					case frameType_aac:
						// smhd
						in_size = ttLibC_HexUtil_makeBuffer("00 00 00 10 73 6D 68 64 00 00 00 00 00 00 00 00", buf, 256);
						ttLibC_DynamicBuffer_append(buffer, buf, in_size);
						break;
					default:
						return false;
					}
					// dinf dref url
					in_size = ttLibC_HexUtil_makeBuffer("00 00 00 24 64 69 6E 66 00 00 00 1C 64 72 65 66 00 00 00 00 00 00 00 01 00 00 00 0C 75 72 6C 20 00 00 00 01", buf, 256);
					ttLibC_DynamicBuffer_append(buffer, buf, in_size);
					// stbl
					uint32_t stblSizePos = ttLibC_DynamicBuffer_refSize(buffer);
					in_size = ttLibC_HexUtil_makeBuffer("00 00 00 00 73 74 62 6C", buf, 256);
					ttLibC_DynamicBuffer_append(buffer, buf, in_size);
						// stsd
						uint32_t stsdSizePos = ttLibC_DynamicBuffer_refSize(buffer);
						in_size = ttLibC_HexUtil_makeBuffer("00 00 00 00 73 74 73 64 00 00 00 00 00 00 00 01", buf, 256); // only 1 element for stsd information.
						ttLibC_DynamicBuffer_append(buffer, buf, in_size);
						switch(track->frame_type) {
						case frameType_h264:
							{
								// avc1
								uint32_t avc1SizePos = ttLibC_DynamicBuffer_refSize(buffer);
								in_size = ttLibC_HexUtil_makeBuffer("00 00 00 7C 61 76 63 31 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00", buf, 256);
								ttLibC_DynamicBuffer_append(buffer, buf, in_size);
								ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_width, 2);
								ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_height, 2);
								in_size = ttLibC_HexUtil_makeBuffer("00 48 00 00 00 48 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 18 FF FF", buf, 256);
								ttLibC_DynamicBuffer_append(buffer, buf, in_size);
									// avcC
									uint32_t avcCSizePos = ttLibC_DynamicBuffer_refSize(buffer);
									in_size = ttLibC_HexUtil_makeBuffer("00 00 00 00 61 76 63 43", buf, 256);
									ttLibC_DynamicBuffer_append(buffer, buf, in_size);
									in_size = ttLibC_H264_readAvccTag((ttLibC_H264 *)track->h26x_configData, buf, 256);
									ttLibC_DynamicBuffer_append(buffer, buf, in_size);
									Mp4Writer_updateSize(buffer, avcCSizePos);
								Mp4Writer_updateSize(buffer, avc1SizePos);
							}
							break;
						case frameType_aac:
							{
								// mp4a
								uint32_t mp4aSizePos = ttLibC_DynamicBuffer_refSize(buffer);
								in_size = ttLibC_HexUtil_makeBuffer("00 00 00 00 6D 70 34 61 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 00", buf, 256);
								ttLibC_DynamicBuffer_append(buffer, buf, in_size);
								// channel 2byte
								uint16_t be_channel_num = be_uint16_t(aac->inherit_super.channel_num);
								ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_channel_num, 2);
								// size 0x10 16bit fixed
								// 00 4byte
								in_size = ttLibC_HexUtil_makeBuffer("00 10 00 00 00 00", buf, 256);
								ttLibC_DynamicBuffer_append(buffer, buf, in_size);
								// sampleRate(16.16)
								uint16_t be_sample_rate = be_uint16_t(aac->inherit_super.sample_rate);
								ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_sample_rate, 2);
								in_size = ttLibC_HexUtil_makeBuffer("00 00", buf, 256);
								ttLibC_DynamicBuffer_append(buffer, buf, in_size);
									// esds
									uint32_t esdsSizePos = ttLibC_DynamicBuffer_refSize(buffer);
									in_size = ttLibC_HexUtil_makeBuffer("00 00 00 00 65 73 64 73 00 00 00 00 03", buf, 256);
									ttLibC_DynamicBuffer_append(buffer, buf, in_size);
									uint32_t esTagSizePos = ttLibC_DynamicBuffer_refSize(buffer);
									in_size = ttLibC_HexUtil_makeBuffer("19 00 02 00 04 11 40 15 00 00 00 00 01 77 00 00 01 77 00 05", buf, 256);
									ttLibC_DynamicBuffer_append(buffer, buf, in_size);
									uint8_t aac_buf[4];
									in_size = ttLibC_Aac_readDsiInfo(aac, aac_buf, 4);
									uint8_t sz = (uint8_t)in_size;
									ttLibC_DynamicBuffer_append(buffer, &sz, 1);
									ttLibC_DynamicBuffer_append(buffer, aac_buf, sz);
									in_size = ttLibC_HexUtil_makeBuffer("06 01 02", buf, 256);
									ttLibC_DynamicBuffer_append(buffer, buf, in_size);
									// update esTagSizePos
									uint8_t *b = ttLibC_DynamicBuffer_refData(buffer);
									b += esTagSizePos;
									*b = (ttLibC_DynamicBuffer_refSize(buffer) - esTagSizePos - 1);
									Mp4Writer_updateSize(buffer, esdsSizePos);
								Mp4Writer_updateSize(buffer, mp4aSizePos);
							}
							break;
						default:
							return false;
						}
						Mp4Writer_updateSize(buffer, stsdSizePos);
						// stts
						in_size = ttLibC_HexUtil_makeBuffer("00 00 00 10 73 74 74 73 00 00 00 00 00 00 00 00", buf, 256);
						ttLibC_DynamicBuffer_append(buffer, buf, in_size);
						// stsc
						in_size = ttLibC_HexUtil_makeBuffer("00 00 00 10 73 74 73 63 00 00 00 00 00 00 00 00", buf, 256);
						ttLibC_DynamicBuffer_append(buffer, buf, in_size);
						// stsz
						in_size = ttLibC_HexUtil_makeBuffer("00 00 00 14 73 74 73 7A 00 00 00 00 00 00 00 00 00 00 00 00", buf, 256);
						ttLibC_DynamicBuffer_append(buffer, buf, in_size);
						// stco
						in_size = ttLibC_HexUtil_makeBuffer("00 00 00 10 73 74 63 6F 00 00 00 00 00 00 00 00", buf, 256);
						ttLibC_DynamicBuffer_append(buffer, buf, in_size);
					Mp4Writer_updateSize(buffer, stblSizePos);
				Mp4Writer_updateSize(buffer, minfSizePos);
			Mp4Writer_updateSize(buffer, mdiaSizePos);
		Mp4Writer_updateSize(buffer, trakSizePos);
		return true;
	}
	return false;
}

/**
 * make init.mp4 data.(ftyp moov)
 */
static bool Mp4Writer_makeInitMp4(ttLibC_Mp4Writer_ *writer) {
	// 初期のinit.mp4を作成する。
	ttLibC_DynamicBuffer *buffer = ttLibC_DynamicBuffer_make();
	if(buffer == NULL) {
		ERR_PRINT("failed to make buffer.");
		return false;
	}
	uint8_t *b = NULL;
	uint8_t buf[256];
	// ftyp
	size_t in_size;
	in_size = ttLibC_HexUtil_makeBuffer("00 00 00 1C 66 74 79 70 69 73 6F 35 00 00 00 01 61 76 63 31 69 73 6F 35 64 61 73 68", buf, 256);
	ttLibC_DynamicBuffer_append(buffer, buf, in_size);
	// moov
	uint32_t moovSizePos = ttLibC_DynamicBuffer_refSize(buffer);
	in_size = ttLibC_HexUtil_makeBuffer("00 00 00 00 6D 6F 6F 76", buf, 256);
	ttLibC_DynamicBuffer_append(buffer, buf, in_size);
		// mvhd
		in_size = ttLibC_HexUtil_makeBuffer("00 00 00 6C 6D 76 68 64 00 00 00 00 00 00 00 00 00 00 00 00 00 00 03 E8 00 00 00 00 00 01 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 40 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 03", buf, 256);
		ttLibC_DynamicBuffer_append(buffer, buf, in_size);
		// mvex
		uint32_t mvexSizePos = ttLibC_DynamicBuffer_refSize(buffer);
		in_size = ttLibC_HexUtil_makeBuffer("00 00 00 00 6D 76 65 78", buf, 256);
		ttLibC_DynamicBuffer_append(buffer, buf, in_size);
			// trex
			ttLibC_StlMap_forEach(writer->track_list, Mp4Writer_makeTrex, buffer);
		Mp4Writer_updateSize(buffer, mvexSizePos);
		// trak
		ttLibC_StlMap_forEach(writer->track_list, Mp4Writer_makeTrak, buffer);
	Mp4Writer_updateSize(buffer, moovSizePos);
	// ready, write now.
	bool result = true;
	if(writer->callback != NULL) {
		result = writer->callback(writer->ptr, ttLibC_DynamicBuffer_refData(buffer), ttLibC_DynamicBuffer_refSize(buffer));
	}
	ttLibC_DynamicBuffer_close(&buffer);
	return result;
}

/**
 * make traf information for track
 * @param ptr
 * @param key
 * @param item
 */
static bool Mp4Writer_makeTraf(void *ptr, void *key, void *item) {
	(void)key;
	if(ptr == NULL || item == NULL) {
		return false;
	}
	ttLibC_Mp4Writer_ *writer = (ttLibC_Mp4Writer_ *)ptr;
	ttLibC_DynamicBuffer *buffer = writer->currentWritingBuffer;
	ttLibC_Mp4WriteTrack *track = (ttLibC_Mp4WriteTrack *)item;
	uint8_t *b = NULL;
	uint8_t buf[256];
	uint32_t in_size;
	if(track->mdat_buffer == NULL) {
		track->mdat_buffer = ttLibC_DynamicBuffer_make();
	}
	else {
		// reuse
		ttLibC_DynamicBuffer_empty(track->mdat_buffer);
	}
	// traf
	uint32_t trafSizePos = ttLibC_DynamicBuffer_refSize(buffer);
	in_size = ttLibC_HexUtil_makeBuffer("00 00 00 00 74 72 61 66", buf, 256);
	ttLibC_DynamicBuffer_append(buffer, buf, in_size);
		// tfhd
		in_size = ttLibC_HexUtil_makeBuffer("00 00 00 10 74 66 68 64 00 02 00 00", buf, 256);
		ttLibC_DynamicBuffer_append(buffer, buf, in_size);
		uint32_t be_track_id = be_uint32_t(track->frame_queue->track_id);
		ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_track_id, 4);
		// tfdt timestamp
		in_size = ttLibC_HexUtil_makeBuffer("00 00 00 10 74 66 64 74 00 00 00 00", buf, 256);
		ttLibC_DynamicBuffer_append(buffer, buf, in_size);
		// just put first frame pts information.
		ttLibC_Frame *first_frame = ttLibC_FrameQueue_ref_first(track->frame_queue);
		uint32_t be_timediff = be_uint32_t((uint32_t)first_frame->pts);
		ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_timediff, 4);

		// trun
		// video305 sz duration fsfp dop
		// audio201 sz dop
		// 32bit count
		// has dop, dop(pos from moof atom.) 32bit
		// has fsfp, flags 32bit
		// has duration, duration 32bit
		// has size, data size 32bit
		switch(track->frame_type) {
		case frameType_h264:
			{
				uint32_t trunSizePos = ttLibC_DynamicBuffer_refSize(buffer);
				if(track->use_dts) {
					in_size = ttLibC_HexUtil_makeBuffer("00 00 00 00 74 72 75 6E 00 00 0B 05 00 00 00 00 00 00 00 00 00 00 00 00", buf, 256);
				}
				else {
					in_size = ttLibC_HexUtil_makeBuffer("00 00 00 00 74 72 75 6E 00 00 03 05 00 00 00 00 00 00 00 00 00 00 00 00", buf, 256);
				}
				ttLibC_DynamicBuffer_append(buffer, buf, in_size);
				// count, update later.
				uint32_t frameCountPos = ttLibC_DynamicBuffer_refSize(buffer) - 12;
				// dop, update later.
				track->dataOffsetPosForTrun = frameCountPos + 4;
				uint32_t frameCount = 0;
				while(true) {
					ttLibC_H264 *h264 = (ttLibC_H264 *)ttLibC_FrameQueue_ref_first(track->frame_queue);
					if(h264 == NULL) {
						break;
					}
					if(h264->inherit_super.inherit_super.dts < writer->target_pos) {
						ttLibC_H264 *h = (ttLibC_H264 *)ttLibC_FrameQueue_dequeue_first(track->frame_queue);
						if(h264 != h) {
							ERR_PRINT("ref frame is invalid.");
							return false;
						}
						// get next frame to get duration of frame.
						ttLibC_Frame *next_frame = ttLibC_FrameQueue_ref_first(track->frame_queue);
						uint32_t duration = next_frame->dts - h264->inherit_super.inherit_super.dts;
						uint32_t be_duration = be_uint32_t(duration);
						ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_duration, 4);

						// put data on mdat_buffer.
						uint8_t *h264_data = h264->inherit_super.inherit_super.data;
						size_t h264_data_size = h264->inherit_super.inherit_super.buffer_size;
						ttLibC_H264_NalInfo nal_info;
						uint32_t h264_size = 0;
						while(ttLibC_H264_getNalInfo(&nal_info, h264_data, h264_data_size)) {
							uint32_t nal_size = nal_info.nal_size - nal_info.data_pos;
							h264_size += 4 + nal_size;
							uint32_t be_nal_size = be_uint32_t(nal_size);
							ttLibC_DynamicBuffer_append(track->mdat_buffer, (uint8_t *)&be_nal_size, 4);
							ttLibC_DynamicBuffer_append(track->mdat_buffer, h264_data + nal_info.data_pos, nal_size);
							h264_data += nal_info.nal_size;
							h264_data_size -= nal_info.nal_size;
						}
						// set size on trun.
						uint32_t be_h264_size = be_uint32_t(h264_size);
						ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_h264_size, 4);
						if(track->use_dts) {
							// get offset to store.
							uint32_t offset = h264->inherit_super.inherit_super.pts - h264->inherit_super.inherit_super.dts + h264->inherit_super.inherit_super.timebase;
							uint32_t be_offset = be_uint32_t(offset);
							ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_offset, 4);
						}
//						ttLibC_H264_close(&h264); // frameQueue will handle to release, DON'T close.
						++ frameCount;
					}
					else {
						break;
					}
				}
				// update frame count.
				b = ttLibC_DynamicBuffer_refData(buffer);
				b += frameCountPos;
				*((uint32_t *)b) = be_uint32_t(frameCount);
				Mp4Writer_updateSize(buffer, trunSizePos);
			}
			break;
		case frameType_aac:
			{
				uint32_t trunSizePos = ttLibC_DynamicBuffer_refSize(buffer);
				in_size = ttLibC_HexUtil_makeBuffer("00 00 00 00 74 72 75 6E 00 00 02 01 00 00 00 00 00 00 00 00", buf, 256);
				ttLibC_DynamicBuffer_append(buffer, buf, in_size);
				uint32_t frameCountPos = ttLibC_DynamicBuffer_refSize(buffer) - 8;
				track->dataOffsetPosForTrun = frameCountPos + 4;
				uint32_t frameCount = 0;
				uint64_t target_pts = 0;
				while(true) {
					ttLibC_Aac *aac = (ttLibC_Aac *)ttLibC_FrameQueue_ref_first(track->frame_queue);
					if(aac == NULL) {
						break;
					}
					if(target_pts == 0) {
						target_pts = (uint64_t)(1.0 * writer->target_pos * aac->inherit_super.inherit_super.timebase / 1000);
					}
					if(aac->inherit_super.inherit_super.pts < target_pts) {
						ttLibC_Aac *a = (ttLibC_Aac *)ttLibC_FrameQueue_dequeue_first(track->frame_queue);
						if(aac != a) {
							ERR_PRINT("ref frame is invalid.");
							return false;
						}
						if(aac->type != AacType_dsi) {
							uint8_t *aac_data = aac->inherit_super.inherit_super.data;
							uint32_t aac_size = aac->inherit_super.inherit_super.buffer_size;
							if(aac->type == AacType_adts) {
								aac_data += 7;
								aac_size -= 7;
							}
							ttLibC_DynamicBuffer_append(track->mdat_buffer, aac_data, aac_size);
							uint32_t be_aac_size = be_uint32_t(aac_size);
							ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_aac_size, 4);
							++ frameCount;
						}
//						ttLibC_Aac_close(&aac); // frameQueue will handle to release, DON'T close.
					}
					else {
						break;
					}
				}
				b = ttLibC_DynamicBuffer_refData(buffer);
				b += frameCountPos;
				*((uint32_t *)b) = be_uint32_t(frameCount);
				Mp4Writer_updateSize(buffer, trunSizePos);
			}
			break;
		default:
			return false;
		}
	Mp4Writer_updateSize(buffer, trafSizePos);
	return true;
}

/**
 * update mdat information for each tracks
 * @param ptr
 * @param key
 * @param item
 */
static bool Mp4Writer_makeMdat(void *ptr, void *key, void *item) {
	(void)key;
	if(ptr == NULL || item == NULL) {
		return false;
	}
	ttLibC_Mp4Writer_ *writer = (ttLibC_Mp4Writer_ *)ptr;
	ttLibC_DynamicBuffer *buffer = writer->currentWritingBuffer;
	ttLibC_Mp4WriteTrack *track = (ttLibC_Mp4WriteTrack *)item;
	uint8_t *b = ttLibC_DynamicBuffer_refData(buffer);
	b += track->dataOffsetPosForTrun;
	*((uint32_t *)b) = be_uint32_t((ttLibC_DynamicBuffer_refSize(buffer) - writer->currentMoofSizePos));
	ttLibC_DynamicBuffer_append(buffer, ttLibC_DynamicBuffer_refData(track->mdat_buffer), ttLibC_DynamicBuffer_refSize(track->mdat_buffer));
	return true;
}

/**
 * make data chunk.
 * @param writer
 */
static bool Mp4Writer_makeData(ttLibC_Mp4Writer_ *writer) {
	// styp, sidx, moof, mdat is required.
	ttLibC_DynamicBuffer *buffer = ttLibC_DynamicBuffer_make();
	uint8_t *b = NULL;
	uint8_t buf[256];
	writer->currentWritingBuffer = buffer;

	// styp
	size_t in_size = ttLibC_HexUtil_makeBuffer("00 00 00 18 73 74 79 70 6D 73 64 68 00 00 00 00 6D 73 64 68 6D 73 69 78", buf, 256);
	ttLibC_DynamicBuffer_append(buffer, buf, in_size);
	// sidx
	in_size = ttLibC_HexUtil_makeBuffer("00 00 00 2C 73 69 64 78 00 00 00 00 00 00 00 01 00 00 03 E8", buf, 256);
	ttLibC_DynamicBuffer_append(buffer, buf, in_size);
	uint32_t earlistPts = be_uint32_t(writer->current_pts_pos);
	ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&earlistPts, 4);
	in_size = ttLibC_HexUtil_makeBuffer("00 00 00 00 00 00 00 01", buf, 256);
	ttLibC_DynamicBuffer_append(buffer, buf, in_size);
	// size data will be update later.(moof + mdat)
	uint32_t sidx_inner_sizePos = ttLibC_DynamicBuffer_refSize(buffer);
	in_size = ttLibC_HexUtil_makeBuffer("00 00 00 00", buf, 256);
	ttLibC_DynamicBuffer_append(buffer, buf, in_size);
	uint32_t duration_diff = writer->target_pos - writer->current_pts_pos;
	uint32_t be_duration = be_uint32_t(duration_diff);
	ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_duration, 4);
	in_size = ttLibC_HexUtil_makeBuffer("90 00 00 00", buf, 256);
	ttLibC_DynamicBuffer_append(buffer, buf, in_size);
	// moof
	uint32_t moofSizePos = ttLibC_DynamicBuffer_refSize(buffer);
	writer->currentMoofSizePos = moofSizePos;
	in_size = ttLibC_HexUtil_makeBuffer("00 00 00 00 6D 6F 6F 66", buf, 256);
	ttLibC_DynamicBuffer_append(buffer, buf, in_size);
		// mfhd
		in_size = ttLibC_HexUtil_makeBuffer("00 00 00 10 6D 66 68 64 00 00 00 00", buf, 256);
		ttLibC_DynamicBuffer_append(buffer, buf, in_size);
		uint32_t be_counter = be_uint32_t(writer->chunk_counter);
		ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_counter, 4);
		// traf
		ttLibC_StlMap_forEach(writer->track_list, Mp4Writer_makeTraf, writer);
	Mp4Writer_updateSize(buffer, moofSizePos);
	// mdat
	uint32_t mdatSizePos = ttLibC_DynamicBuffer_refSize(buffer);
	in_size = ttLibC_HexUtil_makeBuffer("00 00 00 00 6D 64 61 74", buf, 256);
	ttLibC_DynamicBuffer_append(buffer, buf, in_size);
	// now, we put mdat_buffer and update trun dop information.
	ttLibC_StlMap_forEach(writer->track_list, Mp4Writer_makeMdat, writer);
	Mp4Writer_updateSize(buffer, mdatSizePos);
	// now ready to update sidx size data.
	b = ttLibC_DynamicBuffer_refData(buffer);
	b += sidx_inner_sizePos;
	*((uint32_t *)b) = be_uint32_t((ttLibC_DynamicBuffer_refSize(buffer) - moofSizePos));
	// done.

	bool result = true;
	if(writer->callback != NULL) {
		result = writer->callback(writer->ptr, ttLibC_DynamicBuffer_refData(buffer), ttLibC_DynamicBuffer_refSize(buffer));
	}
	ttLibC_DynamicBuffer_close(&buffer);
	writer->currentWritingBuffer = NULL;
	return result;
}

/**
 * check if track has enough information to make init.mp4.
 * @param ptr
 * @param key
 * @param item
 */
static bool Mp4Writer_initCheckTrack(void *ptr, void *key, void *item) {
	(void)key;
	(void)ptr;
	if(item == NULL) {
		return false;
	}
	ttLibC_Mp4WriteTrack *track = (ttLibC_Mp4WriteTrack *)item;
	switch(track->frame_type) {
	case frameType_h264:
		{
			// for h264 need configData.
			return track->h26x_configData != NULL;
		}
	default:
		{
			// for others need at least 1 frame.
			return ttLibC_FrameQueue_ref_first(track->frame_queue) != NULL;
		}
	}
	return true;
}

/**
 * check primary h264 track for target pos for chunk.
 */
static bool Mp4Writer_PrimaryH264TrackCheck(void *ptr, ttLibC_Frame *frame) {
	ttLibC_Mp4Writer_ *writer = (ttLibC_Mp4Writer_ *)ptr;
	ttLibC_H264 *h264 = (ttLibC_H264 *)frame;
	switch(h264->type) {
	case H264Type_slice:
		// exceed max_unit_duration.
		if(h264->inherit_super.inherit_super.pts - writer->current_pts_pos > writer->max_unit_duration) {
			writer->target_pos = writer->current_pts_pos + writer->max_unit_duration;
			return false;
		}
		return true;
	case H264Type_sliceIDR:
		// if sliceIDR is too far away, use max_unit_duration.
		if(writer->current_pts_pos + writer->max_unit_duration < h264->inherit_super.inherit_super.pts) {
			writer->target_pos = writer->current_pts_pos + writer->max_unit_duration;
			return false;
		}
		// find next sliceIDR.
		if(writer->current_pts_pos != h264->inherit_super.inherit_super.pts) {
			writer->target_pos = h264->inherit_super.inherit_super.pts;
			return false;
		}
		return true;
	default:
		return true;
	}
}

/**
 * check data chunk writing for each track.
 * @param ptr
 * @param key
 * @param item
 */
static bool Mp4Writer_dataCheckTrack(void *ptr, void *key, void *item) {
	(void)key;
	if(ptr != NULL && item != NULL) {
		ttLibC_Mp4Writer_ *writer = (ttLibC_Mp4Writer_ *)ptr;
		ttLibC_Mp4WriteTrack *track = (ttLibC_Mp4WriteTrack *)item;
		 // for audio we need to change timebase into 1000.
		uint64_t pts = (uint64_t)(1.0 * track->frame_queue->pts * 1000 / track->frame_queue->timebase);
		if(writer->target_pos > pts) {
			return false;
		}
		return true;
	}
	return false;
}

/**
 * write from queued data.
 */
static bool Mp4Writer_writeFromQueue(
		ttLibC_Mp4Writer_ *writer) {
	switch(writer->status) {
	case status_init_check:
		{
			if(ttLibC_StlMap_forEach(writer->track_list, Mp4Writer_initCheckTrack, NULL)) {
				// now ready to make init.mp4
				writer->status = status_make_init;
				return Mp4Writer_writeFromQueue(writer);
			}
		}
		break;
	case status_make_init:
		{
			if(Mp4Writer_makeInitMp4(writer)) {
				// now ready to make chunk.
				writer->status = status_target_check;
				return Mp4Writer_writeFromQueue(writer);
			}
			else {
				// TODO update.
				ERR_PRINT("something fatal is happen. update later.");
				return false;
			}
		}
		break;
	case status_target_check:
		{
			// check 1st track to decide target_pos.
			ttLibC_Mp4WriteTrack *track = (ttLibC_Mp4WriteTrack *)ttLibC_StlMap_get(writer->track_list, (void *)1);
			switch(track->frame_type) {
			case frameType_h264:
				ttLibC_FrameQueue_ref(track->frame_queue, Mp4Writer_PrimaryH264TrackCheck, writer);
				break;
			case frameType_aac:
				writer->target_pos = writer->current_pts_pos + writer->max_unit_duration;
				break;
			default:
				ERR_PRINT("unexpected frame is found.");
				return false;
			}
			if(writer->target_pos != writer->current_pts_pos) {
				// check each track.
				writer->status = status_data_check;
				return Mp4Writer_writeFromQueue(writer);
			}
		}
		break;
	case status_data_check:
		{
			if(ttLibC_StlMap_forEach(writer->track_list, Mp4Writer_dataCheckTrack, writer)) {
				// all track is ok.
				writer->status = status_make_data; // make chunk data.
				return Mp4Writer_writeFromQueue(writer);
			}
		}
		break;
	case status_make_data:
		{
			if(Mp4Writer_makeData(writer)) {
				// write done.
				writer->status = status_update; // update information for next chunk.
				return Mp4Writer_writeFromQueue(writer);
			}
		}
		break;
	case status_update:
		{
			writer->current_pts_pos = writer->target_pos;
			writer->status = status_target_check;
			++ writer->chunk_counter;
//			return Mp4Writer_writeFromQueue(writer);
		}
		break;
	default:
		break;
	}
	return true;
}

static bool Mp4Writer_appendQueue(
		ttLibC_Mp4WriteTrack *track,
		ttLibC_Frame *frame,
		uint64_t pts) {
	uint64_t original_pts = frame->pts;
	uint32_t original_timebase = frame->timebase;
	frame->pts = pts;
	frame->timebase = 1000;
	bool result = ttLibC_FrameQueue_queue(track->frame_queue, frame);
	frame->pts = original_pts;
	frame->timebase = original_timebase;
	return result;
}

bool ttLibC_Mp4Writer_write(
		ttLibC_Mp4Writer *writer,
		ttLibC_Frame *frame,
		ttLibC_ContainerWriteFunc callback,
		void *ptr) {
	ttLibC_Mp4Writer_ *writer_ = (ttLibC_Mp4Writer_ *)writer;
	if(writer_ == NULL) {
		ERR_PRINT("writer is null.");
		return false;
	}
	if(frame == NULL) {
		return true;
	}
	ttLibC_Mp4WriteTrack *track = (ttLibC_Mp4WriteTrack *)ttLibC_StlMap_get(writer_->track_list, (void *)(long)frame->id);
	if(track == NULL) {
		ERR_PRINT("failed to get correspond track. %d", frame->id);
		return false;
	}
	uint64_t pts = (uint64_t)(1.0 * frame->pts * 1000 / frame->timebase);
	track->enable_dts = writer->enable_dts;
	// for first access.
	switch(frame->type) {
	case frameType_h264:
		{
			ttLibC_H264 *h264 = (ttLibC_H264 *)frame;
			if(h264->type == H264Type_unknown) {
				return true;
			}
			if(h264->type == H264Type_configData) {
				ttLibC_H264 *h = ttLibC_H264_clone(
						(ttLibC_H264 *)track->h26x_configData,
						h264);
				if(h == NULL) {
					ERR_PRINT("failed to make clone data.");
					return false;
				}
				h->inherit_super.inherit_super.pts = 0;
				h->inherit_super.inherit_super.timebase = 1000;
				track->h26x_configData = (ttLibC_Frame *)h;
				return true;
			}
			if(!track->is_appending && h264->type != H264Type_sliceIDR) {
				// no data in queue, and not sliceIDR -> not yet to append.
				return true;
			}
		}
		break;
	default:
		break;
	}
	track->is_appending = true;
	if(writer_->is_first) {
		writer_->current_pts_pos = pts;
		writer_->target_pos = pts;
		writer_->inherit_super.inherit_super.pts = pts;
		writer_->is_first = false;
	}
	if(!Mp4Writer_appendQueue(track, frame, pts)) {
		return false;
	}
	writer_->callback = callback;
	writer_->ptr = ptr;
	return Mp4Writer_writeFromQueue(writer_);
}

/**
 * close each tracks
 * for closing writer.
 * @param ptr
 * @param key
 * @param item
 */
static bool Mp4Writer_closeTracks(void *ptr, void *key, void *item) {
	(void)ptr;
	(void)key;
	if(item != NULL) {
		ttLibC_Mp4WriteTrack *track = (ttLibC_Mp4WriteTrack *)item;
		ttLibC_FrameQueue_close(&track->frame_queue);
		ttLibC_Frame_close(&track->h26x_configData);
		ttLibC_DynamicBuffer_close(&track->mdat_buffer);
		ttLibC_free(track);
	}
	return true;
}

/**
 * close writer
 * @param writer
 */
void ttLibC_Mp4Writer_close(ttLibC_Mp4Writer **writer) {
	ttLibC_Mp4Writer_ *target = (ttLibC_Mp4Writer_ *)*writer;
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.inherit_super.type != containerType_mp4) {
		ERR_PRINT("try to close non Mp4Wrtier.");
		return;
	}
	ttLibC_StlMap_forEach(target->track_list, Mp4Writer_closeTracks, NULL);
	ttLibC_StlMap_close(&target->track_list);
	ttLibC_free(target);
	*writer = NULL;
}

