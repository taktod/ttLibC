#include "../resampler.hpp"
#include <ttLibC/resampler/imageResizer.h>

#define IMAGERESIZER(A, B) TEST_F(ResamplerTest, ImageResizer##A){B();}

IMAGERESIZER(ScaleYuvPlanar, [this](){
  // original
  auto src = makeYuv(
      "00 00 80 80 "
      "00 00 40 20 "
      "FF FF "
      "80 F0 ",
      Yuv420Type_planar,
      4, 2);
  // target
  auto dest = makeYuv(
      "00 00 00 00 00 00 00 00 "
      "00 00 00 00 00 00 00 00 "
      "00 00 00 00 "
      "00 00 00 00 ",
      Yuv420Type_planar,
      8, 2);
  if(!ttLibC_ImageResizer_resize((ttLibC_Video *)dest, (ttLibC_Video *)src, false)) {
    FAIL();
  }
  binaryEq(
      "00 00 00 20 60 80 80 80 "
      "00 00 00 10 30 38 28 20 "
      "FF FF FF FF "
      "80 80 80 F0 ",
      dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Yuv420_close(&dest);
});

IMAGERESIZER(ScaleYuvPlanarToYvuPlanar, [this](){
  // original
  auto src = makeYuv(
      "00 00 80 80 "
      "00 00 40 20 "
      "FF FF "
      "80 F0 ",
      Yuv420Type_planar,
      4, 2);
  // target
  auto dest = makeYuv(
      "00 00 00 00 00 00 00 00 "
      "00 00 00 00 00 00 00 00 "
      "00 00 00 00 "
      "00 00 00 00 ",
      Yvu420Type_planar,
      8, 2);
  if(!ttLibC_ImageResizer_resize((ttLibC_Video *)dest, (ttLibC_Video *)src, false)) {
    FAIL();
  }
  binaryEq(
      "00 00 00 20 60 80 80 80 "
      "00 00 00 10 30 38 28 20 "
      "80 80 80 F0 "
      "FF FF FF FF ",
      dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Yuv420_close(&dest);
});

IMAGERESIZER(ScaleYuvSemiPlanar, [this](){
  // original
  auto src = makeYuv(
      "00 00 80 80 "
      "00 00 40 20 "
      "FF FF "
      "80 F0 ",
      Yuv420Type_semiPlanar,
      4, 2);
  // target
  auto dest = makeYuv(
      "00 00 00 00 00 00 00 00 "
      "00 00 00 00 00 00 00 00 "
      "00 00 00 00 "
      "00 00 00 00 ",
      Yuv420Type_semiPlanar,
      8, 2);
  if(!ttLibC_ImageResizer_resize((ttLibC_Video *)dest, (ttLibC_Video *)src, false)) {
    FAIL();
  }
  binaryEq(
      "00 00 00 20 60 80 80 80 "
      "00 00 00 10 30 38 28 20 "
      "FF FF FF FF "
      "FF FF 80 F0 ",
      dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Yuv420_close(&dest);
});

IMAGERESIZER(ScaleYvuSemiPlanar, [this](){
  // original
  auto src = makeYuv(
      "00 00 80 80 "
      "00 00 40 20 "
      "FF FF "
      "80 F0 ",
      Yvu420Type_semiPlanar,
      4, 2);
  // target
  auto dest = makeYuv(
      "00 00 00 00 00 00 00 00 "
      "00 00 00 00 00 00 00 00 "
      "00 00 00 00 "
      "00 00 00 00 ",
      Yvu420Type_semiPlanar,
      8, 2);
  if(!ttLibC_ImageResizer_resize((ttLibC_Video *)dest, (ttLibC_Video *)src, false)) {
    FAIL();
  }
  binaryEq(
      "00 00 00 20 60 80 80 80 "
      "00 00 00 10 30 38 28 20 "
      "FF FF FF FF "
      "FF FF 80 F0 ",
      dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Yuv420_close(&dest);
});

IMAGERESIZER(ScaleBgra, [this](){
  // original
  auto src = makeBgr(
      "10 20 30 40 11 21 31 41 12 22 32 42 13 23 33 43 "
      "14 24 34 44 15 25 35 45 16 26 36 46 17 27 37 47 ",
      BgrType_bgra,
      4, 2);
  // target
  auto dest = makeBgr(
      "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "
      "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "
      "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "
      "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ",
      BgrType_bgra,
      8, 4);
  if(!ttLibC_ImageResizer_resize((ttLibC_Video *)dest, (ttLibC_Video *)src, false)) {
    FAIL();
  }
  binaryEq(
      "10 20 30 40 10 20 30 40 11 21 31 41 11 21 31 41 12 22 32 42 12 22 32 42 13 23 33 43 13 23 33 43 "
      "10 20 30 40 10 20 30 40 11 21 31 41 11 21 31 41 12 22 32 42 12 22 32 42 13 23 33 43 13 23 33 43 "
      "14 24 34 44 14 24 34 44 15 25 35 45 15 25 35 45 16 26 36 46 16 26 36 46 17 27 37 47 17 27 37 47 "
      "14 24 34 44 14 24 34 44 15 25 35 45 15 25 35 45 16 26 36 46 16 26 36 46 17 27 37 47 17 27 37 47 ",
      dest);
  ttLibC_Bgr_close(&src);
  ttLibC_Bgr_close(&dest);
});

#undef IMAGERESIZER
