/*
 * @file   mkvWriter.c
 * @brief  mkv container writer.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/09/04
 */

#include "mkvWriter.h"

#include "../../log.h"
#include "../../allocator.h"
#include "../../util/hexUtil.h"
#include "../container.h"
#include "../containerCommon.h"
#include "../../frame/audio/aac.h"
#include "../../frame/audio/adpcmImaWav.h"
#include "../../frame/audio/mp3.h"
#include "../../frame/audio/opus.h"
#include "../../frame/audio/speex.h"
#include "../../frame/audio/vorbis.h"
#include "../../frame/video/h264.h"
#include "../../frame/video/h265.h"
#include "../../frame/video/jpeg.h"
#include "../../frame/video/theora.h"
#include "../../frame/video/vp8.h"
#include "../../frame/video/vp9.h"
#include "../../util/byteUtil.h"
#include "../../util/ioUtil.h"

#include <stdlib.h>

ttLibC_MkvWriter *ttLibC_MkvWriter_make(
		ttLibC_Frame_Type* target_frame_types,
		uint32_t types_num) {
	return ttLibC_MkvWriter_make_ex(
			target_frame_types,
			types_num,
			5000); // 5sec for target_unit_duration
}

ttLibC_MkvWriter *ttLibC_MkvWriter_make_ex(
		ttLibC_Frame_Type* target_frame_types,
		uint32_t types_num,
		uint32_t max_unit_duration) {
	ttLibC_MkvWriter_ *writer = (ttLibC_MkvWriter_ *)ttLibC_ContainerWriter_make(
			containerType_mkv,
			sizeof(ttLibC_MkvWriter_),
			1000); // work with 1000 only for now.
	if(writer == NULL) {
		ERR_PRINT("failed to allocate writer.");
		return NULL;
	}
	writer->track_list = ttLibC_StlMap_make();
	for(uint32_t i = 0;i < types_num;++ i) {
		ttLibC_MkvWriteTrack *track = ttLibC_malloc(sizeof(ttLibC_MkvWriteTrack));
		track->frame_queue = ttLibC_FrameQueue_make(i + 1, 255);
		track->h26x_configData = NULL;
		track->frame_type = target_frame_types[i];
		track->is_appending = false;
		track->counter = 0;
		ttLibC_StlMap_put(writer->track_list, (void *)(long)(i + 1), (void *)track);
	}
	writer->inherit_super.is_webm = false; // work as matroska for default
	writer->inherit_super.inherit_super.timebase = 1000;
	writer->inherit_super.inherit_super.pts = 0;
	writer->max_unit_duration = max_unit_duration;
	writer->is_first = true;
	writer->target_pos = 0;
	writer->status = status_init_check;
	return (ttLibC_MkvWriter *)writer;
}

/**
 * check track to have enough data for track entry.
 * @param ptr
 * @param key
 * @param item ttLibC_MkvWriteTrack object.
 */
static bool MkvWriter_initCheckTrack(void *ptr, void *key, void *item) {
	(void)ptr;
	(void)key;
	if(item == NULL) {
		return false;
	}
	ttLibC_MkvWriteTrack *track = (ttLibC_MkvWriteTrack *)item;
	switch(track->frame_type) {
	case frameType_h264:
	case frameType_h265:
		{
			return track->h26x_configData != NULL;
		}
	default:
		{
			return track->is_appending;
		}
	}
	return true;
}

/**
 * try to make trackEntry
 * @param ptr DynamicBuffer for written
 * @param key
 * @param item MkvWriteTrack object.
 */
