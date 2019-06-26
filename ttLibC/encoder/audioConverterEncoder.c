/**
 * @file   audioConverterEncoder.c
 * @brief  osx or iso native audio encode.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/01
 */

#ifdef __ENABLE_APPLE__

#include "audioConverterEncoder.h"
#include "../_log.h"
#include "../allocator.h"
#include "../util/hexUtil.h"
#include "../frame/audio/aac.h"
#include "../frame/audio/pcmAlaw.h"
#include "../frame/audio/pcmMulaw.h"
#include <AudioToolbox/AudioToolbox.h>
#include <string.h>

/**
 * audioConverter encoder detail definition.
 */
typedef struct ttLibC_Encoder_AudioConverter_AcEncoder_ {
	ttLibC_AcEncoder inherit_super;

	AudioConverterRef converter;
	AudioStreamPacketDescription *outputPacketDescriptions; // output packet description.
	uint32_t sample_rate;
	uint32_t channel_num;
	ttLibC_Frame_Type frame_type;
	// buffer for encoded data.
	uint8_t *data;
	size_t data_size;
	uint32_t numOutputPackets;
	uint64_t pts;
	uint32_t unit_samples;
	uint64_t aac_dsi_info;
	bool is_pts_initialized;

	ttLibC_Audio *audio;
} ttLibC_Encoder_AudioConverter_AcEncoder_;

typedef ttLibC_Encoder_AudioConverter_AcEncoder_ ttLibC_AcEncoder_;

ttLibC_AcEncoder TT_ATTRIBUTE_API *ttLibC_AcEncoder_make(
		uint32_t sample_rate,
		uint32_t channel_num,
		uint32_t target_bitrate,
		ttLibC_Frame_Type target_frame_type) {
	ttLibC_AcEncoder_ *encoder = ttLibC_malloc(sizeof(ttLibC_AcEncoder_));
	if(encoder == NULL) {
		return NULL;
	}
	encoder->outputPacketDescriptions = NULL;
	encoder->audio = NULL;
	encoder->data = NULL;
	encoder->converter = NULL;
	encoder->frame_type = target_frame_type;
	encoder->sample_rate = sample_rate;
	encoder->channel_num = channel_num;
	encoder->inherit_super.bitrate = target_bitrate;
	encoder->inherit_super.channel_num = channel_num;
	encoder->inherit_super.frame_type = target_frame_type;
	encoder->inherit_super.sample_rate = sample_rate;
	encoder->is_pts_initialized = false;
	OSStatus err = noErr;
	AudioStreamBasicDescription srcFormat, dstFormat;
	// input
	srcFormat.mFormatID = kAudioFormatLinearPCM;
	srcFormat.mChannelsPerFrame = channel_num;
	srcFormat.mBitsPerChannel = 16; // 16bit
	srcFormat.mBytesPerPacket = 2 * srcFormat.mChannelsPerFrame;
	srcFormat.mBytesPerFrame = srcFormat.mBytesPerPacket;
	srcFormat.mFramesPerPacket = 1;
	srcFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
	srcFormat.mSampleRate = sample_rate;

	// output
	switch(encoder->frame_type) {
	case frameType_aac:
		// use low profile.
		dstFormat.mFormatID = kAudioFormatMPEG4AAC;
		dstFormat.mFormatFlags = kMPEG4Object_AAC_LC;
		// make dsi information for aac.
		ttLibC_Aac_getDsiInfo(AacObject_Low, sample_rate, channel_num, &encoder->aac_dsi_info, sizeof(uint64_t));
		break;
	case frameType_mp3:
		ERR_PRINT("mp3 encode is not support."); // @see https://developer.apple.com/library/content/documentation/MusicAudio/Conceptual/CoreAudioOverview/SupportedAudioFormatsMacOSX/SupportedAudioFormatsMacOSX.html#//apple_ref/doc/uid/TP40003577-CH7-SW1
		ttLibC_free(encoder);
		return NULL;
	case frameType_pcm_alaw:
		dstFormat.mFormatID = kAudioFormatALaw;
		break;
	case frameType_pcm_mulaw:
		dstFormat.mFormatID = kAudioFormatULaw;
		break;
	default:
		ERR_PRINT("unknown frame type:%d", encoder->frame_type);
		ttLibC_free(encoder);
		return NULL;
	}
	dstFormat.mChannelsPerFrame = channel_num;
	dstFormat.mBitsPerChannel = 0;
	dstFormat.mBytesPerFrame = 0;
	dstFormat.mBytesPerPacket = 0;
	dstFormat.mSampleRate = sample_rate;
	dstFormat.mFramesPerPacket = 0;
	err = AudioConverterNew(&srcFormat, &dstFormat, &encoder->converter);
	if(err != noErr) {
		ERR_PRINT("failed to make audioConverter.");
		ttLibC_free(encoder);
		return NULL;
	}
	uint32_t size = 0;
	size = sizeof(target_bitrate);
	switch(encoder->frame_type) {
	case frameType_aac:
		{
			err = AudioConverterSetProperty(encoder->converter, kAudioConverterEncodeBitRate, size, &target_bitrate);
			if(err != noErr) {
				ERR_PRINT("failed to set bitrate for audioConverter.");
				ttLibC_AcEncoder_close((ttLibC_AcEncoder **)&encoder);
				return NULL;
			}
		}
		break;
	default:
		break;
	}
	// setup buffer for data.
	size = sizeof(dstFormat);
	err = AudioConverterGetProperty(encoder->converter, kAudioConverterCurrentOutputStreamDescription, &size, &dstFormat);
	if(err != noErr) {
		ERR_PRINT("failed to get dst format information from converter.");
		ttLibC_AcEncoder_close((ttLibC_AcEncoder **)&encoder);
		return NULL;
	}
	encoder->data_size = 16384; // too big?
	encoder->data = ttLibC_malloc(encoder->data_size);
	encoder->unit_samples = dstFormat.mFramesPerPacket;
	uint32_t outputSizePerPacket = dstFormat.mBytesPerPacket;
	if(outputSizePerPacket == 0) {
		size = sizeof(outputSizePerPacket);
		err = AudioConverterGetProperty(encoder->converter, kAudioConverterPropertyMaximumOutputPacketSize, &size, &outputSizePerPacket);
		if(err != noErr) {
			ERR_PRINT("failed to get vbr packet size.");
			ttLibC_AcEncoder_close((ttLibC_AcEncoder **)&encoder);
			return NULL;
		}
		encoder->outputPacketDescriptions = (AudioStreamPacketDescription *)ttLibC_malloc(sizeof(AudioStreamPacketDescription) * (int)(encoder->data_size / outputSizePerPacket));
	}
	encoder->numOutputPackets = (uint32_t)(encoder->data_size / outputSizePerPacket);
	// done.
	return (ttLibC_AcEncoder *)encoder;
}

