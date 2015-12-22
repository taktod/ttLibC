/**
 * @file   openalUtil.h
 * @brief  openal library support.
 *
 * now work with pcms16 only. it is also possible to work with pcms8.
 *
 * this code is under GPLv3 license.
 *
 * @author taktod
 * @date   2015/07/20
 */

#ifndef TTLIBC_UTIL_OPENALUTIL_H_
#define TTLIBC_UTIL_OPENALUTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../frame/audio/pcms16.h"

/**
 * data for openal play device.
 */
typedef struct ttLibC_Util_OpenalUtil_AlDevice {
	/** holding buffer number. */
	uint32_t buffer_num;
} ttLibC_Util_OpenalUtil_AlDevice;

typedef ttLibC_Util_OpenalUtil_AlDevice ttLibC_AlDevice;

/**
 * make openal play device.
 * @param buffer_num number for queue buffers.
 */
ttLibC_AlDevice *ttLibC_AlDevice_make(uint32_t buffer_num);

/**
 * queue pcm data.
 * @param device openalDevice object.
 * @param pcms16 pcms16 object.
 */
bool ttLibC_AlDevice_queue(ttLibC_AlDevice *device, ttLibC_PcmS16 *pcms16);

/**
 * ref the queue count.
 * @param device openalDevice object.
 */
uint32_t ttLibC_AlDevice_getQueueCount(ttLibC_AlDevice *device);

/**
 * start playing and wait for certain duration.
 * @param device
 * @param milisec wait interval in mili sec, if -1, wait until use all buffer.
 */
void ttLibC_AlDevice_proceed(ttLibC_AlDevice *device, int32_t milisec);

/**
 * close the device.
 * @param device.
 */
void ttLibC_AlDevice_close(ttLibC_AlDevice **device);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_UTIL_OPENALUTIL_H_ */
