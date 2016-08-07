/*
 * @file   flvFrameUtil.c
 * @brief  help util for analyze flv binary.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/23
 */

#include "flvFrameUtil.h"
#include "../log.h"
#include "../allocator.h"
#include <string.h>
#include "hexUtil.h"
#include "ioUtil.h"

#include "../frame/video/video.h"
#include "../frame/video/flv1.h"
#include "../frame/video/h264.h"
#include "../frame/video/vp6.h"
#include "../frame/audio/audio.h"
#include "../frame/audio/aac.h"
#include "../frame/audio/mp3.h"
#include "../frame/audio/pcmAlaw.h"
#include "../frame/audio/pcmMulaw.h"
#include "../frame/audio/speex.h"
#include "../frame/audio/nellymoser.h"

/*
 * detail definition of flvFrameManager.
 */
typedef struct ttLibC_Util_FlvFrameManager_ {
	ttLibC_FlvFrameManager inherit_super;
	/** reuse video frame */
	ttLibC_Video *video_frame;
	/** reuse audio frame */
	ttLibC_Audio *audio_frame;
	/** size length for h264 nal */
	uint32_t size_length;
	/** current aac dsi information. */
	uint64_t dsi_info;
} ttLibC_Util_FlvFrameManager_;

typedef ttLibC_Util_FlvFrameManager_ ttLibC_FlvFrameManager_;

/*
 * make manager
 * @return ttLibC_FlvFrameManager object.
 */
ttLibC_FlvFrameManager *ttLibC_FlvFrameManager_make() {
	ttLibC_FlvFrameManager_ *manager = ttLibC_malloc(sizeof(ttLibC_FlvFrameManager_));
	if(manager == NULL) {
		return NULL;
	}
	manager->audio_frame = NULL;
	manager->video_frame = NULL;
	manager->size_length = 0;
	manager->dsi_info = 0;
	manager->inherit_super.audio_type = frameType_unknown;
	manager->inherit_super.video_type = frameType_unknown;
	return (ttLibC_FlvFrameManager *)manager;
}

