#include "encoder.hpp"

ttLibC_Yuv420 *EncoderTest::makeGradateYuv(ttLibC_Yuv420_Type type, uint32_t width, uint32_t height) {
  auto yuv = ttLibC_Yuv420_makeEmptyFrame(type, width, height);
  if(yuv == nullptr) {
    return nullptr;
  }
  uint8_t *yd = yuv->y_data;
  uint8_t *ud = yuv->u_data;
  uint8_t *vd = yuv->v_data;
  for(int i = 0;i < height;++ i) {
    uint8_t *y = yd;
    for(int j = 0;j < width;++ j) {
      *y = (uint8_t)(j * 255 / width);
      ++ y;
    }
    yd += yuv->y_stride;
  }
  for(int i = 0, imax = ((height + 1) >> 1);i < imax;++ i) {
    uint8_t *u = ud;
    uint8_t *v = vd;
    for(int j = 0, jmax = ((width + 1) >> 1);j < jmax;++ j) {
      *u = 128;
      ++ u;
      *v = 128;
      ++ v;
    }
    ud += yuv->u_stride;
    vd += yuv->v_stride;
  }
  return yuv;
}
