#include "../resampler.hpp"
#include <ttLibC/resampler/libyuvResampler.h>

using namespace std;

#ifdef __ENABLE_LIBYUV__
#define LIBYUV(A, B) TEST_F(ResamplerTest, Libyuv##A){B();}
#else
#define LIBYUV(A, B) TEST_F(ResamplerTest, DISABLED_Libyuv##A){}
#endif

LIBYUV(RotateYuvPlanarToYuvPlanar, [this](){
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
      "00 00 "
      "00 00 "
      "00 00 "
      "00 00 "
      "00 "
      "00 "
      "00 "
      "00 ",
      Yuv420Type_planar,
      2, 4);
  if(!ttLibC_LibyuvResampler_rotate((ttLibC_Video *)dest, (ttLibC_Video *)src, LibyuvRotate_90)) {
    FAIL();
  }
  binaryEq(
      "00 00 "
      "00 00 "
      "40 80 "
      "20 80 "
      "FF "
      "FF "
      "80 "
      "F0 ", dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Yuv420_close(&dest);
});

LIBYUV(RotateClip, [this]() {
  // original
  auto src = makeYuv(
      "00 00 80 80 "
      "00 00 40 20 "
      "F0 E0 "
      "80 F0 ",
      Yuv420Type_planar,
      4, 2);
  // target
  auto dest = makeYuv(
      "FF FF FF FF FF FF "
      "FF FF FF FF FF FF "
      "FF FF FF FF FF FF "
      "FF FF FF FF FF FF "
      "FF FF FF FF FF FF "
      "FF FF FF FF FF FF "
      "FF FF FF FF FF FF "
      "FF FF FF FF FF FF "
      "FF FF FF "
      "FF FF FF "
      "FF FF FF "
      "FF FF FF "
      "FF FF FF "
      "FF FF FF "
      "FF FF FF "
      "FF FF FF ",
      Yuv420Type_planar,
      6, 8);
  dest->inherit_super.width = 2;
  dest->inherit_super.height = 4;
  dest->y_data += 14;
  dest->u_data += 4;
  dest->v_data += 4;
  if(!ttLibC_LibyuvResampler_rotate((ttLibC_Video *)dest, (ttLibC_Video *)src, LibyuvRotate_90)) {
    FAIL();
  }
  ttLibC_Yuv420_resetData(dest);
  binaryEq(
      "FF FF FF FF FF FF "
      "FF FF FF FF FF FF "
      "FF FF 00 00 FF FF "
      "FF FF 00 00 FF FF "
      "FF FF 40 80 FF FF "
      "FF FF 20 80 FF FF "
      "FF FF FF FF FF FF "
      "FF FF FF FF FF FF "
      "FF FF FF "
      "FF F0 FF "
      "FF E0 FF "
      "FF FF FF "
      "FF FF FF "
      "FF 80 FF "
      "FF F0 FF "
      "FF FF FF ",
      dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Yuv420_close(&dest);
});

LIBYUV(RotateYuvPlanarToYvuPlanar, [this](){
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
      "01 02 "
      "03 04 "
      "05 06 "
      "07 08 "
      "09 "
      "0A "
      "0B "
      "0C ",
      Yvu420Type_planar,
      2, 4);
  if(!ttLibC_LibyuvResampler_rotate((ttLibC_Video *)dest, (ttLibC_Video *)src, LibyuvRotate_90)) {
    FAIL();
  }
  binaryEq(
      "00 00 "
      "00 00 "
      "40 80 "
      "20 80 "
      "80 "
      "F0 "
      "FF "
      "FF ", dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Yuv420_close(&dest);
});

LIBYUV(RotateYvuPlanarToYvuPlanar, [this](){
  // original
  auto src = makeYuv(
      "00 00 80 80 "
      "00 00 40 20 "
      "FF FF "
      "80 F0 ",
      Yvu420Type_planar,
      4, 2);
  // target
  auto dest = makeYuv(
      "01 02 "
      "03 04 "
      "05 06 "
      "07 08 "
      "09 "
      "0A "
      "0B "
      "0C ",
      Yvu420Type_planar,
      2, 4);
  if(!ttLibC_LibyuvResampler_rotate((ttLibC_Video *)dest, (ttLibC_Video *)src, LibyuvRotate_90)) {
    FAIL();
  }
  binaryEq(
      "00 00 "
      "00 00 "
      "40 80 "
      "20 80 "
      "FF "
      "FF "
      "80 "
      "F0 ", dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Yuv420_close(&dest);
});

LIBYUV(RotateYvuPlanarToYuvPlanar, [this](){
  // original
  auto src = makeYuv(
      "00 00 80 80 "
      "00 00 40 20 "
      "FF FF "
      "80 F0 ",
      Yvu420Type_planar,
      4, 2);
  // target
  auto dest = makeYuv(
      "01 02 "
      "03 04 "
      "05 06 "
      "07 08 "
      "09 "
      "0A "
      "0B "
      "0C ",
      Yuv420Type_planar,
      2, 4);
  if(!ttLibC_LibyuvResampler_rotate((ttLibC_Video *)dest, (ttLibC_Video *)src, LibyuvRotate_90)) {
    FAIL();
  }
  binaryEq(
      "00 00 "
      "00 00 "
      "40 80 "
      "20 80 "
      "80 "
      "F0 "
      "FF "
      "FF ", dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Yuv420_close(&dest);
});

LIBYUV(RotateYuvSemiPlanarToYuvPlanar, [this](){
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
      "01 02 "
      "03 04 "
      "05 06 "
      "07 08 "
      "09 "
      "0A "
      "0B "
      "0C ",
      Yuv420Type_planar,
      2, 4);
  if(!ttLibC_LibyuvResampler_rotate((ttLibC_Video *)dest, (ttLibC_Video *)src, LibyuvRotate_90)) {
    FAIL();
  }
  binaryEq(
      "00 00 "
      "00 00 "
      "40 80 "
      "20 80 "
      "FF "
      "80 "
      "FF "
      "F0 ", dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Yuv420_close(&dest);
});

LIBYUV(RotateYuvSemiPlanarToYvuPlanar, [this](){
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
      "01 02 "
      "03 04 "
      "05 06 "
      "07 08 "
      "09 "
      "0A "
      "0B "
      "0C ",
      Yvu420Type_planar,
      2, 4);
  if(!ttLibC_LibyuvResampler_rotate((ttLibC_Video *)dest, (ttLibC_Video *)src, LibyuvRotate_90)) {
    FAIL();
  }
  binaryEq(
      "00 00 "
      "00 00 "
      "40 80 "
      "20 80 "
      "FF "
      "F0 "
      "FF "
      "80 ", dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Yuv420_close(&dest);
});

LIBYUV(RotateBgraToBgra, [this](){
  // original
  auto src = makeBgr(
      "10 20 30 40 11 21 31 41 12 22 32 42 13 23 33 43 "
      "14 24 34 44 15 25 35 45 16 26 36 46 17 27 37 47 ",
      BgrType_bgra,
      4, 2);
  // target
  auto dest = makeBgr(
      "00 00 00 00 00 00 00 00 "
      "00 00 00 00 00 00 00 00 "
      "00 00 00 00 00 00 00 00 "
      "00 00 00 00 00 00 00 00 ",
      BgrType_bgra,
      2, 4);
  if(!ttLibC_LibyuvResampler_rotate((ttLibC_Video *)dest, (ttLibC_Video *)src, LibyuvRotate_90)) {
    FAIL();
  }
  binaryEq(
      "14 24 34 44 10 20 30 40 "
      "15 25 35 45 11 21 31 41 "
      "16 26 36 46 12 22 32 42 "
      "17 27 37 47 13 23 33 43 ",
      dest);
  ttLibC_Bgr_close(&src);
  ttLibC_Bgr_close(&dest);
});

LIBYUV(RotateAbgrToAbgr, [this](){
  // original
  auto src = makeBgr(
      "10 20 30 40 11 21 31 41 12 22 32 42 13 23 33 43 "
      "14 24 34 44 15 25 35 45 16 26 36 46 17 27 37 47 ",
      BgrType_abgr,
      4, 2);
  // target
  auto dest = makeBgr(
      "00 00 00 00 00 00 00 00 "
      "00 00 00 00 00 00 00 00 "
      "00 00 00 00 00 00 00 00 "
      "00 00 00 00 00 00 00 00 ",
      BgrType_abgr,
      2, 4);
  if(!ttLibC_LibyuvResampler_rotate((ttLibC_Video *)dest, (ttLibC_Video *)src, LibyuvRotate_90)) {
    FAIL();
  }
  binaryEq(
      "14 24 34 44 10 20 30 40 "
      "15 25 35 45 11 21 31 41 "
      "16 26 36 46 12 22 32 42 "
      "17 27 37 47 13 23 33 43 ",
      dest);
  ttLibC_Bgr_close(&src);
  ttLibC_Bgr_close(&dest);
});

LIBYUV(RotateRgbaToRgba, [this](){
  // original
  auto src = makeBgr(
      "10 20 30 40 11 21 31 41 12 22 32 42 13 23 33 43 "
      "14 24 34 44 15 25 35 45 16 26 36 46 17 27 37 47 ",
      BgrType_rgba,
      4, 2);
  // target
  auto dest = makeBgr(
      "00 00 00 00 00 00 00 00 "
      "00 00 00 00 00 00 00 00 "
      "00 00 00 00 00 00 00 00 "
      "00 00 00 00 00 00 00 00 ",
      BgrType_rgba,
      2, 4);
  if(!ttLibC_LibyuvResampler_rotate((ttLibC_Video *)dest, (ttLibC_Video *)src, LibyuvRotate_90)) {
    FAIL();
  }
  binaryEq(
      "14 24 34 44 10 20 30 40 "
      "15 25 35 45 11 21 31 41 "
      "16 26 36 46 12 22 32 42 "
      "17 27 37 47 13 23 33 43 ",
      dest);
  ttLibC_Bgr_close(&src);
  ttLibC_Bgr_close(&dest);
});

LIBYUV(RotateArgbToArgb, [this](){
  // original
  auto src = makeBgr(
      "10 20 30 40 11 21 31 41 12 22 32 42 13 23 33 43 "
      "14 24 34 44 15 25 35 45 16 26 36 46 17 27 37 47 ",
      BgrType_argb,
      4, 2);
  // target
  auto dest = makeBgr(
      "00 00 00 00 00 00 00 00 "
      "00 00 00 00 00 00 00 00 "
      "00 00 00 00 00 00 00 00 "
      "00 00 00 00 00 00 00 00 ",
      BgrType_argb,
      2, 4);
  if(!ttLibC_LibyuvResampler_rotate((ttLibC_Video *)dest, (ttLibC_Video *)src, LibyuvRotate_90)) {
    FAIL();
  }
  binaryEq(
      "14 24 34 44 10 20 30 40 "
      "15 25 35 45 11 21 31 41 "
      "16 26 36 46 12 22 32 42 "
      "17 27 37 47 13 23 33 43 ",
      dest);
  ttLibC_Bgr_close(&src);
  ttLibC_Bgr_close(&dest);
});

LIBYUV(ScaleYuvPlanar, [this](){
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
  if(!ttLibC_LibyuvResampler_resize((ttLibC_Video *)dest, (ttLibC_Video *)src, LibyuvFilter_None, LibyuvFilter_None)) {
    FAIL();
  }
  binaryEq(
      "00 00 00 00 80 80 80 80 "
      "00 00 00 00 40 40 20 20 "
      "FF FF FF FF "
      "80 80 F0 F0 ",
      dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Yuv420_close(&dest);
});

LIBYUV(ScaleYuvPlanarToYvuPlanar, [this](){
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
  if(!ttLibC_LibyuvResampler_resize((ttLibC_Video *)dest, (ttLibC_Video *)src, LibyuvFilter_None, LibyuvFilter_None)) {
    FAIL();
  }
  binaryEq(
      "00 00 00 00 80 80 80 80 "
      "00 00 00 00 40 40 20 20 "
      "80 80 F0 F0 "
      "FF FF FF FF ",
      dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Yuv420_close(&dest);
});

LIBYUV(ScaleYuvSemiPlanar, [this](){
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
  if(!ttLibC_LibyuvResampler_resize((ttLibC_Video *)dest, (ttLibC_Video *)src, LibyuvFilter_None, LibyuvFilter_None)) {
    FAIL();
  }
  binaryEq(
      "00 00 00 00 80 80 80 80 "
      "00 00 00 00 40 40 20 20 "
      "FF FF FF FF "
      "80 F0 80 F0 ",
      dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Yuv420_close(&dest);
});

LIBYUV(ScaleYvuSemiPlanar, [this](){
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
  if(!ttLibC_LibyuvResampler_resize((ttLibC_Video *)dest, (ttLibC_Video *)src, LibyuvFilter_None, LibyuvFilter_None)) {
    FAIL();
  }
  binaryEq(
      "00 00 00 00 80 80 80 80 "
      "00 00 00 00 40 40 20 20 "
      "FF FF FF FF "
      "80 F0 80 F0 ",
      dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Yuv420_close(&dest);
});

LIBYUV(ScaleBgra, [this](){
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
  if(!ttLibC_LibyuvResampler_resize((ttLibC_Video *)dest, (ttLibC_Video *)src, LibyuvFilter_None, LibyuvFilter_None)) {
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

LIBYUV(YuvPlanarToBgra, [this]() {
  auto src = makeYuv(
    "00 00 80 80 "
    "00 00 40 20 "
    "FF FF "
    "80 F0 ",
    Yuv420Type_planar,
    4, 2);
  auto dest = makeBgr(
    "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "
    "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ",
    BgrType_bgra,
    4, 2);
  if(!ttLibC_LibyuvResampler_ToBgr(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "EB 00 00 FF EB 00 00 FF FF 00 FF FF FF 00 FF FF "
      "EB 00 00 FF EB 00 00 FF FF 00 EA FF FF 00 C5 FF ",
      dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Bgr_close(&dest);
});

LIBYUV(YuvPlanarToAbgr, [this]() {
  auto src = makeYuv(
    "00 00 80 80 "
    "00 00 40 20 "
    "FF FF "
    "80 F0 ",
    Yuv420Type_planar,
    4, 2);
  auto dest = makeBgr(
    "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "
    "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ",
    BgrType_abgr,
    4, 2);
  if(!ttLibC_LibyuvResampler_ToBgr(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "FF EB 00 00 FF EB 00 00 FF FF 00 FF FF FF 00 FF "
      "FF EB 00 00 FF EB 00 00 FF FF 00 EA FF FF 00 C5 ",
      dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Bgr_close(&dest);
});

LIBYUV(YuvPlanarToRgba, [this]() {
  auto src = makeYuv(
    "00 00 80 80 "
    "00 00 40 20 "
    "FF FF "
    "80 F0 ",
    Yuv420Type_planar,
    4, 2);
  auto dest = makeBgr(
    "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "
    "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ",
    BgrType_rgba,
    4, 2);
  if(!ttLibC_LibyuvResampler_ToBgr(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "00 00 EB FF 00 00 EB FF FF 00 FF FF FF 00 FF FF "
      "00 00 EB FF 00 00 EB FF EA 00 FF FF C5 00 FF FF ",
      dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Bgr_close(&dest);
});

LIBYUV(YuvPlanarToArgb, [this]() {
  auto src = makeYuv(
    "00 00 80 80 "
    "00 00 40 20 "
    "FF FF "
    "80 F0 ",
    Yuv420Type_planar,
    4, 2);
  auto dest = makeBgr(
    "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "
    "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ",
    BgrType_argb,
    4, 2);
  if(!ttLibC_LibyuvResampler_ToBgr(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "FF 00 00 EB FF 00 00 EB FF FF 00 FF FF FF 00 FF "
      "FF 00 00 EB FF 00 00 EB FF EA 00 FF FF C5 00 FF ",
      dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Bgr_close(&dest);
});

LIBYUV(YuvPlanarToBgr, [this]() {
  auto src = makeYuv(
    "00 00 80 80 "
    "00 00 40 20 "
    "FF FF "
    "80 F0 ",
    Yuv420Type_planar,
    4, 2);
  auto dest = makeBgr(
    "00 00 00 00 00 00 00 00 00 00 00 00 "
    "00 00 00 00 00 00 00 00 00 00 00 00 ",
    BgrType_bgr,
    4, 2);
  if(!ttLibC_LibyuvResampler_ToBgr(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "EB 00 00 EB 00 00 FF 00 FF FF 00 FF "
      "EB 00 00 EB 00 00 FF 00 EA FF 00 C5 ",
      dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Bgr_close(&dest);
});

LIBYUV(YuvSemiPlanarToBgra, [this]() {
  auto src = makeYuv(
    "00 00 80 80 "
    "00 00 40 20 "
    "FF 80 FF F0 ",
    Yuv420Type_semiPlanar,
    4, 2);
  auto dest = makeBgr(
    "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "
    "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ",
    BgrType_bgra,
    4, 2);
  if(!ttLibC_LibyuvResampler_ToBgr(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "EC 00 00 FF EC 00 00 FF FF 00 FF FF FF 00 FF FF "
      "EC 00 00 FF EC 00 00 FF FF 00 E8 FF FF 00 C3 FF ",
      dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Bgr_close(&dest);
});

LIBYUV(YuvSemiPlanarToRgba, [this]() {
  auto src = makeYuv(
    "00 00 80 80 "
    "00 00 40 20 "
    "FF 80 FF F0 ",
    Yuv420Type_semiPlanar,
    4, 2);
  auto dest = makeBgr(
    "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "
    "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ",
    BgrType_rgba,
    4, 2);
  if(!ttLibC_LibyuvResampler_ToBgr(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "00 00 EB FF 00 00 EB FF FF 00 FF FF FF 00 FF FF "
      "00 00 EB FF 00 00 EB FF EA 00 FF FF C5 00 FF FF ",
      dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Bgr_close(&dest);
});

LIBYUV(YvuSemiPlanarToBgra, [this]() {
  auto src = makeYuv(
    "00 00 80 80 "
    "00 00 40 20 "
    "FF 80 FF F0 ",
    Yvu420Type_semiPlanar,
    4, 2);
  auto dest = makeBgr(
    "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "
    "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ",
    BgrType_bgra,
    4, 2);
  if(!ttLibC_LibyuvResampler_ToBgr(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "00 00 B8 FF 00 00 B8 FF FF 00 FF FF FF 00 FF FF "
      "00 00 B8 FF 00 00 B8 FF FF 00 FF FF F3 00 DD FF ",
      dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Bgr_close(&dest);
});

LIBYUV(YvuSemiPlanarToRgba, [this]() {
  auto src = makeYuv(
    "00 00 80 80 "
    "00 00 40 20 "
    "FF 80 FF F0 ",
    Yvu420Type_semiPlanar,
    4, 2);
  auto dest = makeBgr(
    "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "
    "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ",
    BgrType_rgba,
    4, 2);
  if(!ttLibC_LibyuvResampler_ToBgr(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "B8 00 00 FF B8 00 00 FF FF 00 FF FF FF 00 FF FF "
      "B8 00 00 FF B8 00 00 FF FF 00 FF FF DD 00 F3 FF ",
      dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Bgr_close(&dest);
});

LIBYUV(BgraToBgra, [this](){
  // original
  auto src = makeBgr(
      "10 20 30 40 11 21 31 41 12 22 32 42 13 23 33 43 "
      "14 24 34 44 15 25 35 45 16 26 36 46 17 27 37 47 ",
      BgrType_bgra,
      4, 2);
  // target
  auto dest = makeBgr(
      "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "
      "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ",
      BgrType_bgra,
      4, 2);
  if(!ttLibC_LibyuvResampler_ToBgr(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "10 20 30 40 11 21 31 41 12 22 32 42 13 23 33 43 "
      "14 24 34 44 15 25 35 45 16 26 36 46 17 27 37 47 ",
      dest);
  ttLibC_Bgr_close(&src);
  ttLibC_Bgr_close(&dest);
});

LIBYUV(BgraToArgb, [this](){
  // original
  auto src = makeBgr(
      "10 20 30 40 11 21 31 41 12 22 32 42 13 23 33 43 "
      "14 24 34 44 15 25 35 45 16 26 36 46 17 27 37 47 ",
      BgrType_bgra,
      4, 2);
  // target
  auto dest = makeBgr(
      "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "
      "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ",
      BgrType_argb,
      4, 2);
  if(!ttLibC_LibyuvResampler_ToBgr(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "40 30 20 10 41 31 21 11 42 32 22 12 43 33 23 13 "
      "44 34 24 14 45 35 25 15 46 36 26 16 47 37 27 17 ",
      dest);
  ttLibC_Bgr_close(&src);
  ttLibC_Bgr_close(&dest);
});

LIBYUV(BgraToRgba, [this](){
  // original
  auto src = makeBgr(
      "10 20 30 40 11 21 31 41 12 22 32 42 13 23 33 43 "
      "14 24 34 44 15 25 35 45 16 26 36 46 17 27 37 47 ",
      BgrType_bgra,
      4, 2);
  // target
  auto dest = makeBgr(
      "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "
      "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ",
      BgrType_rgba,
      4, 2);
  if(!ttLibC_LibyuvResampler_ToBgr(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "30 20 10 40 31 21 11 41 32 22 12 42 33 23 13 43 "
      "34 24 14 44 35 25 15 45 36 26 16 46 37 27 17 47 ",
      dest);
  ttLibC_Bgr_close(&src);
  ttLibC_Bgr_close(&dest);
});

LIBYUV(BgraToAbgr, [this](){
  // original
  auto src = makeBgr(
      "10 20 30 40 11 21 31 41 12 22 32 42 13 23 33 43 "
      "14 24 34 44 15 25 35 45 16 26 36 46 17 27 37 47 ",
      BgrType_bgra,
      4, 2);
  // target
  auto dest = makeBgr(
      "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "
      "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ",
      BgrType_abgr,
      4, 2);
  if(!ttLibC_LibyuvResampler_ToBgr(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "40 10 20 30 41 11 21 31 42 12 22 32 43 13 23 33 "
      "44 14 24 34 45 15 25 35 46 16 26 36 47 17 27 37 ",
      dest);
  ttLibC_Bgr_close(&src);
  ttLibC_Bgr_close(&dest);
});

LIBYUV(BgraToBgr, [this](){
  // original
  auto src = makeBgr(
      "10 20 30 40 11 21 31 41 12 22 32 42 13 23 33 43 "
      "14 24 34 44 15 25 35 45 16 26 36 46 17 27 37 47 ",
      BgrType_bgra,
      4, 2);
  // target
  auto dest = makeBgr(
      "00 00 00 00 00 00 00 00 00 00 00 00 "
      "00 00 00 00 00 00 00 00 00 00 00 00 ",
      BgrType_bgr,
      4, 2);
  if(!ttLibC_LibyuvResampler_ToBgr(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "10 20 30 11 21 31 12 22 32 13 23 33 "
      "14 24 34 15 25 35 16 26 36 17 27 37 ",
      dest);
  ttLibC_Bgr_close(&src);
  ttLibC_Bgr_close(&dest);
});

LIBYUV(ArgbToBgra, [this](){
  // original
  auto src = makeBgr(
      "10 20 30 40 11 21 31 41 12 22 32 42 13 23 33 43 "
      "14 24 34 44 15 25 35 45 16 26 36 46 17 27 37 47 ",
      BgrType_argb,
      4, 2);
  // target
  auto dest = makeBgr(
      "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "
      "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ",
      BgrType_bgra,
      4, 2);
  if(!ttLibC_LibyuvResampler_ToBgr(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "40 30 20 10 41 31 21 11 42 32 22 12 43 33 23 13 "
      "44 34 24 14 45 35 25 15 46 36 26 16 47 37 27 17 ",
      dest);
  ttLibC_Bgr_close(&src);
  ttLibC_Bgr_close(&dest);
});

LIBYUV(RgbaToBgra, [this](){
  // original
  auto src = makeBgr(
      "10 20 30 40 11 21 31 41 12 22 32 42 13 23 33 43 "
      "14 24 34 44 15 25 35 45 16 26 36 46 17 27 37 47 ",
      BgrType_rgba,
      4, 2);
  // target
  auto dest = makeBgr(
      "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "
      "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ",
      BgrType_bgra,
      4, 2);
  if(!ttLibC_LibyuvResampler_ToBgr(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "30 20 10 40 31 21 11 41 32 22 12 42 33 23 13 43 "
      "34 24 14 44 35 25 15 45 36 26 16 46 37 27 17 47 ",
      dest);
  ttLibC_Bgr_close(&src);
  ttLibC_Bgr_close(&dest);
});

LIBYUV(AbgrToBgra, [this](){
  // original
  auto src = makeBgr(
      "10 20 30 40 11 21 31 41 12 22 32 42 13 23 33 43 "
      "14 24 34 44 15 25 35 45 16 26 36 46 17 27 37 47 ",
      BgrType_abgr,
      4, 2);
  // target
  auto dest = makeBgr(
      "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "
      "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ",
      BgrType_bgra,
      4, 2);
  if(!ttLibC_LibyuvResampler_ToBgr(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "20 30 40 10 21 31 41 11 22 32 42 12 23 33 43 13 "
      "24 34 44 14 25 35 45 15 26 36 46 16 27 37 47 17 ",
      dest);
  ttLibC_Bgr_close(&src);
  ttLibC_Bgr_close(&dest);
});

LIBYUV(BgrToBgra, [this](){
  // original
  auto src = makeBgr(
      "10 20 30 11 21 31 12 22 32 13 23 33 "
      "14 24 34 15 25 35 16 26 36 17 27 37 ",
      BgrType_bgr,
      4, 2);
  // target
  auto dest = makeBgr(
      "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "
      "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ",
      BgrType_bgra,
      4, 2);
  if(!ttLibC_LibyuvResampler_ToBgr(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "10 20 30 FF 11 21 31 FF 12 22 32 FF 13 23 33 FF "
      "14 24 34 FF 15 25 35 FF 16 26 36 FF 17 27 37 FF ",
      dest);
  ttLibC_Bgr_close(&src);
  ttLibC_Bgr_close(&dest);
});

LIBYUV(YuvPlanarToYuvPlanar, [this](){
  // original
  auto src = makeYuv(
      "10 20 30 40 "
      "11 22 33 44 "
      "A0 B0 "
      "A1 B1 ",
      Yuv420Type_planar,
      4, 2);
  // target
  auto dest = makeYuv(
      "00 00 00 00 "
      "00 00 00 00 "
      "00 00 "
      "00 00 ",
      Yuv420Type_planar,
      4, 2);
  if(!ttLibC_LibyuvResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "10 20 30 40 "
      "11 22 33 44 "
      "A0 B0 "
      "A1 B1 ",
      dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Yuv420_close(&dest);
});

LIBYUV(YuvPlanarToYvuPlanar, [this](){
  // original
  auto src = makeYuv(
      "10 20 30 40 "
      "11 22 33 44 "
      "A0 B0 "
      "A1 B1 ",
      Yuv420Type_planar,
      4, 2);
  // target
  auto dest = makeYuv(
      "00 00 00 00 "
      "00 00 00 00 "
      "00 00 "
      "00 00 ",
      Yvu420Type_planar,
      4, 2);
  if(!ttLibC_LibyuvResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "10 20 30 40 "
      "11 22 33 44 "
      "A1 B1 "
      "A0 B0 ",
      dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Yuv420_close(&dest);
});

LIBYUV(YuvPlanarToYuvSemiPlanar, [this](){
  // original
  auto src = makeYuv(
      "10 20 30 40 "
      "11 22 33 44 "
      "A0 B0 "
      "A1 B1 ",
      Yuv420Type_planar,
      4, 2);
  // target
  auto dest = makeYuv(
      "00 00 00 00 "
      "00 00 00 00 "
      "00 00 00 00 ",
      Yuv420Type_semiPlanar,
      4, 2);
  if(!ttLibC_LibyuvResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "10 20 30 40 "
      "11 22 33 44 "
      "A0 A1 B0 B1 ",
      dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Yuv420_close(&dest);
});

LIBYUV(YuvPlanarToYvuSemiPlanar, [this](){
  // original
  auto src = makeYuv(
      "10 20 30 40 "
      "11 22 33 44 "
      "A0 B0 "
      "A1 B1 ",
      Yuv420Type_planar,
      4, 2);
  // target
  auto dest = makeYuv(
      "00 00 00 00 "
      "00 00 00 00 "
      "00 00 00 00 ",
      Yvu420Type_semiPlanar,
      4, 2);
  if(!ttLibC_LibyuvResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "10 20 30 40 "
      "11 22 33 44 "
      "A1 A0 B1 B0 ",
      dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Yuv420_close(&dest);
});

LIBYUV(YvuPlanarToYuvPlanar, [this](){
  // original
  auto src = makeYuv(
      "10 20 30 40 "
      "11 22 33 44 "
      "A0 B0 "
      "A1 B1 ",
      Yvu420Type_planar,
      4, 2);
  // target
  auto dest = makeYuv(
      "00 00 00 00 "
      "00 00 00 00 "
      "00 00 "
      "00 00 ",
      Yuv420Type_planar,
      4, 2);
  if(!ttLibC_LibyuvResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "10 20 30 40 "
      "11 22 33 44 "
      "A1 B1 "
      "A0 B0 ",
      dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Yuv420_close(&dest);
});

LIBYUV(YvuPlanarToYvuPlanar, [this](){
  // original
  auto src = makeYuv(
      "10 20 30 40 "
      "11 22 33 44 "
      "A0 B0 "
      "A1 B1 ",
      Yvu420Type_planar,
      4, 2);
  // target
  auto dest = makeYuv(
      "00 00 00 00 "
      "00 00 00 00 "
      "00 00 "
      "00 00 ",
      Yvu420Type_planar,
      4, 2);
  if(!ttLibC_LibyuvResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "10 20 30 40 "
      "11 22 33 44 "
      "A0 B0 "
      "A1 B1 ",
      dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Yuv420_close(&dest);
});

LIBYUV(YvuPlanarToYuvSemiPlanar, [this](){
  // original
  auto src = makeYuv(
      "10 20 30 40 "
      "11 22 33 44 "
      "A0 B0 "
      "A1 B1 ",
      Yvu420Type_planar,
      4, 2);
  // target
  auto dest = makeYuv(
      "00 00 00 00 "
      "00 00 00 00 "
      "00 00 00 00 ",
      Yuv420Type_semiPlanar,
      4, 2);
  if(!ttLibC_LibyuvResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "10 20 30 40 "
      "11 22 33 44 "
      "A1 A0 B1 B0 ",
      dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Yuv420_close(&dest);
});

LIBYUV(YvuPlanarToYvuSemiPlanar, [this](){
  // original
  auto src = makeYuv(
      "10 20 30 40 "
      "11 22 33 44 "
      "A0 B0 "
      "A1 B1 ",
      Yvu420Type_planar,
      4, 2);
  // target
  auto dest = makeYuv(
      "00 00 00 00 "
      "00 00 00 00 "
      "00 00 00 00 ",
      Yvu420Type_semiPlanar,
      4, 2);
  if(!ttLibC_LibyuvResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "10 20 30 40 "
      "11 22 33 44 "
      "A0 A1 B0 B1 ",
      dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Yuv420_close(&dest);
});

LIBYUV(YuvSemiPlanarToYuvPlanar, [this](){
  // original
  auto src = makeYuv(
      "10 20 30 40 "
      "11 22 33 44 "
      "A0 B0 A1 B1 ",
      Yuv420Type_semiPlanar,
      4, 2);
  // target
  auto dest = makeYuv(
      "00 00 00 00 "
      "00 00 00 00 "
      "00 00 "
      "00 00 ",
      Yuv420Type_planar,
      4, 2);
  if(!ttLibC_LibyuvResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "10 20 30 40 "
      "11 22 33 44 "
      "A0 A1 B0 B1 ",
      dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Yuv420_close(&dest);
});

LIBYUV(YuvSemiPlanarToYvuPlanar, [this](){
  // original
  auto src = makeYuv(
      "10 20 30 40 "
      "11 22 33 44 "
      "A0 B0 A1 B1 ",
      Yuv420Type_semiPlanar,
      4, 2);
  // target
  auto dest = makeYuv(
      "00 00 00 00 "
      "00 00 00 00 "
      "00 00 "
      "00 00 ",
      Yvu420Type_planar,
      4, 2);
  if(!ttLibC_LibyuvResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "10 20 30 40 "
      "11 22 33 44 "
      "B0 B1 A0 A1 ",
      dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Yuv420_close(&dest);
});

LIBYUV(YvuSemiPlanarToYuvPlanar, [this](){
  // original
  auto src = makeYuv(
      "10 20 30 40 "
      "11 22 33 44 "
      "A0 B0 A1 B1 ",
      Yvu420Type_semiPlanar,
      4, 2);
  // target
  auto dest = makeYuv(
      "00 00 00 00 "
      "00 00 00 00 "
      "00 00 "
      "00 00 ",
      Yuv420Type_planar,
      4, 2);
  if(!ttLibC_LibyuvResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "10 20 30 40 "
      "11 22 33 44 "
      "B0 B1 A0 A1 ",
      dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Yuv420_close(&dest);
});

LIBYUV(YvuSemiPlanarToYvuPlanar, [this](){
  // original
  auto src = makeYuv(
      "10 20 30 40 "
      "11 22 33 44 "
      "A0 B0 A1 B1 ",
      Yvu420Type_semiPlanar,
      4, 2);
  // target
  auto dest = makeYuv(
      "00 00 00 00 "
      "00 00 00 00 "
      "00 00 "
      "00 00 ",
      Yvu420Type_planar,
      4, 2);
  if(!ttLibC_LibyuvResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "10 20 30 40 "
      "11 22 33 44 "
      "A0 A1 B0 B1 ",
      dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Yuv420_close(&dest);
});

LIBYUV(BgraToYuvPlanar, [this](){
  // original
  auto src = makeBgr(
      "10 20 30 40 11 21 31 41 12 22 32 42 13 23 33 43 "
      "14 24 34 44 15 25 35 45 16 26 36 46 17 27 37 47 ",
      BgrType_bgra,
      4, 2);
  // target
  auto dest = makeYuv(
      "00 00 00 00 "
      "00 00 00 00 "
      "00 00 "
      "00 00 ",
      Yuv420Type_planar,
      4, 2);
  if(!ttLibC_LibyuvResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "2E 2F 2F 30 "
      "31 32 33 34 "
      "76 76 "
      "88 88 ",
      dest);
  ttLibC_Bgr_close(&src);
  ttLibC_Yuv420_close(&dest);
});

LIBYUV(BgraToYvuPlanar, [this](){
  // original
  auto src = makeBgr(
      "10 20 30 40 11 21 31 41 12 22 32 42 13 23 33 43 "
      "14 24 34 44 15 25 35 45 16 26 36 46 17 27 37 47 ",
      BgrType_bgra,
      4, 2);
  // target
  auto dest = makeYuv(
      "00 00 00 00 "
      "00 00 00 00 "
      "00 00 "
      "00 00 ",
      Yvu420Type_planar,
      4, 2);
  if(!ttLibC_LibyuvResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "2E 2F 2F 30 "
      "31 32 33 34 "
      "88 88 "
      "76 76 ",
      dest);
  ttLibC_Bgr_close(&src);
  ttLibC_Yuv420_close(&dest);
});

LIBYUV(BgraToYuvSemiPlanar, [this](){
  // original
  auto src = makeBgr(
      "10 20 30 40 11 21 31 41 12 22 32 42 13 23 33 43 "
      "14 24 34 44 15 25 35 45 16 26 36 46 17 27 37 47 ",
      BgrType_bgra,
      4, 2);
  // target
  auto dest = makeYuv(
      "00 00 00 00 "
      "00 00 00 00 "
      "00 00 00 00 ",
      Yuv420Type_semiPlanar,
      4, 2);
  if(!ttLibC_LibyuvResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "2E 2F 2F 30 "
      "31 32 33 34 "
      "76 88 76 88 ",
      dest);
  ttLibC_Bgr_close(&src);
  ttLibC_Yuv420_close(&dest);
});

LIBYUV(BgraToYvuSemiPlanar, [this](){
  // original
  auto src = makeBgr(
      "10 20 30 40 11 21 31 41 12 22 32 42 13 23 33 43 "
      "14 24 34 44 15 25 35 45 16 26 36 46 17 27 37 47 ",
      BgrType_bgra,
      4, 2);
  // target
  auto dest = makeYuv(
      "00 00 00 00 "
      "00 00 00 00 "
      "00 00 00 00 ",
      Yvu420Type_semiPlanar,
      4, 2);
  if(!ttLibC_LibyuvResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "2E 2F 2F 30 "
      "31 32 33 34 "
      "88 76 88 76 ",
      dest);
  ttLibC_Bgr_close(&src);
  ttLibC_Yuv420_close(&dest);
});

LIBYUV(ArgbToYuvPlanar, [this](){
  // original
  auto src = makeBgr(
      "10 20 30 40 11 21 31 41 12 22 32 42 13 23 33 43 "
      "14 24 34 44 15 25 35 45 16 26 36 46 17 27 37 47 ",
      BgrType_argb,
      4, 2);
  // target
  auto dest = makeYuv(
      "00 00 00 00 "
      "00 00 00 00 "
      "00 00 "
      "00 00 ",
      Yuv420Type_planar,
      4, 2);
  if(!ttLibC_LibyuvResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "37 37 38 39 "
      "3A 3B 3C 3D "
      "89 89 "
      "77 77 ",
      dest);
  ttLibC_Bgr_close(&src);
  ttLibC_Yuv420_close(&dest);
});

LIBYUV(ArgbToYvuPlanar, [this](){
  // original
  auto src = makeBgr(
      "10 20 30 40 11 21 31 41 12 22 32 42 13 23 33 43 "
      "14 24 34 44 15 25 35 45 16 26 36 46 17 27 37 47 ",
      BgrType_argb,
      4, 2);
  // target
  auto dest = makeYuv(
      "00 00 00 00 "
      "00 00 00 00 "
      "00 00 "
      "00 00 ",
      Yvu420Type_planar,
      4, 2);
  if(!ttLibC_LibyuvResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "37 37 38 39 "
      "3A 3B 3C 3D "
      "77 77 "
      "89 89 ",
      dest);
  ttLibC_Bgr_close(&src);
  ttLibC_Yuv420_close(&dest);
});
LIBYUV(RgbaToYuvPlanar, [this](){
  // original
  auto src = makeBgr(
      "10 20 30 40 11 21 31 41 12 22 32 42 13 23 33 43 "
      "14 24 34 44 15 25 35 45 16 26 36 46 17 27 37 47 ",
      BgrType_rgba,
      4, 2);
  // target
  auto dest = makeYuv(
      "00 00 00 00 "
      "00 00 00 00 "
      "00 00 "
      "00 00 ",
      Yuv420Type_planar,
      4, 2);
  if(!ttLibC_LibyuvResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "29 2A 2A 2B "
      "2C 2D 2E 2F "
      "89 89 "
      "77 77 ",
      dest);
  ttLibC_Bgr_close(&src);
  ttLibC_Yuv420_close(&dest);
});

LIBYUV(RgbaToYvuPlanar, [this](){
  // original
  auto src = makeBgr(
      "10 20 30 40 11 21 31 41 12 22 32 42 13 23 33 43 "
      "14 24 34 44 15 25 35 45 16 26 36 46 17 27 37 47 ",
      BgrType_rgba,
      4, 2);
  // target
  auto dest = makeYuv(
      "00 00 00 00 "
      "00 00 00 00 "
      "00 00 "
      "00 00 ",
      Yvu420Type_planar,
      4, 2);
  if(!ttLibC_LibyuvResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "29 2A 2A 2B "
      "2C 2D 2E 2F "
      "77 77 "
      "89 89 ",
      dest);
  ttLibC_Bgr_close(&src);
  ttLibC_Yuv420_close(&dest);
});

LIBYUV(AbgrToYuvPlanar, [this](){
  // original
  auto src = makeBgr(
      "10 20 30 40 11 21 31 41 12 22 32 42 13 23 33 43 "
      "14 24 34 44 15 25 35 45 16 26 36 46 17 27 37 47 ",
      BgrType_abgr,
      4, 2);
  // target
  auto dest = makeYuv(
      "00 00 00 00 "
      "00 00 00 00 "
      "00 00 "
      "00 00 ",
      Yuv420Type_planar,
      4, 2);
  if(!ttLibC_LibyuvResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "3C 3C 3D 3E "
      "3F 40 41 42 "
      "76 76 "
      "88 88 ",
      dest);
  ttLibC_Bgr_close(&src);
  ttLibC_Yuv420_close(&dest);
});
LIBYUV(AbgrToYvuPlanar, [this](){
  // original
  auto src = makeBgr(
      "10 20 30 40 11 21 31 41 12 22 32 42 13 23 33 43 "
      "14 24 34 44 15 25 35 45 16 26 36 46 17 27 37 47 ",
      BgrType_abgr,
      4, 2);
  // target
  auto dest = makeYuv(
      "00 00 00 00 "
      "00 00 00 00 "
      "00 00 "
      "00 00 ",
      Yvu420Type_planar,
      4, 2);
  if(!ttLibC_LibyuvResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "3C 3C 3D 3E "
      "3F 40 41 42 "
      "88 88 "
      "76 76 ",
      dest);
  ttLibC_Bgr_close(&src);
  ttLibC_Yuv420_close(&dest);
});
LIBYUV(BgrToYuvPlanar, [this](){
  // original
  auto src = makeBgr(
      "10 20 30 11 21 31 12 22 32 13 23 33 "
      "14 24 34 15 25 35 16 26 36 17 27 37 ",
      BgrType_bgr,
      4, 2);
  // target
  auto dest = makeYuv(
      "00 00 00 00 "
      "00 00 00 00 "
      "00 00 "
      "00 00 ",
      Yuv420Type_planar,
      4, 2);
  if(!ttLibC_LibyuvResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "2E 2F 2F 30 "
      "31 32 33 34 "
      "76 76 "
      "88 88 ",
      dest);
  ttLibC_Bgr_close(&src);
  ttLibC_Yuv420_close(&dest);
});
LIBYUV(BgrToYvuPlanar, [this](){
  // original
  auto src = makeBgr(
      "10 20 30 11 21 31 12 22 32 13 23 33 "
      "14 24 34 15 25 35 16 26 36 17 27 37 ",
      BgrType_bgr,
      4, 2);
  // target
  auto dest = makeYuv(
      "00 00 00 00 "
      "00 00 00 00 "
      "00 00 "
      "00 00 ",
      Yvu420Type_planar,
      4, 2);
  if(!ttLibC_LibyuvResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "2E 2F 2F 30 "
      "31 32 33 34 "
      "88 88 "
      "76 76 ",
      dest);
  ttLibC_Bgr_close(&src);
  ttLibC_Yuv420_close(&dest);
});

#undef LIBYUV
