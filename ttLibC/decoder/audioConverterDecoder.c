/**
 * @file   audioConverterDecoder.c
 * @brief  osx or iso native audio decode.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/01
 */

#ifdef __ENABLE_APPLE__

#include "audioConverterDecoder.h"
#include "../ttLibC_predef.h"
#include "../_log.h"
#include "../allocator.h"
#include "../frame/audio/pcms16.h"
#include "../frame/audio/aac.h"
#include <string.h>
#include <AudioToolbox/AudioToolbox.h>

/**
 * audioConverter decoder detail definition.
 */
typedef struct ttLibC_Decoder_AudioConverter_AcDecoder_ {
	ttLibC_AcDecoder inherit_super;

	AudioConverterRef converter;
	uint32_t sample_rate;
	uint32_t channel_num;
	ttLibC_Frame_Type frame_type;

	uint8_t *data;
	size_t data_size;
	uint32_t packets_capacity;

	// decoded object.
	ttLibC_PcmS16 *pcms16;

	uint64_t pts;
	bool is_pts_initialized;
	ttLibC_Audio *audio;
	AudioStreamPacketDescription aspds;
} ttLibC_Decoder_AudioConverter_AcDecoder_;

typedef ttLibC_Decoder_AudioConverter_AcDecoder_ ttLibC_AcDecoder_;

ttLibC_AcDecoder TT_VISIBILITY_DEFAULT *ttLibC_AcDecoder_make(
		uint32_t sample_rate,
		uint32_t channel_num,
		ttLibC_Frame_Type target_frame_type) {
	ttLibC_AcDecoder_ *decoder = ttLibC_malloc(sizeof(ttLibC_AcDecoder_));
	if(decoder == NULL) {
		return NULL;
	}
	decoder->converter = NULL;
	decoder->pcms16 = NULL;
	decoder->data = NULL;
	decoder->data_size = 0;
	decoder->packets_capacity = 0;
	decoder->sample_rate = sample_rate;
	decoder->channel_num = channel_num;
	decoder->frame_type = target_frame_type;
	decoder->inherit_super.channel_num = channel_num;
	decoder->inherit_super.sample_rate = sample_rate;
	decoder->inherit_super.frame_type = target_frame_type;
	decoder->is_pts_initialized = false;
	OSStatus err = noErr;
	AudioStreamBasicDescription srcFormat, dstFormat;
	// input
	switch(decoder->frame_type) {
	case frameType_aac:
		srcFormat.mFormatID         = kAudioFormatMPEG4AAC;
		// need flags maybe(default = low?)
		break;
	case frameType_mp3:
		srcFormat.mFormatID         = kAudioFormatMPEGLayer3;
		break;
	case frameType_pcm_alaw:
		srcFormat.mFormatID         = kAudioFormatALaw;
		break;
	case frameType_pcm_mulaw:
		srcFormat.mFormatID         = kAudioFormatULaw;
		break;
	default:
		ttLibC_free(decoder);
		ERR_PRINT("unknown frame type:%d", decoder->frame_type);
		return NULL;
	}
	srcFormat.mChannelsPerFrame = channel_num;
	srcFormat.mBitsPerChannel   = 0;
	srcFormat.mBytesPerFrame    = 0;
	srcFormat.mBytesPerPacket   = 0;
	srcFormat.mSampleRate       = sample_rate;
	srcFormat.mFormatFlags      = 0;
	srcFormat.mFramesPerPacket  = 0;

	// output
	dstFormat.mFormatID         = kAudioFormatLinearPCM;
	dstFormat.mChannelsPerFrame = channel_num;
	dstFormat.mBitsPerChannel   = 16;
	dstFormat.mBytesPerFrame    = 2 * dstFormat.mChannelsPerFrame;
	dstFormat.mBytesPerPacket   = dstFormat.mBytesPerFrame;
	dstFormat.mFramesPerPacket  = 1;
	dstFormat.mFormatFlags      = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
	dstFormat.mSampleRate       = sample_rate;

	err = AudioConverterNew(&srcFormat, &dstFormat, &decoder->converter);
	if(err != noErr) {
		ERR_PRINT("failed to create audio converter.");
		ttLibC_free(decoder);
		return NULL;
	}
	// capacity for 0.1 sec.
	decoder->packets_capacity = sample_rate / 10;
	decoder->data_size = decoder->packets_capacity * dstFormat.mBytesPerPacket;
	decoder->data = ttLibC_malloc(decoder->data_size);
	if(decoder->data == NULL) {
		AudioConverterDispose(decoder->converter);
		ttLibC_free(decoder);
		return NULL;
	}
	return (ttLibC_AcDecoder *)decoder;
}

