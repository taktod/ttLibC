/**
 * @file   mmAudioLoopbackUtil.h
 * @brief  windows audio loopback with WASAPI
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2017/05/03
 */

#ifndef TTLIBC_UTIL_MMAUDIOLOOPBACKUTIL_H_
#define TTLIBC_UTIL_MMAUDIOLOOPBACKUTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <ttLibC/frame/frame.h>
#include <ttLibC/frame/audio/pcms16.h>

typedef void * ttLibC_Util_MmAudioLoopback;
typedef ttLibC_Util_MmAudioLoopback ttLibC_MmAudioLoopback;

/**
 * callback function to get device names.
 */
typedef bool (*ttLibC_MmAudioLoopbackDeviceNameFunc)(void *ptr, const char *name);

/**
 * callback function to get pcmS16Frame.
 */
typedef bool (*ttLibC_MmAudioLoopbackFrameFunc)(void *ptr, ttLibC_PcmS16 *pcm);

bool ttLibC_MmAudioLoopback_getDeviceNames(
	ttLibC_MmAudioLoopbackDeviceNameFunc callback,
	void *ptr);

ttLibC_MmAudioLoopback *ttLibC_MmAudioLoopback_make(
	const char *locale,
	const char *deviceName);

bool ttLibC_MmAudioLoopback_queryFrame(
	ttLibC_MmAudioLoopback *device,
	ttLibC_MmAudioLoopbackFrameFunc callback,
	void *ptr);

void ttLibC_MmAudioLoopback_close(ttLibC_MmAudioLoopback **device);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_UTIL_MMAUDIOLOOPBACKUTIL_H_ */

