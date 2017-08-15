/**
 * @file   mp4Writer.c
 * @brief  
 * @author taktod
 * @date   2016/07/03
 */

#include "../mp4.h"
#include "mp4Writer.h"

#include "../../ttLibC_predef.h"
#include "../../_log.h"
#include "../../allocator.h"
#include "../../util/hexUtil.h"
#include "../../util/ioUtil.h"
#include "../container.h"
#include "../containerCommon.h"
#include "../../frame/audio/audio.h"
#include "../../frame/audio/aac.h"
#include "../../frame/audio/mp3.h"
#include "../../frame/audio/vorbis.h"
#include "../../frame/video/video.h"
#include "../../frame/video/h264.h"
#include "../../frame/video/h265.h"
#include "../../frame/video/jpeg.h"

#include <stdlib.h>

// just support fmp4 only for now.
ttLibC_Mp4Writer TT_VISIBILITY_DEFAULT *ttLibC_Mp4Writer_make(
		ttLibC_Frame_Type* target_frame_types,
		uint32_t types_num) {
	return ttLibC_Mp4Writer_make_ex(
			target_frame_types,
			types_num,
			5000); // 5sec for target_unit_duration.
}

ttLibC_Mp4Writer TT_VISIBILITY_DEFAULT *ttLibC_Mp4Writer_make_ex(
		ttLibC_Frame_Type* target_frame_types,
		uint32_t types_num,
		uint32_t unit_duration) {
	ttLibC_Mp4Writer_ *writer = (ttLibC_Mp4Writer_ *)ttLibC_ContainerWriter_make_(
			containerType_mp4,
			sizeof(ttLibC_Mp4Writer_),
			1000,
			sizeof(ttLibC_Mp4WriteTrack),
			1,
			target_frame_types,
			types_num,
			unit_duration);
	if(writer == NULL) {
		return NULL;
	}
	writer->chunk_counter = 1;
	writer->currentMoofSizePos = 0;
	writer->currentWritingBuffer = NULL;
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
		ttLibC_ContainerWriter_WriteTrack *track = (ttLibC_ContainerWriter_WriteTrack *)item;
		uint8_t buf[256];
		size_t in_size;
		switch(track->frame_type) {
		case frameType_h264:
		case frameType_h265:
			{
				in_size = ttLibC_HexUtil_makeBuffer("00 00 00 20 74 72 65 78 00 00 00 00", buf, 256);
				ttLibC_DynamicBuffer_append(buffer, buf, in_size);
				uint32_t be_track_id = be_uint32_t(track->frame_queue->track_id);
				ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_track_id, 4);
				in_size = ttLibC_HexUtil_makeBuffer("00 00 00 01 00 00 00 00 00 00 00 00 00 01 00 00", buf, 256);
				ttLibC_DynamicBuffer_append(buffer, buf, in_size);
			}
			break;
		case frameType_jpeg:
			{
				in_size = ttLibC_HexUtil_makeBuffer("00 00 00 20 74 72 65 78 00 00 00 00", buf, 256);
				ttLibC_DynamicBuffer_append(buffer, buf, in_size);
				uint32_t be_track_id = be_uint32_t(track->frame_queue->track_id);
				ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_track_id, 4);
				in_size = ttLibC_HexUtil_makeBuffer("00 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00", buf, 256);
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
		case frameType_mp3:
			{
				// use 1152 for default duration. this should be change according to frame.
				in_size = ttLibC_HexUtil_makeBuffer("00 00 00 20 74 72 65 78 00 00 00 00", buf, 256);
				ttLibC_DynamicBuffer_append(buffer, buf, in_size);
				uint32_t be_track_id = be_uint32_t(track->frame_queue->track_id);
				ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_track_id, 4);
				in_size = ttLibC_HexUtil_makeBuffer("00 00 00 01 00 00 04 80 00 00 00 00 02 00 00 00", buf, 256);
				ttLibC_DynamicBuffer_append(buffer, buf, in_size);
			}
			break;
		case frameType_vorbis:
			{
				// vorvis frame is not has constant duration.
				in_size = ttLibC_HexUtil_makeBuffer("00 00 00 20 74 72 65 78 00 00 00 00", buf, 256);
				ttLibC_DynamicBuffer_append(buffer, buf, in_size);
				uint32_t be_track_id = be_uint32_t(track->frame_queue->track_id);
				ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_track_id, 4);
