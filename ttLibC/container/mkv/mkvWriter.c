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

#include "../../frame/frame.h"
#include "../../frame/audio/audio.h"
#include "../../frame/audio/aac.h"
#include "../../frame/audio/mp3.h"
#include "../../frame/audio/speex.h"
#include "../../frame/audio/vorbis.h"

#include "../../frame/video/video.h"
#include "../../frame/video/h264.h"
#include "../../frame/video/h265.h"
#include "../../frame/video/theora.h"
#include "../../util/byteUtil.h"
#include "../../util/ioUtil.h"
#include "../../util/dynamicBufferUtil.h"

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
		uint32_t unit_duration) {
	return ttLibC_ContainerWriter_make_(
			containerType_mkv,
			sizeof(ttLibC_MkvWriter_),
			1000,
			sizeof(ttLibC_MkvWriteTrack),
			1,
			target_frame_types,
			types_num,
			unit_duration);
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
		ttLibC_ContainerWriter_WriteTrack *track = (ttLibC_ContainerWriter_WriteTrack *)item;
		track->use_mode = track->enable_mode;
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
		case frameType_pcmS16:
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
				// video information
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
				// video informataion
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
				// video information
				ttLibC_Video *video = (ttLibC_Video *)ttLibC_FrameQueue_ref_first(track->frame_queue);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_PixelWidth, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 2, false);
				ttLibC_ByteConnector_bit(innerConnector, video->width, 16);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_PixelHeight, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 2, false);
				ttLibC_ByteConnector_bit(innerConnector, video->height, 16);

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
				// video information
				ttLibC_Video *video = (ttLibC_Video *)ttLibC_FrameQueue_ref_first(track->frame_queue);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_PixelWidth, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 2, false);
				ttLibC_ByteConnector_bit(innerConnector, video->width, 16);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_PixelHeight, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 2, false);
				ttLibC_ByteConnector_bit(innerConnector, video->height, 16);

				ttLibC_ByteConnector_ebml2(connector, MkvType_Video, true);
				ttLibC_ByteConnector_ebml2(connector, innerConnector->write_size, false);
				ttLibC_ByteConnector_string(connector, (const char *)inner, innerConnector->write_size);
				// make codecPrivate
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
				ttLibC_DynamicBuffer_append(trackEntryBuffer, commentFrame->inherit_super.inherit_super.data,        commentFrame->inherit_super.inherit_super.buffer_size);
				ttLibC_DynamicBuffer_append(trackEntryBuffer, setupFrame->inherit_super.inherit_super.data,          setupFrame->inherit_super.inherit_super.buffer_size);
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
				// video information
				ttLibC_Video *video = (ttLibC_Video *)ttLibC_FrameQueue_ref_first(track->frame_queue);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_PixelWidth, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 2, false);
				ttLibC_ByteConnector_bit(innerConnector, video->width, 16);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_PixelHeight, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 2, false);
				ttLibC_ByteConnector_bit(innerConnector, video->height, 16);

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
				// video information
				ttLibC_Video *video = (ttLibC_Video *)ttLibC_FrameQueue_ref_first(track->frame_queue);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_PixelWidth, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 2, false);
				ttLibC_ByteConnector_bit(innerConnector, video->width, 16);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_PixelHeight, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 2, false);
				ttLibC_ByteConnector_bit(innerConnector, video->height, 16);

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
				// audio information
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
				// audio information
				ttLibC_Audio *audio = (ttLibC_Audio *)ttLibC_FrameQueue_ref_first(track->frame_queue);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_SamplingFrequency, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 4, false);
				float sr = audio->sample_rate;
				ttLibC_ByteConnector_bit(innerConnector, *(uint32_t *)&sr, 32);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_Channels, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 1, false);
				ttLibC_ByteConnector_bit(innerConnector, audio->channel_num, 8);

				ttLibC_ByteConnector_ebml2(connector, MkvType_Audio, true);
				ttLibC_ByteConnector_ebml2(connector, innerConnector->write_size, false);
				ttLibC_ByteConnector_string(connector, (const char *)inner, innerConnector->write_size);
				// codecPrivate (we must have OpusHead information or no sound.)
				ttLibC_ByteConnector_ebml2(connector, MkvType_CodecPrivate, true);
				ttLibC_ByteConnector_ebml2(connector, 0x14, false);
				// wave format ex
				//  codecType 0x0011
				ttLibC_ByteConnector_bit(connector, 0x1100, 16);
				// convert bigendian -> little endian.
				uint16_t be_channel = be_uint16_t(audio->channel_num);
				ttLibC_ByteConnector_bit(connector, be_channel, 16);
				//  sample_rate
				uint32_t be_sample_rate = be_uint32_t(audio->sample_rate);
				ttLibC_ByteConnector_bit(connector, be_sample_rate, 32);
				//  avg bytes per sec
				uint32_t avgBytesPerSec = audio->sample_rate;
				if(audio->channel_num == 1) {
					avgBytesPerSec /= 2;
				}
				uint32_t be_avgBytesPerSec = be_uint32_t(avgBytesPerSec);
				ttLibC_ByteConnector_bit(connector, be_avgBytesPerSec, 32);
				uint16_t nBlockAlign = audio->inherit_super.buffer_size;
				uint16_t be_nBlockAlign = be_uint16_t(nBlockAlign);
				ttLibC_ByteConnector_bit(connector, be_nBlockAlign, 16);
				ttLibC_ByteConnector_bit(connector, 0x0400, 16);
				ttLibC_ByteConnector_bit(connector, 0x0200, 16);
				uint16_t be_sample_num = be_uint16_t(audio->sample_num);
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
				// audio information
				ttLibC_Audio *audio = (ttLibC_Audio *)ttLibC_FrameQueue_ref_first(track->frame_queue);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_SamplingFrequency, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 4, false);
				float sr = audio->sample_rate;
				ttLibC_ByteConnector_bit(innerConnector, *(uint32_t *)&sr, 32);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_Channels, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 1, false);
				ttLibC_ByteConnector_bit(innerConnector, audio->channel_num, 8);

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
				// audio information
				ttLibC_Audio *audio = (ttLibC_Audio *)ttLibC_FrameQueue_ref_first(track->frame_queue);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_SamplingFrequency, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 4, false);
				float sr = audio->sample_rate;
				ttLibC_ByteConnector_bit(innerConnector, *(uint32_t *)&sr, 32);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_Channels, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 1, false);
				ttLibC_ByteConnector_bit(innerConnector, audio->channel_num, 8);

				ttLibC_ByteConnector_ebml2(connector, MkvType_Audio, true);
				ttLibC_ByteConnector_ebml2(connector, innerConnector->write_size, false);
				ttLibC_ByteConnector_string(connector, (const char *)inner, innerConnector->write_size);
				// codecPrivate (we must have OpusHead information or no sound.)
				ttLibC_ByteConnector_ebml2(connector, MkvType_CodecPrivate, true);
				ttLibC_ByteConnector_ebml2(connector, 19, false);
				ttLibC_ByteConnector_string(connector, "OpusHead", 8);
				ttLibC_ByteConnector_bit(connector, 1, 8);
				ttLibC_ByteConnector_bit(connector, audio->channel_num, 8);
				ttLibC_ByteConnector_bit(connector, 0, 16);
				ttLibC_ByteConnector_bit(connector, audio->sample_rate, 16);
				ttLibC_ByteConnector_bit(connector, 0, 24);
				ttLibC_ByteConnector_bit(connector, 0, 16);
				ttLibC_DynamicBuffer_append(trackEntryBuffer, buf, connector->write_size);
			}
			break;
		case frameType_speex:
			{
				// codecID
				ttLibC_ByteConnector_ebml2(connector, MkvType_CodecID, true);
				ttLibC_ByteConnector_ebml2(connector, 8, false);
				ttLibC_ByteConnector_string(connector, "A_MS/ACM", 8);
				// trackType
				ttLibC_ByteConnector_ebml2(connector, MkvType_TrackType, true);
				ttLibC_ByteConnector_ebml2(connector, 1, false);
				ttLibC_ByteConnector_bit(connector, 2, 8);
				// audio information
				ttLibC_Audio *audio = (ttLibC_Audio *)ttLibC_FrameQueue_ref_first(track->frame_queue);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_SamplingFrequency, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 4, false);
				float sr = audio->sample_rate;
				ttLibC_ByteConnector_bit(innerConnector, *(uint32_t *)&sr, 32);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_Channels, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 1, false);
				ttLibC_ByteConnector_bit(innerConnector, audio->channel_num, 8);

				ttLibC_ByteConnector_ebml2(connector, MkvType_Audio, true);
				ttLibC_ByteConnector_ebml2(connector, innerConnector->write_size, false);
				ttLibC_ByteConnector_string(connector, (const char *)inner, innerConnector->write_size);
				// codecPrivate (we must have OpusHead information or no sound.)
				ttLibC_ByteConnector_ebml2(connector, MkvType_CodecPrivate, true);
				ttLibC_ByteConnector_ebml2(connector, 0x62, false); // fixed size?
				// wave format ex
				//  codecType 0xA109
				ttLibC_ByteConnector_bit(connector, 0x09A1, 16);
				uint16_t be_channel = be_uint16_t(audio->channel_num);
				ttLibC_ByteConnector_bit(connector, be_channel, 16);
				//  sample_rate
				uint32_t be_sample_rate = be_uint32_t(audio->sample_rate);
				ttLibC_ByteConnector_bit(connector, be_sample_rate, 32);
				//  avg bytes per sec
				uint8_t *speex_data = (uint8_t *)audio->inherit_super.data;
				uint32_t *speex_data32 = (uint32_t *)(speex_data + 0x34);
				uint32_t avgBytesPerSec = *speex_data32 / 8;
				if(audio->channel_num == 2) {
					avgBytesPerSec += 100; // for stereo data add 100byte, insignaldata = 17bit -> 2byte x 50 frame / sec = 100byte / sec
				}
				uint32_t be_avgBytesPerSec = be_uint32_t(avgBytesPerSec);
				ttLibC_ByteConnector_bit(connector, be_avgBytesPerSec, 32);
				if(audio->channel_num == 2) {
					ttLibC_ByteConnector_bit(connector, 0x0400, 16);
				}
				else {
					ttLibC_ByteConnector_bit(connector, 0x0200, 16);
				}
				ttLibC_ByteConnector_bit(connector, 0x1000, 16);
				ttLibC_ByteConnector_bit(connector, 0x5000, 16);
				ttLibC_DynamicBuffer_append(trackEntryBuffer, buf, connector->write_size);
				ttLibC_DynamicBuffer_append(trackEntryBuffer, audio->inherit_super.data, audio->inherit_super.buffer_size);
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
				// audio information
				ttLibC_Audio *audio = (ttLibC_Audio *)ttLibC_FrameQueue_ref_first(track->frame_queue);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_SamplingFrequency, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 4, false);
				float sr = audio->sample_rate;
				ttLibC_ByteConnector_bit(innerConnector, *(uint32_t *)&sr, 32);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_Channels, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 1, false);
				ttLibC_ByteConnector_bit(innerConnector, audio->channel_num, 8);

				ttLibC_ByteConnector_ebml2(connector, MkvType_Audio, true);
				ttLibC_ByteConnector_ebml2(connector, innerConnector->write_size, false);
				ttLibC_ByteConnector_string(connector, (const char *)inner, innerConnector->write_size);
				// privateData
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
				ttLibC_DynamicBuffer_append(trackEntryBuffer, commentFrame->inherit_super.inherit_super.data,        commentFrame->inherit_super.inherit_super.buffer_size);
				ttLibC_DynamicBuffer_append(trackEntryBuffer, setupFrame->inherit_super.inherit_super.data,          setupFrame->inherit_super.inherit_super.buffer_size);
			}
			break;
		case frameType_pcmS16:
			{
				// codecID
				ttLibC_ByteConnector_ebml2(connector, MkvType_CodecID, true);
				ttLibC_ByteConnector_ebml2(connector, 13, false);
				ttLibC_ByteConnector_string(connector, "A_PCM/INT/LIT", 13);
				// trackType
				ttLibC_ByteConnector_ebml2(connector, MkvType_TrackType, true);
				ttLibC_ByteConnector_ebml2(connector, 1, false);
				ttLibC_ByteConnector_bit(connector, 2, 8);
				// audioInformation
				ttLibC_Audio *audio = (ttLibC_Audio *)ttLibC_FrameQueue_ref_first(track->frame_queue);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_SamplingFrequency, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 4, false);
				float sr = audio->sample_rate;
				ttLibC_ByteConnector_bit(innerConnector, *(uint32_t *)&sr, 32);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_Channels, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 1, false);
				ttLibC_ByteConnector_bit(innerConnector, audio->channel_num, 8);
				ttLibC_ByteConnector_ebml2(innerConnector, MkvType_BitDepth, true);
				ttLibC_ByteConnector_ebml2(innerConnector, 1, false);
				ttLibC_ByteConnector_bit(innerConnector, 0x10, 8);


				ttLibC_ByteConnector_ebml2(connector, MkvType_Audio, true);
				ttLibC_ByteConnector_ebml2(connector, innerConnector->write_size, false);
				ttLibC_ByteConnector_string(connector, (const char *)inner, innerConnector->write_size);
				ttLibC_DynamicBuffer_append(trackEntryBuffer, buf, connector->write_size);
			}
			break;
		default:
			ERR_PRINT("unreachable.");
			return true;
		}
		ttLibC_ByteConnector_close(&innerConnector);
		ttLibC_ByteConnector_close(&connector);
		// trackEntry is done, next to add buffer on track.
		connector = ttLibC_ByteConnector_make(buf, 256, ByteUtilType_default);
		ttLibC_ByteConnector_ebml2(connector, MkvType_TrackEntry, true);
		ttLibC_ByteConnector_ebml2(connector, ttLibC_DynamicBuffer_refSize(trackEntryBuffer), false);
		ttLibC_DynamicBuffer_append(buffer, buf, connector->write_size);
		ttLibC_DynamicBuffer_append(buffer,
				ttLibC_DynamicBuffer_refData(trackEntryBuffer),
				ttLibC_DynamicBuffer_refSize(trackEntryBuffer));
		ttLibC_ByteConnector_close(&connector);
		ttLibC_DynamicBuffer_close(&trackEntryBuffer);
	}
	return true;
}

