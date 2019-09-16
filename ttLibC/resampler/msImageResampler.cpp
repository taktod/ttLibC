#ifdef __ENABLE_WIN32__

#include "msImageResampler.h"
#include "msImageResampler.hpp"
#include "../util/msGlobalUtilCommon.h"
#include "../allocator.h"

#include "../util/hexUtil.h"

#include <windows.h>
#include <mfapi.h>
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
  class MsImageResamplerImpl {
  public:
    MsImageResamplerImpl() {
      HRESULT hr = S_OK;
      hr = CoCreateInstance(
        CLSID_VideoProcessorMFT,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&_transform));
      isInitialized = SUCCEEDED(hr);
    }
    ~MsImageResamplerImpl() {
      if(_transform != nullptr) {
        _transform->Release();
        _transform = nullptr;
      }
    }
    bool resample(
        ttLibC_Video *dest_frame,
        ttLibC_Video *src_frame) {
      if(_transform == nullptr) {
        return false;
      }
      HRESULT hr = S_OK;
      IMFSample      *inSample  = nullptr;
      IMFMediaBuffer *inBuffer  = nullptr;
      uint32_t        inImageSize = 0;
      IMFSample      *outSample = nullptr;
      IMFMediaBuffer *outBuffer = nullptr;
      uint32_t        outImageSize = 0;
      if(SUCCEEDED(hr)) {
        // output
        IMFMediaType *type;
        hr = MFCreateMediaType(&type);
        ReleaseOnExit roeType(type);
        GUID subType = getGuidType(dest_frame);
        if(SUCCEEDED(hr)) {
          type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
          type->SetGUID(MF_MT_SUBTYPE, subType);
          MFSetAttributeSize(type, MF_MT_FRAME_SIZE, dest_frame->width, dest_frame->height);
          hr = _transform->SetOutputType(0, type, 0);
        }
        if(SUCCEEDED(hr)) {
          hr = MFCreateSample(&outSample);
          MFCalculateImageSize(subType, dest_frame->width, dest_frame->height, &outImageSize);
          if(SUCCEEDED(hr)) {
            hr = MFCreateMemoryBuffer(outImageSize, &outBuffer);
          }
          if(SUCCEEDED(hr)) {
            hr = outSample->AddBuffer(outBuffer);
          }
        }
      }
      ReleaseOnExit roeOutBuffer(outBuffer);
      ReleaseOnExit roeOutSample(outSample);
      if(SUCCEEDED(hr)) {
        // input
        IMFMediaType *type;
        hr = MFCreateMediaType(&type);
        ReleaseOnExit roeType(type);
        GUID subType = getGuidType(src_frame);
        if(SUCCEEDED(hr)) {
          type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
          type->SetGUID(MF_MT_SUBTYPE, subType);
          MFSetAttributeSize(type, MF_MT_FRAME_SIZE, src_frame->width, src_frame->height);
          hr = _transform->SetInputType(0, type, 0);
        }
        if(src_frame->inherit_super.type != dest_frame->inherit_super.type) {
          if(SUCCEEDED(hr)) {
            IMFVideoProcessorControl* vpc = nullptr;
            hr = _transform->QueryInterface(&vpc);
            vpc->SetMirror(MIRROR_VERTICAL);
            vpc->Release();
          }
        }
        if(SUCCEEDED(hr)) {
          hr = MFCreateSample(&inSample);
          MFCalculateImageSize(subType, src_frame->width, src_frame->height, &inImageSize);
          if(SUCCEEDED(hr)) {
            hr = MFCreateMemoryBuffer(inImageSize, &inBuffer);
          }
          if(SUCCEEDED(hr)) {
            hr = inSample->AddBuffer(inBuffer);
          }
        }
      }
      ReleaseOnExit roeInBuffer(inBuffer);
      ReleaseOnExit roeInSample(inSample);

      // copy src data
      if(SUCCEEDED(hr)) {
        BYTE* bufferData;
        inBuffer->Lock(&bufferData, nullptr, nullptr);
        switch(src_frame->inherit_super.type) {
        case frameType_bgr:
          {
            ttLibC_Bgr *bgr = reinterpret_cast<ttLibC_Bgr *>(src_frame);
            if(inImageSize == src_frame->inherit_super.buffer_size) {
              memcpy(bufferData, src_frame->inherit_super.data, src_frame->inherit_super.buffer_size);
            }
            else {
              ttLibC_Bgr_getMinimumBinaryBuffer(
                bgr, [](void *ptr, void *data, size_t data_size) {
                  BYTE *bufferData = reinterpret_cast<BYTE *>(ptr);
                  memcpy(bufferData, data, data_size);
                  return true;
                },
                bufferData);
            }
          }
          break;
        case frameType_yuv420:
          {
            ttLibC_Yuv420 *yuv = reinterpret_cast<ttLibC_Yuv420 *>(src_frame);
            if(inImageSize == src_frame->inherit_super.buffer_size) {
              memcpy(bufferData, src_frame->inherit_super.data, src_frame->inherit_super.buffer_size);
            }
            else {
              ttLibC_Yuv420_getMinimumBinaryBuffer(
                yuv, [](void *ptr, void *data, size_t data_size){
                  BYTE *bufferData = reinterpret_cast<BYTE *>(ptr);
                  memcpy(bufferData, data, data_size);
                  return true;
                },
                bufferData);
            }
          }
          break;
        default:
          break;
        }
        inBuffer->Unlock();
        inBuffer->SetCurrentLength((DWORD)src_frame->inherit_super.buffer_size);
      }

      uint64_t pts = src_frame->inherit_super.pts * 10000 / src_frame->inherit_super.timebase;
      inSample->SetSampleTime(pts);
      inSample->SetSampleDuration(100);
      hr = _transform->ProcessInput(0, inSample, 0);
      
      MFT_OUTPUT_DATA_BUFFER outDataBuffer;
      DWORD status = 0;
      outDataBuffer.dwStatus = 0;
      outDataBuffer.dwStreamID = 0;
      outDataBuffer.pEvents = 0;
      outDataBuffer.pSample = outSample;
      hr = _transform->ProcessOutput(0, 1, &outDataBuffer, &status);
      if(SUCCEEDED(hr)) {
        BYTE* pData;
        DWORD size = outImageSize;
        outBuffer->Lock(&pData, NULL, &size);
        // try to copy this data into ttLibC_Video...
        switch(dest_frame->inherit_super.type) {
        case frameType_bgr:
          {
            ttLibC_Bgr *bgr = reinterpret_cast<ttLibC_Bgr *>(dest_frame);
            if (outImageSize == bgr->inherit_super.inherit_super.buffer_size) {
              memcpy(bgr->inherit_super.inherit_super.data, pData, outImageSize);
            }
            else {
              uint8_t *src = pData;
              uint8_t *dst = bgr->data;
              for(uint32_t i = 0;i < bgr->inherit_super.height;++ i) {
                memcpy(dst, src, bgr->inherit_super.width * bgr->unit_size);
                dst += bgr->width_stride;
                src += bgr->inherit_super.width * bgr->unit_size;
              }
            }
          }
          break;
        case frameType_yuv420:
          {
            ttLibC_Yuv420 *yuv = reinterpret_cast<ttLibC_Yuv420 *>(dest_frame);
            if(outImageSize == yuv->inherit_super.inherit_super.buffer_size) {
              memcpy(yuv->inherit_super.inherit_super.data, pData, outImageSize);
            }
            else {
              uint8_t *y_src = pData;
              uint8_t *u_src = nullptr;
              uint8_t *v_src = nullptr;
              uint32_t y_stride = yuv->inherit_super.width;
              uint32_t uv_stride = ((yuv->inherit_super.width + 1) >> 1);
              switch(yuv->type) {
              case Yuv420Type_planar:
              default:
                u_src = y_src + yuv->inherit_super.width * yuv->inherit_super.height;
                v_src = u_src + ((yuv->inherit_super.width + 1) >> 1) * ((yuv->inherit_super.height + 1) >> 1);
                break;
              case Yuv420Type_semiPlanar:
                u_src = y_src + yuv->inherit_super.width * yuv->inherit_super.height;
                v_src = u_src + 1;
                break;
              case Yvu420Type_planar:
                v_src = y_src + yuv->inherit_super.width * yuv->inherit_super.height;
                u_src = v_src + ((yuv->inherit_super.width + 1) >> 1) * ((yuv->inherit_super.height + 1) >> 1);
                break;
              case Yvu420Type_semiPlanar:
                v_src = y_src + yuv->inherit_super.width * yuv->inherit_super.height;
                u_src = v_src + 1;
                break;
              }
              uint8_t *y_dst = yuv->y_data;
              uint8_t *u_dst = yuv->u_data;
              uint8_t *v_dst = yuv->v_data;
              for(uint32_t i = 0;i < yuv->inherit_super.height;++ i) {
                uint8_t *ys = y_src;
                uint8_t *us = u_src;
                uint8_t *vs = v_src;
                uint8_t *yd = y_dst;
                uint8_t *ud = u_dst;
                uint8_t *vd = v_dst;
                for(uint32_t j = 0;j < yuv->inherit_super.width;++ j) {
                  *yd = *ys;
                  ++yd;
                  ++ys;
                  if((i & 1) == 0 && (j & 1) == 0) {
                    *ud = *us;
                    ud += yuv->u_step;
                    us += yuv->u_step;
                    *vd = *vs;
                    vd += yuv->v_step;
                    vs += yuv->v_step;
                  }
                }
                y_dst += yuv->y_stride;
                y_src += yuv->inherit_super.width;
                if((i & 1) == 0) {
                  u_dst += yuv->u_stride;
                  v_dst += yuv->v_stride;
                  u_src += ((yuv->inherit_super.width + 1) >> 1);
                  v_src += ((yuv->inherit_super.width + 1) >> 1);
                }
              }
            }
          }
          break;
        default:
          break;
        }
        outBuffer->Unlock();
      }
      return SUCCEEDED(hr);
    }
    bool isInitialized;
  private:
    GUID getGuidType(ttLibC_Video *video) {
      switch(video->inherit_super.type) {
      case frameType_bgr:
        {
          auto bgr = reinterpret_cast<ttLibC_Bgr *>(video);
          switch(bgr->type) {
          case BgrType_bgr:
            return MFVideoFormat_RGB24;
          case BgrType_bgra:
            return MFVideoFormat_RGB32;
          case BgrType_abgr:
          case BgrType_rgb:
          case BgrType_rgba:
          case BgrType_argb:
          default:
            break;
          }
        }
        break;
      case frameType_yuv420:
        {
          auto yuv = reinterpret_cast<ttLibC_Yuv420 *>(video);
          switch(yuv->type) {
          case Yuv420Type_planar:
            return MFVideoFormat_I420;
          case Yuv420Type_semiPlanar:
            return MFVideoFormat_NV12;
          case Yvu420Type_planar:
            return MFVideoFormat_YV12;
          case Yvu420Type_semiPlanar:
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
  };
}

TT_ATTRIBUTE_API MsImageResampler::MsImageResampler() {
  MsImageResamplerImpl *impl = new MsImageResamplerImpl();
  if(impl != nullptr) {
    _instance = reinterpret_cast<void *>(impl);
    isInitialized = impl->isInitialized;
  }
}

TT_ATTRIBUTE_API MsImageResampler::~MsImageResampler() {
  if(_instance != nullptr) {
    MsImageResamplerImpl *impl = reinterpret_cast<MsImageResamplerImpl *>(_instance);
    delete impl;
    _instance = nullptr;
  }
}

bool TT_ATTRIBUTE_API MsImageResampler::toBgr(
    ttLibC_Bgr   *dest_frame,
    ttLibC_Video *src_frame) {
  if(_instance != nullptr) {
    MsImageResamplerImpl *impl = reinterpret_cast<MsImageResamplerImpl *>(_instance);
    return impl->resample((ttLibC_Video *)dest_frame, src_frame);
  }
  return true;
}

bool TT_ATTRIBUTE_API MsImageResampler::toYuv420(
    ttLibC_Yuv420 *dest_frame,
    ttLibC_Video  *src_frame) {
  if(_instance != nullptr) {
    MsImageResamplerImpl *impl = reinterpret_cast<MsImageResamplerImpl *>(_instance);
    return impl->resample((ttLibC_Video *)dest_frame, src_frame);
  }
  return true;
}

typedef struct ttLibC_Resampler_MsImageResampler_ {
  ttLibC_MsImageResampler inherit_super;
  MsImageResamplerImpl *impl;
} ttLibC_Resampler_MsImageResampler_;

typedef ttLibC_Resampler_MsImageResampler_ ttLibC_MsImageResampler_;

ttLibC_MsImageResampler TT_ATTRIBUTE_API *ttLibC_MsImageResampler_make() {
  ttLibC_MsImageResampler_ *resampler = reinterpret_cast<ttLibC_MsImageResampler_ *>(ttLibC_malloc(sizeof(ttLibC_MsImageResampler_)));
  if(resampler == nullptr) {
    return nullptr;
  }
  resampler->impl = new MsImageResamplerImpl();
  return reinterpret_cast<ttLibC_MsImageResampler *>(resampler);
}

bool TT_ATTRIBUTE_API ttLibC_MsImageResampler_ToBgr(
    ttLibC_MsImageResampler *resampler,
    ttLibC_Bgr   *dest_frame,
    ttLibC_Video *src_frame) {
  ttLibC_MsImageResampler_ *resampler_ = reinterpret_cast<ttLibC_MsImageResampler_ *>(resampler);
  if(resampler_ == nullptr) {
    return false;
  }
  return resampler_->impl->resample((ttLibC_Video *)dest_frame, src_frame);
}

bool TT_ATTRIBUTE_API ttLibC_MsImageResampler_ToYuv420(
    ttLibC_MsImageResampler *resampler,
    ttLibC_Yuv420 *dest_frame,
    ttLibC_Video  *src_frame) {
  ttLibC_MsImageResampler_ *resampler_ = reinterpret_cast<ttLibC_MsImageResampler_ *>(resampler);
  if(resampler_ == nullptr) {
    return false;
  }
  return resampler_->impl->resample((ttLibC_Video *)dest_frame, src_frame);
}

void TT_ATTRIBUTE_API ttLibC_MsImageResampler_close(ttLibC_MsImageResampler **resampler) {
  ttLibC_MsImageResampler_ *target = reinterpret_cast<ttLibC_MsImageResampler_ *>(*resampler);
  if(target == nullptr) {
    return;
  }
  delete target->impl;
  ttLibC_free(target);
  *resampler = nullptr;
}

#endif