static OSStatus AcEncoder_encodeDataProc(
		AudioConverterRef converter,
		uint32_t *ioNumberDataPackets,
		AudioBufferList *ioData,
		AudioStreamPacketDescription **outDataPacketDescription,
		void *inUserData) {
	(void)converter;
	(void)outDataPacketDescription;
	// use double pointer. for overwrite with NULL.
	void **inud = (void **)inUserData;
	ttLibC_PcmS16 *pcm = (ttLibC_PcmS16 *)*inud;
	if(pcm == NULL) {
		// in the case of pcm = NULL, return something, to stop convert buffer.
		return 15;
	}
/*
	// update reply size.
	*ioNumberDataPackets = pcm->inherit_super.inherit_super.buffer_size / 2 / pcm->inherit_super.channel_num;
	ioData->mBuffers[0].mData = pcm->inherit_super.inherit_super.data;
	ioData->mBuffers[0].mDataByteSize = pcm->inherit_super.inherit_super.buffer_size;
	ioData->mBuffers[0].mNumberChannels = pcm->inherit_super.channel_num;*/

	*ioNumberDataPackets = pcm->inherit_super.sample_num;
	ioData->mBuffers[0].mData = pcm->l_data;
	ioData->mBuffers[0].mDataByteSize = pcm->l_stride;
	ioData->mBuffers[0].mNumberChannels = pcm->inherit_super.channel_num;
	*inud = NULL; // put null for next callback.
	return noErr;
}

