#ifdef __ENABLE_APPLE__

#include "audioUnitRecorderUtil.hpp"
#include "audioUnitRecorderUtil.h"
#include <AudioToolbox/AudioToolbox.h>
#include <AudioUnit/AudioUnit.h>
#include "../allocator.h"
#include "../frame/audio/pcms16.h"
#include "../_log.h"

using namespace ttLibC;
using namespace std;

namespace ttLibC {
  class AudioUnitRecorderImpl {
  public:
    static const uint32_t DefaultInput = 0x00;
    static bool getDeviceList(std::function<bool(uint32_t deviceId, std::string name)> callback) {
      OSStatus result;
      AudioDeviceID devID;
      uint32_t size = 0;
      AudioObjectPropertyAddress address;
      AudioValueRange *sampleRates;

      // search all audio hardware
      address.mSelector = kAudioHardwarePropertyDevices;
      address.mScope = kAudioObjectPropertyScopeGlobal;
      address.mElement = kAudioObjectPropertyElementMaster;
      result = AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &address, 0, nullptr, &size);
      if(result != noErr) {
        return false;
      }
      // get the # of device
      uint32_t num = (size / sizeof(AudioDeviceID));
      // get ids
      uint32_t *deviceIds = new uint32_t[num];
      AudioObjectGetPropertyData(kAudioObjectSystemObject, &address, 0, nullptr, &size, deviceIds);
      for(uint32_t i = 0;i < num;++ i) {
        if(result) {
          break;
        }
        // check this device for input or not.
        address.mSelector = kAudioDevicePropertyStreams;
        address.mScope = kAudioDevicePropertyScopeInput;
        result = AudioObjectGetPropertyDataSize(deviceIds[i], &address, 0, nullptr, &size);
        if(result || size == 0) {
          // not for input.
          result = noErr;
          continue;
        }

        // get the name of device.
        CFStringRef deviceName = nullptr;
        address.mSelector = kAudioObjectPropertyName;
        address.mScope = kAudioObjectPropertyScopeGlobal;
        size = sizeof(deviceName);

        AudioObjectGetPropertyData(deviceIds[i], &address, 0, NULL, &size, &deviceName);

        char name[256];
        CFStringGetCString(deviceName, name, 256, kCFStringEncodingUTF8);
        if(!callback(deviceIds[i], name)) {
          result = -1;
        }
      }
      delete[] deviceIds;
      return result == noErr;
    }
    AudioUnitRecorderImpl(
        uint32_t targetId,
        uint32_t sampleRate,
        uint32_t channelNum,
        uint32_t subType,
        std::function<bool(ttLibC_Audio *audio)> callback) {
      OSStatus err = noErr;
      _audio = nullptr;
      _callback = callback;
      _sampleRate = sampleRate;
      _channelNum = channelNum;
      isInitialized = false;

      AudioComponentDescription desc;
      desc.componentType = kAudioUnitType_Output;
      switch(subType) {
      case AudioUnitRecorder::DefaultInput:
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
        desc.componentSubType = subType;
        break;
      }
      desc.componentManufacturer = kAudioUnitManufacturer_Apple;
      desc.componentFlags = 0;
      desc.componentFlagsMask = 0;
      // make component instance.
      AudioComponent component = AudioComponentFindNext(NULL, &desc);
      err = AudioComponentInstanceNew(component, &_unit);
      if(err != noErr) {
        ERR_PRINT("failed to make audioComponentInstance. err:%d", err);
        return;
      }
      // enable input and disable output.
      uint32_t enableIO;
      enableIO = 1; // enable
      err = AudioUnitSetProperty(
          _unit,
          kAudioOutputUnitProperty_EnableIO,
          kAudioUnitScope_Input,
          1, // 1 for input 0 for output.
          &enableIO,
          sizeof(enableIO));
      if(err != noErr) {
        ERR_PRINT("failed to set AudioUnit input enable. err:%d", err);
        AudioComponentInstanceDispose(_unit);
        return;
      }
      enableIO = 0; // disable
      err = AudioUnitSetProperty(
          _unit,
          kAudioOutputUnitProperty_EnableIO,
          kAudioUnitScope_Output,
          0,
          &enableIO,
          sizeof(enableIO));
      if(err != noErr) {
        ERR_PRINT("failed to set AudioUnit output disable. err:%d", err);
        AudioComponentInstanceDispose(_unit);
        return;
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
      if(targetId == 0) {
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
          AudioComponentInstanceDispose(_unit);
          return;
        }
      }
      else {
        inputDevice = targetId;
      }
      // set the device.
      err = AudioUnitSetProperty(
          _unit,
          kAudioOutputUnitProperty_CurrentDevice,
          kAudioUnitScope_Global,
          0,
          &inputDevice,
          sizeof(inputDevice));
      if(err != noErr) {
        ERR_PRINT("failed to set input device.err:%d", err);
        AudioComponentInstanceDispose(_unit);
        return;
      }
      // check frame size.
      uint32_t buffer_size_frames;
      size = sizeof(buffer_size_frames);
      // only for OSX?
      err = AudioUnitGetProperty(
          _unit,
          kAudioDevicePropertyBufferFrameSize,
          kAudioUnitScope_Global,
          0,
          &buffer_size_frames,
          &size);
      if(err != noErr) {
        ERR_PRINT("failed to get buffer size frame informtion. err:%d", err);
        AudioComponentInstanceDispose(_unit);
        return;
      }
#else
      // Unsupported platform
#endif
      // set format.
      AudioStreamBasicDescription audioFormat;
      audioFormat.mSampleRate = sampleRate;
      audioFormat.mFormatID = kAudioFormatLinearPCM;

      audioFormat.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
      audioFormat.mBitsPerChannel = 16;
      size = sizeof(int16_t) * buffer_size_frames * channelNum;

      audioFormat.mFramesPerPacket = 1;
      audioFormat.mChannelsPerFrame = channelNum;
      audioFormat.mBytesPerPacket = 2 * channelNum;
      audioFormat.mBytesPerFrame = 2 * channelNum;
      err = AudioUnitSetProperty(
          _unit,
          kAudioUnitProperty_StreamFormat,
          kAudioUnitScope_Output,
          1, // for input
          &audioFormat,
          sizeof(audioFormat));
      if(err != noErr) {
        ERR_PRINT("failed to set format. err:%d", err);
        AudioComponentInstanceDispose(_unit);
        return;
      }
      // last. set the callback.
      AURenderCallbackStruct _callback;
      _callback.inputProc = AudioUnitRecorderImpl::inputCallback;
      _callback.inputProcRefCon = this;
      err = AudioUnitSetProperty(
          _unit,
          kAudioOutputUnitProperty_SetInputCallback,
          kAudioUnitScope_Global,
          0,
          &_callback,
          sizeof(_callback));
      if(err != noErr) {
        ERR_PRINT("failed to set callback. err:%d", err);
        AudioComponentInstanceDispose(_unit);
        return;
      }
      err = AudioUnitInitialize(_unit);
      if(err != noErr) {
        ERR_PRINT("failed to initialize audioUnit.");
        AudioComponentInstanceDispose(_unit);
        return;
      }
      err = AudioOutputUnitStart(_unit);
      if(err != noErr) {
        ERR_PRINT("failed to start input Unit.:%x %d", err, err);
        return;
      }
      _bufferList.mNumberBuffers = 1;
      _bufferList.mBuffers[0].mNumberChannels = channelNum;
      _bufferList.mBuffers[0].mDataByteSize = size;
      _bufferList.mBuffers[0].mData = ttLibC_malloc(size);
      isInitialized = true;
    }
    bool isInitialized;
    ~AudioUnitRecorderImpl() {
      ttLibC_free(_bufferList.mBuffers[0].mData);
      ttLibC_Audio_close(&_audio);
      AudioUnitUninitialize(_unit);
      AudioComponentInstanceDispose(_unit);
    }
    void setCCallback(void *ptr, ttLibC_AudioUnitRecorderFrameFunc callback) {
      _cPtr = ptr;
      _cCallback = callback;
    }
  private:
    AudioUnit _unit;
    AudioBufferList _bufferList;
    ttLibC_Audio *_audio;
    uint32_t _sampleRate;
    uint32_t _channelNum;
    std::function<bool(ttLibC_Audio *audio)> _callback;
    ttLibC_AudioUnitRecorderFrameFunc _cCallback = nullptr;
    void *_cPtr = nullptr;
    static OSStatus inputCallback(
        void *inRefCon,
        AudioUnitRenderActionFlags *ioActionFlags,
        const AudioTimeStamp *inTimestamp,
        uint32_t inBusNumber,
        uint32_t inNumberFrames,
        AudioBufferList *ioData) {
      AudioUnitRecorderImpl *impl = reinterpret_cast<AudioUnitRecorderImpl *>(inRefCon);
      return impl->callbackBody(ioActionFlags, inTimestamp, inBusNumber, inNumberFrames);
    }
    OSStatus callbackBody(
        AudioUnitRenderActionFlags *ioActionFlags,
        const AudioTimeStamp *inTimestamp,
        uint32_t inBusNumber,
        uint32_t inNumberFrames) {
      OSStatus err = AudioUnitRender(
        _unit,
        ioActionFlags,
        inTimestamp,
        inBusNumber,
        inNumberFrames,
        &_bufferList);
      if(err != noErr) {
        ERR_PRINT("failed to audioUnitRender:%x %d", err, err);
        // 止める必要がある。
        return err;
      }
      auto pcmS16 = ttLibC_PcmS16_make(
          (ttLibC_PcmS16 *)_audio,
          PcmS16Type_littleEndian,
          _sampleRate,
          inNumberFrames,
          _channelNum,
          _bufferList.mBuffers[0].mData,
          _bufferList.mBuffers[0].mDataByteSize,
          _bufferList.mBuffers[0].mData,
          _bufferList.mBuffers[0].mDataByteSize,
          nullptr,
          0,
          true,
          (uint64_t)inTimestamp->mSampleTime,
          _sampleRate);
      if(pcmS16 != nullptr) {
        _audio = (ttLibC_Audio *)pcmS16;
        if(_cCallback != nullptr) {
          return _cCallback(_cPtr, _audio);
        }
        else {
          _callback(_audio);
        }
      }
      return err;
    }
  };
}