/**
 * make initial mkv information(EBML Segment Info Tracks)
 * Tag SeekHead (Cue) is not to use.(no seek)
 * @param writer
 */
static bool MkvWriter_makeInitMkv(ttLibC_ContainerWriter_ *writer) {
	// make initial data of mkv.
	ttLibC_DynamicBuffer *buffer = ttLibC_DynamicBuffer_make();
	uint8_t buf[256];
	size_t in_size;
	// EBML
	if(writer->inherit_super.type == containerType_webm) {
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
	return result;
}

static bool MkvWriter_makeData(
		ttLibC_ContainerWriter_ *writer) {
	// write cluster
	// tmp buffer for generate memory.
	ttLibC_DynamicBuffer *buffer = ttLibC_DynamicBuffer_make();
	uint8_t buf[256];
	ttLibC_ByteConnector *connector = ttLibC_ByteConnector_make(buf, 256, ByteUtilType_default);
	// timecode with 64bit int.
	ttLibC_ByteConnector_ebml2(connector, MkvType_Timecode, true);
	ttLibC_ByteConnector_ebml2(connector, 8, false);
	ttLibC_ByteConnector_bit(connector, (writer->current_pts_pos >> 32), 32);
	ttLibC_ByteConnector_bit(connector, (writer->current_pts_pos), 32);
	ttLibC_DynamicBuffer_append(buffer, buf, connector->write_size);
	ttLibC_ByteConnector_close(&connector);
	// simpleblock
	uint64_t target_track;
	ttLibC_Frame *frame;
	uint64_t pts;
	bool is_found;
	while(true) {
		target_track = 0;
		frame = NULL;
		pts = 0;
		is_found = false;
		// find lowest pts data from all tracks.
		for(uint32_t i = 0;i < writer->track_list->size;++ i) {
			ttLibC_ContainerWriter_WriteTrack *track = (ttLibC_ContainerWriter_WriteTrack *)ttLibC_StlMap_get(writer->track_list, (void *)(long)(1 + i));
			frame = ttLibC_FrameQueue_ref_first(track->frame_queue);
			if(frame != NULL) {
				if(!is_found) { // add any in the case of not found.
					target_track = i + 1;
					pts = frame->pts;
					is_found = true;
				}
				else {
					if(pts > frame->pts) { // if pts is lower, overwrite.
						target_track = i + 1;
						pts = frame->pts;
					}
				}
			}
		}
		if(pts >= writer->target_pos) {
			// already have enough data. done.
			break;
		}
		if(target_track == 0) {
			// no track. done
			break;
		}
		ttLibC_ContainerWriter_WriteTrack *track = (ttLibC_ContainerWriter_WriteTrack *)ttLibC_StlMap_get(writer->track_list, (void *)target_track);
		frame = ttLibC_FrameQueue_dequeue_first(track->frame_queue);
		// check if frame is not written in block.
		switch(frame->type) {
		case frameType_aac:
			{
				ttLibC_Aac *aac = (ttLibC_Aac *)frame;
				if(aac->type == AacType_dsi) {
					continue;
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
		case frameType_mp3:
			{
				ttLibC_Mp3 *mp3 = (ttLibC_Mp3 *)frame;
				switch(mp3->type) {
				case Mp3Type_frame:
					break;
				case Mp3Type_id3:
				case Mp3Type_tag:
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
		// now we should write this frame in simpleblock. not to use lacing.
		connector = ttLibC_ByteConnector_make(buf, 255, ByteUtilType_default);
		switch(frame->type) {
		case frameType_h264:
			{
				ttLibC_H264 *h264 = (ttLibC_H264 *)frame;
				uint8_t *h264_data = frame->data;
				size_t h264_data_size = frame->buffer_size;
				ttLibC_H264_NalInfo nal_info;
				uint32_t h264_size = 0;
				while(ttLibC_H264_getNalInfo(&nal_info, h264_data, h264_data_size)) {
					uint32_t nal_size = nal_info.nal_size - nal_info.data_pos;
					h264_size += 4 + nal_size;
					h264_data += nal_info.nal_size;
					h264_data_size -= nal_info.nal_size;
				}
				h264_size += 4;
				ttLibC_ByteConnector_ebml2(connector, MkvType_SimpleBlock, true);
				ttLibC_ByteConnector_ebml2(connector, h264_size, false);
				// if track is over 0x80, not work. because we expected trackId for 1byte ebml.
				ttLibC_ByteConnector_ebml2(connector, frame->id, false);
				ttLibC_ByteConnector_bit(connector, pts - writer->current_pts_pos, 16);
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
				ttLibC_DynamicBuffer_append(buffer, buf, connector->write_size);
				h264_data = frame->data;
				h264_data_size = frame->buffer_size;
				while(ttLibC_H264_getNalInfo(&nal_info, h264_data, h264_data_size)) {
					uint32_t nal_size = nal_info.nal_size - nal_info.data_pos;
					h264_size += 4 + nal_size;
					uint32_t be_nal_size = be_uint32_t(nal_size);
					ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_nal_size, 4);
					ttLibC_DynamicBuffer_append(buffer, h264_data + nal_info.data_pos, nal_size);
					h264_data += nal_info.nal_size;
					h264_data_size -= nal_info.nal_size;
				}
			}
			break;
		case frameType_h265:
			{
				ttLibC_H265 *h265 = (ttLibC_H265 *)frame;
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
				h265_size += 4;
				ttLibC_ByteConnector_ebml2(connector, MkvType_SimpleBlock, true);
				ttLibC_ByteConnector_ebml2(connector, h265_size, false);
				// TODO must be 1byte ebml value for trackId.
				ttLibC_ByteConnector_ebml2(connector, frame->id, false);
				ttLibC_ByteConnector_bit(connector, pts - writer->current_pts_pos, 16);
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
				h265_data = frame->data;
				h265_data_size = frame->buffer_size;
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
				uint8_t *aac_data = aac->inherit_super.inherit_super.data;
				uint32_t aac_size = aac->inherit_super.inherit_super.buffer_size;
				// use AacType_raw
				if(aac->type == AacType_adts) {
					aac_data += 7;
					aac_size -= 7;
				}
				aac_size += 4;
				ttLibC_ByteConnector_ebml2(connector, MkvType_SimpleBlock, true);
				ttLibC_ByteConnector_ebml2(connector, aac_size, false);
				ttLibC_ByteConnector_ebml2(connector, frame->id, false);
				ttLibC_ByteConnector_bit(connector, pts - writer->current_pts_pos, 16);
				ttLibC_ByteConnector_bit(connector, 0x80, 8);
				ttLibC_DynamicBuffer_append(buffer, buf, connector->write_size);

				aac_size -= 4;
				ttLibC_DynamicBuffer_append(buffer, aac_data, aac_size);
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
		case frameType_pcmS16:
		case frameType_mp3:
		case frameType_adpcm_ima_wav:
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
		ttLibC_ContainerWriter_ *writer) {
	switch(writer->status) {
	case status_init_check:
		{
			if(ttLibC_ContainerWriter_isReadyToStart(writer)) {
				writer->status = status_make_init;
				return MkvWriter_writeFromQueue(writer);
			}
		}
		break;
	case status_make_init:
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
	case status_target_check:
		{
			ttLibC_ContainerWriter_WriteTrack *track = (ttLibC_ContainerWriter_WriteTrack *)ttLibC_StlMap_get(writer->track_list, (void *)1);
			ttLibC_FrameQueue_ref(track->frame_queue, ttLibC_ContainerWriter_primaryTrackCheck, writer);
			if(writer->target_pos != writer->current_pts_pos) {
				writer->status = status_data_check;
				return MkvWriter_writeFromQueue(writer);
			}
		}
		break;
	case status_data_check:
		{
			if(ttLibC_ContainerWriter_isReadyToWrite(writer)) {
				writer->status = status_make_data;
				return MkvWriter_writeFromQueue(writer);
			}
		}
		break;
	case status_make_data:
		{
			if(MkvWriter_makeData(writer)) {
				// write done.
				writer->status = status_update;
				return MkvWriter_writeFromQueue(writer);
			}
		}
		break;
	case status_update:
		{
			writer->current_pts_pos = writer->target_pos;
			writer->inherit_super.pts = writer->target_pos;
			writer->status = status_target_check;
		}
		break;
	default:
		break;
	}
	return true;
}

bool ttLibC_MkvWriter_write(
		ttLibC_MkvWriter *writer,
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
	return MkvWriter_writeFromQueue((ttLibC_ContainerWriter_ *)writer);
}

void ttLibC_MkvWriter_close(ttLibC_MkvWriter **writer) {
	ttLibC_ContainerWriter_ *target = (ttLibC_ContainerWriter_ *)*writer;
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.type != containerType_mkv
	&& target->inherit_super.type != containerType_webm) {
		ERR_PRINT("try to close non mkvWriter.");
		return;
	}
	ttLibC_ContainerWriter_close_((ttLibC_ContainerWriter_ **)writer);
}