static bool MkvWriter_makeTrackEntry(void *ptr, void *key, void *item) {
	(void)key;
	if(ptr != NULL && item != NULL) {
		ttLibC_DynamicBuffer *buffer = (ttLibC_DynamicBuffer *)ptr;
		ttLibC_MkvWriteTrack *track = (ttLibC_MkvWriteTrack *)item;
		switch(track->frame_type) {
		case frameType_h264:
		case frameType_h265:
		case frameType_jpeg:
		case frameType_theora:
		case frameType_vp8:
		case frameType_vp9:
		case frameType_aac:
		case frameType_adpcm_ima_wav:
		case frameType_mp3:
		case frameType_opus:
		case frameType_speex:
		case frameType_vorbis:
			break;
		default:
			ERR_PRINT("unexpected frame type.");
			return true;
		}
		// make inside -> at last, complete track entry.
		ttLibC_DynamicBuffer *trackEntryBuffer = ttLibC_DynamicBuffer_make();
		uint8_t buf[256];
		size_t in_size = 0;
		ttLibC_ByteConnector *connector = ttLibC_ByteConnector_make(buf, 256, ByteUtilType_default);
		// trackNumber
		ttLibC_ByteConnector_ebml2(connector, MkvType_TrackNumber, true);
		ttLibC_ByteConnector_ebml2(connector, 1, false);
		ttLibC_ByteConnector_bit(connector, track->frame_queue->track_id, 8);
		// trackUID
		ttLibC_ByteConnector_ebml2(connector, MkvType_TrackUID, true);
		ttLibC_ByteConnector_ebml2(connector, 1, false);
		ttLibC_ByteConnector_bit(connector, track->frame_queue->track_id, 8);

		uint8_t inner[256];
		ttLibC_ByteConnector *innerConnector = ttLibC_ByteConnector_make(inner, 256, ByteUtilType_default);
		switch(track->frame_type) {
		case frameType_h264:
			{
				// codecID
				ttLibC_ByteConnector_ebml2(connector, MkvType_CodecID, true);
				ttLibC_ByteConnector_ebml2(connector, 15, false);
				ttLibC_ByteConnector_string(connector, "V_MPEG4/ISO/AVC", 15);
				// trackType
				ttLibC_ByteConnector_ebml2(connector, MkvType_TrackType, true);
				ttLibC_ByteConnector_ebml2(connector, 1, false);
				ttLibC_ByteConnector_bit(connector, 1, 8);
				ttLibC_H264 *configData = (ttLibC_H264 *)track->h26x_configData;
				// video要素の中身をつくっていく。
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_PixelWidth, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 2, false);
				ttLibC_ByteConnector_bit(innerConnector, configData->inherit_super.width, 16);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_PixelHeight, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 2, false);
				ttLibC_ByteConnector_bit(innerConnector, configData->inherit_super.height, 16);

				ttLibC_ByteConnector_ebml2(connector, MkvType_Video, true);
				ttLibC_ByteConnector_ebml2(connector, innerConnector->write_size, false);
				ttLibC_ByteConnector_string(connector, (const char *)inner, innerConnector->write_size);
				// codecPrivate
				in_size = ttLibC_H264_readAvccTag(configData, inner, 256);
				ttLibC_ByteConnector_ebml2(connector, MkvType_CodecPrivate, true);
				ttLibC_ByteConnector_ebml2(connector, in_size, false);
				ttLibC_DynamicBuffer_append(trackEntryBuffer, buf, connector->write_size);
				ttLibC_DynamicBuffer_append(trackEntryBuffer, inner, in_size);
			}
			break;
		case frameType_h265:
			{
				// codecID
				ttLibC_ByteConnector_ebml2(connector, MkvType_CodecID, true);
				ttLibC_ByteConnector_ebml2(connector, 16, false);
				ttLibC_ByteConnector_string(connector, "V_MPEGH/ISO/HEVC", 16);
				// trackType
				ttLibC_ByteConnector_ebml2(connector, MkvType_TrackType, true);
				ttLibC_ByteConnector_ebml2(connector, 1, false);
				ttLibC_ByteConnector_bit(connector, 1, 8);
				ttLibC_H265 *configData = (ttLibC_H265 *)track->h26x_configData;
				// video要素の中身をつくっていく。
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_PixelWidth, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 2, false);
				ttLibC_ByteConnector_bit(innerConnector, configData->inherit_super.width, 16);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_PixelHeight, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 2, false);
				ttLibC_ByteConnector_bit(innerConnector, configData->inherit_super.height, 16);

				ttLibC_ByteConnector_ebml2(connector, MkvType_Video, true);
				ttLibC_ByteConnector_ebml2(connector, innerConnector->write_size, false);
				ttLibC_ByteConnector_string(connector, (const char *)inner, innerConnector->write_size);
				// codecPrivate
				in_size = ttLibC_H265_readHvccTag(configData, inner, 256);
				ttLibC_ByteConnector_ebml2(connector, MkvType_CodecPrivate, true);
				ttLibC_ByteConnector_ebml2(connector, in_size, false);
				ttLibC_DynamicBuffer_append(trackEntryBuffer, buf, connector->write_size);
				ttLibC_DynamicBuffer_append(trackEntryBuffer, inner, in_size);
			}
			break;
		case frameType_jpeg:
			{
				// codecID
				ttLibC_ByteConnector_ebml2(connector, MkvType_CodecID, true);
				ttLibC_ByteConnector_ebml2(connector, 7, false);
				ttLibC_ByteConnector_string(connector, "V_MJPEG", 7);
				// trackType
				ttLibC_ByteConnector_ebml2(connector, MkvType_TrackType, true);
				ttLibC_ByteConnector_ebml2(connector, 1, false);
				ttLibC_ByteConnector_bit(connector, 1, 8);
				// video要素の中身をつくっていく。
				ttLibC_Jpeg *jpeg = (ttLibC_Jpeg *)ttLibC_FrameQueue_ref_first(track->frame_queue);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_PixelWidth, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 2, false);
				ttLibC_ByteConnector_bit(innerConnector, jpeg->inherit_super.width, 16);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_PixelHeight, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 2, false);
				ttLibC_ByteConnector_bit(innerConnector, jpeg->inherit_super.height, 16);

				ttLibC_ByteConnector_ebml2(connector, MkvType_Video, true);
				ttLibC_ByteConnector_ebml2(connector, innerConnector->write_size, false);
				ttLibC_ByteConnector_string(connector, (const char *)inner, innerConnector->write_size);
				ttLibC_DynamicBuffer_append(trackEntryBuffer, buf, connector->write_size);
			}
			break;
		case frameType_theora:
			{
				// codecID
				ttLibC_ByteConnector_ebml2(connector, MkvType_CodecID, true);
				ttLibC_ByteConnector_ebml2(connector, 8, false);
				ttLibC_ByteConnector_string(connector, "V_THEORA", 8);
				// trackType
				ttLibC_ByteConnector_ebml2(connector, MkvType_TrackType, true);
				ttLibC_ByteConnector_ebml2(connector, 1, false);
				ttLibC_ByteConnector_bit(connector, 1, 8);
				// video要素の中身をつくっていく。
				ttLibC_Vp8 *vp8 = (ttLibC_Vp8 *)ttLibC_FrameQueue_ref_first(track->frame_queue);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_PixelWidth, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 2, false);
				ttLibC_ByteConnector_bit(innerConnector, vp8->inherit_super.width, 16);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_PixelHeight, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 2, false);
				ttLibC_ByteConnector_bit(innerConnector, vp8->inherit_super.height, 16);

				ttLibC_ByteConnector_ebml2(connector, MkvType_Video, true);
				ttLibC_ByteConnector_ebml2(connector, innerConnector->write_size, false);
				ttLibC_ByteConnector_string(connector, (const char *)inner, innerConnector->write_size);
				// codecPrivateを書き出す frame３つが揃わないとわからないので、まずそれを取り出す。(サイズが決定しない。)
				ttLibC_Theora *identificationFrame = (ttLibC_Theora *)ttLibC_FrameQueue_dequeue_first(track->frame_queue);
				ttLibC_Theora *commentFrame        = (ttLibC_Theora *)ttLibC_FrameQueue_dequeue_first(track->frame_queue);
				ttLibC_Theora *setupFrame          = (ttLibC_Theora *)ttLibC_FrameQueue_dequeue_first(track->frame_queue);
				ttLibC_ByteConnector_ebml2(connector, MkvType_CodecPrivate, true);
				ttLibC_ByteConnector_ebml2(
						connector,
						identificationFrame->inherit_super.inherit_super.buffer_size +
							commentFrame->inherit_super.inherit_super.buffer_size +
							setupFrame->inherit_super.inherit_super.buffer_size +
							3,
						false);
				ttLibC_ByteConnector_bit(connector, 2, 8);
				ttLibC_ByteConnector_bit(connector, identificationFrame->inherit_super.inherit_super.buffer_size, 8);
				ttLibC_ByteConnector_bit(connector, commentFrame->inherit_super.inherit_super.buffer_size, 8);
				ttLibC_DynamicBuffer_append(trackEntryBuffer, buf, connector->write_size);
				ttLibC_DynamicBuffer_append(trackEntryBuffer, identificationFrame->inherit_super.inherit_super.data, identificationFrame->inherit_super.inherit_super.buffer_size);
				ttLibC_DynamicBuffer_append(trackEntryBuffer, commentFrame->inherit_super.inherit_super.data, commentFrame->inherit_super.inherit_super.buffer_size);
				ttLibC_DynamicBuffer_append(trackEntryBuffer, setupFrame->inherit_super.inherit_super.data, setupFrame->inherit_super.inherit_super.buffer_size);
			}
			break;
		case frameType_vp8:
			{
				// codecID
				ttLibC_ByteConnector_ebml2(connector, MkvType_CodecID, true);
				ttLibC_ByteConnector_ebml2(connector, 5, false);
				ttLibC_ByteConnector_string(connector, "V_VP8", 5);
				// trackType
				ttLibC_ByteConnector_ebml2(connector, MkvType_TrackType, true);
				ttLibC_ByteConnector_ebml2(connector, 1, false);
				ttLibC_ByteConnector_bit(connector, 1, 8);
				// video要素の中身をつくっていく。
				ttLibC_Vp8 *vp8 = (ttLibC_Vp8 *)ttLibC_FrameQueue_ref_first(track->frame_queue);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_PixelWidth, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 2, false);
				ttLibC_ByteConnector_bit(innerConnector, vp8->inherit_super.width, 16);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_PixelHeight, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 2, false);
				ttLibC_ByteConnector_bit(innerConnector, vp8->inherit_super.height, 16);

				ttLibC_ByteConnector_ebml2(connector, MkvType_Video, true);
				ttLibC_ByteConnector_ebml2(connector, innerConnector->write_size, false);
				ttLibC_ByteConnector_string(connector, (const char *)inner, innerConnector->write_size);
				ttLibC_DynamicBuffer_append(trackEntryBuffer, buf, connector->write_size);
			}
			break;
		case frameType_vp9:
			{
				// codecID
				ttLibC_ByteConnector_ebml2(connector, MkvType_CodecID, true);
				ttLibC_ByteConnector_ebml2(connector, 5, false);
				ttLibC_ByteConnector_string(connector, "V_VP9", 5);
				// trackType
				ttLibC_ByteConnector_ebml2(connector, MkvType_TrackType, true);
				ttLibC_ByteConnector_ebml2(connector, 1, false);
				ttLibC_ByteConnector_bit(connector, 1, 8);
				// video要素の中身をつくっていく。
				ttLibC_Vp9 *vp9 = (ttLibC_Vp9 *)ttLibC_FrameQueue_ref_first(track->frame_queue);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_PixelWidth, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 2, false);
				ttLibC_ByteConnector_bit(innerConnector, vp9->inherit_super.width, 16);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_PixelHeight, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 2, false);
				ttLibC_ByteConnector_bit(innerConnector, vp9->inherit_super.height, 16);

				ttLibC_ByteConnector_ebml2(connector, MkvType_Video, true);
				ttLibC_ByteConnector_ebml2(connector, innerConnector->write_size, false);
				ttLibC_ByteConnector_string(connector, (const char *)inner, innerConnector->write_size);
				ttLibC_DynamicBuffer_append(trackEntryBuffer, buf, connector->write_size);
			}
			break;
		case frameType_aac:
			{
				// codecID
				ttLibC_ByteConnector_ebml2(connector, MkvType_CodecID, true);
				ttLibC_ByteConnector_ebml2(connector, 5, false);
				ttLibC_ByteConnector_string(connector, "A_AAC", 5);
				// trackType
				ttLibC_ByteConnector_ebml2(connector, MkvType_TrackType, true);
				ttLibC_ByteConnector_ebml2(connector, 1, false);
				ttLibC_ByteConnector_bit(connector, 2, 8);
				// audioの子要素をつくっていく。
				ttLibC_Aac *aac = (ttLibC_Aac *)ttLibC_FrameQueue_ref_first(track->frame_queue);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_SamplingFrequency, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 4, false);
				float sr = aac->inherit_super.sample_rate;
				ttLibC_ByteConnector_bit(innerConnector, *(uint32_t *)&sr, 32);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_Channels, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 1, false);
				ttLibC_ByteConnector_bit(innerConnector, aac->inherit_super.channel_num, 8);

				ttLibC_ByteConnector_ebml2(connector, MkvType_Audio, true);
				ttLibC_ByteConnector_ebml2(connector, innerConnector->write_size, false);
				ttLibC_ByteConnector_string(connector, (const char *)inner, innerConnector->write_size);
				// codecPrivate
				in_size = ttLibC_Aac_readDsiInfo(aac, inner, 256);
				ttLibC_ByteConnector_ebml2(connector, MkvType_CodecPrivate, true);
				ttLibC_ByteConnector_ebml2(connector, in_size, false);
				ttLibC_DynamicBuffer_append(trackEntryBuffer, buf, connector->write_size);
				ttLibC_DynamicBuffer_append(trackEntryBuffer, inner, in_size);
			}
			break;
		case frameType_adpcm_ima_wav:
			{
				// codecID
				ttLibC_ByteConnector_ebml2(connector, MkvType_CodecID, true);
				ttLibC_ByteConnector_ebml2(connector, 8, false);
				ttLibC_ByteConnector_string(connector, "A_MS/ACM", 8);
				// trackType
				ttLibC_ByteConnector_ebml2(connector, MkvType_TrackType, true);
				ttLibC_ByteConnector_ebml2(connector, 1, false);
				ttLibC_ByteConnector_bit(connector, 2, 8);
				// audioの子要素をつくっていく。
				ttLibC_AdpcmImaWav *adpcm = (ttLibC_AdpcmImaWav *)ttLibC_FrameQueue_ref_first(track->frame_queue);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_SamplingFrequency, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 4, false);
				float sr = adpcm->inherit_super.sample_rate;
				ttLibC_ByteConnector_bit(innerConnector, *(uint32_t *)&sr, 32);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_Channels, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 1, false);
				ttLibC_ByteConnector_bit(innerConnector, adpcm->inherit_super.channel_num, 8);

				ttLibC_ByteConnector_ebml2(connector, MkvType_Audio, true);
				ttLibC_ByteConnector_ebml2(connector, innerConnector->write_size, false);
				ttLibC_ByteConnector_string(connector, (const char *)inner, innerConnector->write_size);
				// codecPrivate (we must have OpusHead information or no sound.)
				ttLibC_ByteConnector_ebml2(connector, MkvType_CodecPrivate, true);
				ttLibC_ByteConnector_ebml2(connector, 0x14, false); // サイズは固定だと思う。
				// wave format ex
				//  codecType 0x0011
				ttLibC_ByteConnector_bit(connector, 0x1100, 16);
				//  channel byteConnectorの動作がベースbigEndianなのでbeかけてlittle endianかけるようにする。(なんか妙な気分だけど)
				uint16_t be_channel = be_uint16_t(adpcm->inherit_super.channel_num);
				ttLibC_ByteConnector_bit(connector, be_channel, 16);
				//  sample_rate
				uint32_t be_sample_rate = be_uint32_t(adpcm->inherit_super.sample_rate);
				ttLibC_ByteConnector_bit(connector, be_sample_rate, 32);
				//  avg bytes per sec
				uint32_t avgBytesPerSec = adpcm->inherit_super.sample_rate;
				if(adpcm->inherit_super.channel_num == 1) {
					avgBytesPerSec /= 2;
				}
				uint32_t be_avgBytesPerSec = be_uint32_t(avgBytesPerSec);
				ttLibC_ByteConnector_bit(connector, be_avgBytesPerSec, 32);
				uint16_t nBlockAlign = adpcm->inherit_super.inherit_super.buffer_size;
				uint16_t be_nBlockAlign = be_uint16_t(nBlockAlign);
				ttLibC_ByteConnector_bit(connector, be_nBlockAlign, 16);
				ttLibC_ByteConnector_bit(connector, 0x0400, 16);
				ttLibC_ByteConnector_bit(connector, 0x0200, 16);
				uint16_t be_sample_num = be_uint16_t(adpcm->inherit_super.sample_num);
				ttLibC_ByteConnector_bit(connector, be_sample_num, 16);
				ttLibC_DynamicBuffer_append(trackEntryBuffer, buf, connector->write_size);
			}
			break;
		case frameType_mp3:
			{
				// codecID
				ttLibC_ByteConnector_ebml2(connector, MkvType_CodecID, true);
				ttLibC_ByteConnector_ebml2(connector, 9, false);
				ttLibC_ByteConnector_string(connector, "A_MPEG/L3", 9);
				// trackType
				ttLibC_ByteConnector_ebml2(connector, MkvType_TrackType, true);
				ttLibC_ByteConnector_ebml2(connector, 1, false);
				ttLibC_ByteConnector_bit(connector, 2, 8);
				// audioの子要素をつくっていく。
				ttLibC_Opus *opus = (ttLibC_Opus *)ttLibC_FrameQueue_ref_first(track->frame_queue);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_SamplingFrequency, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 4, false);
				float sr = opus->inherit_super.sample_rate;
				ttLibC_ByteConnector_bit(innerConnector, *(uint32_t *)&sr, 32);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_Channels, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 1, false);
				ttLibC_ByteConnector_bit(innerConnector, opus->inherit_super.channel_num, 8);

				ttLibC_ByteConnector_ebml2(connector, MkvType_Audio, true);
				ttLibC_ByteConnector_ebml2(connector, innerConnector->write_size, false);
				ttLibC_ByteConnector_string(connector, (const char *)inner, innerConnector->write_size);
				ttLibC_DynamicBuffer_append(trackEntryBuffer, buf, connector->write_size);
			}
			break;
		case frameType_opus:
			{
				// codecID
				ttLibC_ByteConnector_ebml2(connector, MkvType_CodecID, true);
				ttLibC_ByteConnector_ebml2(connector, 6, false);
				ttLibC_ByteConnector_string(connector, "A_OPUS", 6);
				// trackType
				ttLibC_ByteConnector_ebml2(connector, MkvType_TrackType, true);
				ttLibC_ByteConnector_ebml2(connector, 1, false);
				ttLibC_ByteConnector_bit(connector, 2, 8);
				// audioの子要素をつくっていく。
				ttLibC_Opus *opus = (ttLibC_Opus *)ttLibC_FrameQueue_ref_first(track->frame_queue);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_SamplingFrequency, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 4, false);
				float sr = opus->inherit_super.sample_rate;
				ttLibC_ByteConnector_bit(innerConnector, *(uint32_t *)&sr, 32);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_Channels, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 1, false);
				ttLibC_ByteConnector_bit(innerConnector, opus->inherit_super.channel_num, 8);

				ttLibC_ByteConnector_ebml2(connector, MkvType_Audio, true);
				ttLibC_ByteConnector_ebml2(connector, innerConnector->write_size, false);
				ttLibC_ByteConnector_string(connector, (const char *)inner, innerConnector->write_size);
				// codecPrivate (we must have OpusHead information or no sound.)
				ttLibC_ByteConnector_ebml2(connector, MkvType_CodecPrivate, true);
				ttLibC_ByteConnector_ebml2(connector, 19, false);
				ttLibC_ByteConnector_string(connector, "OpusHead", 8);
				ttLibC_ByteConnector_bit(connector, 1, 8);
				ttLibC_ByteConnector_bit(connector, opus->inherit_super.channel_num, 8);
				ttLibC_ByteConnector_bit(connector, 0, 16);
				ttLibC_ByteConnector_bit(connector, opus->inherit_super.sample_rate, 16);
				ttLibC_ByteConnector_bit(connector, 0, 24);
				ttLibC_ByteConnector_bit(connector, 0, 16);
				ttLibC_DynamicBuffer_append(trackEntryBuffer, buf, connector->write_size);
			}
			break;
		case frameType_speex:
			{
				// speexをつくっていく。(これがないから、メモリーリークしたのか・・・)
				// codecID
				ttLibC_ByteConnector_ebml2(connector, MkvType_CodecID, true);
				ttLibC_ByteConnector_ebml2(connector, 8, false);
				ttLibC_ByteConnector_string(connector, "A_MS/ACM", 8);
				// trackType
				ttLibC_ByteConnector_ebml2(connector, MkvType_TrackType, true);
				ttLibC_ByteConnector_ebml2(connector, 1, false);
				ttLibC_ByteConnector_bit(connector, 2, 8);
				// audioの子要素をつくっていく。
				ttLibC_Speex *speex = (ttLibC_Speex *)ttLibC_FrameQueue_ref_first(track->frame_queue);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_SamplingFrequency, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 4, false);
				float sr = speex->inherit_super.sample_rate;
				ttLibC_ByteConnector_bit(innerConnector, *(uint32_t *)&sr, 32);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_Channels, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 1, false);
				ttLibC_ByteConnector_bit(innerConnector, speex->inherit_super.channel_num, 8);

				ttLibC_ByteConnector_ebml2(connector, MkvType_Audio, true);
				ttLibC_ByteConnector_ebml2(connector, innerConnector->write_size, false);
				ttLibC_ByteConnector_string(connector, (const char *)inner, innerConnector->write_size);
				// codecPrivate (we must have OpusHead information or no sound.)
				ttLibC_ByteConnector_ebml2(connector, MkvType_CodecPrivate, true);
				ttLibC_ByteConnector_ebml2(connector, 0x62, false); // サイズは固定だと思う。
				// wave format ex
				//  codecType 0xA109
				ttLibC_ByteConnector_bit(connector, 0x09A1, 16);
				//  channel byteConnectorの動作がベースbigEndianなのでbeかけてlittle endianかけるようにする。(なんか妙な気分だけど)
				uint16_t be_channel = be_uint16_t(speex->inherit_super.channel_num);
				ttLibC_ByteConnector_bit(connector, be_channel, 16);
				//  sample_rate
				uint32_t be_sample_rate = be_uint32_t(speex->inherit_super.sample_rate);
				ttLibC_ByteConnector_bit(connector, be_sample_rate, 32);
				//  avg bytes per sec
				uint8_t *speex_data = (uint8_t *)speex->inherit_super.inherit_super.data;
				uint32_t *speex_data32 = (uint32_t *)(speex_data + 0x34);
				uint32_t avgBytesPerSec = *speex_data32 / 8;
				if(speex->inherit_super.channel_num == 2) {
					avgBytesPerSec += 100; // stereoの場合は100byte足しておく。 inSignalのデータ17bit(端数のせいで2byteだけ絶対に増える。) x 50１秒あたり50フレームなので100byteふやす。
				}
				uint32_t be_avgBytesPerSec = be_uint32_t(avgBytesPerSec);
				ttLibC_ByteConnector_bit(connector, be_avgBytesPerSec, 32);
				if(speex->inherit_super.channel_num == 2) {
					ttLibC_ByteConnector_bit(connector, 0x0400, 16);
				}
				else {
					ttLibC_ByteConnector_bit(connector, 0x0200, 16);
				}
				ttLibC_ByteConnector_bit(connector, 0x1000, 16);
				ttLibC_ByteConnector_bit(connector, 0x5000, 16);
				ttLibC_DynamicBuffer_append(trackEntryBuffer, buf, connector->write_size);
				ttLibC_DynamicBuffer_append(trackEntryBuffer, speex->inherit_super.inherit_super.data, speex->inherit_super.inherit_super.buffer_size);
			}
			break;
		case frameType_vorbis:
			{
				// codecID
				ttLibC_ByteConnector_ebml2(connector, MkvType_CodecID, true);
				ttLibC_ByteConnector_ebml2(connector, 8, false);
				ttLibC_ByteConnector_string(connector, "A_VORBIS", 8);
				// trackType
				ttLibC_ByteConnector_ebml2(connector, MkvType_TrackType, true);
				ttLibC_ByteConnector_ebml2(connector, 1, false);
				ttLibC_ByteConnector_bit(connector, 2, 8);
				// audioの子要素をつくっていく。
				ttLibC_Aac *aac = (ttLibC_Aac *)ttLibC_FrameQueue_ref_first(track->frame_queue);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_SamplingFrequency, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 4, false);
				float sr = aac->inherit_super.sample_rate;
				ttLibC_ByteConnector_bit(innerConnector, *(uint32_t *)&sr, 32);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_Channels, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 1, false);
				ttLibC_ByteConnector_bit(innerConnector, aac->inherit_super.channel_num, 8);

				ttLibC_ByteConnector_ebml2(connector, MkvType_Audio, true);
				ttLibC_ByteConnector_ebml2(connector, innerConnector->write_size, false);
				ttLibC_ByteConnector_string(connector, (const char *)inner, innerConnector->write_size);
				// あとはprivateData
				ttLibC_Vorbis *identificationFrame = (ttLibC_Vorbis *)ttLibC_FrameQueue_dequeue_first(track->frame_queue);
				ttLibC_Vorbis *commentFrame        = (ttLibC_Vorbis *)ttLibC_FrameQueue_dequeue_first(track->frame_queue);
				ttLibC_Vorbis *setupFrame          = (ttLibC_Vorbis *)ttLibC_FrameQueue_dequeue_first(track->frame_queue);
				ttLibC_ByteConnector_ebml2(connector, MkvType_CodecPrivate, true);
				ttLibC_ByteConnector_ebml2(
						connector,
						identificationFrame->inherit_super.inherit_super.buffer_size +
							commentFrame->inherit_super.inherit_super.buffer_size +
							setupFrame->inherit_super.inherit_super.buffer_size +
							3,
						false);
				ttLibC_ByteConnector_bit(connector, 2, 8);
				ttLibC_ByteConnector_bit(connector, identificationFrame->inherit_super.inherit_super.buffer_size, 8);
				ttLibC_ByteConnector_bit(connector, commentFrame->inherit_super.inherit_super.buffer_size, 8);
				ttLibC_DynamicBuffer_append(trackEntryBuffer, buf, connector->write_size);
				ttLibC_DynamicBuffer_append(trackEntryBuffer, identificationFrame->inherit_super.inherit_super.data, identificationFrame->inherit_super.inherit_super.buffer_size);
				ttLibC_DynamicBuffer_append(trackEntryBuffer, commentFrame->inherit_super.inherit_super.data, commentFrame->inherit_super.inherit_super.buffer_size);
				ttLibC_DynamicBuffer_append(trackEntryBuffer, setupFrame->inherit_super.inherit_super.data, setupFrame->inherit_super.inherit_super.buffer_size);
			}
			break;
		default:
			ERR_PRINT("unreachable.");
			return true;
		}
		ttLibC_ByteConnector_close(&innerConnector);
		ttLibC_ByteConnector_close(&connector);
		// trackEntryが作成完了したので、trackのbufferに追加しなければならない。
		connector = ttLibC_ByteConnector_make(buf, 256, ByteUtilType_default);
		ttLibC_ByteConnector_ebml2(connector, MkvType_TrackEntry, true);
		ttLibC_ByteConnector_ebml2(connector, ttLibC_DynamicBuffer_refSize(trackEntryBuffer), false);
		ttLibC_DynamicBuffer_append(buffer, buf, connector->write_size);
		ttLibC_DynamicBuffer_append(buffer,
				ttLibC_DynamicBuffer_refData(trackEntryBuffer),
				ttLibC_DynamicBuffer_refSize(trackEntryBuffer));
		ttLibC_ByteConnector_close(&connector);
		ttLibC_DynamicBuffer_close(&trackEntryBuffer);
		// 追加してみる。
		// trackNumber
		// trackUID(trackNumberと同じにしておく。)
		// ここまでは共通。
		// 以下はframeTypeで別れる。
		// CodecID
		// TrackType
		// Video子要素 or Audio子要素
		// CodecPrivate
		// ここまで書き出せばOKだと思われ。
		// ここでconnectしてデータをつくらなければならない。
		// 解析おわり。
		// h264の場合
		//  trackNumber
		//  trackUID
		//  frameLacing 00:未設定 (なくてもいいと思う)
		//  Language undでOK なくてよさげ
		//  CodecID:V_MPEG4/ISO/AVC
		//  TrackType 01:video
		//  DefaultDuration:よくわからん。なくてもいいんじゃない？33000000になってる。
		//  videoの子要素
		//   PixelWidth
		//   PixelHeight
		//   DisplayWidth
		//   DisplayHeight
		//  CodecPrivate:(h264のavcCがはいってる。)
		// aacの場合
		//  trackNumber
		//  trackUID
		//  frameLacing 00:未設定 (なくてもいいと思う)
		//  Language undでOK
		//  CodecID:A_AAC
		//  TrackType 02:Audioたと思われ
		//  Audioの子要素
		//   Channel
		//   SampleFrequency:
		//   bitDepth 16bitになってる。
		//  CodecPrivate:(aacのdsi情報がはいってる。)
		// vp8の場合
		//  TrackNumber:01
		//  TrackUID:ちゃんとはいってる。なにかは不明
		//  trackType:01 video
		//  defaultDurationがはいってる。なにこれ？33333333になってる。
		//  CodecType:V_VP8
		//  Video子要素
		//   PixelWidth
		//   PixelHeight
		//   Videoの子要素としてframeRateがはいってた。ほぅ・・・
		// opusの場合
		//  TrackNumber:02
		//  TrackUID:いろいろ
		//  TrackType:02 audio
		//  CodecType:A_OPUS
		//  CodecPrivate:Opusのhead情報がはいってる。
		//   これだけちょっと気になるOpusHead 01 [チャンネル数1byte] 00 00 [周波数 2byte] 00 00 00 00 00
		//   でOKっぽい。なるへそ
		//  Audioの子要素
		//   Hz:48000(ただしfloatではいってた。)
		//   channel:01モノラルなるほどね。
	}
	return true;
}