static OSStatus AcDecoder_decodeDataProc(
		AudioConverterRef converter,
		uint32_t *ioNumberDataPackets,
		AudioBufferList *ioData,
		AudioStreamPacketDescription **outDataPacketDescription,
		void *inUserData) {
	(void)converter;
	ttLibC_AcDecoder_ *decoder = (ttLibC_AcDecoder_ *)inUserData;
	ttLibC_Audio *audio = decoder->audio;
	if(audio == NULL) {
		return 15;
	}
	uint8_t *data = audio->inherit_super.data;
	size_t data_size = audio->inherit_super.buffer_size;
	switch(audio->inherit_super.type) {
	case frameType_aac:
		{
			ttLibC_Aac *aac = (ttLibC_Aac *)audio;
			// use aac data as raw type.
			switch(aac->type) {
			case AacType_adts:
				data += 7;
				data_size -= 7;
				break;
			case AacType_raw:
				break;
			case AacType_dsi:
			default:
				return 0;
			}
		}
		/* no break */
	case frameType_mp3:
		{
			decoder->aspds.mDataByteSize = data_size;
			decoder->aspds.mStartOffset = 0;
			decoder->aspds.mVariableFramesInPacket = audio->sample_num;
			*ioNumberDataPackets = 1;
		}
		break;
	case frameType_pcm_alaw:
	case frameType_pcm_mulaw:
		{
			decoder->aspds.mDataByteSize = data_size;
			decoder->aspds.mStartOffset = 0;
			decoder->aspds.mVariableFramesInPacket = 1;
			*ioNumberDataPackets = audio->sample_num;
		}
		break;
	default:
		break;
	}
	// supply data.
	ioData->mBuffers[0].mData = data;
	ioData->mBuffers[0].mDataByteSize = data_size;
	ioData->mBuffers[0].mNumberChannels = audio->channel_num;
	if(outDataPacketDescription != NULL) {
		*outDataPacketDescription = &decoder->aspds;
	}
	decoder->audio = NULL;
	return 0;
}

bool TT_VISIBILITY_DEFAULT ttLibC_AcDecoder_decode(
		ttLibC_AcDecoder *decoder,
		ttLibC_Audio *audio,
		ttLibC_AcDecodeFunc callback,
		void *ptr) {
	if(decoder == NULL) {
		return false;
	}
	if(audio == NULL) {
		return true;
	}
	if(audio->inherit_super.type == frameType_aac) {
		ttLibC_Aac *aac = (ttLibC_Aac *)audio;
		switch(aac->type) {
		case AacType_raw:
		case AacType_adts:
			break;
		case AacType_dsi:
		default:
			return true;
		}
	}
	ttLibC_AcDecoder_ *decoder_ = (ttLibC_AcDecoder_ *)decoder;
	if(!decoder_->is_pts_initialized) {
		decoder_->is_pts_initialized = true;
		decoder_->pts = audio->inherit_super.pts * decoder_->sample_rate / audio->inherit_super.timebase;
	}
	AudioBufferList fillBufList;
	fillBufList.mNumberBuffers = 1;
	fillBufList.mBuffers[0].mNumberChannels = decoder_->channel_num;
	fillBufList.mBuffers[0].mDataByteSize = decoder_->data_size;
	fillBufList.mBuffers[0].mData = decoder_->data;
	uint32_t ioOutputDataPackets = decoder_->packets_capacity;
	OSStatus err = noErr;
	decoder_->audio = audio;
	do {
		err = AudioConverterFillComplexBuffer(
				decoder_->converter,
				AcDecoder_decodeDataProc,
				decoder_,
				&ioOutputDataPackets,
				&fillBufList,
				NULL);
		if(err != noErr && err != 15) {
			ERR_PRINT("unexpected error is happened:%x %d", err, err);
			return false;
		}
		decoder_->audio = NULL;
		ttLibC_PcmS16 *p = ttLibC_PcmS16_make(
				decoder_->pcms16,
				PcmS16Type_littleEndian,
				decoder_->sample_rate,
				ioOutputDataPackets,
				decoder_->channel_num,
				decoder_->data,
				ioOutputDataPackets * 2 * decoder_->channel_num,
				decoder_->data,
				ioOutputDataPackets * 2 * decoder_->channel_num,
				NULL,
				0,
				true,
				decoder_->pts,
				decoder_->sample_rate);
		if(p == NULL) {
			return false;
		}
		decoder_->pcms16 = p;
		if(!callback(ptr, decoder_->pcms16)) {
			return false;
		}
		decoder_->pts += ioOutputDataPackets;
	} while(err == noErr);
	return true;
}

void TT_VISIBILITY_DEFAULT ttLibC_AcDecoder_close(ttLibC_AcDecoder **decoder) {
	ttLibC_AcDecoder_ *target = (ttLibC_AcDecoder_ *)*decoder;
	if(target == NULL) {
		return;
	}
	if(target->converter != NULL) {
		AudioConverterDispose(target->converter);
		target->converter = NULL;
	}
	if(target->data != NULL) {
		ttLibC_free(target->data);
		target->data = NULL;
	}
	ttLibC_PcmS16_close(&target->pcms16);
	ttLibC_free(target);
	*decoder = NULL;
}

#endif
