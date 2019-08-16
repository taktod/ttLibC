/**
 * 
 */
#ifndef TTLIBC_UTIL_MSAUDIOCAPTURER_H_
#define TTLIBC_UTIL_MSAUDIOCAPTURER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../ttLibC_predef.h"
#include "../frame/audio/audio.h"

typedef struct ttLibC_Util_MsAudioCapturer {

} ttLibC_Util_MsAudioCapturer;

typedef ttLibC_Util_MsAudioCapturer ttLibC_MsAudioCapturer;

typedef bool (* ttLibC_MsAudioCapturerNameFunc)(void *ptr, const wchar_t *name);
typedef bool (* ttLibC_MsAudioCapturerFrameFunc)(void *ptr, ttLibC_Audio *Audio);

bool TT_ATTRIBUTE_API ttLibC_MsAudioCapturer_getDeviceNames(ttLibC_MsAudioCapturerNameFunc callback, void *ptr);

ttLibC_MsAudioCapturer TT_ATTRIBUTE_API *ttLibC_MsAudioCapturer_make(
  const wchar_t *target,
  uint32_t sample_rate,
  uint32_t channel_num);

bool TT_ATTRIBUTE_API ttLibC_MsAudioCapturer_requestFrame(
  ttLibC_MsAudioCapturer *capturer,
  ttLibC_MsAudioCapturerFrameFunc callback,
  void *ptr);

void TT_ATTRIBUTE_API ttLibC_MsAudioCapturer_close(ttLibC_MsAudioCapturer **capturer);

#ifdef __cplusplus
}
#endif

#endif