/**
 * make initial mkv information(EBML Segment Info Tracks)
 * Tag SeekHead (Cue) is not to use.(no seek)
 * @param writer
 */
static bool MkvWriter_makeInitMkv(ttLibC_MkvWriter_ *writer) {
	// 初期データのmkvを作成します。
	ttLibC_DynamicBuffer *buffer = ttLibC_DynamicBuffer_make();
	uint8_t buf[256];
	size_t in_size;
	// EBML
	if(writer->inherit_super.is_webm) {
		in_size = ttLibC_HexUtil_makeBuffer("1A 45 DF A3 9F 42 86 81 01 42 F7 81 01 42 F2 81 04 42 F3 81 08 42 82 84 77 65 62 6D 42 87 81 04 42 85 81 02", buf, 256);
	}
	else {
		in_size = ttLibC_HexUtil_makeBuffer("1A 45 DF A3 A3 42 86 81 01 42 F7 81 01 42 F2 81 04 42 F3 81 08 42 82 88 6D 61 74 72 6F 73 6B 61 42 87 81 04 42 85 81 02", buf, 256);
	}
	ttLibC_DynamicBuffer_append(buffer, buf, in_size);
	// segment.(infinite for live streaming)
	in_size = ttLibC_HexUtil_makeBuffer("18 53 80 67 01 FF FF FF FF FF FF FF", buf, 256);
	ttLibC_DynamicBuffer_append(buffer, buf, in_size);
	// Info (timecodeScale, WritingApp MuxingApp only)
	in_size = ttLibC_HexUtil_makeBuffer("15 49 A9 66 99 2A D7 B1 83 0F 42 40 4D 80 86 74 74 4C 69 62 43 57 41 86 74 74 4C 69 62 43", buf, 256);
	ttLibC_DynamicBuffer_append(buffer, buf, in_size);

	// Tracks
	ttLibC_DynamicBuffer *trackBuffer = ttLibC_DynamicBuffer_make();
	// make TrackEntry for each tracks.
	ttLibC_StlMap_forEach(writer->track_list, MkvWriter_makeTrackEntry, trackBuffer);
	ttLibC_ByteConnector *connector = ttLibC_ByteConnector_make(buf, 256, ByteUtilType_default);
	ttLibC_ByteConnector_ebml2(connector, MkvType_Tracks, true);
	ttLibC_ByteConnector_ebml2(connector, ttLibC_DynamicBuffer_refSize(trackBuffer), false);
	ttLibC_DynamicBuffer_append(buffer, buf, connector->write_size);
	ttLibC_DynamicBuffer_append(
			buffer,
			ttLibC_DynamicBuffer_refData(trackBuffer),
			ttLibC_DynamicBuffer_refSize(trackBuffer));
	ttLibC_ByteConnector_close(&connector);
	ttLibC_DynamicBuffer_close(&trackBuffer);

	// done.
	bool result = true;
	if(writer->callback != NULL) {
		result = writer->callback(writer->ptr, ttLibC_DynamicBuffer_refData(buffer), ttLibC_DynamicBuffer_refSize(buffer));
	}
	ttLibC_DynamicBuffer_close(&buffer);
	// あとはclusterを並べていけばOK
	return result;
}

