#include "../util.hpp"
#include <ttLibC/util/msGlobalUtil.h>
#include <ttLibC/util/mmAudioLoopbackUtil.h>

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
  ttLibC_MmAudioLoopback_getDeviceNames([](void *ptr, const char *name){
    cout << name << endl;
    int *counter = reinterpret_cast<int *>(ptr);
    *counter += 1;
    return true;
  }, &counter);
  EXPECT_GT(counter, 0);

  ttLibC_MsGlobal_MFShutdown();
  ttLibC_MsGlobal_CoUninitialize();
});