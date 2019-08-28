#include <iostream>
#include <functional>
#include "../frame/audio/audio.h"

namespace ttLibC {
  class AudioUnitRecorder {
  public:
    static const uint32_t DefaultInput = 0x00;
    static bool getDeviceList(std::function<bool(uint32_t deviceId, std::string name)> callback);
    AudioUnitRecorder(
        uint32_t targetId,
        uint32_t sampleRate,
        uint32_t channelNum,
        uint32_t subType,
        std::function<bool(ttLibC_Audio *audio)> callback);
    AudioUnitRecorder(
        uint32_t targetId,
        uint32_t sampleRate,
        uint32_t channelNum,
        std::function<bool(ttLibC_Audio *audio)> callback);
    AudioUnitRecorder(
        uint32_t sampleRate,
        uint32_t channelNum,
        std::function<bool(ttLibC_Audio *audio)> callback);
    bool isInitialized;
    ~AudioUnitRecorder();
  private:
    void *_instance;
  };
}