//				in_size = ttLibC_HexUtil_makeBuffer("00 00 00 01 00 00 04 00 00 00 00 00 02 00 00 00", buf, 256);
				in_size = ttLibC_HexUtil_makeBuffer("00 00 00 01 00 00 00 00 00 00 00 00 02 00 00 00", buf, 256);
				ttLibC_DynamicBuffer_append(buffer, buf, in_size);
			}
			break;
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
		ttLibC_ContainerWriter_WriteTrack *track = (ttLibC_ContainerWriter_WriteTrack *)item;
		track->use_mode = track->enable_mode;
		// setup trak
		size_t in_size;
		uint8_t buf[256];
		uint16_t be_width, be_height;
		ttLibC_Audio *audio = NULL;
		ttLibC_Video *video = NULL;
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
			case frameType_h265:
			case frameType_jpeg:
				{
					in_size = ttLibC_HexUtil_makeBuffer("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 40 00 00 00", buf, 256);
					ttLibC_DynamicBuffer_append(buffer, buf, in_size);
					if(track->h26x_configData != NULL) {
						video = (ttLibC_Video *)track->h26x_configData;
					}
					else {
						video = (ttLibC_Video *)ttLibC_FrameQueue_ref_first(track->frame_queue);
					}
					be_width = be_uint16_t(video->width);
					ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_width, 2);
					uint16_t zero = 0;
					ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&zero, 2);
					be_height = be_uint16_t(video->height);
					ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_height, 2);
					ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&zero, 2);
				}
				break;
			case frameType_aac:
			case frameType_mp3:
			case frameType_vorbis:
				in_size = ttLibC_HexUtil_makeBuffer("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 01 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 40 00 00 00 00 00 00 00 00 00 00 00", buf, 256);
				ttLibC_DynamicBuffer_append(buffer, buf, in_size);
				break;
			default:
				return false;
			}
			// add edts and elst for dts.
			switch(track->frame_type) {
			case frameType_h264:
			case frameType_h265:
				if((track->use_mode & containerWriter_enable_dts) != 0) {
					// edts elst([duration 0] [mediatime = timebase] [rate 1.0])
					in_size = ttLibC_HexUtil_makeBuffer("00 00 00 24 65 64 74 73 00 00 00 1C 65 6C 73 74 00 00 00 00 00 00 00 01 00 00 00 00", buf, 256);
					ttLibC_DynamicBuffer_append(buffer, buf, in_size);
					uint32_t be_mediatime = be_uint32_t(track->h26x_configData->timebase / 5);
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
				case frameType_h265:
				case frameType_jpeg:
					in_size = ttLibC_HexUtil_makeBuffer("00 00 00 20 6D 64 68 64 00 00 00 00 00 00 00 00 00 00 00 00 00 00 03 E8 00 00 00 00 55 C4 00 00", buf, 256);
					ttLibC_DynamicBuffer_append(buffer, buf, in_size);
					break;
				case frameType_aac:
				case frameType_mp3:
				case frameType_vorbis:
					{
						in_size = ttLibC_HexUtil_makeBuffer("00 00 00 20 6D 64 68 64 00 00 00 00 00 00 00 00 00 00 00 00", buf, 256);
						ttLibC_DynamicBuffer_append(buffer, buf, in_size);
						audio = (ttLibC_Audio *)ttLibC_FrameQueue_ref_first(track->frame_queue);
						uint32_t be_timescale = be_uint32_t(audio->sample_rate);
						ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_timescale, 4);
						in_size = ttLibC_HexUtil_makeBuffer("00 00 00 00 55 C4 00 00", buf, 256);
						ttLibC_DynamicBuffer_append(buffer, buf, in_size);
					}
					break;
				default:
					return false;
				}
				// hdlr
				switch(track->frame_type) {
				case frameType_h264:
				case frameType_h265:
				case frameType_jpeg:
					in_size = ttLibC_HexUtil_makeBuffer("00 00 00 21 68 64 6C 72 00 00 00 00 00 00 00 00 76 69 64 65 00 00 00 00 00 00 00 00 00 00 00 00 00", buf, 256);
					break;
				case frameType_aac:
				case frameType_mp3:
				case frameType_vorbis:
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
					case frameType_h265:
					case frameType_jpeg:
						// vmhd
						in_size = ttLibC_HexUtil_makeBuffer("00 00 00 14 76 6D 68 64 00 00 00 01 00 00 00 00 00 00 00 00", buf, 256);
						ttLibC_DynamicBuffer_append(buffer, buf, in_size);
						break;
					case frameType_aac:
					case frameType_mp3:
					case frameType_vorbis:
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
									in_size = ttLibC_H264_readAvccTag((ttLibC_H264 *)video, buf, 256);
									ttLibC_DynamicBuffer_append(buffer, buf, in_size);
									Mp4Writer_updateSize(buffer, avcCSizePos);
								Mp4Writer_updateSize(buffer, avc1SizePos);
							}
							break;
						case frameType_h265:
							{
								// hev1
								uint32_t hev1SizePos = ttLibC_DynamicBuffer_refSize(buffer);
								in_size = ttLibC_HexUtil_makeBuffer("00 00 04 B4 68 65 76 31 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00", buf, 256);
								ttLibC_DynamicBuffer_append(buffer, buf, in_size);
								ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_width, 2);
								ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_height, 2);
								in_size = ttLibC_HexUtil_makeBuffer("00 48 00 00 00 48 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 18 FF FF", buf, 256);
								ttLibC_DynamicBuffer_append(buffer, buf, in_size);
									// hvcC
									uint32_t hvcCSizePos = ttLibC_DynamicBuffer_refSize(buffer);
									in_size = ttLibC_HexUtil_makeBuffer("00 00 00 00 68 76 63 43", buf, 256);
									ttLibC_DynamicBuffer_append(buffer, buf, in_size);
									in_size = ttLibC_H265_readHvccTag((ttLibC_H265 *)video, buf, 256);
									ttLibC_DynamicBuffer_append(buffer, buf, in_size);
									Mp4Writer_updateSize(buffer, hvcCSizePos);
								Mp4Writer_updateSize(buffer, hev1SizePos);
							}
							break;
						case frameType_jpeg:
							{
								// mp4v
								uint32_t mp4vSizePos = ttLibC_DynamicBuffer_refSize(buffer);
								in_size = ttLibC_HexUtil_makeBuffer("00 00 00 79 6D 70 34 76 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00", buf, 256);
								ttLibC_DynamicBuffer_append(buffer, buf, in_size);
								ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_width, 2);
								ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_height, 2);
								// esds is leave as constant.(actually we have to update ave bps, max bps, leave as 1mbps)
								in_size = ttLibC_HexUtil_makeBuffer("00 48 00 00 00 48 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 18 FF FF 00 00 00 23 65 73 64 73 00 00 00 00 03 15 00 01 00 04 0D 6C 11 00 00 00 00 1E 84 80 00 1E 84 80 06 01 02", buf, 256);
								ttLibC_DynamicBuffer_append(buffer, buf, in_size);
								Mp4Writer_updateSize(buffer, mp4vSizePos);
							}
							break;
						case frameType_aac:
							{
								// mp4a
								uint32_t mp4aSizePos = ttLibC_DynamicBuffer_refSize(buffer);
								in_size = ttLibC_HexUtil_makeBuffer("00 00 00 00 6D 70 34 61 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 00", buf, 256);
								ttLibC_DynamicBuffer_append(buffer, buf, in_size);
								// channel 2byte
								uint16_t be_channel_num = be_uint16_t(audio->channel_num);
								ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_channel_num, 2);
								// size 0x10 16bit fixed
								// 00 4byte
								in_size = ttLibC_HexUtil_makeBuffer("00 10 00 00 00 00", buf, 256);
								ttLibC_DynamicBuffer_append(buffer, buf, in_size);
								// sampleRate(16.16)
								uint16_t be_sample_rate = be_uint16_t(audio->sample_rate);
								ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_sample_rate, 2);
								in_size = ttLibC_HexUtil_makeBuffer("00 00", buf, 256);
								ttLibC_DynamicBuffer_append(buffer, buf, in_size);
									// esds
									uint32_t esdsSizePos = ttLibC_DynamicBuffer_refSize(buffer);
									// ugly code.
									// 00 00 00 00 65 73 64 73 00 00 00 00
									// 03 sz 00 02 00
									// 04 sz 40(aac) 15(flag) 00 00 00  00 01 77 00  00 01 77 00(00 01 77 00 = 96000bps)
									// 05 sz 12 10(dsi)
									// 06 01 02
									in_size = ttLibC_HexUtil_makeBuffer("00 00 00 00 65 73 64 73 00 00 00 00 03", buf, 256);
									ttLibC_DynamicBuffer_append(buffer, buf, in_size);
									uint32_t esTagSizePos = ttLibC_DynamicBuffer_refSize(buffer);
									in_size = ttLibC_HexUtil_makeBuffer("19 00 00 00 04 11 40 15 00 00 00 00 01 77 00 00 01 77 00 05", buf, 256);
									ttLibC_DynamicBuffer_append(buffer, buf, in_size);
									uint8_t aac_buf[4];
									in_size = ttLibC_Aac_readDsiInfo((ttLibC_Aac *)audio, aac_buf, 4);
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
						case frameType_mp3:
							{
								// mp4a
								uint32_t mp4aSizePos = ttLibC_DynamicBuffer_refSize(buffer);
								in_size = ttLibC_HexUtil_makeBuffer("00 00 00 00 6D 70 34 61 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 00", buf, 256);
								ttLibC_DynamicBuffer_append(buffer, buf, in_size);
								// channel 2byte
								uint16_t be_channel_num = be_uint16_t(audio->channel_num);
								ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_channel_num, 2);
								// size 0x10 16bit fixed
								// 00 4byte
								in_size = ttLibC_HexUtil_makeBuffer("00 10 00 00 00 00", buf, 256);
								ttLibC_DynamicBuffer_append(buffer, buf, in_size);
								// sampleRate(16.16)
								uint16_t be_sample_rate = be_uint16_t(audio->sample_rate);
								ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_sample_rate, 2);
								in_size = ttLibC_HexUtil_makeBuffer("00 00", buf, 256);
								ttLibC_DynamicBuffer_append(buffer, buf, in_size);
									// esds
									uint32_t esdsSizePos = ttLibC_DynamicBuffer_refSize(buffer);
									// esds is constant(in some case, we need to update bitrate ave.bitrate.)
									in_size = ttLibC_HexUtil_makeBuffer("00 00 00 00 65 73 64 73 00 00 00 00 03", buf, 256);
									ttLibC_DynamicBuffer_append(buffer, buf, in_size);
									in_size = ttLibC_HexUtil_makeBuffer("15 00 00 00 04 0D 6B 15 00 00 00 00 01 77 00 00 01 77 00 06 01 02", buf, 256);
									ttLibC_DynamicBuffer_append(buffer, buf, in_size);
									Mp4Writer_updateSize(buffer, esdsSizePos);
								Mp4Writer_updateSize(buffer, mp4aSizePos);
							}
							break;
						case frameType_vorbis:
							{
								// mp4a
								uint32_t mp4aSizePos = ttLibC_DynamicBuffer_refSize(buffer);
								in_size = ttLibC_HexUtil_makeBuffer("00 00 00 00 6D 70 34 61 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 00", buf, 256);
								ttLibC_DynamicBuffer_append(buffer, buf, in_size);
								// channel 2byte
								uint16_t be_channel_num = be_uint16_t(audio->channel_num);
								ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_channel_num, 2);
								// size 0x10 16bit fixed
								// 00 4byte
								in_size = ttLibC_HexUtil_makeBuffer("00 10 00 00 00 00", buf, 256);
								ttLibC_DynamicBuffer_append(buffer, buf, in_size);
								// sampleRate(16.16)
								uint16_t be_sample_rate = be_uint16_t(audio->sample_rate);
								ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_sample_rate, 2);
								in_size = ttLibC_HexUtil_makeBuffer("00 00", buf, 256);
								ttLibC_DynamicBuffer_append(buffer, buf, in_size);
									// esds
									uint32_t esdsSizePos = ttLibC_DynamicBuffer_refSize(buffer);
									in_size = ttLibC_HexUtil_makeBuffer("00 00 0F 96 65 73 64 73 00 00 00 00", buf, 256);
									ttLibC_DynamicBuffer_append(buffer, buf, in_size);
									// prepare codecPrivate buffer.
									ttLibC_Vorbis *identificationFrame = (ttLibC_Vorbis *)ttLibC_FrameQueue_dequeue_first(track->frame_queue);
									ttLibC_Vorbis *commentFrame        = (ttLibC_Vorbis *)ttLibC_FrameQueue_dequeue_first(track->frame_queue);
									ttLibC_Vorbis *setupFrame          = (ttLibC_Vorbis *)ttLibC_FrameQueue_dequeue_first(track->frame_queue);
									ttLibC_DynamicBuffer *vorbisSpecificBuffer = ttLibC_DynamicBuffer_make();
									uint8_t vsb_buf[3];
									vsb_buf[0] = 2;
									vsb_buf[1] = identificationFrame->inherit_super.inherit_super.buffer_size;
									vsb_buf[2] = commentFrame->inherit_super.inherit_super.buffer_size;
									ttLibC_DynamicBuffer_append(vorbisSpecificBuffer, vsb_buf, 3);
									ttLibC_DynamicBuffer_append(vorbisSpecificBuffer, identificationFrame->inherit_super.inherit_super.data, identificationFrame->inherit_super.inherit_super.buffer_size);
									ttLibC_DynamicBuffer_append(vorbisSpecificBuffer, commentFrame->inherit_super.inherit_super.data, commentFrame->inherit_super.inherit_super.buffer_size);
									ttLibC_DynamicBuffer_append(vorbisSpecificBuffer, setupFrame->inherit_super.inherit_super.data, setupFrame->inherit_super.inherit_super.buffer_size);

									// 03 size 00 00 00
									// 04 size DD(vorbis) 15(flag?) 00 00 00  00 01 77 00  00 01 77 00
									// 05 size vorbiscodecprivate
									// 06 01 02
									struct sz{
										uint8_t buf[3];
										uint32_t length;
									};
									struct sz size03;
									struct sz size04;
									struct sz size05;
									uint32_t vsb_size = ttLibC_DynamicBuffer_refSize(vorbisSpecificBuffer);
									if(vsb_size <= 127) {
										size05.length = 2;
										size05.buf[0] = 0x05;
										size05.buf[1] = vsb_size;
									}
									else {
										size05.length = 3;
										size05.buf[0] = 0x05;
										size05.buf[1] = ((vsb_size >> 7) & 0x7F) | 0x80;
										size05.buf[2] = vsb_size & 0x7F;
									}
									vsb_size += 0x0D + size05.length;
									if(vsb_size <= 127) {
										size04.length = 2;
										size04.buf[0] = 0x04;
										size04.buf[1] = vsb_size;
									}
									else {
										size04.length = 3;
										size04.buf[0] = 0x04;
										size04.buf[1] = ((vsb_size >> 7) & 0x7F) | 0x80;
										size04.buf[2] = vsb_size & 0x7F;
									}
									vsb_size += 0x06 + size04.length;
									if(vsb_size <= 127) {
										size03.length = 2;
										size03.buf[0] = 0x03;
										size03.buf[1] = vsb_size;
									}
									else {
										size03.length = 3;
										size03.buf[0] = 0x03;
										size03.buf[1] = ((vsb_size >> 7) & 0x7F) | 0x80;
										size03.buf[2] = vsb_size & 0x7F;
									}
									// 03
									ttLibC_DynamicBuffer_append(buffer, size03.buf, size03.length);
									in_size = ttLibC_HexUtil_makeBuffer("00 00 00", buf, 256);
									ttLibC_DynamicBuffer_append(buffer, buf, in_size);

									// 04
									ttLibC_DynamicBuffer_append(buffer, size04.buf, size04.length);
									in_size = ttLibC_HexUtil_makeBuffer("DD 15 00 00 00 00 01 77 00 00 01 77 00", buf, 256);
									ttLibC_DynamicBuffer_append(buffer, buf, in_size);

									// 05
									ttLibC_DynamicBuffer_append(buffer, size05.buf, size05.length);
									ttLibC_DynamicBuffer_append(buffer,
											ttLibC_DynamicBuffer_refData(vorbisSpecificBuffer),
											ttLibC_DynamicBuffer_refSize(vorbisSpecificBuffer));

									// 06
									in_size = ttLibC_HexUtil_makeBuffer("06 01 02", buf, 256);
									ttLibC_DynamicBuffer_append(buffer, buf, in_size);

									ttLibC_DynamicBuffer_close(&vorbisSpecificBuffer);
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
static bool Mp4Writer_makeInitMp4(ttLibC_ContainerWriter_ *writer) {
	ttLibC_DynamicBuffer *buffer = ttLibC_DynamicBuffer_make();
	if(buffer == NULL) {
		ERR_PRINT("failed to make buffer.");
		return false;
	}
//	uint8_t *b = NULL;
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
		uint32_t be_track_id = be_uint32_t(track->inherit_super.frame_queue->track_id);
		ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_track_id, 4);
		// tfdt timestamp
		in_size = ttLibC_HexUtil_makeBuffer("00 00 00 10 74 66 64 74 00 00 00 00", buf, 256);
		ttLibC_DynamicBuffer_append(buffer, buf, in_size);
		// just put first frame pts information.
		ttLibC_Frame *first_frame = ttLibC_FrameQueue_ref_first(track->inherit_super.frame_queue);
		uint32_t be_timediff = be_uint32_t((uint32_t)first_frame->pts);
		// for audio, timediff is crazy, cause round for timebase = 1000. for write function.
		switch(track->inherit_super.frame_type) {
		case frameType_h264:
		case frameType_h265:
			{
				if((track->inherit_super.use_mode & containerWriter_enable_dts) != 0) {
					uint32_t timediff = first_frame->dts;
					be_timediff = be_uint32_t((uint32_t)timediff);
				}
			}
			break;
		case frameType_jpeg:
			break;
		case frameType_aac:
		case frameType_mp3:
		case frameType_vorbis:
			{
				ttLibC_Audio *audio = (ttLibC_Audio *)first_frame;
				uint32_t timediff = (uint32_t)(1.0 * first_frame->pts * audio->sample_rate / 1000);
				be_timediff = be_uint32_t((uint32_t)timediff);
			}
			break;
		default:
			break;
		}
		ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_timediff, 4);

		// trun
		switch(track->inherit_super.frame_type) {
		case frameType_h264:
			{
				uint32_t trunSizePos = ttLibC_DynamicBuffer_refSize(buffer);
				if((track->inherit_super.use_mode & containerWriter_enable_dts) != 0) {
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
					ttLibC_H264 *h264 = (ttLibC_H264 *)ttLibC_FrameQueue_ref_first(track->inherit_super.frame_queue);
					if(h264 == NULL) {
						break;
					}
					if(h264->inherit_super.inherit_super.dts < writer->inherit_super.target_pos) {
						ttLibC_H264 *h = (ttLibC_H264 *)ttLibC_FrameQueue_dequeue_first(track->inherit_super.frame_queue);
						if(h264 != h) {
							ERR_PRINT("ref frame is invalid.");
							return false;
						}
						// get next frame to get duration of frame.
						ttLibC_Frame *next_frame = ttLibC_FrameQueue_ref_first(track->inherit_super.frame_queue);
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
						if((track->inherit_super.use_mode & containerWriter_enable_dts) != 0) {
							// get offset to store.
							uint32_t offset = h264->inherit_super.inherit_super.pts + (h264->inherit_super.inherit_super.timebase / 5) - h264->inherit_super.inherit_super.dts;
							uint32_t be_offset = be_uint32_t(offset);
							ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_offset, 4);
						}
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
		case frameType_h265:
			{
				uint32_t trunSizePos = ttLibC_DynamicBuffer_refSize(buffer);
				if((track->inherit_super.use_mode & containerWriter_enable_dts) != 0) {
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
					ttLibC_H265 *h265 = (ttLibC_H265 *)ttLibC_FrameQueue_ref_first(track->inherit_super.frame_queue);
					if(h265 == NULL) {
						break;
					}
					if(h265->inherit_super.inherit_super.dts < writer->inherit_super.target_pos) {
						ttLibC_H265 *h = (ttLibC_H265 *)ttLibC_FrameQueue_dequeue_first(track->inherit_super.frame_queue);
						if(h265 != h) {
							ERR_PRINT("ref frame is invalid.");
							return false;
						}
						// get next frame to get duration of frame.
						ttLibC_Frame *next_frame = ttLibC_FrameQueue_ref_first(track->inherit_super.frame_queue);
						uint32_t duration = next_frame->dts - h265->inherit_super.inherit_super.dts;
						uint32_t be_duration = be_uint32_t(duration);
						ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_duration, 4);

						// put data on mdat_buffer.
						uint8_t *h265_data = h265->inherit_super.inherit_super.data;
						size_t h265_data_size = h265->inherit_super.inherit_super.buffer_size;
						ttLibC_H265_NalInfo nal_info;
						uint32_t h265_size = 0;
						while(ttLibC_H265_getNalInfo(&nal_info, h265_data, h265_data_size)) {
							uint32_t nal_size = nal_info.nal_size - nal_info.data_pos;
							h265_size += 4 + nal_size;
							uint32_t be_nal_size = be_uint32_t(nal_size);
							ttLibC_DynamicBuffer_append(track->mdat_buffer, (uint8_t *)&be_nal_size, 4);
							ttLibC_DynamicBuffer_append(track->mdat_buffer, h265_data + nal_info.data_pos, nal_size);
							h265_data += nal_info.nal_size;
							h265_data_size -= nal_info.nal_size;
						}
						// set size on trun.
						uint32_t be_h265_size = be_uint32_t(h265_size);
						ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_h265_size, 4);
						if((track->inherit_super.use_mode & containerWriter_enable_dts) != 0) {
							// get offset to store.
							uint32_t offset = h265->inherit_super.inherit_super.pts + (h265->inherit_super.inherit_super.timebase / 5) - h265->inherit_super.inherit_super.dts;
							uint32_t be_offset = be_uint32_t(offset);
							ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_offset, 4);
						}
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
		case frameType_jpeg:
			{
				// trun use 0301
				uint32_t trunSizePos = ttLibC_DynamicBuffer_refSize(buffer);
				in_size = ttLibC_HexUtil_makeBuffer("00 00 00 00 74 72 75 6E 00 00 03 01 00 00 00 00 00 00 00 00", buf, 256);
				ttLibC_DynamicBuffer_append(buffer, buf, in_size);
				uint32_t frameCountPos = ttLibC_DynamicBuffer_refSize(buffer) - 8;
				track->dataOffsetPosForTrun = frameCountPos + 4;
				uint32_t frameCount = 0;
//				uint64_t target_pts = 0;
				while(true) {
					ttLibC_Video *jpeg =(ttLibC_Video *)ttLibC_FrameQueue_ref_first(track->inherit_super.frame_queue);
					if(jpeg == NULL) {
						break;
					}
					if(jpeg->inherit_super.pts < writer->inherit_super.target_pos) {
						ttLibC_Video *j = (ttLibC_Video *)ttLibC_FrameQueue_dequeue_first(track->inherit_super.frame_queue);
						if(jpeg != j) {
							ERR_PRINT("ref frame is invalid.");
							return false;
						}
						// get next frame to get duration of frame.
						ttLibC_Frame *next_frame = ttLibC_FrameQueue_ref_first(track->inherit_super.frame_queue);
						uint32_t duration = next_frame->pts - jpeg->inherit_super.pts;
						uint32_t be_duration = be_uint32_t(duration);
						ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_duration, 4);

						// put data on mdat_buffer.
						uint8_t *jpeg_data = jpeg->inherit_super.data;
						size_t jpeg_data_size = jpeg->inherit_super.buffer_size;
						ttLibC_DynamicBuffer_append(track->mdat_buffer, jpeg_data, jpeg_data_size);
						// set size on trun.
						uint32_t be_jpeg_size = be_uint32_t(jpeg_data_size);
						ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_jpeg_size, 4);
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
					ttLibC_Aac *aac = (ttLibC_Aac *)ttLibC_FrameQueue_ref_first(track->inherit_super.frame_queue);
					if(aac == NULL) {
						break;
					}
					if(target_pts == 0) {
						target_pts = (uint64_t)(1.0 * writer->inherit_super.target_pos * aac->inherit_super.inherit_super.timebase / 1000);
					}
					if(aac->inherit_super.inherit_super.pts < target_pts) {
						ttLibC_Aac *a = (ttLibC_Aac *)ttLibC_FrameQueue_dequeue_first(track->inherit_super.frame_queue);
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
		case frameType_mp3:
			{
				uint32_t trunSizePos = ttLibC_DynamicBuffer_refSize(buffer);
				in_size = ttLibC_HexUtil_makeBuffer("00 00 00 00 74 72 75 6E 00 00 03 01 00 00 00 00 00 00 00 00", buf, 256);
				ttLibC_DynamicBuffer_append(buffer, buf, in_size);
				uint32_t frameCountPos = ttLibC_DynamicBuffer_refSize(buffer) - 8;
				track->dataOffsetPosForTrun = frameCountPos + 4;
				uint32_t frameCount = 0;
				uint64_t target_pts = 0;
				while(true) {
					ttLibC_Mp3 *mp3 = (ttLibC_Mp3 *)ttLibC_FrameQueue_ref_first(track->inherit_super.frame_queue);
					if(mp3 == NULL) {
						break;
					}
					if(target_pts == 0) {
						target_pts = (uint64_t)(1.0 * writer->inherit_super.target_pos * mp3->inherit_super.inherit_super.timebase / 1000);
					}
					if(mp3->inherit_super.inherit_super.pts < target_pts) {
						ttLibC_Mp3 *m = (ttLibC_Mp3 *)ttLibC_FrameQueue_dequeue_first(track->inherit_super.frame_queue);
						if(mp3 != m) {
							ERR_PRINT("ref frame is invalid.");
							return false;
						}
						if(mp3->type == Mp3Type_frame) {
							uint8_t *mp3_data = mp3->inherit_super.inherit_super.data;
							uint32_t mp3_size = mp3->inherit_super.inherit_super.buffer_size;
							ttLibC_DynamicBuffer_append(track->mdat_buffer, mp3_data, mp3_size);
							uint32_t be_mp3_duration = be_uint32_t(mp3->inherit_super.sample_num);
							ttLibC_DynamicBuffer_append(buffer,(uint8_t *)&be_mp3_duration, 4);
							uint32_t be_mp3_size = be_uint32_t(mp3_size);
							ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_mp3_size, 4);
							++ frameCount;
						}
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
		case frameType_vorbis:
			{
				uint32_t trunSizePos = ttLibC_DynamicBuffer_refSize(buffer);
				in_size = ttLibC_HexUtil_makeBuffer("00 00 00 00 74 72 75 6E 00 00 03 01 00 00 00 00 00 00 00 00", buf, 256);
				ttLibC_DynamicBuffer_append(buffer, buf, in_size);
				uint32_t frameCountPos = ttLibC_DynamicBuffer_refSize(buffer) - 8;
				track->dataOffsetPosForTrun = frameCountPos + 4;
				uint32_t frameCount = 0;
				uint64_t target_pts = 0;
				while(true) {
					ttLibC_Vorbis *vorbis = (ttLibC_Vorbis *)ttLibC_FrameQueue_ref_first(track->inherit_super.frame_queue);
					if(vorbis == NULL) {
						break;
					}
					if(target_pts == 0) {
						target_pts = (uint64_t)(1.0 * writer->inherit_super.target_pos * vorbis->inherit_super.inherit_super.timebase / 1000);
					}
					if(vorbis->inherit_super.inherit_super.pts < target_pts) {
						ttLibC_Vorbis *v = (ttLibC_Vorbis *)ttLibC_FrameQueue_dequeue_first(track->inherit_super.frame_queue);
						if(vorbis != v) {
							ERR_PRINT("ref frame is invalid.");
							return false;
						}
						if(vorbis->type == VorbisType_frame) {
							uint8_t *vorbis_data = vorbis->inherit_super.inherit_super.data;
							uint32_t vorbis_size = vorbis->inherit_super.inherit_super.buffer_size;
							ttLibC_DynamicBuffer_append(track->mdat_buffer, vorbis_data, vorbis_size);
							uint32_t be_vorbis_duration = be_uint32_t(vorbis->inherit_super.sample_num);
							ttLibC_DynamicBuffer_append(buffer,(uint8_t *)&be_vorbis_duration, 4);
							uint32_t be_vorbis_size = be_uint32_t(vorbis_size);
							ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_vorbis_size, 4);
							++ frameCount;
						}
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
	uint32_t earlistPts = be_uint32_t(writer->inherit_super.current_pts_pos);
	ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&earlistPts, 4);
	in_size = ttLibC_HexUtil_makeBuffer("00 00 00 00 00 00 00 01", buf, 256);
	ttLibC_DynamicBuffer_append(buffer, buf, in_size);
	// size data will be update later.(moof + mdat)
	uint32_t sidx_inner_sizePos = ttLibC_DynamicBuffer_refSize(buffer);
	in_size = ttLibC_HexUtil_makeBuffer("00 00 00 00", buf, 256);
	ttLibC_DynamicBuffer_append(buffer, buf, in_size);
	uint32_t duration_diff = writer->inherit_super.target_pos - writer->inherit_super.current_pts_pos;
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
		ttLibC_StlMap_forEach(writer->inherit_super.track_list, Mp4Writer_makeTraf, writer);
	Mp4Writer_updateSize(buffer, moofSizePos);
	// mdat
	uint32_t mdatSizePos = ttLibC_DynamicBuffer_refSize(buffer);
	in_size = ttLibC_HexUtil_makeBuffer("00 00 00 00 6D 64 61 74", buf, 256);
	ttLibC_DynamicBuffer_append(buffer, buf, in_size);
	// now, we put mdat_buffer and update trun dop information.
	ttLibC_StlMap_forEach(writer->inherit_super.track_list, Mp4Writer_makeMdat, writer);
	Mp4Writer_updateSize(buffer, mdatSizePos);
	// now ready to update sidx size data.
	b = ttLibC_DynamicBuffer_refData(buffer);
	b += sidx_inner_sizePos;
	*((uint32_t *)b) = be_uint32_t((ttLibC_DynamicBuffer_refSize(buffer) - moofSizePos));
	// done.

	bool result = true;
	if(writer->inherit_super.callback != NULL) {
		result = writer->inherit_super.callback(writer->inherit_super.ptr, ttLibC_DynamicBuffer_refData(buffer), ttLibC_DynamicBuffer_refSize(buffer));
	}
	ttLibC_DynamicBuffer_close(&buffer);
	writer->currentWritingBuffer = NULL;
	return result;
}

/**
 * write from queued data.
 */
static bool Mp4Writer_writeFromQueue(
		ttLibC_Mp4Writer_ *writer) {
	switch(writer->inherit_super.status) {
	case status_init_check:
		{
			if(ttLibC_ContainerWriter_isReadyToStart((ttLibC_ContainerWriter_ *)writer)) {
				writer->inherit_super.status = status_make_init;
				return Mp4Writer_writeFromQueue(writer);
			}
		}
		break;
	case status_make_init:
		{
			if(Mp4Writer_makeInitMp4((ttLibC_ContainerWriter_ *)writer)) {
				// now ready to make chunk.
				writer->inherit_super.status = status_target_check;
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
			ttLibC_ContainerWriter_WriteTrack *track = (ttLibC_ContainerWriter_WriteTrack *)ttLibC_StlMap_get(writer->inherit_super.track_list, (void *)1);
			ttLibC_FrameQueue_ref(track->frame_queue, ttLibC_ContainerWriter_primaryTrackCheck, writer);
			if(writer->inherit_super.target_pos != writer->inherit_super.current_pts_pos) {
				// check each track.
				writer->inherit_super.status = status_data_check;
				return Mp4Writer_writeFromQueue(writer);
			}
		}
		break;
	case status_data_check:
		{
			if(ttLibC_ContainerWriter_isReadyToWrite((ttLibC_ContainerWriter_ *)writer)) {
				writer->inherit_super.status = status_make_data;
				return Mp4Writer_writeFromQueue(writer);
			}
		}
		break;
	case status_make_data:
		{
			if(Mp4Writer_makeData(writer)) {
				// write done.
				writer->inherit_super.status = status_update; // update information for next chunk.
				return Mp4Writer_writeFromQueue(writer);
			}
		}
		break;
	case status_update:
		{
			writer->inherit_super.current_pts_pos = writer->inherit_super.target_pos;
			writer->inherit_super.status = status_target_check;
			writer->inherit_super.inherit_super.pts = writer->inherit_super.target_pos;
			++ writer->chunk_counter;
		}
		break;
	default:
		break;
	}
	return true;
}

bool TT_VISIBILITY_DEFAULT ttLibC_Mp4Writer_write(
		ttLibC_Mp4Writer *writer,
		ttLibC_Frame *frame,
		ttLibC_ContainerWriteFunc callback,
		void *ptr) {
	switch(ttLibC_ContainerWriter_write_(
			(ttLibC_ContainerWriter_ *)writer,
			frame,
			callback,
			ptr)) {
	case -1:
	default:
		return false;
	case 0:
		return true;
	case 1:
		break;
	}
	return Mp4Writer_writeFromQueue((ttLibC_Mp4Writer_ *)writer);
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
		ttLibC_DynamicBuffer_close(&track->mdat_buffer);
		ttLibC_ContainerWriteTrack_close((ttLibC_ContainerWriter_WriteTrack **)&track);
	}
	return true;
}

/**
 * close writer
 * @param writer
 */
void TT_VISIBILITY_DEFAULT ttLibC_Mp4Writer_close(ttLibC_Mp4Writer **writer) {
	ttLibC_Mp4Writer_ *target = (ttLibC_Mp4Writer_ *)*writer;
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.inherit_super.type != containerType_mp4) {
		ERR_PRINT("try to close non Mp4Wrtier.");
		return;
	}
	ttLibC_StlMap_forEach(target->inherit_super.track_list, Mp4Writer_closeTracks, NULL);
	ttLibC_StlMap_close(&target->inherit_super.track_list);
	ttLibC_ContainerWriter_close_((ttLibC_ContainerWriter_ **)writer);
}

