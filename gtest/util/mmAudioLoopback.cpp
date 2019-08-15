#include "../util.hpp"
#include <ttLibC/util/msGlobalUtil.h>
#include <ttLibC/util/mmAudioLoopbackUtil.h>
#include <ttLibC/util/hexUtil.h>

using namespace std;

#ifdef __ENABLE_WIN32__
#define MMAUDIOLOOPBACK(A, B) TEST_F(UtilTest, MmAudioLoopback##A){B();}
#else
#define MMAUDIOLOOPBACK(A, B) TEST_F(UtilTest, DISABLED_MmAudioLoopback##A){}
#endif

MMAUDIOLOOPBACK(Listup, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();

  int counter = 0;
  ttLibC_MmAudioLoopback_getDeviceNames([](void *ptr, const wchar_t *name){
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

/*
MMAUDIOLOOPBACK(Capture, [this](){
  // try to capture audio actually.
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();

  int counter = 0;

  auto loopback = ttLibC_MmAudioLoopback_make("", L"");
  for(int i = 0;i < 10;++ i) {
    ttLibC_MsGlobal_sleep(10);
    ttLibC_MmAudioLoopback_queryFrame(loopback, [](void *ptr, ttLibC_PcmS16 *pcm) {
      int *counter = reinterpret_cast<int *>(ptr);
      *counter += 1;
      cout << "pcm found." << endl;
      return true;
    }, &counter);
  }

  EXPECT_GT(counter, 0);

  ttLibC_MmAudioLoopback_close(&loopback);
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});
*/