// check video track to decide chunk size.
static bool MkvWirter_PrimaryVideoTrackCheck(void *ptr, ttLibC_Frame *frame) {
	ttLibC_MkvWriter_ *writer = (ttLibC_MkvWriter_ *)ptr;
	ttLibC_Video *video = (ttLibC_Video *)frame;
	switch(video->type) {
	case videoType_inner:
		{
			if(video->inherit_super.pts - writer->current_pts_pos > writer->max_unit_duration) {
				writer->target_pos = writer->current_pts_pos + writer->max_unit_duration;
				return false;
			}
		}
		break;
	case videoType_key:
		{
			if(writer->current_pts_pos + writer->max_unit_duration < video->inherit_super.pts) {
				writer->target_pos = writer->current_pts_pos + writer->max_unit_duration;
				return false;
			}
			if(writer->current_pts_pos != video->inherit_super.pts) {
				writer->target_pos = video->inherit_super.pts;
				return false;
			}
		}
		break;
	default:
		return true;
	}
	return true;
}

static bool MkvWriter_dataCheckTrack(void *ptr, void *key, void *item) {
	(void)key;
	if(ptr != NULL && item != NULL) {
		ttLibC_MkvWriter_ *writer = (ttLibC_MkvWriter_ *)ptr;
		ttLibC_MkvWriteTrack *track = (ttLibC_MkvWriteTrack *)item;

		uint64_t pts = (uint64_t)(1.0 * track->frame_queue->pts * 1000 / track->frame_queue->timebase);
		if(writer->target_pos > pts) {
			return false;
		}
		return true;
	}
	return false;
}