static ttLibC_Video *FlvFrameManager_readH264Binary(
		ttLibC_FlvFrameManager_ *manager,
		uint8_t *data,
		size_t data_size,
		uint64_t pts) {
	if(data_size <= 4) {
		return NULL;
	}
	uint32_t dts = (data[1] << 16) | (data[2] << 8) | data[3];
	switch(data[0]) {
	case 0x00: // configData
		{
			data += 4;
			data_size -= 4;
			uint32_t size_length;
			ttLibC_H264 *h264 = ttLibC_H264_analyzeAvccTag(
					(ttLibC_H264 *)manager->video_frame,
					data,
					data_size,
					&size_length);
			if(h264 == NULL) {
				ERR_PRINT("failed to read avccTag.");
				return NULL;
			}
			manager->size_length = size_length;
			h264->inherit_super.inherit_super.pts = pts;
			h264->inherit_super.inherit_super.timebase = 1000;
			return (ttLibC_Video *)h264;
		}
		break;
	case 0x01: // slice frame.
		{
			data += 4;
			data_size -= 4;
			uint8_t *buf = data;
			size_t buf_size = data_size;
			do {
				uint32_t size = 0;
				for(int i = 1;i <= manager->size_length;++ i) {
					size = (size << 8) | *buf;
					if(i != manager->size_length) {
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
					(ttLibC_H264 *)manager->video_frame,
					data,
					data_size,
					true,
					pts,
					1000);
			if(h264 == NULL) {
				ERR_PRINT("failed to make h264 data.");
				return NULL;
			}
			return (ttLibC_Video *)h264;
		}
		break;
	default:
	case 0x02: // end of stream.
		break;
	}
	return NULL;
}

ttLibC_Video *ttLibC_FlvFrameManager_readVideoBinary(
		ttLibC_FlvFrameManager *manager,
		void *data,
		size_t data_size,
		uint64_t pts) {
	ttLibC_FlvFrameManager_ *manager_ = (ttLibC_FlvFrameManager_ *)manager;
	if(manager_ == NULL) {
		return NULL;
	}
	if(data_size <= 1) {
		return NULL;
	}
	uint8_t *buffer = (uint8_t *)data;
	ttLibC_Video *video_frame = NULL;
	switch((*buffer) & 0x0F) {
	case FlvVideoCodec_jpeg:
		return NULL;
	case FlvVideoCodec_flv1:
		{
			video_frame = (ttLibC_Video *)ttLibC_Flv1_getFrame(
					(ttLibC_Flv1 *)manager_->video_frame,
					buffer + 1,
					data_size - 1,
					true,
					pts,
					1000);
		}
		break;
	case FlvVideoCodec_screenVideo:
		return NULL;
	case FlvVideoCodec_on2Vp6:
		return NULL;
	case FlvVideoCodec_on2Vp6Alpha:
		return NULL;
	case FlvVideoCodec_screenVideoV2:
		return NULL;
	case FlvVideoCodec_avc:
		{
			video_frame = (ttLibC_Video *)FlvFrameManager_readH264Binary(
					manager_,
					buffer + 1,
					data_size - 1,
					pts);
		}
		break;
	default:
		return NULL;
	}
	if(video_frame != NULL) {
		video_frame->inherit_super.id = 9; // flv related system, video id should be 9.
		manager_->video_frame = video_frame;
	}
	return video_frame;
}

static ttLibC_Audio *FlvFrameManager_readMp3Binary(
		ttLibC_FlvFrameManager_ *manager,
		uint32_t sample_rate,
		uint32_t bit_depth,
		uint32_t channel_num,
		uint8_t *data,
		size_t data_size,
		uint64_t pts) {
	ttLibC_Mp3 *mp3 = ttLibC_Mp3_getFrame(
			(ttLibC_Mp3 *)manager->audio_frame,
			data,
			data_size,
			true,
			pts,
			1000);
	if(mp3 == NULL) {
		return NULL;
	}
	return (ttLibC_Audio *)mp3;
}

static ttLibC_Audio *FlvFrameManager_readAacBinary(
		ttLibC_FlvFrameManager_ *manager,
		uint32_t sample_rate,
		uint32_t bit_depth,
		uint32_t channel_num,
		uint8_t *data,
		size_t data_size,
		uint64_t pts) {
	if(data_size <= 1) {
		return NULL;
	}
	switch(*data) {
	case 0x00:
		{
			memcpy(&manager->dsi_info, data + 1, data_size - 1);
		}
		return NULL;
	case 0x01:
		{
			ttLibC_Aac *aac = ttLibC_Aac_make(
					(ttLibC_Aac *)manager->audio_frame,
					AacType_raw,
					sample_rate,
					1024,
					channel_num,
					data + 1,
					data_size - 1,
					true,
					pts,
					1000,
					manager->dsi_info);
			if(aac == NULL) {
				ERR_PRINT("failed to make aac raw frame.");
				return NULL;
			}
			return (ttLibC_Audio *)aac;
		}
	default:
		return NULL;
	}
}

ttLibC_Audio *ttLibC_FlvFrameManager_readAudioBinary(
		ttLibC_FlvFrameManager *manager,
		void *data,
		size_t data_size,
		uint64_t pts) {
	ttLibC_FlvFrameManager_ *manager_ = (ttLibC_FlvFrameManager_ *)manager;
	uint32_t sample_rate = 0;
	uint32_t bit_depth = 0;
	uint32_t channel_num = 0;
	uint8_t *buffer = (uint8_t *)data;
	if(data_size <= 1) {
		return NULL;
	}
	ttLibC_Audio *audio = NULL;
	// 1st byte tells codecID, sampleRate, bitdepth, channelnum
	switch((*buffer >> 2) & 0x03) {
	default:
	case 0:
		sample_rate = 5512;
		break;
	case 1:
		sample_rate = 11025;
		break;
	case 2:
		sample_rate = 22050;
		break;
	case 3:
		sample_rate = 44100;
		break;
	}
	switch((*buffer >> 1) & 0x01) {
	default:
	case 0:
		bit_depth = 8;
		break;
	case 1:
		bit_depth = 16;
		break;
	}
	switch(*buffer & 0x01) {
	default:
	case 0:
		channel_num = 1;
		break;
	case 1:
		channel_num = 2;
		break;
	}
	switch((*buffer >> 4) & 0x0F) {
	case FlvAudioCodec_pcmBigEndian:
		return NULL;
	case FlvAudioCodec_swfAdpcm:
		return NULL;
	case FlvAudioCodec_mp3:
		{
			audio = (ttLibC_Audio *)FlvFrameManager_readMp3Binary(
					manager_,
					sample_rate,
					bit_depth,
					channel_num,
					buffer + 1,
					data_size - 1,
					pts);
		}
		break;
	case FlvAudioCodec_pcmLittleEndian:
		return NULL;
	case FlvAudioCodec_nellymoser16:
		return NULL;
	case FlvAudioCodec_nellymoser8:
		return NULL;
	case FlvAudioCodec_nellymoser:
		return NULL;
	case FlvAudioCodec_pcmAlaw:
		return NULL;
	case FlvAudioCodec_pcmMulaw:
		return NULL;
	case FlvAudioCodec_reserved:
		return NULL;
	case FlvAudioCodec_aac:
		{
			audio = FlvFrameManager_readAacBinary(
					manager_,
					sample_rate,
					bit_depth,
					channel_num,
					buffer + 1,
					data_size - 1,
					pts);
		}
		break;
	case FlvAudioCodec_speex:
		return NULL;
	case FlvAudioCodec_unknown:
		return NULL;
	case FlvAudioCodec_undefined:
		return NULL;
	case FlvAudioCodec_mp38:
		{
			audio = (ttLibC_Audio *)FlvFrameManager_readMp3Binary(
					manager_,
					8000,
					bit_depth,
					channel_num,
					buffer + 1,
					data_size - 1,
					pts);
		}
		break;
	case FlvAudioCodec_deviceSpecific:
		return NULL;
	}
	if(audio != NULL) {
		audio->inherit_super.id = 8;
		manager_->audio_frame = audio;
	}
	return audio;
}

static bool FlvFrameManager_getAudioCodecByte(
		ttLibC_Audio *audio_frame,
		ttLibC_DynamicBuffer *buffer) {
	uint8_t byte = 0;
	switch(audio_frame->sample_rate) {
	case 44100:
		byte |= 0x0C;
		break;
	case 22050:
		byte |= 0x08;
		break;
	case 11025:
		byte |= 0x04;
		break;
	default:
	case 5512:
		break;
	}
	byte |= 2;
	switch(audio_frame->channel_num) {
	case 2:
		byte |= 1;
		break;
	case 1:
		break;
	default:
		ERR_PRINT("channel_num is not compatible for flv audio.");
		return false;
	}
	switch(audio_frame->inherit_super.type) {
	case frameType_aac:
		byte |= (FlvAudioCodec_aac << 4);
		break;
	case frameType_mp3:
		if(audio_frame->sample_rate == 8000) {
			byte |= (FlvAudioCodec_mp38 << 4);
		}
		else {
			byte |= (FlvAudioCodec_mp3 << 4);
		}
		break;
	case frameType_nellymoser:
		switch(audio_frame->sample_rate) {
		case 16000:
			byte |= (FlvAudioCodec_nellymoser16 << 4);
			break;
		case 8000:
			byte |= (FlvAudioCodec_nellymoser8 << 4);
			break;
		default:
			byte |= (FlvAudioCodec_nellymoser << 4);
			break;
		}
		break;
	case frameType_pcm_alaw:
		byte |= (FlvAudioCodec_pcmAlaw << 4);
		break;
	case frameType_pcm_mulaw:
		byte |= (FlvAudioCodec_pcmMulaw << 4);
		break;
	case frameType_speex:
		byte |= (FlvAudioCodec_speex << 4);
		break;
	default:
		ERR_PRINT("frame is not compatible for flv audio.");
		return false;
	}
	ttLibC_DynamicBuffer_append(buffer, &byte, 1);
	return true;
}

bool ttLibC_FlvFrameManager_getAacDsiData(
		ttLibC_Frame *frame,
		ttLibC_DynamicBuffer *buffer) {
	if(frame->type != frameType_aac) {
		return false;
	}
	ttLibC_Aac *aac = (ttLibC_Aac *)frame;
	if(!FlvFrameManager_getAudioCodecByte(
			(ttLibC_Audio *)aac,
			buffer)) {
		return false;
	}
	uint64_t dsi_info = 0;
	size_t dsi_info_size = ttLibC_Aac_readDsiInfo(aac, (void *)&dsi_info, 8);
	uint8_t data = 0x00;
	ttLibC_DynamicBuffer_append(buffer, &data, 1);
	ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&dsi_info, dsi_info_size);
	return true;
}

static bool FlvFrameManager_getAacData(
		ttLibC_Aac *aac,
		ttLibC_DynamicBuffer *buffer) {
	if(!FlvFrameManager_getAudioCodecByte(
			(ttLibC_Audio *)aac,
			buffer)) {
		return false;
	}
	uint8_t data = 0x01;
	ttLibC_DynamicBuffer_append(buffer, &data, 1);
	uint8_t *aac_data = aac->inherit_super.inherit_super.data;
	size_t aac_data_size = aac->inherit_super.inherit_super.buffer_size;
	if(aac->type == AacType_adts) {
		aac_data += 7;
		aac_data_size -= 7;
	}
	ttLibC_DynamicBuffer_append(buffer, aac_data, aac_data_size);
	return true;
}
static bool FlvFrameManager_getFlv1Data(
		ttLibC_Flv1 *flv1,
		ttLibC_DynamicBuffer *buffer) {
	return true;
}

static bool FlvFrameManager_getH264Data(
		ttLibC_H264 *h264,
		ttLibC_DynamicBuffer *buffer) {
	switch(h264->type) {
	case H264Type_configData:
		{
			uint8_t first5byte[5] = {
					0x17, 0x00, 0x00, 0x00, 0x00
			};
			uint8_t avcc[256];
			ttLibC_DynamicBuffer_append(buffer, first5byte, 5);
			size_t size = ttLibC_H264_readAvccTag(h264, avcc, 256);
			ttLibC_DynamicBuffer_append(buffer, avcc, size);
			return true;
		}
		break;
	case H264Type_sliceIDR:
		{
			uint8_t first5byte[5] = {
					0x17, 0x01, 0x00, 0x00, 0x00
			};
			ttLibC_DynamicBuffer_append(buffer, first5byte, 5);
			uint8_t *data = h264->inherit_super.inherit_super.data;
			size_t data_size = h264->inherit_super.inherit_super.buffer_size;
			ttLibC_H264_NalInfo nal_info;
			// nal -> sizenal.
			while(ttLibC_H264_getNalInfo(&nal_info, data, data_size)) {
				uint32_t size = nal_info.nal_size - nal_info.data_pos;
				uint32_t size_be = be_uint32_t(size);
				ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&size_be, 4);
				ttLibC_DynamicBuffer_append(buffer, data + nal_info.data_pos, size);
				data += nal_info.nal_size;
				data_size -= nal_info.nal_size;
			}
			return true;
		}
		break;
	case H264Type_slice:
		{
			uint8_t first5byte[5] = {
					0x27, 0x01, 0x00, 0x00, 0x00
			};
			ttLibC_DynamicBuffer_append(buffer, first5byte, 5);
			uint8_t *data = h264->inherit_super.inherit_super.data;
			size_t data_size = h264->inherit_super.inherit_super.buffer_size;
			ttLibC_H264_NalInfo nal_info;
			// nal -> sizenal.
			while(ttLibC_H264_getNalInfo(&nal_info, data, data_size)) {
				uint32_t size = nal_info.nal_size - nal_info.data_pos;
				uint32_t size_be = be_uint32_t(size);
				ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&size_be, 4);
				ttLibC_DynamicBuffer_append(buffer, data + nal_info.data_pos, size);
				data += nal_info.nal_size;
				data_size -= nal_info.nal_size;
			}
			return true;
		}
		break;
	case H264Type_unknown:
	default:
		return false;
	}
}
static bool FlvFrameManager_getMp3Data(
		ttLibC_Mp3 *mp3,
		ttLibC_DynamicBuffer *buffer) {
	return true;
}
static bool FlvFrameManager_getNellymoserData(
		ttLibC_Nellymoser *nellymoser,
		ttLibC_DynamicBuffer *buffer) {
	return true;
}
static bool FlvFrameManager_getPcmAlawData(
		ttLibC_PcmAlaw *pcm_alaw,
		ttLibC_DynamicBuffer *buffer) {
	return true;
}
static bool FlvFrameManager_getPcmMulawData(
		ttLibC_PcmMulaw *pcm_mulaw,
		ttLibC_DynamicBuffer *buffer) {
	return true;
}
static bool FlvFrameManager_getSpeexData(
		ttLibC_Speex *speex,
		ttLibC_DynamicBuffer *buffer) {
	return true;
}
static bool FlvFrameManager_getVp6Data(
		ttLibC_Vp6 *vp6,
		ttLibC_DynamicBuffer *buffer) {
	return true;
}

