/*
 * @file   audioUnitUtil.c
 * @brief  play or record sound by audioUnit in OSX or iOS
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2015/12/22
 */

#ifdef __ENABLE_APPLE__

#include "audioUnitUtil.h"
#include "../log.h"
#include "../allocator.h"
#include <AudioToolbox/AudioToolbox.h>
#include <AudioUnit/AudioUnit.h>
#include <string.h>

#ifdef __APPLE__
#	include "TargetConditionals.h"
#endif

/*
 * structure for audioUnit player.
 */
typedef struct {
	/** inherit data from ttLibC_AuPlayer */
	ttLibC_AuPlayer inherit_super;
	/** audioUnit for play */
 	AudioUnit outputUnit;
	/** hold waiting pcm data. */
	int16_t *buffer;
	/** size of buffer */
	size_t buffer_size;
	/** read index */
	uint32_t read_index;
	/** write index */
	uint32_t write_index;
	/** target sample_rate */
	uint32_t sample_rate;
	/** target channel_num */
	uint32_t channel_num;
	/** current played pts */
	uint64_t pts;
} ttLibC_Util_AudioUnit_AuPlayer_;

typedef ttLibC_Util_AudioUnit_AuPlayer_ ttLibC_AuPlayer_;

/*
 * get the pcm data from buffer.
 * @param player     ttLibC_AuPlayer object.
 * @param ptr        pointer for play buffer.
 * @param sample_num request sample number.
 */
static bool AuPlayer_dequeue(
		ttLibC_AuPlayer *player,
		int16_t *ptr,
		uint32_t sample_num) {
	if(player == NULL) {
		return false;
	}
	if(ptr == NULL) {
		return false;
	}
	ttLibC_AuPlayer_ *player_ = (ttLibC_AuPlayer_ *)player;

	uint32_t err = 0;
	int max_index = player_->buffer_size / sizeof(int16_t);
	uint32_t added_sample_num = 0;
	// try to fill ptr.
	for(int i = 0;i < sample_num * player_->channel_num;++ i) {
		if(player_->read_index < player_->write_index) {
			++ added_sample_num;
			*(ptr + i) = player_->buffer[(player_->read_index % max_index)];
			// update read_index.
			++ player_->read_index;
		}
		else {
			// data is missing, try to fill with 0(no sound).
			err ++;
			*(ptr + i) = 0;
		}
	}
	// for stereo, sample_num is double counted. fix the number.
	if(player_->channel_num == 2) {
		added_sample_num >>= 1;
	}
	// update pts.
	player_->pts += added_sample_num;
	player_->inherit_super.pts = player_->pts;
	return true;
}

/*
 * callback for audio unit.
 * @param inRefCon
 * @param ioActionFlags
 * @param inTimeStamp
 * @param inBusNumber
 * @param inNumberFrames
 * @param ioData
 * @return OSStatus number.
 */
static OSStatus AuPlayer_outputCallback(
		void *inRefCon,
		AudioUnitRenderActionFlags *ioActionFlags,
		const AudioTimeStamp *inTimeStamp,
		uint32_t inBusNumber,
		uint32_t inNumberFrames,
		AudioBufferList *ioData) {
	OSStatus err = noErr;
	if(ioData->mNumberBuffers != 1) {
		ERR_PRINT("only in buffers = 1.");
	}
	else {
		// need to update buffer data.
		int16_t *ptr = ioData->mBuffers[0].mData;
		ttLibC_AuPlayer *player = (inRefCon);
		// drain data from queue.
		AuPlayer_dequeue(player, ptr, inNumberFrames);
	}
	return err;
}

/*
 * make audio player by audioUnit.
 * @param sample_rate target sample rate
 * @param channel_num target channel num
 * @param type        device type(kAudioUnitSubType)
 * @return ttLibC_AuPlayer object.
 */
