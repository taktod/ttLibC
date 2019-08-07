#include "resampler.hpp"
#include <ttLibC/resampler/imageResampler.h>

ttLibC_Yuv420 *ResamplerTest::makeYuv(string hex, ttLibC_Yuv420_Type type,
    uint32_t width, uint32_t height) {
  uint32_t length = hex.length() / 3;
  uint8_t *buffer = static_cast<uint8_t*>(ttLibC_malloc(length));
  ttLibC_HexUtil_makeBuffer(hex.c_str(), buffer, length);
  uint32_t uv_stride = 0;
  uint32_t yOffset = 0;
  uint32_t uOffset = 0;
  uint32_t vOffset = 0;
  switch(type) {
  case Yuv420Type_planar:
    uv_stride = (width + 1) >> 1;
    uOffset = width * height;
    vOffset = uOffset + uv_stride * (height >> 1);
    break;
  case Yvu420Type_planar:
  default:
    uv_stride = (width + 1) >> 1;
    vOffset = width * height;
    uOffset = vOffset + uv_stride * (height >> 1);
    break;
  case Yuv420Type_semiPlanar:
    uv_stride = width;
    uOffset = width * height;
    vOffset = uOffset + 1;
    break;
  case Yvu420Type_semiPlanar:
    uv_stride = width;
    vOffset = width * height;
    uOffset = vOffset + 1;
    break;
  }
  ttLibC_Yuv420 *yuv = ttLibC_Yuv420_make(
      nullptr, 
      type,
      width,
      height,
      buffer,
      length,
      buffer + yOffset, width,
      buffer + uOffset, uv_stride,
      buffer + vOffset, uv_stride,
      true,
      0, 1000);
  if(yuv != nullptr) {
    yuv->inherit_super.inherit_super.is_non_copy = false;
  }
  return yuv;
}
ttLibC_Bgr *ResamplerTest::makeBgr(string hex, ttLibC_Bgr_Type type,
    uint32_t width, uint32_t height) {
  uint32_t length = hex.length() / 3;
  uint8_t *buffer = static_cast<uint8_t*>(ttLibC_malloc(length));
  ttLibC_HexUtil_makeBuffer(hex.c_str(), buffer, length);
  uint32_t width_stride = width * 3;
  switch(type) {
  case BgrType_bgr:
  case BgrType_rgb:
  default:
    break;
  case BgrType_abgr:
  case BgrType_bgra:
  case BgrType_argb:
  case BgrType_rgba:
    width_stride = width * 4;
    break;
  }
  ttLibC_Bgr *bgr = ttLibC_Bgr_make(
      nullptr,
      type,
      width,
      height,
      width_stride,
      buffer,
      length,
      true,
      0, 1000);
  if(bgr != nullptr) {
    bgr->inherit_super.inherit_super.is_non_copy = false;
  }
  return bgr;
}
void ResamplerTest::binaryEq(string hex, ttLibC_Video *video) {
  bool result = true;
  switch(video->inherit_super.type) {
  case frameType_bgr:
    {
      ttLibC_Bgr *target = (ttLibC_Bgr *)video;
      ttLibC_Bgr *bgr = makeBgr(hex, target->type, video->width, video->height);
      uint8_t *tp = target->data;
      uint8_t *ep = bgr->data;
      for(int i = 0;i < video->height;i ++) {
        uint8_t *t = tp;
        uint8_t *e = ep;
        for(int j = 0;j < video->width * target->unit_size;j ++) {
          if(abs(*t - *e) > 3) {
            result = false;
            break;
          }
        }
        tp += target->width_stride;
        ep += bgr->width_stride;
        if(!result) {
          break;
        }
      }
      ttLibC_Bgr_close(&bgr);
    }
    break;
  case frameType_yuv420:
    {
      ttLibC_Yuv420 *target = (ttLibC_Yuv420 *)video;
      ttLibC_Yuv420 *yuv = makeYuv(hex, target->type, video->width, video->height);
      uint8_t *typ = target->y_data;
      uint8_t *eyp = yuv->y_data;
      uint8_t *tup = target->u_data;
      uint8_t *eup = yuv->u_data;
      uint8_t *tvp = target->v_data;
      uint8_t *evp = yuv->v_data;
      for(int i = 0;i < video->height;++ i) {
        uint8_t *ty = typ;
        uint8_t *ey = eyp;
        uint8_t *tu = tup;
        uint8_t *eu = eup;
        uint8_t *tv = tvp;
        uint8_t *ev = evp;
        for(int j = 0;j < video->width;++ j) {
          if(abs(*ty - *ey) > 3) {
            result = false;
            break;
          }
          ty += target->y_step;
          ey += yuv->y_step;
          if((i & 1) == 0 && (j & 1) == 0) {
            if(abs(*tu - *eu) > 3) {
              result = false;
              break;
            }
            tu += target->u_step;
            eu += yuv->u_step;
            if(abs(*tv - *ev) > 3) {
              result = false;
              break;
            }
            tv += target->v_step;
            ev += yuv->v_step;
          }
        }
        if(!result) {
          break;
        }
        typ += target->y_stride;
        eyp += yuv->y_stride;
        if((i & 1) == 0) {
          tup += target->u_stride;
          eup += yuv->u_stride;
          tvp += target->v_stride;
          evp += yuv->v_stride;
        }
      }
      ttLibC_Yuv420_close(&yuv);
    }
    break;
  default:
    FAIL();
  }
  if(!result) {
    LOG_DUMP(video->inherit_super.data, video->inherit_super.buffer_size, true);
  }
  EXPECT_TRUE(result);
  /*
  uint32_t length = hex.length() / 3;
  uint8_t *buffer = static_cast<uint8_t*>(ttLibC_malloc(length));
  ttLibC_HexUtil_makeBuffer(hex.c_str(), buffer, length);
  bool result = true;
  uint8_t *buf = static_cast<uint8_t*>(video->inherit_super.data);
  if(length > video->inherit_super.buffer_size) {
    length = video->inherit_super.buffer_size;
  }
  for(int i = 0;i < length;++ i) {
    if(abs(buffer[i] - buf[i]) > 3) {
      result = false;
    }
  }
  if(!result) {
    LOG_DUMP(buf, length, true);
  }
  EXPECT_TRUE(result);
  ttLibC_free(buffer);*/
}
void ResamplerTest::binaryEq(string hex, ttLibC_Yuv420 *yuv) {
  binaryEq(hex, (ttLibC_Video *)yuv);
}
void ResamplerTest::binaryEq(string hex, ttLibC_Bgr *bgr) {
  binaryEq(hex, (ttLibC_Video *)bgr);
}

ttLibC_Yuv420 *ResamplerTest::makeAlignedYuv(string hex, ttLibC_Yuv420_Type type,
    uint32_t width, uint32_t height) {
  ttLibC_Yuv420 *yuv = ttLibC_Yuv420_makeEmptyFrame(type, width, height);
  ttLibC_Yuv420 *base = makeYuv(hex, type, width, height);
  ttLibC_ImageResampler_ToYuv420(yuv, (ttLibC_Video *)base);
  ttLibC_Yuv420_close(&base);
  return yuv;
}
ttLibC_Bgr *ResamplerTest::makeAlignedBgr(string hex, ttLibC_Bgr_Type type,
    uint32_t width, uint32_t height) {
  ttLibC_Bgr *bgr = ttLibC_Bgr_makeEmptyFrame(type, width, height);
  ttLibC_Bgr *base = makeBgr(hex, type, width, height);
  ttLibC_ImageResampler_ToBgr(bgr, (ttLibC_Video *)base);
  ttLibC_Bgr_close(&base);
  return bgr;
}
