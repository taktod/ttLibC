#ifdef __ENABLE_WIN32__

#include "msAudioCapturerUtil.h"
#include "msGlobalUtilCommon.h"
#include "../allocator.h"
#include "../frame/audio/pcms16.h"
#include "../frame/audio/pcmf32.h"
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <Dbt.h>
#include <shlwapi.h>
#include <iostream>
#include <cstring>
#include <functional>

#pragma comment(lib, "mfplat")
#pragma comment(lib, "mf")
#pragma comment(lib, "mfreadwrite")
#pragma comment(lib, "mfuuid")
#pragma comment(lib, "shlwapi")

using namespace std;

class TTMsAudioCapturer : public IMFSourceReaderCallback {
public:
  STDMETHODIMP QueryInterface(REFIID iid, void** ppv) {
    static const QITAB qit[] = {
      QITABENT(TTMsAudioCapturer, IMFSourceReaderCallback),
      {0},
    };
    return QISearch(this, qit, iid, ppv);
  }
  STDMETHODIMP_(ULONG) AddRef() {
    return InterlockedIncrement(&_refCount);
  }
  STDMETHODIMP_(ULONG) Release() {
    ULONG uCount = InterlockedDecrement(&_refCount);
    if (uCount == 0) {
      delete this;
    }
    return uCount;
  }
  STDMETHODIMP OnReadSample(
      HRESULT hrStatus,
      DWORD dwStreamIndex,
      DWORD dwStreamFlags,
      LONGLONG llTimestamp,
      IMFSample *pSample) {
    puts("get sample!");
    if(pSample != nullptr) {
      IMFMediaBuffer *pMediaBuffer = nullptr;
      HRESULT hr = pSample->ConvertToContiguousBuffer(&pMediaBuffer);
      ReleaseOnExit roeBuffer(pMediaBuffer);
      if(SUCCEEDED(hr)) {
        if (bufferToAudio(pMediaBuffer, llTimestamp)) {
          _callback(_audio);
        }
      }
    }
    // here do request.
    HRESULT hr = _pReader->ReadSample(
      (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
      0,
      NULL,
      NULL,
      NULL,
      NULL);
    return hr;
  }
  STDMETHODIMP OnEvent(DWORD, IMFMediaEvent*) {
    return S_OK;
  }
  STDMETHODIMP OnFlush(DWORD) {
    return S_OK;
  }
  static bool getDevices(function<bool(IMFActivate *device)> callback) {
    IMFAttributes *pConfig = nullptr;
    IMFActivate** ppDevices = nullptr;
    CoTaskMemFreeOnExit ctmfoe(ppDevices);
    uint32_t count = 0;

    HRESULT hr = MFCreateAttributes(&pConfig, 1);
    if(SUCCEEDED(hr)) {
      hr = pConfig->SetGUID(
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID);
    }
    if(SUCCEEDED(hr)) {
      hr = MFEnumDeviceSources(pConfig, &ppDevices, &count);
    }
    bool result = true;
    if(SUCCEEDED(hr)) {
      for(uint32_t i = 0;i < count;++ i) {
        if(result) {
          result = callback(ppDevices[i]);
        }
        ReleaseOnExit roe(ppDevices[i]);
      }
    }
    return result;
  }
  TTMsAudioCapturer(const wchar_t *target, uint32_t sample_rate, uint32_t channel_num,
      function<bool(ttLibC_Audio *audio)> callback) {
    _audio = nullptr;
    _pReader = nullptr;
    isInitialized = false;
    _callback = callback;

    bool result = getDevices([&](auto device) {
      WCHAR* pszName = nullptr;
      uint32_t nameLength;
      HRESULT hr = device->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &pszName, &nameLength);
      if (SUCCEEDED(hr)) {
        CoTaskMemFreeOnExit ctmfoe(pszName);
        if(wcscmp(target, pszName) == 0) {
          puts("find target device.");
          IMFMediaSource* pSource = nullptr;
          ReleaseOnExit roeSource(pSource);

          hr = device->ActivateObject(__uuidof(IMFMediaSource), (void **)&pSource);

          if(SUCCEEDED(hr)) {
            // get IMFSourceReader
            IMFAttributes* pAttribute = nullptr;
            ReleaseOnExit roeAttribute(pAttribute);
            hr = MFCreateAttributes(&pAttribute, 2);
            if (SUCCEEDED(hr)) {
              hr = pAttribute->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, this);
            }
            if (SUCCEEDED(hr)) {
              hr = MFCreateSourceReaderFromMediaSource(
                pSource, pAttribute, &_pReader);
            }
          }
          if(SUCCEEDED(hr)) {
            // register target
            IMFMediaType* pType = nullptr;
            ReleaseOnExit roeType(pType);
            hr = _pReader->GetNativeMediaType(
              (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
              0,
              &pType);
            if(SUCCEEDED(hr)) {
              puts("now try to set information for pType.");
//              hr = pType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
//              hr = pType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
              if(SUCCEEDED(hr)) {
                hr = pType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, sample_rate);
              }
              if(SUCCEEDED(hr)) {
                hr = pType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, channel_num);
              }
            }
            // update with current target MediaType.
            if(SUCCEEDED(hr)) {
              puts("now set currentMediaType");
              hr = _pReader->SetCurrentMediaType(
                  (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
                  NULL,
                  pType);
//              printf("%x %d\r\n", hr, hr);
            }
          }
          if(SUCCEEDED(hr)) {
            puts("get generate data type.");
            // get current MediaType for generate image object.
            IMFMediaType* pType = nullptr;
            ReleaseOnExit roeType(pType);
            hr = _pReader->GetCurrentMediaType(
              (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
              &pType);
            if(SUCCEEDED(hr)) {
              puts("succeeded to get mediaType");
              // sampleRate channelNum type subtype
      			  uint32_t sample_rate;
              uint32_t channel_num;
              hr = pType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &sample_rate);
              if(SUCCEEDED(hr)) {
                hr = pType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &channel_num);
              }
              if(SUCCEEDED(hr)) {
                _sample_rate = sample_rate;
                _channel_num = channel_num;
              }
            }
            if (SUCCEEDED(hr)) {
              GUID subtype = { 0 };
              hr = pType->GetGUID(MF_MT_SUBTYPE, &subtype);
              if (subtype == MFAudioFormat_PCM) {
                puts("pcmS16");
                _type = frameType_pcmS16;
                _subType = PcmS16Type_littleEndian;
              }
              if(subtype == MFAudioFormat_Float) {
                puts("pcmF32");
                _type = frameType_pcmF32;
                _subType = PcmF32Type_interleave;
              }
            }
          }
          if(SUCCEEDED(hr)) {
            // here do request.
            hr = _pReader->ReadSample(
              (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
              0,
              NULL,
              NULL,
              NULL,
              NULL);
          }
          if(SUCCEEDED(hr)) {
            isInitialized = true;
          }
        }
      }
      return true;
    });
  }
  ~TTMsAudioCapturer() {
    if (_pReader != nullptr) {
      _pReader->Release();
      _pReader = nullptr;
    }
    ttLibC_Audio_close(&_audio);
  }
  bool isInitialized;