ttLibC_AuPlayer *ttLibC_AuPlayer_make(
		uint32_t sample_rate,
		uint32_t channel_num,
		uint32_t type) {
	ttLibC_AuPlayer_ *player = ttLibC_malloc(sizeof(ttLibC_AuPlayer_));
	if(player == NULL) {
		ERR_PRINT("failed to alloc device object.");
		return NULL;
	}
	OSStatus err = noErr;
	AudioComponentDescription desc;
	desc.componentType = kAudioUnitType_Output;
	switch(type) {
	case AuPlayerType_DefaultOutput:
#if TARGET_IPHONE_SIMULATOR
		// iOS Simulator
		desc.componentSubType = kAudioUnitSubType_RemoteIO;
#elif TARGET_OS_IPHONE
		// iOS device
		desc.componentSubType = kAudioUnitSubType_RemoteIO;
#elif TARGET_OS_MAC
		// Other kinds of Mac OS
		desc.componentSubType = kAudioUnitSubType_DefaultOutput;
#else
		// Unsupported platform
#endif
		break;
	default:
		desc.componentSubType = type;
		break;
	}
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	desc.componentFlags = 0;
	desc.componentFlagsMask = 0;
	// make audioComponentInstance
	AudioComponent component = AudioComponentFindNext(NULL, &desc);
	err = AudioComponentInstanceNew(component, &player->outputUnit);
	if(err != noErr) {
		ERR_PRINT("failed to make audioComponentInstance");
		ttLibC_free(player);
		return NULL;
	}
	// set callback.
	AURenderCallbackStruct callback;
	callback.inputProc = AuPlayer_outputCallback;
	callback.inputProcRefCon = player;
	err = AudioUnitSetProperty(
			player->outputUnit,
			kAudioUnitProperty_SetRenderCallback,
			kAudioUnitScope_Global,
			0,
			&callback,
			sizeof(AURenderCallbackStruct));
	if(err != noErr) {
		ERR_PRINT("failed to set callback.");
		AudioComponentInstanceDispose(player->outputUnit);
		ttLibC_free(player);
		return NULL;
	}
	// set working audio format.
	AudioStreamBasicDescription outputFormat;
	outputFormat.mSampleRate = sample_rate;
	outputFormat.mFormatID = kAudioFormatLinearPCM;
	outputFormat.mFormatFlags = kAudioFormatFlagIsPacked | kAudioFormatFlagIsSignedInteger;
	outputFormat.mBitsPerChannel = 16;
	outputFormat.mChannelsPerFrame = channel_num;
	outputFormat.mFramesPerPacket = 1;
	outputFormat.mBytesPerFrame = outputFormat.mBitsPerChannel / 8 * outputFormat.mChannelsPerFrame;
	outputFormat.mBytesPerPacket = outputFormat.mBytesPerFrame * outputFormat.mFramesPerPacket;
	err = AudioUnitSetProperty(
			player->outputUnit,
			kAudioUnitProperty_StreamFormat,
			kAudioUnitScope_Input,
			0,
			&outputFormat,
			sizeof(AudioStreamBasicDescription));
	if(err != noErr) {
		ERR_PRINT("failed to set audio format.");
		AudioComponentInstanceDispose(player->outputUnit);
		ttLibC_free(player);
		return NULL;
	}
	// initialize.
	err = AudioUnitInitialize(player->outputUnit);
	if(err != noErr) {
		ERR_PRINT("failed to initialize outputUnit.");
		AudioComponentInstanceDispose(player->outputUnit);
		ttLibC_free(player);
		return NULL;
	}
	player->sample_rate = sample_rate;
	player->inherit_super.sample_rate = sample_rate;
	player->channel_num = channel_num;
	player->inherit_super.channel_num = channel_num;
	player->pts = 0;
	player->inherit_super.pts = 0;
	player->read_index = 0;
	player->write_index = 0;
	// TODO wanna use extentable buffer.
	player->buffer_size = sample_rate * sizeof(int16_t) * channel_num * 2; // for 2 sec.
	player->buffer = (int16_t *)ttLibC_malloc(player->buffer_size);
	err = AudioOutputUnitStart(player->outputUnit);
	if(err != noErr) {
		ERR_PRINT("failed to start output unit.");
		AudioUnitUninitialize(player->outputUnit);
		AudioComponentInstanceDispose(player->outputUnit);
		ttLibC_free(player);
		return NULL;
	}
	return (ttLibC_AuPlayer *)player;
}

/*
 * queue pcm data.
 * @param player target ttLibC_AuPlayer object.
 * @param pcmS16 pcmS16 object for play.
 * @return true:success to put queue. false:error to put queue(queue is full, you need to try again later.)
 */
