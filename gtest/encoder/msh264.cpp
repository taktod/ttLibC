#include "../encoder.hpp"
#include <ttLibC/encoder/msH264Encoder.h>
#include <ttLibC/util/msGlobalUtil.h>
#include <ttLibC/container/flv.h>

using namespace std;

#ifdef __ENABLE_WIN32__
#define MSH264(A, B) TEST_F(EncoderTest, MsH264##A){B();}
#else
#define MSH264(A, B) TEST_F(EncoderTest, DISABLED_MsH264##A){}
#endif

MSH264(Listup, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  int counter = 0;
  ttLibC_MsH264Encoder_listEncoders([](void *ptr, const wchar_t *name){
    char buffer[256];
    ttLibC_MsGlobal_unicodeToSjis(name, buffer, 256);
    cout << buffer << endl;
    int *counter = reinterpret_cast<int *>(ptr);
    *counter += 1;
    return true;
  }, &counter);
  EXPECT_GT(counter, 0);
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSH264(EncodeTest, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  auto yuv = makeGradateYuv(Yuv420Type_semiPlanar, 320, 240);
  int counter = 0;
  wstring name(L"");
  ttLibC_MsH264Encoder_listEncoders([](void *ptr, const wchar_t *name){
    wstring* str = reinterpret_cast<wstring*>(ptr);
    str->append(name);
    return false;
  }, &name);
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
  ttLibC_MsGlobal_sleep(100);
  ttLibC_MsH264Encoder_close(&encoder);
  ttLibC_Yuv420_close(&yuv);
  // anyway get some frames -> ok.
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
  EXPECT_GT(counter, 7);
});

MSH264(FlvOutputTest, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  auto yuv = makeGradateYuv(Yuv420Type_semiPlanar, 320, 240);
  typedef struct {
    ttLibC_FlvWriter *writer;
    FILE *fp;
    int counter;
    uint64_t pts;
  } holder_t;
  holder_t holder;
  holder.counter = 0;
  holder.fp = fopen("msh264_rec.flv", "wb");
  holder.writer = ttLibC_FlvWriter_make(frameType_h264, frameType_unknown);
  if(holder.fp) {
    wstring name(L"");
    ttLibC_MsH264Encoder_listEncoders([](void *ptr, const wchar_t *name){
      wstring* str = reinterpret_cast<wstring*>(ptr);
      str->append(name);
      return false;
    }, &name);
    auto encoder = ttLibC_MsH264Encoder_make(name.c_str(), yuv->inherit_super.width, yuv->inherit_super.height, 960000);
    if (encoder == nullptr) {
      cout << "encoder is null" << endl;
    }
    for(int i = 0;i < 10;++ i) {
      yuv->inherit_super.inherit_super.pts = i * 100;
      ttLibC_MsH264Encoder_encode(encoder, yuv, [](void *ptr, ttLibC_H264 * h264) {
        holder_t *holder = reinterpret_cast<holder_t *>(ptr);
        h264->inherit_super.inherit_super.id = 9;
        h264->inherit_super.inherit_super.pts = holder->pts;
        holder->pts += 100;
        h264->inherit_super.inherit_super.timebase = 1000;
        
        return ttLibC_FlvWriter_write(holder->writer, (ttLibC_Frame *)h264, [](void *ptr, void *data, size_t data_size) {
          holder_t *holder = reinterpret_cast<holder_t *>(ptr);
          holder->counter ++;
          fwrite(data, 1, data_size, holder->fp);
          return true;
        }, ptr);
      }, &holder);
    }
    ttLibC_MsGlobal_sleep(100);
    fclose(holder.fp);
    ttLibC_MsH264Encoder_close(&encoder);
  }
  ttLibC_FlvWriter_close(&holder.writer);
  ttLibC_Yuv420_close(&yuv);

  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});
