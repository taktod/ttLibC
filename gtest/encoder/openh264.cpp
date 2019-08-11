#include "../encoder.hpp"
#include <ttLibC/encoder/openh264Encoder.h>

using namespace std;

#ifdef __ENABLE_OPENH264__
#define OPENH264(A, B) TEST_F(EncoderTest, Openh264##A){B();}
#else
#define OPENH264(A, B) TEST_F(EncoderTest, DISABLED_Openh264##A){}
#endif

OPENH264(EncodeTest, [this](){
  auto yuv = makeGradateYuv(Yuv420Type_planar, 320, 240);
  auto encoder = ttLibC_Openh264Encoder_make(yuv->inherit_super.width, yuv->inherit_super.height);
  int counter = 0;
  for(int i = 0;i < 10;++ i) {
    ttLibC_Openh264Encoder_encode(encoder, yuv, [](void *ptr, ttLibC_H264 * h264) {
      int *counter = reinterpret_cast<int *>(ptr);
      *counter += 1;
      return true;
    }, &counter);
  }
  ttLibC_Yuv420_close(&yuv);
  ttLibC_Openh264Encoder_close(&encoder);
  // anyway get some frames -> ok.
  EXPECT_GT(counter, 7);
});

#undef OPENH264