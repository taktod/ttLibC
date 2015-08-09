/*
 * @file   audio.c
 * @brief  audio frame information.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/19
 */

#include "audio.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "aac.h"
#include "adpcmImaWav.h"
#include "mp3.h"
#include "nellymoser.h"
#include "opus.h"
#include "pcmAlaw.h"
#include "pcmf32.h"
#include "pcmMulaw.h"
#include "pcms16.h"
#include "speex.h"
#include "vorbis.h"

#include "../../log.h"

/*
 * make audio frame.
 * @param prev_frame    reuse frame.
 * @param frame_size    allocate frame size
 * @param frame_type    type of frame
 * @param sample_rate   sample rate
 * @param sample_num    sample number
 * @param channel_num   channel number
 * @param data          data
 * @param data_size     data size
 * @param non_copy_mode true:hold the data pointer, false:data will copy.
 * @param pts           pts for data.
 * @param timebase      timebase for pts.
 */
ttLibC_Audio *ttLibC_Audio_make(
		ttLibC_Audio *prev_frame,
		size_t frame_size,
		ttLibC_Frame_Type frame_type,
		uint32_t sample_rate,
		uint32_t sample_num,
		uint32_t channel_num,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	ttLibC_Audio *audio = prev_frame;
	size_t buffer_size_ = data_size;
	size_t data_size_ = data_size;
	switch(frame_type) {
	case frameType_aac:
	case frameType_adpcm_ima_wav:
	case frameType_mp3:
	case frameType_nellymoser:
	case frameType_opus:
	case frameType_pcm_alaw:
	case frameType_pcmF32:
	case frameType_pcm_mulaw:
	case frameType_pcmS16:
	case frameType_speex:
	case frameType_vorbis:
		break;
	default:
		ERR_PRINT("unknown audio frame type.%d", frame_type);
		return NULL;
	}
	if(frame_size == 0) {
		frame_size = sizeof(ttLibC_Audio);
	}
	if(audio == NULL) {
		audio = malloc(frame_size);
		if(audio == NULL) {
			ERR_PRINT("failed to allocate memory for audio frame.");
			return NULL;
		}
		audio->inherit_super.data = NULL;
	}
	else {
		if(!audio->inherit_super.is_non_copy) {
			if(non_copy_mode || audio->inherit_super.data_size < data_size) {
				free(audio->inherit_super.data);
				audio->inherit_super.data = NULL;
			}
			else {
				data_size_ = audio->inherit_super.data_size;
			}
		}
	}
	audio->channel_num               = channel_num;
	audio->sample_num                = sample_num;
	audio->sample_rate               = sample_rate;
	audio->inherit_super.buffer_size = buffer_size_;
	audio->inherit_super.data_size   = data_size_;
	audio->inherit_super.is_non_copy = non_copy_mode;
	audio->inherit_super.pts         = pts;
	audio->inherit_super.timebase    = timebase;
	audio->inherit_super.type        = frame_type;
	if(non_copy_mode) {
		audio->inherit_super.data = data;
	}
	else {
		if(audio->inherit_super.data == NULL) {
			audio->inherit_super.data = malloc(data_size);
			if(audio->inherit_super.data == NULL) {
				ERR_PRINT("failed to allocate memory for data.");
				if(prev_frame == NULL) {
					free(audio);
				}
				return NULL;
			}
		}
		memcpy(audio->inherit_super.data, data, data_size);
	}
	return audio;
}

/*
 * close frame(use internal)
 * @param frame
 */
void ttLibC_Audio_close_(ttLibC_Audio **frame) {
	ttLibC_Audio *target = *frame;
	if(target == NULL) {
		return;
	}
	if(!target->inherit_super.is_non_copy) {
		free(target->inherit_super.data);
	}
	free(target);
	*frame = NULL;
}

/*
 * close frame
 * @param frame
 */
void ttLibC_Audio_close(ttLibC_Audio **frame) {
	ttLibC_Audio *target = *frame;
	if(target == NULL) {
		return;
	}
	switch(target->inherit_super.type) {
	case frameType_aac:
		ttLibC_Aac_close((ttLibC_Aac **)frame);
		break;
	case frameType_adpcm_ima_wav:
		ttLibC_AdpcmImaWav_close((ttLibC_AdpcmImaWav **)frame);
		break;
	case frameType_mp3:
		ttLibC_Mp3_close((ttLibC_Mp3 **)frame);
		break;
	case frameType_nellymoser:
		ttLibC_Nellymoser_close((ttLibC_Nellymoser **)frame);
		break;
	case frameType_opus:
		ttLibC_Opus_close((ttLibC_Opus **)frame);
		break;
	case frameType_pcm_alaw:
		ttLibC_PcmAlaw_close((ttLibC_PcmAlaw **)frame);
		break;
	case frameType_pcmF32:
		ttLibC_PcmF32_close((ttLibC_PcmF32 **)frame);
		break;
	case frameType_pcm_mulaw:
		ttLibC_PcmMulaw_close((ttLibC_PcmMulaw **)frame);
		break;
	case frameType_pcmS16:
		ttLibC_PcmS16_close((ttLibC_PcmS16 **)frame);
		break;
	case frameType_speex:
		ttLibC_Speex_close((ttLibC_Speex **)frame);
		break;
	case frameType_vorbis:
		ttLibC_Vorbis_close((ttLibC_Vorbis **)frame);
		break;
	default:
		ERR_PRINT("unknown type:%d", target->inherit_super.type);
		break;
	}
}


