#include "../util.hpp"
#include <ttLibC/util/msAudioCapturerUtil.h>
#include <ttLibC/util/msGlobalUtil.h>

using namespace std;

#ifdef __ENABLE_WIN32__
#define MSAUDIOCAPTURER(A, B) TEST_F(UtilTest, MsAudioCapturer##A){B();}
#else
#define MSAUDIOCAPTURER(A, B) TEST_F(UtilTest, DISABLED_MsAudioCapturer##A){}
#endif

MSAUDIOCAPTURER(Listup, [this](){
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  int counter = 0;
  bool result = ttLibC_MsAudioCapturer_getDeviceNames([](void *ptr, const wchar_t *name){
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

MSAUDIOCAPTURER(Capture, [this]() {
  // try to capture audio actually.
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  wstring name(L"");
  bool result = ttLibC_MsAudioCapturer_getDeviceNames([](void* ptr, const wchar_t* name) {
    wstring* str = reinterpret_cast<wstring*>(ptr);
    str->append(name);
    return false;
    }, &name);
  int counter = 0;
  auto capturer = ttLibC_MsAudioCapturer_make(name.c_str(), 48000, 2, [](void *ptr, ttLibC_Audio* audio) {
    int *counter = reinterpret_cast<int *>(ptr);
    *counter += 1;
    return true;
  }, &counter);
  if(capturer != nullptr) {
    ttLibC_MsGlobal_sleep(500);
    ttLibC_MsAudioCapturer_close(&capturer);
  }
  ASSERT_GT(counter, 0);
  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});