bool ttLibC_AuPlayer_queue(ttLibC_AuPlayer *player, ttLibC_PcmS16 *pcmS16) {
	if(player == NULL) {
		return false;
	}
	if(pcmS16 == NULL) {
		return false;
	}
	ttLibC_AuPlayer_ *player_ = (ttLibC_AuPlayer_ *)player;
	if(player_->pts == 0) {
		// if the pts is 0. merge the pts to pcm data.
		player_->pts = pcmS16->inherit_super.inherit_super.pts * player_->inherit_super.sample_rate / pcmS16->inherit_super.inherit_super.timebase;
		player_->inherit_super.pts = player_->pts;
	}
	int max_index = player_->buffer_size / sizeof(int16_t);
	int last_pos = player_->read_index + max_index;
	int index_gap = last_pos - player_->write_index;
	int target_sample = index_gap / player_->inherit_super.channel_num;
	if(target_sample < pcmS16->inherit_super.sample_num) {
		// buffer doesn't have enough space for this pcm data.
		// do nothing.
		// because of this, we cannot work with huge size of pcmS16 object.
		return false;
	}
	else {
		switch (pcmS16->type) {
		case PcmS16Type_littleEndian:
		case PcmS16Type_bigEndian:
			{
				int target_index = pcmS16->inherit_super.sample_num * pcmS16->inherit_super.channel_num;
				int16_t *src_ptr = (int16_t *)pcmS16->l_data;
				int16_t *dst_ptr = player_->buffer;
				for(int i = 0;i < target_index;++ i) {
					dst_ptr[(player_->write_index % max_index)] = src_ptr[i];
					++ player_->write_index;
				}
			}
			break;
		case PcmS16Type_littleEndian_planar:
		case PcmS16Type_bigEndian_planar:
			{
				// TODO want to support planar data.
				return false;
			}
			break;
		default:
			break;
		}
	}
	// to prevent overflow, rewind index.
	if(player_->write_index > (max_index << 2)) {
		player_->write_index -= max_index;
		player_->read_index -= max_index;
	}
	return true;
}

/*
 * close player
 * @param player ttLibC_AuPlayer object.
 */
void ttLibC_AuPlayer_close(ttLibC_AuPlayer **player) {
	ttLibC_AuPlayer_ *target = (ttLibC_AuPlayer_ *)*player;
	if(target == NULL) {
		return;
	}
	AudioOutputUnitStop(target->outputUnit);
	AudioUnitUninitialize(target->outputUnit);
	AudioComponentInstanceDispose(target->outputUnit);
	ttLibC_free(target->buffer);
	ttLibC_free(target);
	*player = NULL;
}


/*
 * structure for audioUnit recorder.
 */
typedef struct ttLibC_Util_AudioUnit_AuRecorder_ {
	/** inherit data from ttLibC_AuRecorder */
	ttLibC_AuRecorder inherit_super;
	/** audioUnit for record */
	AudioUnit inputUnit;
	/** bufferList */
	AudioBufferList *bufferList;
	/** generate ttLibC_AudioFrame(reuse). */
	ttLibC_Audio *audio;

	/** target sample_rate */
	uint32_t sample_rate;
	/** target channel_num */
	uint32_t channel_num;
	/** current record pts. */
	uint64_t pts;

	/** callback for recording. */
	ttLibC_AuRecorderFunc callback;
	/** data pointer passed by user. */
	void *ptr;

	/** flag for now recording or not. */
	bool is_recording;
} ttLibC_Util_AudioUnit_AuRecorder_;

typedef ttLibC_Util_AudioUnit_AuRecorder_ ttLibC_AuRecorder_;

/**
 * callback for audio unit.
 * @param inRefCon
 * @param ioActionFlags
 * @param inTimeStamp
 * @param inBusNumber
 * @param inNumberFrames
 * @param ioData
 * @return OSStatus number.
 */