static bool MkvWriter_makeData(
		ttLibC_MkvWriter_ *writer) {
	// 書き出しを実施する
	// clusterを書き出す。(中身完了してからやらないとだめ)
	// bufferをつくる。ここにデータを書き出す。
	ttLibC_DynamicBuffer *buffer = ttLibC_DynamicBuffer_make();
	uint8_t buf[256]; // 一時データ処理用buffer
	ttLibC_ByteConnector *connector = ttLibC_ByteConnector_make(buf, 256, ByteUtilType_default);
	// timecodeを書き出します。
	ttLibC_ByteConnector_ebml2(connector, MkvType_Timecode, true);
	// とりあえず8byteにしておこうと思う。
	ttLibC_ByteConnector_ebml2(connector, 8, false);
	ttLibC_ByteConnector_bit(connector, (writer->current_pts_pos >> 32), 32); // 64bitで書き出ししておく。
	ttLibC_ByteConnector_bit(connector, (writer->current_pts_pos), 32); // 64bitで書き出ししておく。
	ttLibC_DynamicBuffer_append(buffer, buf, connector->write_size);
	// このデータがtimecodeとして書き込むべきデータ
	ttLibC_ByteConnector_close(&connector);
	// simpleBlockを書き出していく。
	// trackからデータを取り出して、一番若いフレームを書き出すという処理をかかないとだめ。
	// トラックの最大数を知る必要がある。
	uint64_t target_track;
	ttLibC_Frame *frame;
	uint64_t pts;
	bool is_found;
	while(true) {
		target_track = 0;
		frame = NULL;
		pts = 0;
		is_found = false;
		// 全トラックから、一番ptsの低いものを見つけなければならない。
		for(uint32_t i = 0;i < writer->track_list->size;++ i) {
			ttLibC_MkvWriteTrack *track = (ttLibC_MkvWriteTrack *)ttLibC_StlMap_get(writer->track_list, (void *)(long)(1 + i));
			frame = ttLibC_FrameQueue_ref_first(track->frame_queue);
			if(frame != NULL) {
				if(!is_found) {
					// まだ見つかってなければなんであれ、登録する。
					target_track = i + 1;
					pts = frame->pts;
					is_found = true;
				}
				else {
					if(pts > frame->pts) {
						target_track = i + 1;
						pts = frame->pts;
					}
				}
			}
		}
		if(pts >= writer->target_pos) {
			// すでに必要データ以上になった場合は、処理やめる。
			break;
		}
		if(target_track == 0) {
//			LOG_PRINT("");
			// 処理すべきトラックがみつからなかった。もうデータがない。処理終わり
			break;
		}
		ttLibC_MkvWriteTrack *track = (ttLibC_MkvWriteTrack *)ttLibC_StlMap_get(writer->track_list, (void *)target_track);
		frame = ttLibC_FrameQueue_dequeue_first(track->frame_queue);
		switch(frame->type) {
		case frameType_aac:
			{
				ttLibC_Aac *aac = (ttLibC_Aac *)frame;
				if(aac->type == AacType_dsi) {
					// dsi情報は処理にまわさない。
					continue; // これできちんとwhileループのcontinueになってる模様。
				}
			}
			break;
		case frameType_h264:
			{
				ttLibC_H264 *h264 = (ttLibC_H264 *)frame;
				switch(h264->type) {
				case H264Type_unknown:
				case H264Type_configData:
					continue;
				default:
					break;
				}
			}
			break;
		case frameType_h265:
			{
				ttLibC_H265 *h265 = (ttLibC_H265 *)frame;
				switch(h265->type) {
				case H265Type_unknown:
				case H265Type_configData:
					continue;
				default:
					break;
				}
			}
			break;
		case frameType_speex:
			{
				ttLibC_Speex *speex = (ttLibC_Speex *)frame;
				switch(speex->type) {
				case SpeexType_comment:
				case SpeexType_header:
					continue;
				default:
					break;
				}
			}
			break;
		case frameType_theora:
			{
				ttLibC_Theora *theora = (ttLibC_Theora *)frame;
				switch(theora->type) {
				case TheoraType_identificationHeaderDecodeFrame:
				case TheoraType_commentHeaderFrame:
				case TheoraType_setupHeaderFrame:
					continue;
				default:
					break;
				}
			}
			break;
		case frameType_vorbis:
			{
				ttLibC_Vorbis *vorbis = (ttLibC_Vorbis *)frame;
				switch(vorbis->type) {
				case VorbisType_identification:
				case VorbisType_comment:
				case VorbisType_setup:
					continue;
				default:
					break;
				}
			}
			break;
		default:
			break;
		}
		// このframeの内容が書き出すべきもの。
		// simpleBlockで書き出しておく。lacingはしない。
		connector = ttLibC_ByteConnector_make(buf, 255, ByteUtilType_default);
		switch(frame->type) {
		case frameType_h264:
			{
				ttLibC_H264 *h264 = (ttLibC_H264 *)frame;
				// まずデータサイズがわからないとどうしようもない。
				// sizeNalに改良する。
				uint8_t *h264_data = frame->data;
				size_t h264_data_size = frame->buffer_size;
				ttLibC_H264_NalInfo nal_info;
				uint32_t h264_size = 0;
				// ちょっと無駄がおおいけど、２周してごまかすか・・・
				while(ttLibC_H264_getNalInfo(&nal_info, h264_data, h264_data_size)) {
					uint32_t nal_size = nal_info.nal_size - nal_info.data_pos;
					h264_size += 4 + nal_size;
					h264_data += nal_info.nal_size;
					h264_data_size -= nal_info.nal_size;
				}
				// サイズはわかった。あとはこのサイズ + 4byteにするだけ。
				h264_size += 4;
				ttLibC_ByteConnector_ebml2(connector, MkvType_SimpleBlock, true);
				ttLibC_ByteConnector_ebml2(connector, h264_size, false);
				// トラックIDがあまり増えないことを期待しておくことにする。
				ttLibC_ByteConnector_ebml2(connector, frame->id, false);
				// 時間情報
				ttLibC_ByteConnector_bit(connector, pts - writer->current_pts_pos, 16);
				// とりあえずデータをつくるわけだが・・・
				switch(h264->type) {
				case H264Type_slice:
					ttLibC_ByteConnector_bit(connector, 0x00, 8);
					break;
				case H264Type_sliceIDR:
					ttLibC_ByteConnector_bit(connector, 0x80, 8);
					break;
				default:
					LOG_PRINT("unexpected h264 type.");
					break;
				}
				// ここまでであとはsizeNalを書き出していけばOKのはず。
				ttLibC_DynamicBuffer_append(buffer, buf, connector->write_size);
				// あとはh264のデータを書き出していく。
				h264_data = frame->data;
				h264_data_size = frame->buffer_size;
				// ちょっと無駄がおおいけど、２周してごまかすか・・・
				while(ttLibC_H264_getNalInfo(&nal_info, h264_data, h264_data_size)) {
					uint32_t nal_size = nal_info.nal_size - nal_info.data_pos;
					h264_size += 4 + nal_size;
					uint32_t be_nal_size = be_uint32_t(nal_size);
					ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_nal_size, 4);
					ttLibC_DynamicBuffer_append(buffer, h264_data + nal_info.data_pos, nal_size);
					h264_data += nal_info.nal_size;
					h264_data_size -= nal_info.nal_size;
				}
				// ここまで終わればきっと大丈夫なデータになってるはず。

				// A3 4304 81 00 00 80 00 00 02 FC 65 ...
				// simpleBlock
				//    データサイズ
				//         トラックID(ebml)
				//            timestamp
				//                  データフラグ
				//                     以降データ実体
				// よって実体をつくっていかなければならない。
			}
			break;
		case frameType_h265:
			{
				// ここでh265のフレーム書き出しをつくらなければならない。
				ttLibC_H265 *h265 = (ttLibC_H265 *)frame;
				// まずsizeNalにする。
				uint8_t *h265_data = frame->data;
				size_t h265_data_size = frame->buffer_size;
				ttLibC_H265_NalInfo nal_info;
				uint32_t h265_size = 0;
				while(ttLibC_H265_getNalInfo(&nal_info, h265_data, h265_data_size)) {
					uint32_t nal_size = nal_info.nal_size - nal_info.data_pos;
					h265_size += 4 + nal_size;
					h265_data += nal_info.nal_size;
					h265_data_size -= nal_info.nal_size;
				}
				// サイズはわかった。あとはこのサイズ + 4byteにするだけ。
				h265_size += 4;
				ttLibC_ByteConnector_ebml2(connector, MkvType_SimpleBlock, true);
				ttLibC_ByteConnector_ebml2(connector, h265_size, false);
				// トラックIDがあまり増えないことを期待しておくことにする。
				ttLibC_ByteConnector_ebml2(connector, frame->id, false);
				// 時間情報
				ttLibC_ByteConnector_bit(connector, pts - writer->current_pts_pos, 16);
				// とりあえずデータをつくるわけだが・・・
				switch(h265->type) {
				case H265Type_slice:
					ttLibC_ByteConnector_bit(connector, 0x00, 8);
					break;
				case H265Type_sliceIDR:
					ttLibC_ByteConnector_bit(connector, 0x80, 8);
					break;
				default:
					LOG_PRINT("unexpected h265 type.");
					break;
				}
				ttLibC_DynamicBuffer_append(buffer, buf, connector->write_size);
				// あとはh265のデータを書き出していく。
				h265_data = frame->data;
				h265_data_size = frame->buffer_size;
				// ちょっと無駄がおおいけど、２周してごまかすか・・・
				while(ttLibC_H265_getNalInfo(&nal_info, h265_data, h265_data_size)) {
					uint32_t nal_size = nal_info.nal_size - nal_info.data_pos;
					h265_size += 4 + nal_size;
					uint32_t be_nal_size = be_uint32_t(nal_size);
					ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_nal_size, 4);
					ttLibC_DynamicBuffer_append(buffer, h265_data + nal_info.data_pos, nal_size);
					h265_data += nal_info.nal_size;
					h265_data_size -= nal_info.nal_size;
				}
			}
			break;
		case frameType_aac:
			{
				ttLibC_Aac *aac = (ttLibC_Aac *)frame;
				// データサイズを知る必要がある。
				uint8_t *aac_data = aac->inherit_super.inherit_super.data;
				uint32_t aac_size = aac->inherit_super.inherit_super.buffer_size;
				if(aac->type == AacType_adts) {
					// dsiが1くることはないと思うが・・・
					aac_data += 7;
					aac_size -= 7;
				}
				aac_size += 4;
				// サイズはわかった。あとはこのサイズ + 4byteにするだけ。
				ttLibC_ByteConnector_ebml2(connector, MkvType_SimpleBlock, true);
				ttLibC_ByteConnector_ebml2(connector, aac_size, false);
				// トラックIDがあまり増えないことを期待しておくことにする。
				ttLibC_ByteConnector_ebml2(connector, frame->id, false);
				// 時間情報
				ttLibC_ByteConnector_bit(connector, pts - writer->current_pts_pos, 16);
				ttLibC_ByteConnector_bit(connector, 0x80, 8);
				// あとはデータをかけばOK
				ttLibC_DynamicBuffer_append(buffer, buf, connector->write_size);

				aac_size -= 4;
				ttLibC_DynamicBuffer_append(buffer, aac_data, aac_size);
				// A3 8D 82 00 00 80 21...
				// simpleBlock
				//    データサイズ
				//       track
				//          timestamp
				//                データフラグ(keyFrameの映像と同じ扱いっぽい。)
				//                   あとはデータ実体
			}
			break;
		case frameType_vp8:
		case frameType_vp9:
		case frameType_theora:
		case frameType_jpeg:
			{
				ttLibC_Video *video = (ttLibC_Video *)frame;
				uint8_t *data = frame->data;
				size_t data_size = frame->buffer_size;
				ttLibC_ByteConnector_ebml2(connector, MkvType_SimpleBlock, true);
				ttLibC_ByteConnector_ebml2(connector, data_size + 4, false);
				ttLibC_ByteConnector_ebml2(connector, frame->id, false);
				ttLibC_ByteConnector_bit(connector, pts - writer->current_pts_pos, 16);
				switch(video->type) {
				case videoType_inner:
					ttLibC_ByteConnector_bit(connector, 0x00, 8);
					break;
				case videoType_key:
					ttLibC_ByteConnector_bit(connector, 0x80, 8);
					break;
				default:
					LOG_PRINT("unexpected frame");
					break;
				}
				ttLibC_DynamicBuffer_append(buffer, buf, connector->write_size);
				ttLibC_DynamicBuffer_append(buffer, data, data_size);
			}
			break;
		case frameType_adpcm_ima_wav:
		case frameType_mp3:
		case frameType_opus:
		case frameType_speex:
		case frameType_vorbis:
			{
				uint8_t *data = frame->data;
				size_t data_size = frame->buffer_size;
				ttLibC_ByteConnector_ebml2(connector, MkvType_SimpleBlock, true);
				ttLibC_ByteConnector_ebml2(connector, data_size + 4, false);
				ttLibC_ByteConnector_ebml2(connector, frame->id, false);
				ttLibC_ByteConnector_bit(connector, pts - writer->current_pts_pos, 16);
				ttLibC_ByteConnector_bit(connector, 0x80, 8);
				ttLibC_DynamicBuffer_append(buffer, buf, connector->write_size);
				ttLibC_DynamicBuffer_append(buffer, data, data_size);
			}
			break;
		default:
			break;
		}
		ttLibC_ByteConnector_close(&connector);
	}
	ttLibC_DynamicBuffer *clusterBuffer = ttLibC_DynamicBuffer_make();
	// data is complete.
	connector = ttLibC_ByteConnector_make(buf, 256, ByteUtilType_default);
	ttLibC_ByteConnector_ebml2(connector, MkvType_Cluster, true);
	ttLibC_ByteConnector_ebml2(connector, ttLibC_DynamicBuffer_refSize(buffer), false);
	ttLibC_DynamicBuffer_append(clusterBuffer, buf, connector->write_size);
	ttLibC_ByteConnector_close(&connector);
	ttLibC_DynamicBuffer_append(
			clusterBuffer,
			ttLibC_DynamicBuffer_refData(buffer),
			ttLibC_DynamicBuffer_refSize(buffer));
	ttLibC_DynamicBuffer_close(&buffer);

	bool result = true;
	if(writer->callback != NULL) {
		result = writer->callback(writer->ptr, ttLibC_DynamicBuffer_refData(clusterBuffer), ttLibC_DynamicBuffer_refSize(clusterBuffer));
	}
	ttLibC_DynamicBuffer_close(&clusterBuffer);
	return result;
}