bool AudioUnitRecorder::getDeviceList(function<bool(uint32_t deviceId, string name)> callback) {
  return AudioUnitRecorderImpl::getDeviceList(callback);
}

AudioUnitRecorder::AudioUnitRecorder(
    uint32_t deviceId,
    uint32_t sampleRate,
    uint32_t channelNum,
    uint32_t type,
    function<bool(ttLibC_Audio *audio)> callback) {
  AudioUnitRecorderImpl *impl = new AudioUnitRecorderImpl(
      deviceId,
      sampleRate, 
      channelNum,
      type,
      callback);
  _instance = reinterpret_cast<void *>(impl);
  isInitialized = impl->isInitialized;
}

AudioUnitRecorder::AudioUnitRecorder(
    uint32_t deviceId,
    uint32_t sampleRate,
    uint32_t channelNum,
    function<bool(ttLibC_Audio *audio)> callback) {
  AudioUnitRecorderImpl *impl = new AudioUnitRecorderImpl(
      deviceId,
      sampleRate, 
      channelNum,
      AudioUnitRecorderImpl::DefaultInput,
      callback);
  _instance = reinterpret_cast<void *>(impl);
  isInitialized = impl->isInitialized;
}

AudioUnitRecorder::AudioUnitRecorder(
    uint32_t sampleRate,
    uint32_t channelNum,
    function<bool(ttLibC_Audio *audio)> callback) {
  AudioUnitRecorderImpl *impl = new AudioUnitRecorderImpl(
      0,
      sampleRate, 
      channelNum,
      AudioUnitRecorderImpl::DefaultInput,
      callback);
  _instance = reinterpret_cast<void *>(impl);
  isInitialized = impl->isInitialized;
}

