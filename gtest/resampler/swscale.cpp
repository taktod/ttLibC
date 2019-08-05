#include "../resampler.hpp"
#include <ttLibC/resampler/swscaleResampler.h>

#ifdef __ENABLE_SWSCALE__
#define SWSCALE(A, B) TEST_F(ResamplerTest, Swscale##A){B();}
#else
#define SWSCALE(A, B) TEST_F(ResamplerTest, DISABLED_Swscale##A){}
#endif

SWSCALE(ScaleYuvPlanar, [this](){
  auto src = makeYuv(
    "00 00 80 80 "
    "00 00 40 20 "
    "FF FF "
    "80 F0 ",
    Yuv420Type_planar,
    4, 2,
    0, 8, 10);
    // target
  auto dest = makeYuv(
      "00 00 00 00 00 00 00 00 "
      "00 00 00 00 00 00 00 00 "
      "00 00 00 00 "
      "00 00 00 00 ",
      Yuv420Type_planar,
      8, 2,
      0, 16, 20);
  ttLibC_SwscaleResampler *resampler = ttLibC_SwscaleResampler_make(
    src->inherit_super.inherit_super.type,
    src->type,
    src->inherit_super.width,
    src->inherit_super.height,
    dest->inherit_super.inherit_super.type,
    dest->type,
    dest->inherit_super.width,
    dest->inherit_super.height,
    SwscaleResampler_FastBiLinear);
  if(!ttLibC_SwscaleResampler_resample(
      resampler,
      (ttLibC_Video *)dest,
      (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "00 00 00 20 60 80 80 80 "
      "00 00 00 10 30 38 28 20 "
      "FF FF FF FF "
      "80 80 F0 F0 ",
      dest);
  ttLibC_SwscaleResampler_close(&resampler);
  ttLibC_Yuv420_close(&src);
  ttLibC_Yuv420_close(&dest);
});

SWSCALE(ScaleYuvPlanarClipTest, [this](){
  auto src = makeYuv(
    "00 00 80 80 "
    "00 00 40 20 "
    "FF FF "
    "80 F0 ",
    Yuv420Type_planar,
    4, 2,
    0, 8, 10);
    // target
  auto dest = makeYuv(
      "FF FF FF FF FF FF FF FF FF FF "
      "FF FF FF FF FF FF FF FF FF FF "
      "FF FF FF FF FF FF FF FF FF FF "
      "FF FF FF FF FF FF FF FF FF FF "
      "FF FF FF FF FF "
      "FF FF FF FF FF "
      "FF FF FF FF FF "
      "FF FF FF FF FF ",
      Yuv420Type_planar,
      10, 4,
      0, 40, 50);
  dest->inherit_super.width = 8;
  dest->inherit_super.height = 2;
  dest->y_data += 11;
  ttLibC_SwscaleResampler *resampler = ttLibC_SwscaleResampler_make(
    src->inherit_super.inherit_super.type,
    src->type,
    src->inherit_super.width,
    src->inherit_super.height,
    dest->inherit_super.inherit_super.type,
    dest->type,
    dest->inherit_super.width,
    dest->inherit_super.height,
    SwscaleResampler_FastBiLinear);
  if(!ttLibC_SwscaleResampler_resample(
      resampler,
      (ttLibC_Video *)dest,
      (ttLibC_Video *)src)) {
    FAIL();
  }
  ttLibC_Yuv420_resetData(dest);
  binaryEq(
      "FF FF FF FF FF FF FF FF FF FF "
      "FF 00 00 00 20 60 80 80 80 FF "
      "FF 00 00 00 10 30 38 28 20 FF "
      "FF FF FF FF FF FF FF FF FF FF "
      "FF FF FF FF FF "
      "FF FF FF FF FF "
      "80 80 F0 F0 FF "
      "FF FF FF FF FF ",
      dest);
  ttLibC_SwscaleResampler_close(&resampler);
  ttLibC_Yuv420_close(&src);
  ttLibC_Yuv420_close(&dest);
});
