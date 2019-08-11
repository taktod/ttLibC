#include "../encoder.hpp"
#include <ttLibC/encoder/msH264Encoder.h>
#include <ttLibC/util/msGlobalUtil.h>

using namespace std;

#ifdef __ENABLE_WIN32__
#define MSH264(A, B) TEST_F(EncoderTest, MsH264##A){B();}
#else
#define MSH264(A, B) TEST_F(EncoderTest, DISABLED_MsH264##A){}
#endif

MSH264(EncodeTest, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  auto yuv = makeGradateYuv(Yuv420Type_semiPlanar, 320, 240);
  int counter = 0;
  string name("");
  ttLibC_MsH264Encoder_listEncoders([](void *ptr, const char *name){
      string *str = reinterpret_cast<string *>(ptr);
      str->append(name);
      return false;
  }, &name);
  cout << name << endl;
  auto encoder = ttLibC_MsH264Encoder_make(name.c_str(), yuv->inherit_super.width, yuv->inherit_super.height, 960000);
  if (encoder == nullptr) {
	  cout << "encoder is null" << endl;
  }
  for(int i = 0;i < 10;++ i) {
	  yuv->inherit_super.inherit_super.pts = i * 100;
    ttLibC_MsH264Encoder_encode(encoder, yuv, [](void *ptr, ttLibC_H264 * h264) {
      int *counter = reinterpret_cast<int *>(ptr);
      *counter += 1;
      return true;
    }, &counter);
  }
  Sleep(10);
  ttLibC_MsH264Encoder_close(&encoder);
  ttLibC_Yuv420_close(&yuv);
  // anyway get some frames -> ok.
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
  EXPECT_GT(counter, 7);
});