AudioUnitRecorder::~AudioUnitRecorder() {
  if(_instance != nullptr) {
    AudioUnitRecorderImpl *impl = reinterpret_cast<AudioUnitRecorderImpl *>(_instance);
    delete impl;
    _instance = nullptr;
  }
}

bool TT_ATTRIBUTE_API ttLibC_AudioUnitRecorder_getDeviceList(ttLibC_AudioUnitRecorderNameFunc callback, void *ptr) {
  return AudioUnitRecorder::getDeviceList([&](uint32_t id, string name){
    return callback(ptr, id, name.c_str());
  });
}

typedef struct ttLibC_Util_AudioUnitRecorder_ {
  ttLibC_AudioUnitRecorder inherit_super;
  AudioUnitRecorderImpl *impl;
} ttLibC_Util_AudioUnitRecorder_;

typedef ttLibC_Util_AudioUnitRecorder_ ttLibC_AudioUnitRecorder_;

ttLibC_AudioUnitRecorder TT_ATTRIBUTE_API *ttLibC_AudioUnitRecorder_make(
    uint32_t sample_rate,
    uint32_t channel_num,
    ttLibC_AudioUnitRecorderFrameFunc callback,
    void *ptr) {
  return ttLibC_AudioUnitRecorder_makeWithTarget(
    sample_rate,
    channel_num,
    0,
    callback,
    ptr);
}

ttLibC_AudioUnitRecorder TT_ATTRIBUTE_API *ttLibC_AudioUnitRecorder_makeWithTarget(
    uint32_t sample_rate,
    uint32_t channel_num,
    uint32_t target_id,
    ttLibC_AudioUnitRecorderFrameFunc callback,
    void *ptr) {
  return ttLibC_AudioUnitRecorder_makeWithTargetType(
    sample_rate,
    channel_num,
    target_id,
    AudioUnitRecorderImpl::DefaultInput,
    callback,
    ptr);
}

ttLibC_AudioUnitRecorder TT_ATTRIBUTE_API *ttLibC_AudioUnitRecorder_makeWithTargetType(
    uint32_t sample_rate,
    uint32_t channel_num,
    uint32_t target_id,
    uint32_t type,
    ttLibC_AudioUnitRecorderFrameFunc callback,
    void *ptr) {
  ttLibC_AudioUnitRecorder_ *recorder = reinterpret_cast<ttLibC_AudioUnitRecorder_ *>(ttLibC_malloc(sizeof(ttLibC_AudioUnitRecorder_)));
  if(recorder == nullptr) {
    return nullptr;
  }
  recorder->impl = new AudioUnitRecorderImpl(
    target_id,
    sample_rate,
    channel_num,
    type,
    [&](ttLibC_Audio *audio){
      return true;
    });
  recorder->impl->setCCallback(ptr, callback);
  return reinterpret_cast<ttLibC_AudioUnitRecorder *>(recorder);
}

void TT_ATTRIBUTE_API ttLibC_AudioUnitRecorder_close(ttLibC_AudioUnitRecorder **capturer) {
  ttLibC_AudioUnitRecorder_ *target = reinterpret_cast<ttLibC_AudioUnitRecorder_ *>(*capturer);
  if(target == nullptr) {
    return;
  }
  delete target->impl;
  ttLibC_free(target);
}

#endif