static bool MkvWriter_writeFromQueue(
		ttLibC_MkvWriter_ *writer) {
	switch(writer->status) {
	case status_init_check: // 初期データ作成可能か確認
		{
			if(ttLibC_StlMap_forEach(writer->track_list, MkvWriter_initCheckTrack, NULL)) {
				writer->status = status_make_init;
				return MkvWriter_writeFromQueue(writer);
			}
		}
		break;
	case status_make_init: // 初期データ作成
		// 初期データ作成動作を実際に発動させる。
		if(MkvWriter_makeInitMkv(writer)) {
			writer->status = status_target_check;
			return MkvWriter_writeFromQueue(writer);
		}
		else {
			// TODO update
			ERR_PRINT("something fatal is happen. update later");
			return false;
		}
		break;
	case status_target_check: // cluster書き込み先がどの程度になるか初めのトラックから判断
		{
			ttLibC_MkvWriteTrack *track = (ttLibC_MkvWriteTrack *)ttLibC_StlMap_get(writer->track_list, (void *)1);
			switch(track->frame_type) {
			case frameType_h264:
			case frameType_h265:
			case frameType_theora:
			case frameType_vp8:
			case frameType_vp9:
				ttLibC_FrameQueue_ref(track->frame_queue, MkvWirter_PrimaryVideoTrackCheck, writer);
				break;
			case frameType_jpeg: // jpegの場合はすべてがkeyFrameなので、keyFrameわけすると、すごく小さなunitになってしまう。よってmax_unit_duration分とりにいく。
			case frameType_mp3:
			case frameType_aac:
			case frameType_opus:
			case frameType_speex:
			case frameType_adpcm_ima_wav:
			case frameType_vorbis:
				writer->target_pos = writer->current_pts_pos + writer->max_unit_duration;
				break;
			default:
				ERR_PRINT("unexpected frame is found.%d", track->frame_type);
				return false;
			}
			// 位置情報が更新されたら次い進む。
			if(writer->target_pos != writer->current_pts_pos) {
				writer->status = status_data_check;
				return MkvWriter_writeFromQueue(writer);
			}
		}
		break;
	case status_data_check: // cluster分書き込むのに必要なデータがあるか確認
		{
			if(ttLibC_StlMap_forEach(writer->track_list, MkvWriter_dataCheckTrack, writer)) {
				writer->status = status_make_data;
				return MkvWriter_writeFromQueue(writer);
			}
		}
		break;
	case status_make_data: // clusterの書き込みを実施
		{
			if(MkvWriter_makeData(writer)) {
				// write done.
				writer->status = status_update;
				return MkvWriter_writeFromQueue(writer);
			}
		}
		break;
	case status_update: // 次の処理へ移動する。(status_target_checkにいく。)
		{
			writer->current_pts_pos = writer->target_pos;
			writer->status = status_target_check;
			// 無限ループが怖いのでここで止めとく。
			// 呼び出しループが大きすぎるとstackエラーになる環境がありえるので、こうしとく。
		}
		break;
	default:
		break;
	}
	return true;
}

