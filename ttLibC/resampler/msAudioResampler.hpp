#ifndef TTLIBC_RESAMPLER_MSAUDIORESAMPLER_HPP_
#define TTLIBC_RESAMPLER_MSAUDIORESAMPLER_HPP_

#include <iostream>
#include <functional>

#include "../ttLibC_predef.h"
#include "../frame/audio/pcms16.h"
#include "../frame/audio/pcmf32.h"

namespace ttLibC {
  class TT_ATTRIBUTE_API MsAudioResampler {
  public:
    MsAudioResampler(
      ttLibC_Frame_Type inType,
      uint32_t inSubType,
      uint32_t inSampleRate,
      uint32_t inChannelNum,
      ttLibC_Frame_Type outType,
      uint32_t outSubType,
      uint32_t outSampleRate,
      uint32_t outChannelNum);
    bool resample(
      ttLibC_Audio *srcFrame,
      std::function<bool(ttLibC_Audio *audio)> callback);
    ~MsAudioResampler();
    bool isInitialized;
  private:
    void *_instance;
  };
}

#endif
