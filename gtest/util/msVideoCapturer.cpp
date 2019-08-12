#include "../util.hpp"
#include <ttLibC/util/msVideoCapturerUtil.h>
#include <ttLibC/util/msGlobalUtil.h>

using namespace std;

#ifdef __ENABLE_WIN32__
#define MSVIDEOCAPTURER(A, B) TEST_F(UtilTest, MsVideoCapturer##A){B();}
#else
#define MSVIDEOCAPTURER(A, B) TEST_F(UtilTest, DISABLED_MsVideoCapturer##A){}
#endif

MSVIDEOCAPTURER(Listup, [this](){
  int counter = 0;
  bool result = ttLibC_MsVideoCapturer_getDeviceNames([](void *ptr, const char *name){
    int *counter = reinterpret_cast<int *>(ptr);
    *counter += 1;
    return true;
  }, &counter);
  ASSERT_GT(counter, 0);
});

MSVIDEOCAPTURER(Capture, [this](){
  // try to capture image actually.
  ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal);
  ttLibC_MsGlobal_MFStartup();
  string name("");
  bool result = ttLibC_MsVideoCapturer_getDeviceNames([](void *ptr, const char *name){
    string *str = reinterpret_cast<string *>(ptr);
    str->append(name);
    return false;
  }, &name);
  cout << name << endl;
  auto capturer = ttLibC_MsVideoCapturer_make(name.c_str(), 320, 240);
  if(capturer != nullptr) {
    ttLibC_MsVideoCapturer_requestFrame(capturer, [](void *ptr, ttLibC_Video *video){
    cout << "capturer image" << endl;
    return true;
    }, nullptr);
  ttLibC_MsVideoCapturer_close(&capturer);
  ttLibC_MsGlobal_MFShutdown();
	ttLibC_MsGlobal_CoUninitialize();
  }
});