static OSStatus AuRecorder_inputCallback(
		void *inRefCon,
		AudioUnitRenderActionFlags *ioActionFlags,
		const AudioTimeStamp *inTimeStamp,
		uint32_t inBusNumber,
		uint32_t inNumberFrames,
		AudioBufferList *ioData) {
	ttLibC_AuRecorder_ *recorder = (ttLibC_AuRecorder_ *)inRefCon;
	OSStatus err = AudioUnitRender(
			recorder->inputUnit,
			ioActionFlags,
			inTimeStamp,
			inBusNumber,
			inNumberFrames,
			recorder->bufferList);
	if(err != noErr) {
		ERR_PRINT("failed to audioUnitRender:%d", err);
		ttLibC_AuRecorder_stop((ttLibC_AuRecorder *)recorder);
		return err;
	}
	// make pcmS16 from interleave data.
	ttLibC_PcmS16 *pcmS16 = ttLibC_PcmS16_make(
			(ttLibC_PcmS16 *)recorder->audio,
			PcmS16Type_littleEndian,
			recorder->sample_rate,
			inNumberFrames,
			recorder->channel_num,
			recorder->bufferList->mBuffers[0].mData,
			recorder->bufferList->mBuffers[0].mDataByteSize,
			recorder->bufferList->mBuffers[0].mData,
			recorder->bufferList->mBuffers[0].mDataByteSize,
			NULL,
			0,
			true,
			recorder->pts,
			recorder->sample_rate);
	if(pcmS16 == NULL) {
		ERR_PRINT("failed to make pcm buffer.");
		ttLibC_AuRecorder_stop((ttLibC_AuRecorder *)recorder);
		// TODO what number should I return?
		return noErr;
	}
	recorder->pts += pcmS16->inherit_super.sample_num;
	recorder->inherit_super.pts = recorder->pts;
	recorder->audio = (ttLibC_Audio *)pcmS16;
	if(!recorder->callback(recorder->ptr, recorder->audio)) {
		// stop is requested by callback.
		ttLibC_AuRecorder_stop((ttLibC_AuRecorder *)recorder);
	}
	return noErr;
}

/*
 * make audio recorder by audioUnit.
 * @param sample_rate target sample rate
 * @param channel_num target channel num
 * @param type        device type(kAudioUnitSubType)
 * @param device_id   device_id
 * @return ttLibC_AuRecorder object.
 */
