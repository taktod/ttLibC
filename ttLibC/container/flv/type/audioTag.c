/*
 * @file   audioTag.c
 * @brief  flvTag for audio
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/12
 */

#include "audioTag.h"
#include "../flvTag.h"
#include "../../../_log.h"
#include "../../../util/hexUtil.h"

#include "../../../frame/audio/aac.h"
#include "../../../frame/audio/mp3.h"
#include "../../../util/dynamicBufferUtil.h"
#include "../../../util/ioUtil.h"

ttLibC_FlvAudioTag TT_ATTRIBUTE_INNER *ttLibC_FlvAudioTag_make(
		ttLibC_FlvTag *prev_tag,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		uint32_t track_id,
		uint8_t codec_id,
		uint8_t sample_rate_flag,
		uint8_t bit_count_flag,
		uint8_t channel_flag) {
	ttLibC_FlvAudioTag *audio_tag = (ttLibC_FlvAudioTag *)ttLibC_FlvTag_make(
			prev_tag,
			data,
			data_size,
			non_copy_mode,
			pts,
			timebase,
			FlvType_audio,
			track_id);
	if(audio_tag != NULL) {
		if(bit_count_flag == 1) {
			audio_tag->bit_count = 16;
		}
		else {
			audio_tag->bit_count = 8;
		}
		if(channel_flag == 1) {
			audio_tag->channel_num = 2;
		}
		else {
			audio_tag->channel_num = 1;
		}
		switch(sample_rate_flag) {
		case 0:
			audio_tag->sample_rate = 5512;
			break;
		case 1:
			audio_tag->sample_rate = 11025;
			break;
		case 2:
			audio_tag->sample_rate = 22050;
			break;
		case 3:
			audio_tag->sample_rate = 44100;
			break;
		}
		audio_tag->codec_id = codec_id;
		switch(codec_id) {
		case 0: // pcm(be)
			if(bit_count_flag == 0) {
				// pcmS8 // bigendian
				audio_tag->frame_type = frameType_unknown;
			}
			else {
				// pcmS16 // bigendian
				audio_tag->frame_type = frameType_pcmS16;
			}
			break;
		case 1: // adpcm (for flv)
			audio_tag->frame_type = frameType_unknown;
			break;
		case 2: // mp3
			audio_tag->frame_type = frameType_mp3;
			break;
		case 3: // little endian pcm
			if(bit_count_flag == 0) {
				// pcmS8 // bigendian
				audio_tag->frame_type = frameType_unknown;
			}
			else {
				// pcmS16 // bigendian
				audio_tag->frame_type = frameType_pcmS16;
			}
			break;
		case 4: // nelly 16k
			audio_tag->frame_type = frameType_nellymoser;
			audio_tag->sample_rate = 16000;
			break;
		case 5: // nelly 8k
			audio_tag->frame_type = frameType_nellymoser;
			audio_tag->sample_rate = 8000;
			break;
		case 6: // nelly 44.1 22.05 11.025 5k
			audio_tag->frame_type = frameType_nellymoser;
			break;
		case 7: // g711_a
			audio_tag->frame_type = frameType_pcm_alaw;
			break;
		case 8: // g711_u
			audio_tag->frame_type = frameType_pcm_mulaw;
			break;
		case 9: // reserved
			audio_tag->frame_type = frameType_unknown;
			break;
		case 10: // aac
			audio_tag->frame_type = frameType_aac;
			break;
		case 11: // speex 16kHz monoral
			audio_tag->frame_type = frameType_speex;
			break;
		case 12: // unknown
		case 13: // undefined
		default:
			audio_tag->frame_type = frameType_unknown;
			break;
		case 14: // mp3 8kHz
			audio_tag->frame_type = frameType_mp3;
			audio_tag->sample_rate = 8000;
			break;
		case 15: // device specific
			audio_tag->frame_type = frameType_unknown;
			break;
		}
	}
	return audio_tag;
}

ttLibC_FlvAudioTag TT_ATTRIBUTE_INNER *ttLibC_FlvAudioTag_getTag(
		ttLibC_FlvTag *prev_tag,
		uint8_t *data,
		size_t data_size) {
	/**
	 * 1byte flag
	 * 3byte size
	 * 3byte timestamp
	 * 1byte timestamp-ext
	 * 3byte track_id
	 * 4bit codec_type
	 * 2bit sample_rate flag
	 * 1bit bit count flag
	 * 1bit channel flag
	 */
	size_t   size      = ((data[1] << 16) & 0xFF0000) | ((data[2] << 8) & 0xFF00) | (data[3] & 0xFF);
	uint32_t timestamp = ((data[4] << 16) & 0xFF0000) | ((data[5] << 8) & 0xFF00) | (data[6] & 0xFF) | ((data[7] << 24) & 0xFF000000);
	uint32_t track_id  = ((data[8] << 16) & 0xFF0000) | ((data[9] << 8) & 0xFF00) | (data[10] & 0xFF);
	uint8_t codec_id         = (data[11] >> 4) & 0x0F;
	uint8_t sample_rate_flag = (data[11] >> 2) & 0x03;
	uint8_t bit_count_flag   = (data[11] >> 1) & 0x01;
	uint8_t channel_flag     = (data[11]) & 0x01;
	data += 11;
	data_size -= 11;

	size_t post_size = ((*(data + data_size - 4)) << 24) | ((*(data + data_size - 3)) << 16) | ((*(data + data_size - 2)) << 8) | (*(data + data_size - 1));
	if(size + 11 != post_size) {
		ERR_PRINT("crazy size data, out of flv format?");
		return NULL;
	}
	return ttLibC_FlvAudioTag_make(
			prev_tag,
			data,
			data_size - 4,
			true,
			timestamp,
			1000,
			track_id,
			codec_id,
			sample_rate_flag,
			bit_count_flag,
			channel_flag);
}

