/**
 * 
 */
#ifndef TTLIBC_UTIL_MSVIDEOCAPTURER_H_
#define TTLIBC_UTIL_MSVIDEOCAPTURER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../ttLibC_predef.h"
#include "../frame/video/video.h"

typedef struct ttLibC_Util_MsVideoCapturer{
} ttLibC_Util_MsVideoCapturer;

typedef ttLibC_Util_MsVideoCapturer ttLibC_MsVideoCapturer;

typedef bool (* ttLibC_MsVideoCapturerNameFunc)(void *ptr, const char *name);
typedef bool (* ttLibC_MsVideoCapturerFrameFunc)(void *ptr, ttLibC_Video *video);

bool TT_ATTRIBUTE_API ttLibC_MsVideoCapturer_getDeviceNames(ttLibC_MsVideoCapturerNameFunc callback, void *ptr);

ttLibC_MsVideoCapturer TT_ATTRIBUTE_API *ttLibC_MsVideoCapturer_make(
  const char *target,
  uint32_t width,
  uint32_t height);

bool TT_ATTRIBUTE_API ttLibC_MsVideoCapturer_requestFrame(
  ttLibC_MsVideoCapturer *capturer,
  ttLibC_MsVideoCapturerFrameFunc callback,
  void *ptr);

void TT_ATTRIBUTE_API ttLibC_MsVideoCapturer_close(ttLibC_MsVideoCapturer **capturer);

#ifdef __cplusplus
}
#endif

#endif
