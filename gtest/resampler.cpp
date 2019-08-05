#include "resampler.hpp"

ttLibC_Yuv420 *ResamplerTest::makeYuv(string hex, ttLibC_Yuv420_Type type,
    uint32_t width, uint32_t height,
    uint32_t yOffset, uint32_t uOffset, uint32_t vOffset) {
  uint32_t length = hex.length() / 3;
  uint8_t *buffer = static_cast<uint8_t*>(ttLibC_malloc(length));
  ttLibC_HexUtil_makeBuffer(hex.c_str(), buffer, length);
  uint32_t uv_stride = 0;
  switch(type) {
  case Yuv420Type_planar:
  case Yvu420Type_planar:
  default:
    uv_stride = (width + 1) >> 1;
    break;
  case Yuv420Type_semiPlanar:
  case Yvu420Type_semiPlanar:
    uv_stride = width;
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
  ttLibC_free(buffer);
}
void ResamplerTest::binaryEq(string hex, ttLibC_Yuv420 *yuv) {
  binaryEq(hex, (ttLibC_Video *)yuv);
}
void ResamplerTest::binaryEq(string hex, ttLibC_Bgr *bgr) {
  binaryEq(hex, (ttLibC_Video *)bgr);
}