ttLibC_AuRecorder *ttLibC_AuRecorder_make(
		uint32_t sample_rate,
		uint32_t channel_num,
		uint32_t type,
		uint32_t device_id) {
	ttLibC_AuRecorder_ *recorder = ttLibC_malloc(sizeof(ttLibC_AuRecorder_));
	if(recorder == NULL) {
		ERR_PRINT("failed to allocate recorder object.");
		return NULL;
	}
	OSStatus err = noErr;
	AudioComponentDescription desc;
	desc.componentType = kAudioUnitType_Output;
	switch(type) {
	case AuRecorderType_DefaultInput:
#if TARGET_IPHONE_SIMULATOR
		// iOS Simulator
		desc.componentSubType = kAudioUnitSubType_RemoteIO;
#elif TARGET_OS_IPHONE
		// iOS device
		desc.componentSubType = kAudioUnitSubType_RemoteIO;
#elif TARGET_OS_MAC
		// Other kinds of Mac OS
		desc.componentSubType = kAudioUnitSubType_HALOutput;
#else
		// Unsupported platform
#endif
		break;
	default:
		desc.componentSubType = type;
		break;
	}
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	desc.componentFlags = 0;
	desc.componentFlagsMask = 0;
	// make component instance.
	AudioComponent component = AudioComponentFindNext(NULL, &desc);
	err = AudioComponentInstanceNew(component, &recorder->inputUnit);
	if(err != noErr) {
		ERR_PRINT("failed to make audioComponentInstance. err:%d", err);
		ttLibC_free(recorder);
		return NULL;
	}
	// enable input and disable output.
	uint32_t enableIO;
	enableIO = 1; // enable
	err = AudioUnitSetProperty(
			recorder->inputUnit,
			kAudioOutputUnitProperty_EnableIO,
			kAudioUnitScope_Input,
			1, // 1 for input 0 for output.
			&enableIO,
			sizeof(enableIO));
	if(err != noErr) {
		ERR_PRINT("failed to set AudioUnit input enable. err:%d", err);
		AudioComponentInstanceDispose(recorder->inputUnit);
		ttLibC_free(recorder);
		return NULL;
	}
	enableIO = 0; // disable
	err = AudioUnitSetProperty(
			recorder->inputUnit,
			kAudioOutputUnitProperty_EnableIO,
			kAudioUnitScope_Output,
			0,
			&enableIO,
			sizeof(enableIO));
	if(err != noErr) {
		ERR_PRINT("failed to set AudioUnit output disable. err:%d", err);
		AudioComponentInstanceDispose(recorder->inputUnit);
		ttLibC_free(recorder);
		return NULL;
	}
	uint32_t size;
#if TARGET_IPHONE_SIMULATOR
	// iOS Simulator
/*	Float64 sample_rate;
	size = sizeof(sample_rate);
	AudioSessionGetProperty(
			kAudioSessionProperty_CurrentHardwareSampleRate,
			&size,
			&sample_rate);*/
	Float32 buffer_duration;
	size = sizeof(buffer_duration);
	err = AudioSessionGetProperty(
			kAudioSessionProperty_CurrentHardwareIOBufferDuration,
			&size,
			&buffer_duration);
	if(err != noErr) {
		ERR_PRINT("failed to get buffer duration. err:%d", err);
		AudioComponentInstanceDispose(recorder->inputUnit);
		ttLibC_free(recorder);
		return NULL;
	}

	uint32_t buffer_size_frames = sample_rate * buffer_duration;
#elif TARGET_OS_IPHONE
	// iOS device
/*	Float64 sample_rate;
	size = sizeof(sample_rate);
	AudioSessionGetProperty(
			kAudioSessionProperty_CurrentHardwareSampleRate,
			&size,
			&sample_rate);*/
	Float32 buffer_duration;
	size = sizeof(buffer_duration);
	err = AudioSessionGetProperty(
			kAudioSessionProperty_CurrentHardwareIOBufferDuration,
			&size,
			&buffer_duration);
	if(err != noErr) {
		ERR_PRINT("failed to get buffer duration. err:%d", err);
		AudioComponentInstanceDispose(recorder->inputUnit);
		ttLibC_free(recorder);
		return NULL;
	}

	uint32_t buffer_size_frames = sample_rate * buffer_duration;
#elif TARGET_OS_MAC
	AudioDeviceID inputDevice;
	size = sizeof(AudioDeviceID);
	if(device_id == 0) {
		// try to get default device.
		AudioObjectPropertyAddress address;
		address.mElement = kAudioObjectPropertyElementMaster;
		address.mScope = kAudioObjectPropertyScopeGlobal;
		address.mSelector = kAudioHardwarePropertyDefaultInputDevice;
		err = AudioObjectGetPropertyData(
				kAudioObjectSystemObject,
				&address,
				0,
				NULL,
				&size,
				&inputDevice);
		if(err != noErr) {
			ERR_PRINT("failed to get default device id. err:%d", err);
			AudioComponentInstanceDispose(recorder->inputUnit);
			ttLibC_free(recorder);
			return NULL;
		}
	}
	else {
		inputDevice = device_id;
	}
	// set the device.
	err = AudioUnitSetProperty(
			recorder->inputUnit,
			kAudioOutputUnitProperty_CurrentDevice,
			kAudioUnitScope_Global,
			0,
			&inputDevice,
			sizeof(inputDevice));
	if(err != noErr) {
		ERR_PRINT("failed to set input device.err:%d", err);
		AudioComponentInstanceDispose(recorder->inputUnit);
		ttLibC_free(recorder);
		return NULL;
	}
	// check frame size.
	uint32_t buffer_size_frames;
	size = sizeof(buffer_size_frames);
	// only for OSX?
	err = AudioUnitGetProperty(
			recorder->inputUnit,
			kAudioDevicePropertyBufferFrameSize,
			kAudioUnitScope_Global,
			0,
			&buffer_size_frames,
			&size);
	if(err != noErr) {
		ERR_PRINT("failed to get buffer size frame informtion. err:%d", err);
		AudioComponentInstanceDispose(recorder->inputUnit);
		ttLibC_free(recorder);
		return NULL;
	}
#else
	// Unsupported platform
#endif
	// set format.
	AudioStreamBasicDescription audioFormat;
	audioFormat.mSampleRate = sample_rate;
	audioFormat.mFormatID = kAudioFormatLinearPCM;

	audioFormat.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
	audioFormat.mBitsPerChannel = 16;
	size = sizeof(int16_t) * buffer_size_frames * channel_num;

	audioFormat.mFramesPerPacket = 1;
	audioFormat.mChannelsPerFrame = channel_num;
	audioFormat.mBytesPerPacket = 2 * channel_num;
	audioFormat.mBytesPerFrame = 2 * channel_num;
	err = AudioUnitSetProperty(
			recorder->inputUnit,
			kAudioUnitProperty_StreamFormat,
			kAudioUnitScope_Output,
			1, // for input
			&audioFormat,
			sizeof(audioFormat));
	if(err != noErr) {
		ERR_PRINT("failed to set format. err:%d", err);
		AudioComponentInstanceDispose(recorder->inputUnit);
		ttLibC_free(recorder);
		return NULL;
	}
	// last. set the callback.
	AURenderCallbackStruct callback;
	callback.inputProc = AuRecorder_inputCallback;
	callback.inputProcRefCon = recorder;
	err = AudioUnitSetProperty(
			recorder->inputUnit,
			kAudioOutputUnitProperty_SetInputCallback,
			kAudioUnitScope_Global,
			0,
			&callback,
			sizeof(callback));
	if(err != noErr) {
		ERR_PRINT("failed to set callback. err:%d", err);
		AudioComponentInstanceDispose(recorder->inputUnit);
		ttLibC_free(recorder);
		return NULL;
	}
	err = AudioUnitInitialize(recorder->inputUnit);
	if(err != noErr) {
		ERR_PRINT("failed to initialize audioUnit.");
		AudioComponentInstanceDispose(recorder->inputUnit);
		ttLibC_free(recorder);
		return NULL;
	}
	recorder->bufferList = (AudioBufferList *)ttLibC_malloc(sizeof(AudioBufferList) + 1 * sizeof(AudioBuffer));
	recorder->bufferList->mNumberBuffers = 1;
	for(int i = 0;i < 1;++ i) {
		recorder->bufferList->mBuffers[i].mNumberChannels = channel_num;
		recorder->bufferList->mBuffers[i].mDataByteSize = size;
		recorder->bufferList->mBuffers[i].mData = ttLibC_malloc(size);
	}
	recorder->audio                      = NULL;
	recorder->is_recording               = false;
	recorder->inherit_super.is_recording = false;
	recorder->callback                   = NULL;
	recorder->ptr                        = NULL;
	recorder->channel_num                = channel_num;
	recorder->inherit_super.channel_num  = channel_num;
	recorder->sample_rate                = sample_rate;
	recorder->inherit_super.sample_rate  = sample_rate;
	recorder->pts                        = 0;
	recorder->inherit_super.pts          = 0;
	return (ttLibC_AuRecorder *)recorder;
}