private:
  bool bufferToAudio(IMFMediaBuffer *buffer, LONGLONG llTimestamp) {
    DWORD dataSize = 0;
    bool result = false;
    HRESULT hr = buffer->GetCurrentLength(&dataSize);
    if(FAILED(hr)) {
      return result;
    }
    BYTE* buf = nullptr;
    hr = buffer->Lock(&buf, &dataSize, &dataSize);
    if(SUCCEEDED(hr)) {
      // here to make ttLibCAudioFrame.
      if(_audio == nullptr) {
        _startPts = llTimestamp;
      }
      printf("dataSize:%d\r\n", dataSize);
      // make audio frame and send callback
      switch(_type) {
      case frameType_pcmS16:
        {
        }
        break;
      case frameType_pcmF32:
        {
          auto f = ttLibC_PcmF32_make(
            (ttLibC_PcmF32 *)_audio,
            (ttLibC_PcmF32_Type)_subType,
            _sample_rate,
            dataSize / _channel_num / 4,
            _channel_num,
            buf,
            dataSize,
            buf,
            dataSize,
            nullptr,
            0,
            true,
            (llTimestamp - _startPts) / 10000,
            1000);
          if(f != nullptr) {
            _audio = (ttLibC_Audio *)f;
            _callback(_audio);
          }
        }
        break;
      default:
        break;
      }
    }
    buffer->Unlock();
    return result;
  }
  long _refCount;
  uint64_t _startPts = 0;
  IMFSourceReader* _pReader;
  function<bool(ttLibC_Audio *audio)> _callback;
  ttLibC_Frame_Type _type;
  uint32_t _subType; // only interleave?
  uint32_t _sample_rate;
  uint32_t _channel_num;
  ttLibC_Audio *_audio;
};

typedef struct ttLibC_Util_MsAudioCapturer_ {
  ttLibC_MsAudioCapturer inherit_super;
  TTMsAudioCapturer *capturer;
} ttLibC_Util_MsAudioCapturer_;

typedef ttLibC_Util_MsAudioCapturer_ ttLibC_MsAudioCapturer_;

bool TT_ATTRIBUTE_API ttLibC_MsAudioCapturer_getDeviceNames(ttLibC_MsAudioCapturerNameFunc callback, void *ptr) {
  return TTMsAudioCapturer::getDevices([&](auto device){
    WCHAR *pszName = nullptr;
    CoTaskMemFreeOnExit ctmfoe(pszName);
    uint32_t nameLength;
    HRESULT hr = device->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &pszName, &nameLength);
    if(SUCCEEDED(hr)) {
      return callback(ptr, pszName);
    }
    return true;
  });
}

ttLibC_MsAudioCapturer TT_ATTRIBUTE_API *ttLibC_MsAudioCapturer_make(
    const wchar_t *target,
    uint32_t sample_rate,
    uint32_t channel_num,
    ttLibC_MsAudioCapturerFrameFunc callback,
    void *ptr) {
  ttLibC_MsAudioCapturer_ *capturer = (ttLibC_MsAudioCapturer_ *)ttLibC_malloc(sizeof(ttLibC_MsAudioCapturer_));
  if(capturer == nullptr) {
    return nullptr;
  }
  capturer->capturer = new (std::nothrow)TTMsAudioCapturer(target, sample_rate, channel_num, [&](ttLibC_Audio *audio) {
    return callback(ptr, audio);
  });
  if (capturer->capturer == nullptr) {
    return nullptr;
  }
  if (!capturer->capturer->isInitialized) {
    delete capturer;
    ttLibC_free(capturer);
    return nullptr;
  }
  return (ttLibC_MsAudioCapturer *)capturer;
}

void TT_ATTRIBUTE_API ttLibC_MsAudioCapturer_close(ttLibC_MsAudioCapturer **capturer) {
  ttLibC_MsAudioCapturer_ *target = (ttLibC_MsAudioCapturer_ *)*capturer;
  if (target != nullptr) {
    if(target->capturer != nullptr) {
      delete target->capturer;
    }
    ttLibC_free(target);
    *capturer = nullptr;
  }
}

#endif
