#ifdef __ENABLE_WIN32__

#include "msAudioResampler.h"
#include "msAudioResampler.hpp"

#include "../util/msGlobalUtilCommon.h"
#include "../allocator.h"
#include "../_log.h"

#include <windows.h>
#include <mfapi.h>
#include <mferror.h>
#include <Mfidl.h>
#include <shlwapi.h>
#include <Codecapi.h>
#include <comdef.h>
#include <mftransform.h>
#include <wmcodecdsp.h>

using namespace ttLibC;
using namespace std;

#pragma comment(lib, "mfplat")
#pragma comment(lib, "mf")
#pragma comment(lib, "mfuuid")
#pragma comment(lib, "uuid")
#pragma comment(lib, "wmcodecdspuuid")

namespace ttLibC {
  // https://docs.microsoft.com/en-us/windows/win32/medfound/audioresampler
  // https://sourceforge.net/p/playpcmwin/wiki/HowToUseResamplerMFT/
  class MsAudioResamplerImpl {
  public:
    MsAudioResamplerImpl(
        ttLibC_Frame_Type inType,
        uint32_t inSubType,
        uint32_t inSampleRate,
        uint32_t inChannelNum,
        ttLibC_Frame_Type outType,
        uint32_t outSubType,
        uint32_t outSampleRate,
        uint32_t outChannelNum) {
      _outputType = outType;
      _outputSubType = outSubType;
      _outputSampleRate = outSampleRate;
      _outputChannelNum = outChannelNum;
      _outputPcm = nullptr;
      _outputPts = 0;

      HRESULT hr = S_OK;
      hr = CoCreateInstance(
        CLSID_CResamplerMediaObject,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&_transform));
      if(FAILED(hr)) {
        ERR_PRINT("failed to make _transform object. %x %d", hr, hr);
        return;
      }
      // set quality property.
      IWMResamplerProps *props = nullptr;
      hr = _transform->QueryInterface(&props);
      if(SUCCEEDED(hr)) {
        // 30 -> normal quality.
        // 60 -> best quality.
        hr = props->SetHalfFilterLength(30);
      }
      if(FAILED(hr)) {
        ERR_PRINT("failed to get _transform property. %x %d", hr, hr);
        return;
      }
      // set input mediaType.
      {
        IMFMediaType *type;
        hr = MFCreateMediaType(&type);
        if(SUCCEEDED(hr)) {
          type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
          switch(inType) {
          case frameType_pcmF32:
            if (inSubType == PcmF32Type_interleave) {
              type->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_Float);
              type->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 32);
              type->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, 4 * inChannelNum);
              type->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, inSampleRate * inChannelNum * 4);
            }
            break;
          case frameType_pcmS16:
            if (inSubType == PcmS16Type_littleEndian) {
              type->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
              type->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
              type->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, 2 * inChannelNum);
              type->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, inSampleRate * inChannelNum * 2);
            }
            break;
          default:
            break;
          }
          type->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, inChannelNum);
          type->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, inSampleRate);
          type->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, true);
          hr = _transform->SetInputType(0, type, 0);
        }
        if(FAILED(hr)) {
          ERR_PRINT("failed to set input mediaType %x %d", hr, hr);
          return;
        }
      }
      // set output mediaType.
      {
        IMFMediaType *type;
        hr = MFCreateMediaType(&type);
        if(SUCCEEDED(hr)) {
          type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
          switch(outType) {
          case frameType_pcmF32:
            type->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_Float);
            type->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 32);
            type->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, 4 * outChannelNum);
            type->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, outSampleRate * outChannelNum * 4);
            break;
          case frameType_pcmS16:
            type->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
            type->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
            type->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, 2 * outChannelNum);
            type->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, outSampleRate * outChannelNum * 2);
            break;
          default:
            break;
          }
          type->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, outChannelNum);
          type->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, outSampleRate);
          type->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, true);
          hr = _transform->SetOutputType(0, type, 0);
        }
        if(FAILED(hr)) {
          ERR_PRINT("failed to set output mediaType %x %d", hr, hr);
          return;
        }
      }
      hr = _transform->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, NULL);
      if(SUCCEEDED(hr)) {
        hr = _transform->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, NULL);
      }
      if(SUCCEEDED(hr)) {
        hr = _transform->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, NULL);
      }
      if(FAILED(hr)) {
        ERR_PRINT("failed to setup _transform %x %d", hr, hr);
        return;
      }
      isInitialized = SUCCEEDED(hr);
    }
    ~MsAudioResamplerImpl() {
      if(_transform != nullptr) {
        _transform->Release();
        _transform = nullptr;
      }
      ttLibC_Audio_close(&_outputPcm);
    }
    bool resample(
        ttLibC_Audio *src_frame,
        std::function<bool(ttLibC_Audio *audio)> callback) {
      // try to resample!
      HRESULT hr = S_OK;
      {
        // input
        IMFSample *sample = nullptr;
        IMFMediaBuffer *buffer = nullptr;
        BYTE *bufferData;
        // timebase should be 100 nsec.
        uint64_t pts = src_frame->inherit_super.pts * 10000 / src_frame->inherit_super.timebase;
        uint32_t duration = src_frame->sample_num * 10000 / src_frame->sample_rate;
        uint32_t memorySize = 0;
        uint8_t *memoryPtr = nullptr;
        switch(src_frame->inherit_super.type) {
        case frameType_pcmS16:
          {
            auto pcm = reinterpret_cast<ttLibC_PcmS16 *>(src_frame);
            if(pcm->type != PcmS16Type_littleEndian) {
              ERR_PRINT("subType is not supported.");
              return false;
            }
            memorySize = src_frame->sample_num * src_frame->channel_num * 2;
            memoryPtr = pcm->l_data;
          }
          break;
        case frameType_pcmF32:
          {
            auto pcm = reinterpret_cast<ttLibC_PcmF32 *>(src_frame);
            if (pcm->type == PcmF32Type_planar) {
              memorySize = src_frame->sample_num * src_frame->channel_num * 4;
              memoryPtr = pcm->l_data;
            }
          }
          break;
        default:
          break;
        }
        if(memorySize == 0) {
          return false;
        }
        // CreateEmptySample
        hr = MFCreateSample(&sample);
        ReleaseOnExit roeSample(sample);
        if(FAILED(hr)) {
          ERR_PRINT("failed to create sample %x %d", hr, hr);
        }
        // memoryBuffer
        hr = MFCreateMemoryBuffer(memorySize, &buffer);
        if(SUCCEEDED(hr)) {
          hr = buffer->Lock(&bufferData, nullptr, nullptr);
        }
        if(SUCCEEDED(hr)) {
          memcpy(bufferData, memoryPtr, memorySize);
          hr = buffer->Unlock();
        }
        if(SUCCEEDED(hr)) {
          hr = buffer->SetCurrentLength(memorySize);
        }
        if(SUCCEEDED(hr)) {
          hr = sample->AddBuffer(buffer);
        }
        ReleaseOnExit rowBuffer(buffer);
        // now try to write pcm buffer.
        if(FAILED(hr)) {
          ERR_PRINT("failed for memoryBuffer %x %d", hr, hr);
          return false;
        }
        // update information
/*        if(SUCCEEDED(hr)) {
          hr = sample->SetSampleTime(pts);
        }
        // try to set duration
        if(SUCCEEDED(hr)) {
          hr = sample->SetSampleDuration(duration);
        }*/
        if(FAILED(hr)) {
          ERR_PRINT("failed to set time information. %x %d", hr, hr);
          return false;
        }
        hr = _transform->ProcessInput(0, sample, 0);
      }
      while(true) {
        // try to get output pcm frame.
        DWORD /*outputFlags, */outputStatus;
        MFT_OUTPUT_STREAM_INFO outputInfo = {0};

        MFT_OUTPUT_DATA_BUFFER output = {0};
        IMFSample *sample = NULL;
        IMFMediaBuffer *buffer = NULL;
        BYTE *bufferData;
//        DWORD bufferLength;
        bool result = false;
//        int64_t samplePts;
/*        if(SUCCEEDED(hr)) {
          hr = _transform->GetOutputStreamInfo(0, &outputInfo);
        }
        // try to make sample.
        if(FAILED(hr)) {
          ERR_PRINT("failed to get outputInfo from transform %x %d", hr, hr);
          return false;
        }*/
        hr = MFCreateSample(&sample);
        if(FAILED(hr)) {
          ERR_PRINT("failed to make applying sample.%x %d", hr, hr);
          return false;
        }
        ReleaseOnExit roeSample(sample);
        if(SUCCEEDED(hr)) {
          hr = MFCreateMemoryBuffer(4096, &buffer);
        }
        ReleaseOnExit rowBuffer(buffer);
        if(SUCCEEDED(hr)) {
          hr = sample->AddBuffer(buffer);
        }
        if(FAILED(hr)) {
          ERR_PRINT("failed in process of buffer %x %d", hr, hr);
          return false;
        }
        output.pSample = sample;
        if(SUCCEEDED(hr)) {
          hr = _transform->ProcessOutput(0, 1, &output, &outputStatus);
        }
        if(hr == MF_E_TRANSFORM_NEED_MORE_INPUT) {
          // need more input do next.
          return true;
        }
        if(FAILED(hr)) {
          ERR_PRINT("something happen on processOutput %x %d", hr, hr);
          return false;
        }
        DWORD currentLength = 0;
        // pts is corrupted.
        buffer->GetCurrentLength(&currentLength);
        buffer->Lock(&bufferData, nullptr, &currentLength);
        if(_outputType == frameType_pcmF32) {
          auto p = ttLibC_PcmF32_make((ttLibC_PcmF32*)_outputPcm,
            (ttLibC_PcmF32_Type)_outputSubType,
            _outputSampleRate,
            currentLength / _outputChannelNum / 4,
            _outputChannelNum,
            bufferData,
            currentLength,
            bufferData,
            currentLength,
            nullptr,
            0,
            true,
            _outputPts,
            _outputSampleRate);
          if (p != nullptr) {
            _outputPcm = (ttLibC_Audio*)p;
            _outputPts += _outputPcm->sample_num;
            result = callback(_outputPcm);
          }
        }
        else {
          auto p = ttLibC_PcmS16_make((ttLibC_PcmS16*)_outputPcm,
            (ttLibC_PcmS16_Type)_outputSubType,
            _outputSampleRate,
            currentLength / _outputChannelNum / 2,
            _outputChannelNum,
            bufferData,
            currentLength,
            bufferData,
            currentLength,
            nullptr,
            0,
            true,
            _outputPts,
            _outputSampleRate);
          if (p != nullptr) {
            _outputPcm = (ttLibC_Audio*)p;
            _outputPts += _outputPcm->sample_num;
            result = callback(_outputPcm);
          }
        }
        buffer->Unlock();
        if(!result) {
          return false;
        }
      }
    }
    bool isInitialized;
  private:
    GUID getGuidType(ttLibC_Audio *audio) {
      switch(audio->inherit_super.type) {
      case frameType_pcmS16:
        {
          ttLibC_PcmS16 *pcm = reinterpret_cast<ttLibC_PcmS16 *>(audio);
          return getGuidType(frameType_pcmS16, pcm->type);
        }
        break;
      case frameType_pcmF32:
        {
          ttLibC_PcmF32 *pcm = reinterpret_cast<ttLibC_PcmF32 *>(audio);
          return getGuidType(frameType_pcmF32, pcm->type);
        }
        break;
      default:
        break;
      }
      return {0};
    }
    GUID getGuidType(ttLibC_Frame_Type type, uint32_t subType) {
      switch(type) {
      case frameType_pcmS16:
        {
          switch(subType) {
          case PcmS16Type_littleEndian:
            return MFAudioFormat_PCM;
          case PcmS16Type_littleEndian_planar:
            break;
          case PcmS16Type_bigEndian:
            break;
          case PcmS16Type_bigEndian_planar:
            break;
          default:
            break;
          }
        }
        break;
      case frameType_pcmF32:
        {
          switch(subType) {
          case PcmF32Type_interleave:
            return MFAudioFormat_Float;
          case PcmF32Type_planar:
            break;
          default:
            break;
          }
        }
        break;
      default:
        break;
      }
      return {0};
    }
    IMFTransform *_transform = nullptr;
    ttLibC_Frame_Type _outputType;
    uint32_t _outputSubType;
    uint32_t _outputSampleRate;
    uint32_t _outputChannelNum;
    ttLibC_Audio *_outputPcm;
    uint64_t _outputPts;
  };
}

