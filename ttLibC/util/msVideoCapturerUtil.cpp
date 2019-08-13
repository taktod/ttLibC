#ifdef __ENABLE_WIN32__

#include "msVideoCapturerUtil.h"
#include "msGlobalUtilCommon.h"
#include "../allocator.h"
#include "../frame/video/bgr.h"
#include "../frame/video/yuv420.h"
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

// make class and use it.
class TTMsVideoCapturer : public IMFSourceReaderCallback {
public:
  STDMETHODIMP QueryInterface(REFIID iid, void** ppv) {
    static const QITAB qit[] = {
      QITABENT(TTMsVideoCapturer, IMFSourceReaderCallback),
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
    if(pSample != nullptr) {
      IMFMediaBuffer *pMediaBuffer = nullptr;
      ReleaseOnExit roeBuffer(pMediaBuffer);
      HRESULT hr = pSample->ConvertToContiguousBuffer(&pMediaBuffer);
      if(SUCCEEDED(hr)) {
        if (bufferToVideo(pMediaBuffer)) {
          _callback(_image);
        }
      }
    }
    _captureCount --;
    return S_OK;
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
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
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
    return true;
  }
  TTMsVideoCapturer(string target, uint32_t width, uint32_t height) {
    _image = nullptr;
    _pReader = nullptr;
    isInitialized = false;
    bool result = getDevices([&](auto device) {
      WCHAR* pszName = nullptr;
      CoTaskMemFreeOnExit ctmfoe(pszName);
      uint32_t nameLength;
      HRESULT hr = device->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &pszName, &nameLength);
      if (SUCCEEDED(hr)) {
        if (strcmp(target.c_str(), ttLibC_MsGlobal_wcharToUtf8string(pszName).c_str()) == 0) {
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
              (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
              0,
              &pType);
            if(SUCCEEDED(hr)) {
              hr = MFSetAttributeSize(pType, MF_MT_FRAME_SIZE, width, height);
            }
            // update with current target MediaType.
            if(SUCCEEDED(hr)) {
              hr = _pReader->SetCurrentMediaType(
                  (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                  NULL,
                  pType);
            }
          }
          if(SUCCEEDED(hr)) {
            // get current MediaType for generate image object.
            IMFMediaType* pType = nullptr;
            ReleaseOnExit roeType(pType);
            hr = _pReader->GetCurrentMediaType(
              (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
              &pType);
            if(SUCCEEDED(hr)) {
              // width height type subtype?
              uint32_t width, height;
              hr = MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &width, &height);
              // need to copy data or evaporate data.
              _width = width;
              _height = height;
            }
            if (SUCCEEDED(hr)) {
              GUID subtype = { 0 };
              hr = pType->GetGUID(MF_MT_SUBTYPE, &subtype);
              if (subtype == MFVideoFormat_NV12) {
                _type = frameType_yuv420;
                _subType = Yuv420Type_semiPlanar;
              }
            }
          }
          if(SUCCEEDED(hr)) {
            isInitialized = true;
          }
        }
      }
      return true;
    });
  }
  ~TTMsVideoCapturer() {
    if (_pReader != nullptr) {
      while(_captureCount > 0) {
        Sleep(1);
      }
      _pReader->Release();
      _pReader = nullptr;
    }
    ttLibC_Video_close(&_image);
  }
  bool requestFrame(function<bool(ttLibC_Video *video)> callback) {
    _callback = callback;
    if(_pReader != nullptr) {
      HRESULT hr = _pReader->ReadSample(
        (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
        0,
        NULL,
        NULL,
        NULL,
        NULL);
      if(SUCCEEDED(hr)) {
        _captureCount ++;
        return true;
      }
    }
    return false;
  }
  bool isInitialized;
private:
  // update image with data.
  bool bufferToVideo(IMFMediaBuffer *buffer) {
    DWORD dataSize = 0;
    bool result = false;
    HRESULT hr = buffer->GetCurrentLength(&dataSize);
    if (FAILED(hr)) {
      return result;
    }
    // get the data
    BYTE* buf = nullptr;
    hr = buffer->Lock(&buf, &dataSize, &dataSize);
    if (SUCCEEDED(hr)) {
      switch (_type) {
      case frameType_bgr:
      case frameType_yuv420:
        {
          switch (_subType) {
          case Yuv420Type_planar:
            break;
          case Yuv420Type_semiPlanar:
            {
              uint8_t* yData = buf;
              uint32_t yStride = _width;
              uint8_t* uData = yData + (_width * _height);
              uint32_t uStride = (((_width + 1) >> 1) << 1);
              uint8_t* vData = uData + 1;
              uint32_t vStride = (((_width + 1) >> 1) << 1);
              auto y = ttLibC_Yuv420_make((ttLibC_Yuv420*)_image, (ttLibC_Yuv420_Type)_subType, _width, _height, (void*)buf, (size_t)dataSize,
                yData, yStride,
                uData, uStride,
                vData, vStride,
                false,
                0, 1000);
              if (y != nullptr) {
                _image = (ttLibC_Video*)y;
                result = true;
              }
            }
            break;
          case Yvu420Type_planar:
            break;
          case Yvu420Type_semiPlanar:
            break;
          default:
            break;
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
  long _captureCount = 0;
  IMFSourceReader* _pReader;
  function<bool(ttLibC_Video *video)> _callback;
  uint32_t _width;
  uint32_t _height;
  ttLibC_Frame_Type _type;
  uint32_t _subType;
  ttLibC_Video* _image;
};

typedef struct ttLibC_Util_MsVideoCapturer_ {
  ttLibC_MsVideoCapturer inherit_super;
  TTMsVideoCapturer *capturer;
} ttLibC_Util_MsVideoCapturer_;

typedef ttLibC_Util_MsVideoCapturer_ ttLibC_MsVideoCapturer_;

bool TT_ATTRIBUTE_API ttLibC_MsVideoCapturer_getDeviceNames(ttLibC_MsVideoCapturerNameFunc callback, void *ptr) {
  return TTMsVideoCapturer::getDevices([&](auto device){
    WCHAR *pszName = nullptr;
    CoTaskMemFreeOnExit ctmfoe(pszName);
    uint32_t nameLength;
    HRESULT hr = device->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &pszName, &nameLength);
    if(SUCCEEDED(hr)) {
        return callback(ptr, ttLibC_MsGlobal_wcharToUtf8string(pszName).c_str());
    }
    return true;
  });
}

ttLibC_MsVideoCapturer TT_ATTRIBUTE_API *ttLibC_MsVideoCapturer_make(
  const char *target, uint32_t width, uint32_t height) {
  ttLibC_MsVideoCapturer_ *capturer = (ttLibC_MsVideoCapturer_ *)ttLibC_malloc(sizeof(ttLibC_MsVideoCapturer_));
  if(capturer == nullptr) {
    return nullptr;
  }
  capturer->capturer = new (std::nothrow)TTMsVideoCapturer(target, width, height);
  if (capturer == nullptr) {
    return nullptr;
  }
  if (!capturer->capturer->isInitialized) {
    delete capturer;
    ttLibC_free(capturer);
    return nullptr;
  }
  return (ttLibC_MsVideoCapturer *)capturer;
}

bool TT_ATTRIBUTE_API ttLibC_MsVideoCapturer_requestFrame(
  ttLibC_MsVideoCapturer *capturer,
  ttLibC_MsVideoCapturerFrameFunc callback,
  void *ptr) {
  if(capturer == nullptr) {
      return false;
  }
  ttLibC_MsVideoCapturer_ *capturer_ = (ttLibC_MsVideoCapturer_ *)capturer;
  return capturer_->capturer->requestFrame([&](ttLibC_Video *video) {
    callback(ptr, video);
    return true;
  });
}

void TT_ATTRIBUTE_API ttLibC_MsVideoCapturer_close(ttLibC_MsVideoCapturer **capturer) {
  ttLibC_MsVideoCapturer_ *target = (ttLibC_MsVideoCapturer_ *)*capturer;
  if (target != nullptr) {
    if(target->capturer != nullptr) {
      delete target->capturer;
    }
    ttLibC_free(target);
    *capturer = nullptr;
  }
}

#endif
