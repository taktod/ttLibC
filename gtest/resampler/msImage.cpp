#include "../resampler.hpp"
#include <ttLibC/resampler/msImageResampler.h>
#include <ttLibC/resampler/msImageResampler.hpp>
#include <ttLibC/util/msGlobalUtil.h>

using namespace std;

#ifdef __ENABLE_WIN32__
#define MSIMAGE(A, B) TEST_F(ResamplerTest, MsImage##A){B();}
#else
#define MSIMAGE(A, B) TEST_F(ResamplerTest, DISABLED_MsImage##A){}
#endif

MSIMAGE(YuvPlanarToBgra, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    auto resampler = ttLibC_MsImageResampler_make();
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
    if(!ttLibC_MsImageResampler_ToBgr(resampler, dest, (ttLibC_Video *)src)) {
      FAIL();
    }
    binaryEq(
        "EB 00 00 FF EB 00 00 FF FF 00 FF FF FF 00 FF FF "
        "EB 00 00 FF EB 00 00 FF FF 00 EA FF FF 00 C5 FF ",
        dest);
    ttLibC_Yuv420_close(&src);
    ttLibC_Bgr_close(&dest);
    ttLibC_MsImageResampler_close(&resampler);
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSIMAGE(CxxYuvPlanarToBgra, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    // this ... maybe upside down?
    ttLibC::MsImageResampler resampler;
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
    if(!resampler.toBgr(dest, (ttLibC_Video *)src)) {
      FAIL();
    }
    binaryEq(
        "EB 00 00 FF EB 00 00 FF FF 00 FF FF FF 00 FF FF "
        "EB 00 00 FF EB 00 00 FF FF 00 EA FF FF 00 C5 FF ",
        dest);
    ttLibC_Yuv420_close(&src);
    ttLibC_Bgr_close(&dest);
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSIMAGE(CxxYuvPlanarToBgraAliened, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    ttLibC::MsImageResampler resampler;
    auto src = makeAlignedYuv(
      "00 00 80 80 "
      "00 00 40 20 "
      "FF FF "
      "80 F0 ",
      Yuv420Type_planar,
      4, 2);
    auto dest = makeAlignedBgr(
      "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "
      "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ",
      BgrType_bgra,
      4, 2);
    if(!resampler.toBgr(dest, (ttLibC_Video *)src)) {
      FAIL();
    }
    binaryEq(
        "EB 00 00 FF EB 00 00 FF FF 00 FF FF FF 00 FF FF "
        "EB 00 00 FF EB 00 00 FF FF 00 EA FF FF 00 C5 FF ",
        dest);
    ttLibC_Yuv420_close(&src);
    ttLibC_Bgr_close(&dest);
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSIMAGE(CxxYuvPlanarToBgr, [this]() {
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    ttLibC::MsImageResampler resampler;
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
    if(!resampler.toBgr(dest, (ttLibC_Video *)src)) {
      FAIL();
    }
    binaryEq(
        "EB 00 00 EB 00 00 FF 00 FF FF 00 FF "
        "EB 00 00 EB 00 00 FF 00 EA FF 00 C5 ",
        dest);
    ttLibC_Yuv420_close(&src);
    ttLibC_Bgr_close(&dest);
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSIMAGE(CxxYuvPlanarToBgrAligned, [this]() {
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    ttLibC::MsImageResampler resampler;
    auto src = makeAlignedYuv(
      "00 00 80 80 "
      "00 00 40 20 "
      "FF FF "
      "80 F0 ",
      Yuv420Type_planar,
      4, 2);
    auto dest = makeAlignedBgr(
      "00 00 00 00 00 00 00 00 00 00 00 00 "
      "00 00 00 00 00 00 00 00 00 00 00 00 ",
      BgrType_bgr,
      4, 2);
    if(!resampler.toBgr(dest, (ttLibC_Video *)src)) {
      FAIL();
    }
    binaryEq(
        "EB 00 00 EB 00 00 FF 00 FF FF 00 FF "
        "EB 00 00 EB 00 00 FF 00 EA FF 00 C5 ",
        dest);
    ttLibC_Yuv420_close(&src);
    ttLibC_Bgr_close(&dest);
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSIMAGE(CxxYuvPlanarToYuvPlanar, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    ttLibC::MsImageResampler resampler;
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
    if(!resampler.toYuv420(dest, (ttLibC_Video *)src)) {
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
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSIMAGE(CxxYuvPlanarToYuvPlanarAligned, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    ttLibC::MsImageResampler resampler;
    // original
    auto src = makeAlignedYuv(
        "10 20 30 40 "
        "11 22 33 44 "
        "A0 B0 "
        "A1 B1 ",
        Yuv420Type_planar,
        4, 2);
    // target
    auto dest = makeAlignedYuv(
        "00 00 00 00 "
        "00 00 00 00 "
        "00 00 "
        "00 00 ",
        Yuv420Type_planar,
        4, 2);
    if(!resampler.toYuv420(dest, (ttLibC_Video *)src)) {
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
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSIMAGE(CxxYuvPlanarToYuvSemiPlanar, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    ttLibC::MsImageResampler resampler;
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
    if(!resampler.toYuv420(dest, (ttLibC_Video *)src)) {
      FAIL();
    }
    binaryEq(
        "10 20 30 40 "
        "11 22 33 44 "
        "A0 A1 B0 B1 ",
        dest);
    ttLibC_Yuv420_close(&src);
    ttLibC_Yuv420_close(&dest);
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSIMAGE(CxxYuvPlanarToYuvSemiPlanarAligned, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    ttLibC::MsImageResampler resampler;
    // original
    auto src = makeAlignedYuv(
        "10 20 30 40 "
        "11 22 33 44 "
        "A0 B0 "
        "A1 B1 ",
        Yuv420Type_planar,
        4, 2);
    // target
    auto dest = makeAlignedYuv(
        "00 00 00 00 "
        "00 00 00 00 "
        "00 00 00 00 ",
        Yuv420Type_semiPlanar,
        4, 2);
    if(!resampler.toYuv420(dest, (ttLibC_Video *)src)) {
      FAIL();
    }
    binaryEq(
        "10 20 30 40 "
        "11 22 33 44 "
        "A0 A1 B0 B1 ",
        dest);
    ttLibC_Yuv420_close(&src);
    ttLibC_Yuv420_close(&dest);
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSIMAGE(CxxYuvPlanarToYvuPlanar, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    ttLibC::MsImageResampler resampler;
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
        Yvu420Type_planar,
        4, 2);
    if(!resampler.toYuv420(dest, (ttLibC_Video *)src)) {
      FAIL();
    }
    binaryEq(
        "10 20 30 40 "
        "11 22 33 44 "
        "A1 B1 A0 B0 ",
        dest);
    ttLibC_Yuv420_close(&src);
    ttLibC_Yuv420_close(&dest);
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSIMAGE(CxxYuvPlanarToYvuPlanarAligned, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    ttLibC::MsImageResampler resampler;
    // original
    auto src = makeAlignedYuv(
        "10 20 30 40 "
        "11 22 33 44 "
        "A0 B0 "
        "A1 B1 ",
        Yuv420Type_planar,
        4, 2);
    // target
    auto dest = makeAlignedYuv(
        "00 00 00 00 "
        "00 00 00 00 "
        "00 00 00 00 ",
        Yvu420Type_planar,
        4, 2);
    if(!resampler.toYuv420(dest, (ttLibC_Video *)src)) {
      FAIL();
    }
    binaryEq(
        "10 20 30 40 "
        "11 22 33 44 "
        "A1 B1 A0 B0 ",
        dest);
    ttLibC_Yuv420_close(&src);
    ttLibC_Yuv420_close(&dest);
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSIMAGE(CxxYuvSemiPlanarToBgra, [this]() {
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    ttLibC::MsImageResampler resampler;
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
    if(!resampler.toBgr(dest, (ttLibC_Video *)src)) {
      FAIL();
    }
    binaryEq(
        "EC 00 00 FF EC 00 00 FF FF 00 FF FF FF 00 FF FF "
        "EC 00 00 FF EC 00 00 FF FF 00 E8 FF FF 00 C3 FF ",
        dest);
    ttLibC_Yuv420_close(&src);
    ttLibC_Bgr_close(&dest);
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSIMAGE(CxxYuvSemiPlanarToBgraAligned, [this]() {
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    ttLibC::MsImageResampler resampler;
    auto src = makeAlignedYuv(
      "00 00 80 80 "
      "00 00 40 20 "
      "FF 80 FF F0 ",
      Yuv420Type_semiPlanar,
      4, 2);
    auto dest = makeAlignedBgr(
      "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "
      "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ",
      BgrType_bgra,
      4, 2);
    if(!resampler.toBgr(dest, (ttLibC_Video *)src)) {
      FAIL();
    }
    binaryEq(
        "EC 00 00 FF EC 00 00 FF FF 00 FF FF FF 00 FF FF "
        "EC 00 00 FF EC 00 00 FF FF 00 E8 FF FF 00 C3 FF ",
        dest);
    ttLibC_Yuv420_close(&src);
    ttLibC_Bgr_close(&dest);
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSIMAGE(CxxYuvSemiPlanarToYuvPlanar, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    ttLibC::MsImageResampler resampler;
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
    if(!resampler.toYuv420(dest, (ttLibC_Video *)src)) {
      FAIL();
    }
    binaryEq(
        "10 20 30 40 "
        "11 22 33 44 "
        "A0 A1 B0 B1 ",
        dest);
    ttLibC_Yuv420_close(&src);
    ttLibC_Yuv420_close(&dest);
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSIMAGE(CxxYuvSemiPlanarToYuvPlanarAligned, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    ttLibC::MsImageResampler resampler;
    // original
    auto src = makeAlignedYuv(
        "10 20 30 40 "
        "11 22 33 44 "
        "A0 B0 A1 B1 ",
        Yuv420Type_semiPlanar,
        4, 2);
    // target
    auto dest = makeAlignedYuv(
        "00 00 00 00 "
        "00 00 00 00 "
        "00 00 "
        "00 00 ",
        Yuv420Type_planar,
        4, 2);
    if(!resampler.toYuv420(dest, (ttLibC_Video *)src)) {
      FAIL();
    }
    binaryEq(
        "10 20 30 40 "
        "11 22 33 44 "
        "A0 A1 B0 B1 ",
        dest);
    ttLibC_Yuv420_close(&src);
    ttLibC_Yuv420_close(&dest);
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSIMAGE(CxxYuvSemiPlanarToYvuPlanar, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    ttLibC::MsImageResampler resampler;
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
    if(!resampler.toYuv420(dest, (ttLibC_Video *)src)) {
      FAIL();
    }
    binaryEq(
        "10 20 30 40 "
        "11 22 33 44 "
        "B0 B1 A0 A1 ",
        dest);
    ttLibC_Yuv420_close(&src);
    ttLibC_Yuv420_close(&dest);
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSIMAGE(CxxYuvSemiPlanarToYvuPlanarAligned, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    ttLibC::MsImageResampler resampler;
    // original
    auto src = makeAlignedYuv(
        "10 20 30 40 "
        "11 22 33 44 "
        "A0 B0 A1 B1 ",
        Yuv420Type_semiPlanar,
        4, 2);
    // target
    auto dest = makeAlignedYuv(
        "00 00 00 00 "
        "00 00 00 00 "
        "00 00 "
        "00 00 ",
        Yvu420Type_planar,
        4, 2);
    if(!resampler.toYuv420(dest, (ttLibC_Video *)src)) {
      FAIL();
    }
    binaryEq(
        "10 20 30 40 "
        "11 22 33 44 "
        "B0 B1 A0 A1 ",
        dest);
    ttLibC_Yuv420_close(&src);
    ttLibC_Yuv420_close(&dest);
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSIMAGE(CxxYvuPlanarToYuvPlanar, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    ttLibC::MsImageResampler resampler;
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
    if(!resampler.toYuv420(dest, (ttLibC_Video *)src)) {
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
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSIMAGE(CxxYvuPlanarToYuvPlanarAligned, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    ttLibC::MsImageResampler resampler;
    // original
    auto src = makeAlignedYuv(
        "10 20 30 40 "
        "11 22 33 44 "
        "A0 B0 "
        "A1 B1 ",
        Yvu420Type_planar,
        4, 2);
    // target
    auto dest = makeAlignedYuv(
        "00 00 00 00 "
        "00 00 00 00 "
        "00 00 "
        "00 00 ",
        Yuv420Type_planar,
        4, 2);
    if(!resampler.toYuv420(dest, (ttLibC_Video *)src)) {
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
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSIMAGE(CxxYvuPlanarToYvuPlanar, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    ttLibC::MsImageResampler resampler;
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
    if(!resampler.toYuv420(dest, (ttLibC_Video *)src)) {
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
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSIMAGE(CxxYvuPlanarToYvuPlanarAligned, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    ttLibC::MsImageResampler resampler;
    // original
    auto src = makeAlignedYuv(
        "10 20 30 40 "
        "11 22 33 44 "
        "A0 B0 "
        "A1 B1 ",
        Yvu420Type_planar,
        4, 2);
    // target
    auto dest = makeAlignedYuv(
        "00 00 00 00 "
        "00 00 00 00 "
        "00 00 "
        "00 00 ",
        Yvu420Type_planar,
        4, 2);
    if(!resampler.toYuv420(dest, (ttLibC_Video *)src)) {
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
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSIMAGE(CxxYvuPlanarToYuvSemiPlanar, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    ttLibC::MsImageResampler resampler;
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
    if(!resampler.toYuv420(dest, (ttLibC_Video *)src)) {
      FAIL();
    }
    binaryEq(
        "10 20 30 40 "
        "11 22 33 44 "
        "A1 A0 B1 B0 ",
        dest);
    ttLibC_Yuv420_close(&src);
    ttLibC_Yuv420_close(&dest);
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSIMAGE(CxxYvuPlanarToYuvSemiPlanarAligned, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    ttLibC::MsImageResampler resampler;
    // original
    auto src = makeAlignedYuv(
        "10 20 30 40 "
        "11 22 33 44 "
        "A0 B0 "
        "A1 B1 ",
        Yvu420Type_planar,
        4, 2);
    // target
    auto dest = makeAlignedYuv(
        "00 00 00 00 "
        "00 00 00 00 "
        "00 00 00 00 ",
        Yuv420Type_semiPlanar,
        4, 2);
    if(!resampler.toYuv420(dest, (ttLibC_Video *)src)) {
      FAIL();
    }
    binaryEq(
        "10 20 30 40 "
        "11 22 33 44 "
        "A1 A0 B1 B0 ",
        dest);
    ttLibC_Yuv420_close(&src);
    ttLibC_Yuv420_close(&dest);
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSIMAGE(CxxBgraToBgra, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    ttLibC::MsImageResampler resampler;
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
    if(!resampler.toBgr(dest, (ttLibC_Video *)src)) {
      FAIL();
    }
    binaryEq(
        "10 20 30 40 11 21 31 41 12 22 32 42 13 23 33 43 "
        "14 24 34 44 15 25 35 45 16 26 36 46 17 27 37 47 ",
        dest);
    ttLibC_Bgr_close(&src);
    ttLibC_Bgr_close(&dest);
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSIMAGE(CxxBgraToBgraAligned, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    ttLibC::MsImageResampler resampler;
    // original
    auto src = makeAlignedBgr(
        "10 20 30 40 11 21 31 41 12 22 32 42 13 23 33 43 "
        "14 24 34 44 15 25 35 45 16 26 36 46 17 27 37 47 ",
        BgrType_bgra,
        4, 2);
    // target
    auto dest = makeAlignedBgr(
        "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "
        "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ",
        BgrType_bgra,
        4, 2);
    if(!resampler.toBgr(dest, (ttLibC_Video *)src)) {
      FAIL();
    }
    binaryEq(
        "10 20 30 40 11 21 31 41 12 22 32 42 13 23 33 43 "
        "14 24 34 44 15 25 35 45 16 26 36 46 17 27 37 47 ",
        dest);
    ttLibC_Bgr_close(&src);
    ttLibC_Bgr_close(&dest);
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSIMAGE(CxxBgraToBgr, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    ttLibC::MsImageResampler resampler;
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
    if(!resampler.toBgr(dest, (ttLibC_Video *)src)) {
      FAIL();
    }
    binaryEq(
        "10 20 30 11 21 31 12 22 32 13 23 33 "
        "14 24 34 15 25 35 16 26 36 17 27 37 ",
        dest);
    ttLibC_Bgr_close(&src);
    ttLibC_Bgr_close(&dest);
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSIMAGE(CxxBgraToBgrAligned, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    ttLibC::MsImageResampler resampler;
    // original
    auto src = makeAlignedBgr(
        "10 20 30 40 11 21 31 41 12 22 32 42 13 23 33 43 "
        "14 24 34 44 15 25 35 45 16 26 36 46 17 27 37 47 ",
        BgrType_bgra,
        4, 2);
    // target
    auto dest = makeAlignedBgr(
        "00 00 00 00 00 00 00 00 00 00 00 00 "
        "00 00 00 00 00 00 00 00 00 00 00 00 ",
        BgrType_bgr,
        4, 2);
    if(!resampler.toBgr(dest, (ttLibC_Video *)src)) {
      FAIL();
    }
    binaryEq(
        "10 20 30 11 21 31 12 22 32 13 23 33 "
        "14 24 34 15 25 35 16 26 36 17 27 37 ",
        dest);
    ttLibC_Bgr_close(&src);
    ttLibC_Bgr_close(&dest);
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSIMAGE(CxxBgraToYuvPlanar, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    ttLibC::MsImageResampler resampler;
    // original
    // upside down??
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
    if(!resampler.toYuv420(dest, (ttLibC_Video *)src)) {
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
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSIMAGE(CxxBgraToYuvPlanarAligned, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    ttLibC::MsImageResampler resampler;
    // original
    auto src = makeAlignedBgr(
        "10 20 30 40 11 21 31 41 12 22 32 42 13 23 33 43 "
        "14 24 34 44 15 25 35 45 16 26 36 46 17 27 37 47 ",
        BgrType_bgra,
        4, 2);
    // target
    auto dest = makeAlignedYuv(
        "00 00 00 00 "
        "00 00 00 00 "
        "00 00 "
        "00 00 ",
        Yuv420Type_planar,
        4, 2);
    if(!resampler.toYuv420(dest, (ttLibC_Video *)src)) {
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
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSIMAGE(CxxBgraToYvuPlanar, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    ttLibC::MsImageResampler resampler;
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
    if(!resampler.toYuv420(dest, (ttLibC_Video *)src)) {
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
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSIMAGE(CxxBgraToYvuPlanarAligned, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    ttLibC::MsImageResampler resampler;
    // original
    auto src = makeAlignedBgr(
        "10 20 30 40 11 21 31 41 12 22 32 42 13 23 33 43 "
        "14 24 34 44 15 25 35 45 16 26 36 46 17 27 37 47 ",
        BgrType_bgra,
        4, 2);
    // target
    auto dest = makeAlignedYuv(
        "00 00 00 00 "
        "00 00 00 00 "
        "00 00 "
        "00 00 ",
        Yvu420Type_planar,
        4, 2);
    if(!resampler.toYuv420(dest, (ttLibC_Video *)src)) {
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
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSIMAGE(CxxBgraToYuvSemiPlanar, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    ttLibC::MsImageResampler resampler;
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
    if(!resampler.toYuv420(dest, (ttLibC_Video *)src)) {
      FAIL();
    }
    binaryEq(
        "2E 2F 2F 30 "
        "31 32 33 34 "
        "76 88 76 88 ",
        dest);
    ttLibC_Bgr_close(&src);
    ttLibC_Yuv420_close(&dest);
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSIMAGE(CxxBgraToYuvSemiPlanarAligned, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    ttLibC::MsImageResampler resampler;
    // original
    auto src = makeAlignedBgr(
        "10 20 30 40 11 21 31 41 12 22 32 42 13 23 33 43 "
        "14 24 34 44 15 25 35 45 16 26 36 46 17 27 37 47 ",
        BgrType_bgra,
        4, 2);
    // target
    auto dest = makeAlignedYuv(
        "00 00 00 00 "
        "00 00 00 00 "
        "00 00 00 00 ",
        Yuv420Type_semiPlanar,
        4, 2);
    if(!resampler.toYuv420(dest, (ttLibC_Video *)src)) {
      FAIL();
    }
    binaryEq(
        "2E 2F 2F 30 "
        "31 32 33 34 "
        "76 88 76 88 ",
        dest);
    ttLibC_Bgr_close(&src);
    ttLibC_Yuv420_close(&dest);
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSIMAGE(CxxBgrToBgra, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    ttLibC::MsImageResampler resampler;
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
    if(!resampler.toBgr(dest, (ttLibC_Video *)src)) {
      FAIL();
    }
    binaryEq(
        "10 20 30 FF 11 21 31 FF 12 22 32 FF 13 23 33 FF "
        "14 24 34 FF 15 25 35 FF 16 26 36 FF 17 27 37 FF ",
        dest);
    ttLibC_Bgr_close(&src);
    ttLibC_Bgr_close(&dest);
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSIMAGE(CxxBgrToBgraAligned, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    ttLibC::MsImageResampler resampler;
    // original
    auto src = makeAlignedBgr(
        "10 20 30 11 21 31 12 22 32 13 23 33 "
        "14 24 34 15 25 35 16 26 36 17 27 37 ",
        BgrType_bgr,
        4, 2);
    // target
    auto dest = makeAlignedBgr(
        "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 "
        "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ",
        BgrType_bgra,
        4, 2);
    if(!resampler.toBgr(dest, (ttLibC_Video *)src)) {
      FAIL();
    }
    binaryEq(
        "10 20 30 FF 11 21 31 FF 12 22 32 FF 13 23 33 FF "
        "14 24 34 FF 15 25 35 FF 16 26 36 FF 17 27 37 FF ",
        dest);
    ttLibC_Bgr_close(&src);
    ttLibC_Bgr_close(&dest);
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSIMAGE(CxxBgrToYuvPlanar, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    ttLibC::MsImageResampler resampler;
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
    if(!resampler.toYuv420(dest, (ttLibC_Video *)src)) {
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
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSIMAGE(CxxBgrToYuvPlanarAligned, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    ttLibC::MsImageResampler resampler;
    // original
    auto src = makeAlignedBgr(
        "10 20 30 11 21 31 12 22 32 13 23 33 "
        "14 24 34 15 25 35 16 26 36 17 27 37 ",
        BgrType_bgr,
        4, 2);
    // target
    auto dest = makeAlignedYuv(
        "00 00 00 00 "
        "00 00 00 00 "
        "00 00 "
        "00 00 ",
        Yuv420Type_planar,
        4, 2);
    if(!resampler.toYuv420(dest, (ttLibC_Video *)src)) {
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
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSIMAGE(CxxBgrToYvuPlanar, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    ttLibC::MsImageResampler resampler;
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
    if(!resampler.toYuv420(dest, (ttLibC_Video *)src)) {
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
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSIMAGE(CxxBgrToYvuPlanarAligned, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  {
    ttLibC::MsImageResampler resampler;
    // original
    auto src = makeAlignedBgr(
        "10 20 30 11 21 31 12 22 32 13 23 33 "
        "14 24 34 15 25 35 16 26 36 17 27 37 ",
        BgrType_bgr,
        4, 2);
    // target
    auto dest = makeAlignedYuv(
        "00 00 00 00 "
        "00 00 00 00 "
        "00 00 "
        "00 00 ",
        Yvu420Type_planar,
        4, 2);
    if(!resampler.toYuv420(dest, (ttLibC_Video *)src)) {
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
  }
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

#undef MSIMAGE