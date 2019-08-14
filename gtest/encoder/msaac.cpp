#include "../encoder.hpp"
#include <ttLibC/encoder/msAacEncoder.h>
#include <ttLibC/util/msGlobalUtil.h>
#include <ttLibC/util/beepUtil.h>

using namespace std;

#ifdef __ENABLE_WIN32__
#define MSAAC(A, B) TEST_F(EncoderTest, MsAac##A){B();}
#else
#define MSAAC(A, B) TEST_F(EncoderTest, DISABLED_MsAac##A){}
#endif

MSAAC(EncodeTest, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();

  auto generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian, 440, 44100, 1);
  ttLibC_PcmS16 *pcm = nullptr;
  auto encoder = ttLibC_MsAacEncoder_make(44100, 1, 96000);
  int counter = 0;
  for(int i = 0;i < 5;++ i) {
    pcm = ttLibC_BeepGenerator_makeBeepBySampleNum(generator, pcm, 1000);
    ttLibC_MsAacEncoder_encode(encoder, pcm, [](void *ptr, ttLibC_Aac *aac) {
      int *counter = reinterpret_cast<int *>(ptr);
      *counter += 1;
      return true;
    }, &counter);
  }
  ttLibC_PcmS16_close(&pcm);
  ttLibC_MsAacEncoder_close(&encoder);
  ttLibC_BeepGenerator_close(&generator);

  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
  EXPECT_GT(counter, 2);
});

#undef MSAAC