/*
 * start recorder
 * @param recorder target ttLibC_AuRecorder object.
 * @param callback set the callback.
 * @param ptr      set the data pointer which will passing in callback.
 */
bool ttLibC_AuRecorder_start(
		ttLibC_AuRecorder *recorder,
		ttLibC_AuRecorderFunc callback,
		void *ptr) {
	// need to clear pts?
	ttLibC_AuRecorder_ *recorder_ = (ttLibC_AuRecorder_ *)recorder;
	if(recorder_ == NULL) {
		return false;
	}
	recorder_->callback = callback;
	recorder_->ptr = ptr;
	OSStatus err = AudioOutputUnitStart(recorder_->inputUnit);
	if(err != noErr) {
		ERR_PRINT("failed to start input Unit.:%d", err);
		return false;
	}
	// update flag
	recorder_->is_recording = true;
	recorder_->inherit_super.is_recording = true;
	return true;
}

/*
 * stop recorder
 * @param recorder target ttLibC_AuRecorder object.
 */
bool ttLibC_AuRecorder_stop(ttLibC_AuRecorder *recorder) {
	ttLibC_AuRecorder_ *recorder_ = (ttLibC_AuRecorder_ *)recorder;
	if(recorder_ == NULL) {
		return false;
	}
	OSStatus err = AudioOutputUnitStop(recorder_->inputUnit);
	if(err != noErr) {
		ERR_PRINT("failed to stop input Unit.:%d", err);
		return false;
	}
	// update flag
	recorder_->is_recording = false;
	recorder_->inherit_super.is_recording = false;
	return true;
}

/*
 * close recorder
 * @param recorder
 */
void ttLibC_AuRecorder_close(ttLibC_AuRecorder **recorder) {
	ttLibC_AuRecorder_ *target = (ttLibC_AuRecorder_ *)*recorder;
	if(target == NULL) {
		return;
	}
	if(target->is_recording) {
		AudioOutputUnitStop(target->inputUnit);
	}
	if(target->bufferList) {
		for(int i = 0;i < 1;++ i) {
			ttLibC_free(target->bufferList->mBuffers[i].mData);
		}
		ttLibC_free(target->bufferList);
		target->bufferList = NULL;
	}
	ttLibC_Audio_close(&target->audio);
	AudioUnitUninitialize(target->inputUnit);
	AudioComponentInstanceDispose(target->inputUnit);
	ttLibC_free(target);
	*recorder = NULL;
}

#endif