static bool MkvWriter_appendQueue(
		ttLibC_MkvWriteTrack *track,
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

bool ttLibC_MkvWriter_write(
		ttLibC_MkvWriter *writer,
		ttLibC_Frame *frame,
		ttLibC_ContainerWriteFunc callback,
		void *ptr) {
	ttLibC_MkvWriter_ *writer_ = (ttLibC_MkvWriter_ *)writer;
	if(writer_ == NULL) {
		ERR_PRINT("writer is null.");
		return false;
	}
	if(frame == NULL) {
		return true;
	}
	// 該当trackを取得
	ttLibC_MkvWriteTrack *track = (ttLibC_MkvWriteTrack *)ttLibC_StlMap_get(writer_->track_list, (void *)(long)frame->id);
	if(track == NULL) {
		ERR_PRINT("failed to get correspond track. %d", frame->id);
		return false;
	}
	uint64_t pts = (uint64_t)(1.0 * frame->pts * 1000 / frame->timebase);
	// trackにframeを追加する。
	switch(frame->type) {
	case frameType_h265:
		{
			ttLibC_H265 *h265 = (ttLibC_H265 *)frame;
			if(h265->type == H265Type_unknown) {
				return true;
			}
			if(h265->type == H265Type_configData) {
				ttLibC_H265 *h = ttLibC_H265_clone(
						(ttLibC_H265 *)track->h26x_configData,
						h265);
				if(h == NULL) {
					ERR_PRINT("failed to make clone data.");
					return false;
				}
				h->inherit_super.inherit_super.pts = 0;
				h->inherit_super.inherit_super.timebase = 1000;
				track->h26x_configData = (ttLibC_Frame *)h;
				return true;
			}
			if(!track->is_appending && h265->type != H265Type_sliceIDR) {
				// queueにデータがなく、sliceIDRでない場合は、まだはじめる時ではない。
				return true;
			}
		}
		break;
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
				// queueにデータがなく、sliceIDRでない場合は、まだはじめる時ではない。
				return true;
			}
		}
		break;
	case frameType_vp8:
	case frameType_vp9:
		{
			ttLibC_Video *video = (ttLibC_Video *)frame;
			// フレームが１つもなければ・・・というのがあるか・・・
			if(!track->is_appending && video->type != videoType_key) {
				return true;
			}
		}
		break;
	case frameType_theora:
		{
			ttLibC_Theora *theora = (ttLibC_Theora *)frame;
			// theoraのframeは1つめはidentification
			// ２つめはcomment
			// ３つめはsetupとframeを追加するようにしなければいけない。それ以外のがきたらアウト
			// 3つめがはいるのが可能だったら、下に処理をのばしていけばよし。
			// それまではframeデータを捨てるべしだけど・・・
			if(!track->is_appending) {
				switch(track->counter) {
				case 0:
					if(theora->type != TheoraType_identificationHeaderDecodeFrame) {
						return true;
					}
					++ track->counter;
					break;
				case 1:
					if(theora->type != TheoraType_commentHeaderFrame) {
						return true;
					}
					++ track->counter;
					break;
				case 2:
					if(theora->type != TheoraType_setupHeaderFrame) {
						return true;
					}
					++ track->counter;
					break;
				default:
					break;
				}
				if(track->counter < 3) {
					// 現在のをqueueにいれて、ほっとく。
					return MkvWriter_appendQueue(track, frame, 0);
				}
			}
		}
		break;
	case frameType_vorbis:
		{
			ttLibC_Vorbis *vorbis = (ttLibC_Vorbis *)frame;
			if(!track->is_appending) {
				switch(track->counter) {
				case 0:
					if(vorbis->type != VorbisType_identification) {
						return true;
					}
					++ track->counter;
					break;
				case 1:
					if(vorbis->type != VorbisType_comment) {
						return true;
					}
					++ track->counter;
					break;
				case 2:
					if(vorbis->type != VorbisType_setup) {
						return true;
					}
					++ track->counter;
					break;
				default:
					break;
				}
				if(track->counter < 3) {
					return MkvWriter_appendQueue(track, frame, 0);
				}
			}
		}
		break;
	case frameType_speex:
		{
			// speexはheaderはほしい。
			// headerがきたら、下に進んでOKなのだが・・・
			ttLibC_Speex *speex = (ttLibC_Speex *)frame;
			if(!track->is_appending && speex->type != SpeexType_header) {
				// 初めはheaderじゃないとだめ。
				return true;
			}
			if(speex->type == SpeexType_comment) {
				// commentはいらない。
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
	if(!MkvWriter_appendQueue(track, frame, pts)) {
		return false;
	}
	writer_->callback = callback;
	writer_->ptr = ptr;
	// Queue処理に進む。
	return MkvWriter_writeFromQueue(writer_);
}

static bool MkvWriter_closeTracks(void *ptr, void *key, void *item) {
	(void)ptr;
	(void)key;
	if(item != NULL) {
		// trackの内容データを解放する処理を書いておく。
		ttLibC_MkvWriteTrack *track = (ttLibC_MkvWriteTrack *)item;
		ttLibC_FrameQueue_close(&track->frame_queue);
		ttLibC_Frame_close(&track->h26x_configData);
		ttLibC_free(track);
	}
	return true;
}

void ttLibC_MkvWriter_close(ttLibC_MkvWriter **writer) {
	ttLibC_MkvWriter_ *target = (ttLibC_MkvWriter_ *)*writer;
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.inherit_super.type != containerType_mkv) {
		ERR_PRINT("try to close non mkvWriter.");
		return;
	}
	ttLibC_StlMap_forEach(target->track_list, MkvWriter_closeTracks, NULL);
	ttLibC_StlMap_close(&target->track_list);
	ttLibC_free(target);
	*writer = NULL;
}