TT_ATTRIBUTE_API MsAudioResampler::MsAudioResampler(
    ttLibC_Frame_Type inType,
    uint32_t inSubType,
    uint32_t inSampleRate,
    uint32_t inChannelNum,
    ttLibC_Frame_Type outType,
    uint32_t outSubType,
    uint32_t outSampleRate,
    uint32_t outChannelNum) {
  MsAudioResamplerImpl *impl = new MsAudioResamplerImpl(
    inType,
    inSubType,
    inSampleRate,
    inChannelNum,
    outType,
    outSubType,
    outSampleRate,
    outChannelNum);
  if(impl != nullptr) {
    _instance = reinterpret_cast<void *>(impl);
    isInitialized = impl->isInitialized;
  }
}

TT_ATTRIBUTE_API MsAudioResampler::~MsAudioResampler() {
  if(_instance != nullptr) {
    MsAudioResamplerImpl *impl = reinterpret_cast<MsAudioResamplerImpl *>(_instance);
    delete impl;
    _instance = nullptr;
  }
}

bool TT_ATTRIBUTE_API MsAudioResampler::resample(
    ttLibC_Audio *srcFrame,
    std::function<bool(ttLibC_Audio *audio)> callback) {
  MsAudioResamplerImpl *impl = reinterpret_cast<MsAudioResamplerImpl *>(_instance);
  return impl->resample(srcFrame, callback);
}