static bool AcEncoder_checkEncodedData(
		ttLibC_AcEncoder_ *encoder,
		void *data,
		size_t data_size,
		ttLibC_AcEncodeFunc callback,
		void *ptr) {
	switch(encoder->frame_type) {
	case frameType_aac:
		{
			ttLibC_Aac *aac = ttLibC_Aac_make(
					(ttLibC_Aac *)encoder->audio,
					AacType_raw,
					encoder->sample_rate,
					encoder->unit_samples,
					encoder->channel_num,
					data,
					data_size,
					true,
					encoder->pts,
					encoder->sample_rate,
					encoder->aac_dsi_info);
			if(aac == NULL) {
				ERR_PRINT("failed to make raw aac frame.");
				return false;
			}
			encoder->pts += encoder->unit_samples;
			encoder->audio = (ttLibC_Audio *)aac;
			return callback(ptr, encoder->audio);
		}
		break;
	case frameType_pcm_alaw:
		{
			ttLibC_PcmAlaw *pcmAlaw = ttLibC_PcmAlaw_make(
				(ttLibC_PcmAlaw *)encoder->audio,
				encoder->sample_rate,
				data_size,
				encoder->channel_num,
				data,
				data_size,
				true,
				encoder->pts,
				encoder->sample_rate);
			if(pcmAlaw == NULL) {
				ERR_PRINT("failed to make pamalaw frame.");
				return false;
			}
			encoder->pts += data_size;
			encoder->audio = (ttLibC_Audio *)pcmAlaw;
			return callback(ptr, encoder->audio);
		}
		break;
	case frameType_pcm_mulaw:
		{
			ttLibC_PcmMulaw *pcmMulaw = ttLibC_PcmMulaw_make(
				(ttLibC_PcmMulaw *)encoder->audio,
				encoder->sample_rate,
				data_size,
				encoder->channel_num,
				data,
				data_size,
				true,
				encoder->pts,
				encoder->sample_rate);
			if(pcmMulaw == NULL) {
				ERR_PRINT("failed to make pamalaw frame.");
				return false;
			}
			encoder->pts += data_size;
			encoder->audio = (ttLibC_Audio *)pcmMulaw;
			return callback(ptr, encoder->audio);
		}
		break;
	default:
		LOG_PRINT("unsupported audio frame:%d", encoder->frame_type);
		return false;
	}
	return true;
}

bool TT_ATTRIBUTE_API ttLibC_AcEncoder_encode(
		ttLibC_AcEncoder *encoder,
		ttLibC_PcmS16 *pcm,
		ttLibC_AcEncodeFunc callback,
		void *ptr) {
	ttLibC_AcEncoder_ *encoder_ = (ttLibC_AcEncoder_ *)encoder;
	if(encoder_ == NULL) {
		return false;
	}
	if(pcm == NULL) {
		return true;
	}
	switch(pcm->type) {
		case PcmS16Type_bigEndian:
		case PcmS16Type_bigEndian_planar:
		case PcmS16Type_littleEndian_planar:
			ERR_PRINT("only support little endian interleave.");
			return false;
		default:
			break;
	}
	if(!encoder_->is_pts_initialized) {
		encoder_->is_pts_initialized = true;
		encoder_->pts = pcm->inherit_super.inherit_super.pts * encoder_->sample_rate / pcm->inherit_super.inherit_super.timebase;
	}
	AudioBufferList fillBufList;
	fillBufList.mNumberBuffers = 1;
	fillBufList.mBuffers[0].mNumberChannels = encoder_->channel_num;
	fillBufList.mBuffers[0].mDataByteSize = encoder_->data_size;
	fillBufList.mBuffers[0].mData = encoder_->data;
	uint32_t ioOutputDataPackets = encoder_->numOutputPackets;
	OSStatus err = noErr;
	do {
		err = AudioConverterFillComplexBuffer(encoder_->converter, AcEncoder_encodeDataProc, &pcm, &ioOutputDataPackets, &fillBufList, encoder_->outputPacketDescriptions);
		if(err != noErr && err != 15) {
			ERR_PRINT("unexpected error is happened:%x %d", err, err);
			return false;
		}
		// check for generate data.
		pcm = NULL;
		
		if(encoder_->outputPacketDescriptions == NULL) {
			if(!AcEncoder_checkEncodedData(
				encoder_,
				encoder_->data,
				ioOutputDataPackets,
				callback,
				ptr
			)) {
				return false;
			}
		}
		else {
			for(uint32_t i = 0;i < ioOutputDataPackets;++ i) {
				if(!AcEncoder_checkEncodedData(
						encoder_,
						encoder_->data + encoder_->outputPacketDescriptions[i].mStartOffset,
						encoder_->outputPacketDescriptions[i].mDataByteSize,
						callback,
						ptr)) {
					return false;
				}
			}
		}
	} while(err == noErr);
	return true;
}

void TT_ATTRIBUTE_API ttLibC_AcEncoder_close(ttLibC_AcEncoder **encoder) {
	ttLibC_AcEncoder_ *target = (ttLibC_AcEncoder_ *)*encoder;
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
	if(target->outputPacketDescriptions != NULL) {
		ttLibC_free(target->outputPacketDescriptions);
		target->outputPacketDescriptions = NULL;
	}
	ttLibC_Audio_close(&target->audio);
	ttLibC_free(target);
	*encoder = NULL;
}

#endif
