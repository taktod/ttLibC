#include "../util.hpp"
#include <ttLibC/util/audioUnitRecorderUtil.h>
#include <ttLibC/util/audioUnitRecorderUtil.hpp>

using namespace std;

#ifdef __ENABLE_APPLE___
#define AUDIOUNITRECORDER(A, B) TEST_F(UtilTest, AudioUnitRecorder##A){B();}
#else
#define AUDIOUNITRECORDER(A, B) TEST_F(UtilTest, DISABLED_AudioUnitRecorder##A){}
#endif

AUDIOUNITRECORDER(Listup, [this](){
  int counter = 0;
  bool result = ttLibC_AudioUnitRecorder_getDeviceList([](void *ptr, uint32_t deviceId, const char *name){
    int *counter = reinterpret_cast<int *>(ptr);
    (*counter) ++;
    cout << deviceId << ":" << name << endl;
    return true;
  }, &counter);
  ASSERT_GT(counter, 0);
});

AUDIOUNITRECORDER(CxxListup, [this](){
  int counter = 0;
  bool result = ttLibC::AudioUnitRecorder::getDeviceList([&](uint32_t id, string name) {
    counter ++;
    cout << id << ":" << name << endl;
    return true;
  });
  ASSERT_GT(counter, 0);
});

AUDIOUNITRECORDER(CxxCapture, [this](){
  int counter = 0;
  ttLibC::AudioUnitRecorder *recorder = new ttLibC::AudioUnitRecorder(44100, 2, [&](ttLibC_Audio *audio) {
    counter ++;
    return true;
  });
  sleep(1);
  delete recorder;
  ASSERT_GT(counter, 0);
});

AUDIOUNITRECORDER(Capture, [this](){
  int counter = 0;
  auto recorder = ttLibC_AudioUnitRecorder_make(
      44100, 2, [](void *ptr, ttLibC_Audio *audio) {
        int *counter = reinterpret_cast<int *>(ptr);
        (*counter) ++;
        return true;
      },
      &counter);
  sleep(1);
  ttLibC_AudioUnitRecorder_close(&recorder);
  ASSERT_GT(counter, 0);
});
