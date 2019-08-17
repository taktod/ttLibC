#include "../util.hpp"
#include <ttLibC/util/msVideoCapturerUtil.h>
#include <ttLibC/util/msGlobalUtil.h>
#include <ttLibC/frame/video/bgr.h>
#include <ttLibC/resampler/imageResampler.h>
#include <ttLibC/container/flv.h>
#include <ttLibC/encoder/msH264Encoder.h>

using namespace std;

#ifdef __ENABLE_WIN32__
#define MSVIDEOCAPTURER(A, B) TEST_F(UtilTest, MsVideoCapturer##A){B();}
#else
#define MSVIDEOCAPTURER(A, B) TEST_F(UtilTest, DISABLED_MsVideoCapturer##A){}
#endif

MSVIDEOCAPTURER(Listup, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  int counter = 0;
  bool result = ttLibC_MsVideoCapturer_getDeviceNames([](void *ptr, const wchar_t *name){
    char buffer[256];
    ttLibC_MsGlobal_unicodeToSjis(name, buffer, 256);
    cout << buffer << endl;
    int *counter = reinterpret_cast<int *>(ptr);
    *counter += 1;
    return true;
  }, &counter);
  ASSERT_GT(counter, 0);
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSVIDEOCAPTURER(Capture, [this](){
  // try to capture image actually.
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  wstring name(L"");
  bool result = ttLibC_MsVideoCapturer_getDeviceNames([](void *ptr, const wchar_t *name){
    wstring *str = reinterpret_cast<wstring *>(ptr);
    str->append(name);
    return false;
  }, &name);
  int counter = 0;
  auto capturer = ttLibC_MsVideoCapturer_make(name.c_str(), 320, 240);
  if(capturer != nullptr) {
    for(int i = 0;i < 5;++ i) {
      ttLibC_MsVideoCapturer_requestFrame(capturer, [](void *ptr, ttLibC_Video *video){
        int *counter = reinterpret_cast<int *>(ptr);
        *counter += 1;
        // capturer ok.
        // try to convert yuv -> bgr, and save it as bitmap.
        auto bgr = ttLibC_Bgr_makeEmptyFrame(BgrType_bgr, video->width, video->height);
        if (bgr != nullptr) {
          if (ttLibC_ImageResampler_ToBgr(bgr, video)) {
            ttLibC_MsGlobal_WriteBitmap("out.bmp", bgr);
          }
          ttLibC_Bgr_close(&bgr);
        }
        return true;
      }, &counter);
      ttLibC_MsGlobal_sleep(10);
    }
    ttLibC_MsVideoCapturer_close(&capturer);
  }
  ASSERT_GT(counter, 0);
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});

MSVIDEOCAPTURER(FlvOutputTest, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  typedef struct {
    ttLibC_FlvWriter *writer;
    ttLibC_MsH264Encoder *encoder;
    FILE *fp;
    int counter;
  } holder_t;
  holder_t holder;
  int width = 320;
  int height = 240;
  holder.writer = ttLibC_FlvWriter_make(frameType_h264, frameType_unknown);
  holder.fp = fopen("msVideoCapturer_rec.flv", "wb");
  holder.counter = 0;
  if(holder.fp) {
    {
      wstring name(L"");
      // setup encoder
      ttLibC_MsH264Encoder_listEncoders([](void *ptr, const wchar_t *name){
        wstring* str = reinterpret_cast<wstring*>(ptr);
        str->append(name);
        return false;
      }, &name);
      holder.encoder = ttLibC_MsH264Encoder_make(name.c_str(), width, height, 960000);
    }
    if(holder.encoder != nullptr) {
      // for capturer
      wstring name(L"");
      bool result = ttLibC_MsVideoCapturer_getDeviceNames([](void *ptr, const wchar_t *name){
        wstring *str = reinterpret_cast<wstring *>(ptr);
        str->append(name);
        return false;
      }, &name);
      auto capturer = ttLibC_MsVideoCapturer_make(name.c_str(), width, height);
      if(capturer != nullptr) {
        for(int i = 0;i < 200;++ i) {
          ttLibC_MsVideoCapturer_requestFrame(capturer, [](void *ptr, ttLibC_Video *video){
            holder_t *holder = reinterpret_cast<holder_t *>(ptr);
            // capturer ok.
            return ttLibC_MsH264Encoder_encode(holder->encoder, (ttLibC_Yuv420 *)video, [](void *ptr, ttLibC_H264 *h264){
              holder_t* holder = reinterpret_cast<holder_t*>(ptr);
              h264->inherit_super.inherit_super.id = 9;
              return ttLibC_FlvWriter_write(holder->writer, (ttLibC_Frame*)h264, [](void* ptr, void* data, size_t data_size) {
                holder_t* holder = reinterpret_cast<holder_t*>(ptr);
                // write into file.
                fwrite(data, 1, data_size, holder->fp);
                holder->counter++;
                return true;
              }, ptr);
            }, ptr);
          }, &holder);
          ttLibC_MsGlobal_sleep(100);
        }
        ttLibC_MsVideoCapturer_close(&capturer);
      }
      ttLibC_MsGlobal_sleep(100);
      ttLibC_MsH264Encoder_close(&holder.encoder);
    }
    fclose(holder.fp);
  }
  ttLibC_FlvWriter_close(&holder.writer);
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
  ASSERT_GT(holder.counter, 0);
});
