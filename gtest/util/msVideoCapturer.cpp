#include "../util.hpp"
#include <ttLibC/util/msVideoCapturerUtil.h>
#include <ttLibC/util/msGlobalUtil.h>
#include <ttLibC/frame/video/bgr.h>
#include <ttLibC/resampler/imageResampler.h>

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
