#include "../encoder.hpp"
#include <ttLibC/util/beepUtil.h>
#include <ttLibC/encoder/fdkaacEncoder.h>

using namespace std;

#ifdef __ENABLE_FDKAAC__
#define FDKAAC(A, B) TEST_F(EncoderTest, Fdkaac##A){B();}
#else
#define FDKAAC(A, B) TEST_F(EncoderTest, DISABLED_Fdkaac##A){}
#endif

FDKAAC(EncodeTest, [this](){
  auto generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian, 440, 44100, 1);
  ttLibC_PcmS16 *pcm = nullptr;
  auto encoder = ttLibC_FdkaacEncoder_make("AOT_AAC_LC", 44100, 1, 96000);
  int counter = 0;
  for(int i = 0;i < 5;++ i) {
    pcm = ttLibC_BeepGenerator_makeBeepBySampleNum(generator, pcm, 1000);
    ttLibC_FdkaacEncoder_encode(encoder, pcm, [](void *ptr, ttLibC_Aac *aac) {
      int *counter = reinterpret_cast<int *>(ptr);
      *counter += 1;
      return true;
    }, &counter);
  }
  ttLibC_PcmS16_close(&pcm);
  ttLibC_FdkaacEncoder_close(&encoder);
  ttLibC_BeepGenerator_close(&generator);
  EXPECT_GT(counter, 2);
});

#undef Fdkaac
