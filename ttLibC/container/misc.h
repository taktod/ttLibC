/**
 * @file   misc.h
 * @brief  misc for container.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2015/08/22
 */

#ifndef TTLIBC_CONTAINER_MISC_H_
#define TTLIBC_CONTAINER_MISC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../frame/frame.h"

/**
 * definition of frame queue.
 */
typedef struct ttLibC_Container_Misc_FrameQueue {
	uint32_t track_id;
	uint64_t pts;
	uint32_t timebase;
} ttLibC_Container_Misc_FrameQueue;

typedef ttLibC_Container_Misc_FrameQueue ttLibC_FrameQueue;

/**
 * callback function for frame queue dequeue or ref.
 * @param ptr   user def value pointer.
 * @param frame frame object.
 */
typedef bool (* ttLibC_FrameQueueFunc)(void *ptr, ttLibC_Frame *frame);

/**
 * make frame queue
 * @param track_id track_id for container.
 * @param max_size max number of holding frame object.
 * @return frame queue object.
 */
ttLibC_FrameQueue *ttLibC_FrameQueue_make(
		uint32_t track_id,
		uint32_t max_size);

/**
 * reference of value in queue.
 * call func in order.
 * @param queue    target queue object.
 * @param callback callback func. if return false, this task is ended.
 * @param ptr      user def pointer.
 * @return true:call all data. false:stopped.
 */
bool ttLibC_FrameQueue_ref(
		ttLibC_FrameQueue *queue,
		ttLibC_FrameQueueFunc callback,
		void *ptr);

/**
 * ref the first object only.
 * @param queue target queue object.
 * @return frame object.
 */
ttLibC_Frame *ttLibC_FrameQueue_ref_first(ttLibC_FrameQueue *queue);

/**
 * same as ref, however, remove called object.
 * if call func returns false. stop task and the last frame object is NOT removed.
 * @param queue    target queue object.
 * @param callback callback func. if return false, this task is ended.
 * @param ptr      user def pointer.
 * @return
 */
bool ttLibC_FrameQueue_dequeue(
		ttLibC_FrameQueue *queue,
		ttLibC_FrameQueueFunc callback,
		void *ptr);

/**
 * dequeue the first object only.
 * return frame is removed from queue.
 * @param queue target queue object
 * @return frame object.
 */
ttLibC_Frame *ttLibC_FrameQueue_dequeue_first(ttLibC_FrameQueue *queue);

/**
 * add frame on queue.
 * @param queue target queue object.
 * @param frame add frame object.
 * @return true:success false:error.
 */
bool ttLibC_FrameQueue_queue(
		ttLibC_FrameQueue *queue,
		ttLibC_Frame *frame);

/**
 * close queue object
 * @param queue
 */
void ttLibC_FrameQueue_close(ttLibC_FrameQueue **queue);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MISC_H_ */
