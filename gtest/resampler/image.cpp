#include "../resampler.hpp"
#include <ttLibC/resampler/imageResampler.h>


#define IMAGE(A, B) TEST_F(ResamplerTest, Image##A){B();}

IMAGE(YuvPlanarToBgra, [this]() {
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
  if(!ttLibC_ImageResampler_ToBgr(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "EB 00 00 FF EB 00 00 FF FF 00 FF FF FF 00 FF FF "
      "EB 00 00 FF EB 00 00 FF FF 00 EA FF FF 00 C5 FF ",
      dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Bgr_close(&dest);
});

IMAGE(YuvPlanarToAbgr, [this]() {
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
  if(!ttLibC_ImageResampler_ToBgr(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "FF EB 00 00 FF EB 00 00 FF FF 00 FF FF FF 00 FF "
      "FF EB 00 00 FF EB 00 00 FF FF 00 EA FF FF 00 C5 ",
      dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Bgr_close(&dest);
});

IMAGE(YuvPlanarToRgba, [this]() {
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
  if(!ttLibC_ImageResampler_ToBgr(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "00 00 EB FF 00 00 EB FF FF 00 FF FF FF 00 FF FF "
      "00 00 EB FF 00 00 EB FF EA 00 FF FF C5 00 FF FF ",
      dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Bgr_close(&dest);
});

IMAGE(YuvPlanarToArgb, [this]() {
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
  if(!ttLibC_ImageResampler_ToBgr(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "FF 00 00 EB FF 00 00 EB FF FF 00 FF FF FF 00 FF "
      "FF 00 00 EB FF 00 00 EB FF EA 00 FF FF C5 00 FF ",
      dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Bgr_close(&dest);
});

IMAGE(YuvPlanarToBgr, [this]() {
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
  if(!ttLibC_ImageResampler_ToBgr(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "EB 00 00 EB 00 00 FF 00 FF FF 00 FF "
      "EB 00 00 EB 00 00 FF 00 EA FF 00 C5 ",
      dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Bgr_close(&dest);
});

IMAGE(YuvSemiPlanarToBgra, [this]() {
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
  if(!ttLibC_ImageResampler_ToBgr(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "EC 00 00 FF EC 00 00 FF FF 00 FF FF FF 00 FF FF "
      "EC 00 00 FF EC 00 00 FF FF 00 E8 FF FF 00 C3 FF ",
      dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Bgr_close(&dest);
});

IMAGE(YuvSemiPlanarToRgba, [this]() {
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
  if(!ttLibC_ImageResampler_ToBgr(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "00 00 EB FF 00 00 EB FF FF 00 FF FF FF 00 FF FF "
      "00 00 EB FF 00 00 EB FF EA 00 FF FF C5 00 FF FF ",
      dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Bgr_close(&dest);
});


IMAGE(YvuSemiPlanarToBgra, [this]() {
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
  if(!ttLibC_ImageResampler_ToBgr(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "00 00 B8 FF 00 00 B8 FF FF 00 FF FF FF 00 FF FF "
      "00 00 B8 FF 00 00 B8 FF FF 00 FF FF F3 00 DD FF ",
      dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Bgr_close(&dest);
});

IMAGE(YvuSemiPlanarToRgba, [this]() {
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
  if(!ttLibC_ImageResampler_ToBgr(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "B8 00 00 FF B8 00 00 FF FF 00 FF FF FF 00 FF FF "
      "B8 00 00 FF B8 00 00 FF FF 00 FF FF DD 00 F3 FF ",
      dest);
  ttLibC_Yuv420_close(&src);
  ttLibC_Bgr_close(&dest);
});

IMAGE(BgraToBgra, [this](){
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
  if(!ttLibC_ImageResampler_ToBgr(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "10 20 30 40 11 21 31 41 12 22 32 42 13 23 33 43 "
      "14 24 34 44 15 25 35 45 16 26 36 46 17 27 37 47 ",
      dest);
  ttLibC_Bgr_close(&src);
  ttLibC_Bgr_close(&dest);
});

IMAGE(BgraToArgb, [this](){
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
  if(!ttLibC_ImageResampler_ToBgr(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "40 30 20 10 41 31 21 11 42 32 22 12 43 33 23 13 "
      "44 34 24 14 45 35 25 15 46 36 26 16 47 37 27 17 ",
      dest);
  ttLibC_Bgr_close(&src);
  ttLibC_Bgr_close(&dest);
});

IMAGE(BgraToRgba, [this](){
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
  if(!ttLibC_ImageResampler_ToBgr(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "30 20 10 40 31 21 11 41 32 22 12 42 33 23 13 43 "
      "34 24 14 44 35 25 15 45 36 26 16 46 37 27 17 47 ",
      dest);
  ttLibC_Bgr_close(&src);
  ttLibC_Bgr_close(&dest);
});

IMAGE(BgraToAbgr, [this](){
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
  if(!ttLibC_ImageResampler_ToBgr(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "40 10 20 30 41 11 21 31 42 12 22 32 43 13 23 33 "
      "44 14 24 34 45 15 25 35 46 16 26 36 47 17 27 37 ",
      dest);
  ttLibC_Bgr_close(&src);
  ttLibC_Bgr_close(&dest);
});

IMAGE(BgraToBgr, [this](){
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
  if(!ttLibC_ImageResampler_ToBgr(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "10 20 30 11 21 31 12 22 32 13 23 33 "
      "14 24 34 15 25 35 16 26 36 17 27 37 ",
      dest);
  ttLibC_Bgr_close(&src);
  ttLibC_Bgr_close(&dest);
});

IMAGE(ArgbToBgra, [this](){
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
  if(!ttLibC_ImageResampler_ToBgr(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "40 30 20 10 41 31 21 11 42 32 22 12 43 33 23 13 "
      "44 34 24 14 45 35 25 15 46 36 26 16 47 37 27 17 ",
      dest);
  ttLibC_Bgr_close(&src);
  ttLibC_Bgr_close(&dest);
});

IMAGE(RgbaToBgra, [this](){
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
  if(!ttLibC_ImageResampler_ToBgr(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "30 20 10 40 31 21 11 41 32 22 12 42 33 23 13 43 "
      "34 24 14 44 35 25 15 45 36 26 16 46 37 27 17 47 ",
      dest);
  ttLibC_Bgr_close(&src);
  ttLibC_Bgr_close(&dest);
});

IMAGE(AbgrToBgra, [this](){
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
  if(!ttLibC_ImageResampler_ToBgr(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "20 30 40 10 21 31 41 11 22 32 42 12 23 33 43 13 "
      "24 34 44 14 25 35 45 15 26 36 46 16 27 37 47 17 ",
      dest);
  ttLibC_Bgr_close(&src);
  ttLibC_Bgr_close(&dest);
});

IMAGE(BgrToBgra, [this](){
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
  if(!ttLibC_ImageResampler_ToBgr(dest, (ttLibC_Video *)src)) {
    FAIL();
  }
  binaryEq(
      "10 20 30 FF 11 21 31 FF 12 22 32 FF 13 23 33 FF "
      "14 24 34 FF 15 25 35 FF 16 26 36 FF 17 27 37 FF ",
      dest);
  ttLibC_Bgr_close(&src);
  ttLibC_Bgr_close(&dest);
});

IMAGE(YuvPlanarToYuvPlanar, [this](){
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
  if(!ttLibC_ImageResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
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

IMAGE(YuvPlanarToYvuPlanar, [this](){
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
  if(!ttLibC_ImageResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
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

IMAGE(YuvPlanarToYuvSemiPlanar, [this](){
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
  if(!ttLibC_ImageResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
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

IMAGE(YuvPlanarToYvuSemiPlanar, [this](){
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
  if(!ttLibC_ImageResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
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

IMAGE(YvuPlanarToYuvPlanar, [this](){
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
  if(!ttLibC_ImageResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
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

IMAGE(YvuPlanarToYvuPlanar, [this](){
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
  if(!ttLibC_ImageResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
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

IMAGE(YvuPlanarToYuvSemiPlanar, [this](){
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
  if(!ttLibC_ImageResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
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

IMAGE(YvuPlanarToYvuSemiPlanar, [this](){
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
  if(!ttLibC_ImageResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
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

IMAGE(YuvSemiPlanarToYuvPlanar, [this](){
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
  if(!ttLibC_ImageResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
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

IMAGE(YuvSemiPlanarToYvuPlanar, [this](){
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
  if(!ttLibC_ImageResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
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

IMAGE(YvuSemiPlanarToYuvPlanar, [this](){
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
  if(!ttLibC_ImageResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
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

IMAGE(YvuSemiPlanarToYvuPlanar, [this](){
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
  if(!ttLibC_ImageResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
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

IMAGE(BgraToYuvPlanar, [this](){
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
  if(!ttLibC_ImageResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
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

IMAGE(BgraToYvuPlanar, [this](){
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
  if(!ttLibC_ImageResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
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

IMAGE(BgraToYuvSemiPlanar, [this](){
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
  if(!ttLibC_ImageResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
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

IMAGE(BgraToYvuSemiPlanar, [this](){
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
  if(!ttLibC_ImageResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
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

IMAGE(ArgbToYuvPlanar, [this](){
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
  if(!ttLibC_ImageResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
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

IMAGE(ArgbToYvuPlanar, [this](){
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
  if(!ttLibC_ImageResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
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
IMAGE(RgbaToYuvPlanar, [this](){
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
  if(!ttLibC_ImageResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
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

IMAGE(RgbaToYvuPlanar, [this](){
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
  if(!ttLibC_ImageResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
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

IMAGE(AbgrToYuvPlanar, [this](){
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
  if(!ttLibC_ImageResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
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
IMAGE(AbgrToYvuPlanar, [this](){
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
  if(!ttLibC_ImageResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
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
IMAGE(BgrToYuvPlanar, [this](){
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
  if(!ttLibC_ImageResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
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
IMAGE(BgrToYvuPlanar, [this](){
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
  if(!ttLibC_ImageResampler_ToYuv420(dest, (ttLibC_Video *)src)) {
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