typedef struct ttLibC_Resampler_MsAudioResampler_ {
  ttLibC_MsAudioResampler inherit_super;
  MsAudioResamplerImpl *impl;
} ttLibC_MsAudioResampler_;

ttLibC_MsAudioResampler TT_ATTRIBUTE_API *ttLibC_MsAudioResampler_make(
    ttLibC_Frame_Type in_type,
    uint32_t in_sub_type,
    uint32_t in_sample_rate,
    uint32_t in_channel_num,
    ttLibC_Frame_Type out_type,
    uint32_t out_sub_type,
    uint32_t out_sample_rate,
    uint32_t out_channel_num) {
  ttLibC_MsAudioResampler_ *resampler = reinterpret_cast<ttLibC_MsAudioResampler_ *>(ttLibC_malloc(sizeof(ttLibC_MsAudioResampler_)));
  if(resampler == nullptr) {
    return nullptr;
  }
  resampler->impl = new MsAudioResamplerImpl(
    in_type,
    in_sub_type,
    in_sample_rate,
    in_channel_num,
    out_type,
    out_sub_type,
    out_sample_rate,
    out_channel_num);
  return reinterpret_cast<ttLibC_MsAudioResampler *>(resampler);
}

bool TT_ATTRIBUTE_API ttLibC_MsAudioResampler_resample(
    ttLibC_MsAudioResampler *resampler,
    ttLibC_Audio *src_frame,
    ttLibC_getMsAudioResamplerFrameFunc callback,
    void *ptr) {
  ttLibC_MsAudioResampler_ *resampler_ = reinterpret_cast<ttLibC_MsAudioResampler_ *>(resampler);
  if(resampler == nullptr) {
    return false;
  }
  return resampler_->impl->resample(src_frame, [&](ttLibC_Audio *audio) {
    return callback(ptr, audio);
  });
}

void TT_ATTRIBUTE_API ttLibC_MsAudioResampler_close(ttLibC_MsAudioResampler **resampler) {
  ttLibC_MsAudioResampler_ *target = reinterpret_cast<ttLibC_MsAudioResampler_ *>(*resampler);
  if(target == nullptr) {
    return;
  }
  delete target->impl;
  ttLibC_free(target);
  *resampler = nullptr;
}

#endif