bool TT_ATTRIBUTE_INNER ttLibC_FlvAudioTag_getFrame(
		ttLibC_FlvAudioTag *audio_tag,
		ttLibC_getFrameFunc callback,
		void *ptr) {
	return ttLibC_FlvFrameManager_readAudioBinary(
			audio_tag->inherit_super.frameManager,
			audio_tag->inherit_super.inherit_super.inherit_super.data,
			audio_tag->inherit_super.inherit_super.inherit_super.buffer_size,
			audio_tag->inherit_super.inherit_super.inherit_super.pts,
			callback,
			ptr);
}

bool TT_ATTRIBUTE_INNER ttLibC_FlvAudioTag_writeTag(
		ttLibC_FlvWriter_ *writer,
		ttLibC_Frame *frame,
		ttLibC_ContainerWriteFunc callback,
		void *ptr) {
	if(frame->type == frameType_aac) {
		ttLibC_Aac *aac = (ttLibC_Aac *)frame;
		if(aac->type == AacType_dsi) {
			return true;
		}
	}
	ttLibC_DynamicBuffer *buffer = ttLibC_DynamicBuffer_make();
	ttLibC_DynamicBuffer_alloc(buffer, 11);
	uint8_t *data = ttLibC_DynamicBuffer_refData(buffer);
	data[0] = 0x08;
	// size(update later.)
	data[1]  = 0x00;
	data[2]  = 0x00;
	data[3]  = 0x00;
	// pts from frame information.
	data[4]  = (frame->pts >> 16) & 0xFF;
	data[5]  = (frame->pts >> 8) & 0xFF;
	data[6]  = frame->pts & 0xFF;
	data[7]  = (frame->pts >> 24) & 0xFF;
	// streamId
	data[8]  = 0x00;
	data[9]  = 0x00;
	data[10] = 0x00;
	if(frame->type == frameType_aac) {
		ttLibC_Aac *aac = (ttLibC_Aac *)frame;
		// check crc32 to decide msh
		uint32_t crc32_value = ttLibC_Aac_getConfigCrc32(aac);
		if(writer->audio_track.crc32 == 0 || writer->audio_track.crc32 != crc32_value) {
			// add information
			ttLibC_FlvFrameManager_getAacDsiData(
					frame,
					buffer);
			// update size
			uint32_t size = ttLibC_DynamicBuffer_refSize(buffer) - 11;
			data = ttLibC_DynamicBuffer_refData(buffer);
			data[1]  = (size >> 16) & 0xFF;
			data[2]  = (size >> 8) & 0xFF;
			data[3]  = size & 0xFF;
			uint32_t endSize = ttLibC_DynamicBuffer_refSize(buffer);
			uint32_t be_endSize = be_uint32_t(endSize);
			ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_endSize, 4);
			// done.
			if(callback != NULL) {
				if(!callback(
						ptr,
						ttLibC_DynamicBuffer_refData(buffer),
						ttLibC_DynamicBuffer_refSize(buffer))) {
					ttLibC_DynamicBuffer_close(&buffer);
					return false;
				}
			}
			// update buffer size to 11 byte.
			ttLibC_DynamicBuffer_alloc(buffer, 11);
			writer->audio_track.crc32 = crc32_value;
		}
	}
	// add body data.
	ttLibC_FlvFrameManager_getData(
			frame,
			buffer);
	// update size
	uint32_t size = ttLibC_DynamicBuffer_refSize(buffer) - 11;
	data = ttLibC_DynamicBuffer_refData(buffer);
	data[1]  = (size >> 16) & 0xFF;
	data[2]  = (size >> 8) & 0xFF;
	data[3]  = size & 0xFF;
	uint32_t endSize = ttLibC_DynamicBuffer_refSize(buffer);
	uint32_t be_endSize = be_uint32_t(endSize);
	ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_endSize, 4);

	bool result = true;
	if(callback != NULL) {
		result = callback(
				ptr,
				ttLibC_DynamicBuffer_refData(buffer),
				ttLibC_DynamicBuffer_refSize(buffer));
	}
	ttLibC_DynamicBuffer_close(&buffer);
	return result;
}
