/**
 *
 */

#ifndef TTLIBC_UTIL_AUDIOUNITRECORDER_H_
#define TTLIBC_UTIL_AUDIOUNITRECORDER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../ttLibC_predef.h"
#include "../frame/audio/audio.h"

#define ttLibC_AudioUnitRecorder_DefaultInput 0x00

typedef void *ttLibC_Util_AudioUnitRecorder;

typedef ttLibC_Util_AudioUnitRecorder ttLibC_AudioUnitRecorder;

typedef bool (* ttLibC_AudioUnitRecorderNameFunc)(void *ptr, uint32_t deviceId, const char *name);
typedef bool (* ttLibC_AudioUnitRecorderFrameFunc)(void *ptr, ttLibC_Audio *Audio);

bool TT_ATTRIBUTE_API ttLibC_AudioUnitRecorder_getDeviceList(ttLibC_AudioUnitRecorderNameFunc callback, void *ptr);

ttLibC_AudioUnitRecorder TT_ATTRIBUTE_API *ttLibC_AudioUnitRecorder_make(
  uint32_t sample_rate,
  uint32_t channel_num,
  ttLibC_AudioUnitRecorderFrameFunc callback,
  void *ptr);

ttLibC_AudioUnitRecorder TT_ATTRIBUTE_API *ttLibC_AudioUnitRecorder_makeWithTarget(
  uint32_t sample_rate,
  uint32_t channel_num,
  uint32_t target_id,
  ttLibC_AudioUnitRecorderFrameFunc callback,
  void *ptr);

ttLibC_AudioUnitRecorder TT_ATTRIBUTE_API *ttLibC_AudioUnitRecorder_makeWithTargetType(
  uint32_t sample_rate,
  uint32_t channel_num,
  uint32_t target_id,
  uint32_t type,
  ttLibC_AudioUnitRecorderFrameFunc callback,
  void *ptr);

void TT_ATTRIBUTE_API ttLibC_AudioUnitRecorder_close(ttLibC_AudioUnitRecorder **capturer);

#ifdef __cplusplus
}
#endif

#endif