bool ttLibC_FlvFrameManager_getData(
		ttLibC_Frame *frame,
		ttLibC_DynamicBuffer *buffer) {
	switch(frame->type) {
	case frameType_aac:
		return FlvFrameManager_getAacData(
				(ttLibC_Aac *)frame,
				buffer);
	case frameType_flv1:
		return FlvFrameManager_getFlv1Data(
				(ttLibC_Flv1 *)frame,
				buffer);
	case frameType_h264:
		return FlvFrameManager_getH264Data(
				(ttLibC_H264 *)frame,
				buffer);
	case frameType_mp3:
		return FlvFrameManager_getMp3Data(
				(ttLibC_Mp3 *)frame,
				buffer);
	case frameType_nellymoser:
		return FlvFrameManager_getNellymoserData(
				(ttLibC_Nellymoser *)frame,
				buffer);
	case frameType_pcm_alaw:
		return FlvFrameManager_getPcmAlawData(
				(ttLibC_PcmAlaw *)frame,
				buffer);
	case frameType_pcm_mulaw:
		return FlvFrameManager_getPcmMulawData(
				(ttLibC_PcmMulaw *)frame,
				buffer);
	case frameType_speex:
		return FlvFrameManager_getSpeexData(
				(ttLibC_Speex *)frame,
				buffer);
	case frameType_vp6:
		return FlvFrameManager_getVp6Data(
				(ttLibC_Vp6 *)frame,
				buffer);
	default:
		ERR_PRINT("frame is not compatible for flv.");
		return false;
	}
	return true;
}

void ttLibC_FlvFrameManager_close(ttLibC_FlvFrameManager **manager) {
	ttLibC_FlvFrameManager_ *target = (ttLibC_FlvFrameManager_ *)*manager;
	if(target == NULL) {
		return;
	}
	ttLibC_Audio_close(&target->audio_frame);
	ttLibC_Video_close(&target->video_frame);
	ttLibC_free(target);
}
