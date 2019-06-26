/*
 * @file   misc.c
 * @brief  misc for container.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2015/08/22
 */

#include "misc.h"
#include "../_log.h"
#include "../allocator.h"
#include "containerCommon.h"

#include <stdlib.h>

/*
 * detailed queue definition.
 */
typedef struct {
	/** inherit ttLibC_FrameQueue object. */
	ttLibC_FrameQueue inherit_super;
	/** frame array(to hold queue data.) */
	ttLibC_Frame **frame_array;
	/** size of frame_array */
	uint32_t stack_count;
	/** index pointer for first object. */
	uint32_t start_pos;
	/** index pointer for adding position */
	uint32_t end_pos;
} ttLibC_Container_Misc_FrameQueue_;

typedef ttLibC_Container_Misc_FrameQueue_ ttLibC_FrameQueue_;

/*
 * make frame queue
 * @param track_id track_id for container.
 * @param max_size max number of holding frame object.
 * @return frame queue object.
 */
ttLibC_FrameQueue TT_ATTRIBUTE_INNER *ttLibC_FrameQueue_make(
		uint32_t track_id,
		uint32_t max_size) {
	ttLibC_FrameQueue_ *queue = ttLibC_malloc(sizeof(ttLibC_FrameQueue_));
	if(queue == NULL) {
		ERR_PRINT("failed to allocate queue object.");
		return NULL;
	}
	queue->frame_array = ttLibC_malloc(sizeof(ttLibC_Frame*) * max_size);
	if(queue->frame_array == NULL) {
		ERR_PRINT("failed to allocate frame list.");
		ttLibC_free(queue);
		return NULL;
	}
	queue->stack_count            = max_size;
	queue->start_pos              = 0;
	queue->end_pos                = 0;
	queue->inherit_super.track_id = track_id;
	queue->inherit_super.timebase = 1000;
	queue->inherit_super.pts      = 0;
	// clear with NULL
	for(int i = 0;i < queue->stack_count;++ i) {
		queue->frame_array[i] = NULL;
	}
	return (ttLibC_FrameQueue *)queue;
}

/*
 * reference of value in queue.
 * call func in order.
 * @param queue    target queue object.
 * @param callback callback func. if return false, this task is ended.
 * @param ptr      user def pointer.
 * @return true:call all data. false:stopped.
 */
bool TT_ATTRIBUTE_INNER ttLibC_FrameQueue_ref(ttLibC_FrameQueue *queue, ttLibC_FrameQueueFunc callback, void *ptr) {
	ttLibC_FrameQueue_ *queue_ = (ttLibC_FrameQueue_ *)queue;
	if(queue_ == NULL) {
		return false;
	}
	uint32_t current_pos = queue_->start_pos;
	while(current_pos != queue_->end_pos) {
		if(!callback(ptr, queue_->frame_array[current_pos])) {
			return false;
		}
		current_pos = (current_pos + 1) % queue_->stack_count;
	}
	return true;
}

/*
 * ref the first object only.
 * @param queue target queue object.
 * @return frame object.
 */
ttLibC_Frame TT_ATTRIBUTE_INNER *ttLibC_FrameQueue_ref_first(ttLibC_FrameQueue *queue) {
	ttLibC_FrameQueue_ *queue_ = (ttLibC_FrameQueue_ *)queue;
	if(queue_ == NULL) {
		return NULL;
	}
	if(queue_->start_pos == queue_->end_pos) {
		// no data.
		return NULL;
	}
	return queue_->frame_array[queue_->start_pos];
}

/*
 * same as ref, however, remove called object.
 * if call func returns false. stop task and the last frame object is NOT removed.
 * @param queue    target queue object.
 * @param callback callback func. if return false, this task is ended.
 * @param ptr      user def pointer.
 * @return
 */
bool TT_ATTRIBUTE_INNER ttLibC_FrameQueue_dequeue(ttLibC_FrameQueue *queue, ttLibC_FrameQueueFunc callback, void *ptr) {
	ttLibC_FrameQueue_ *queue_ = (ttLibC_FrameQueue_ *)queue;
	if(queue_ == NULL) {
		return false;
	}
	while(queue_->start_pos != queue_->end_pos) {
		if(!callback(ptr, queue_->frame_array[queue_->start_pos])) {
			return false;
		}
		queue_->start_pos = (queue_->start_pos + 1) % queue_->stack_count;
	}
	return true;
}

/*
 * dequeue the first object only.
 * return frame is removed from queue.
 * @param queue target queue object
 * @return frame object.
 */
ttLibC_Frame TT_ATTRIBUTE_INNER *ttLibC_FrameQueue_dequeue_first(ttLibC_FrameQueue *queue) {
	ttLibC_FrameQueue_ *queue_ = (ttLibC_FrameQueue_ *)queue;
	if(queue_ == NULL) {
		return NULL;
	}
	if(queue_->start_pos == queue_->end_pos) {
		// no data.
		return NULL;
	}
	ttLibC_Frame *frame = queue_->frame_array[queue_->start_pos];
	queue_->start_pos = (queue_->start_pos + 1) % queue_->stack_count;
	return frame;
}

/*
 * add frame on queue.
 * @param queue target queue object.
 * @param frame add frame object.
 * @return true:success false:error.
 */
bool TT_ATTRIBUTE_INNER ttLibC_FrameQueue_queue(ttLibC_FrameQueue *queue, ttLibC_Frame *frame) {
	ttLibC_FrameQueue_ *queue_ = (ttLibC_FrameQueue_ *)queue;
	if(queue_ == NULL) {
		return false;
	}
	// clone frame, cuz frame object is reuse by supplier and easy to modify.
	ttLibC_Frame *f = ttLibC_Frame_clone(
			queue_->frame_array[queue_->end_pos],
			frame);
	if(f == NULL) {
		ERR_PRINT("failed to clone frame.");
		return false;
	}
	queue_->frame_array[queue_->end_pos] = f;
	queue_->end_pos = (queue_->end_pos + 1) % queue_->stack_count;
	queue_->inherit_super.pts = f->pts;
	queue_->inherit_super.timebase = f->timebase;
	return true;
}

/*
 * close queue object
 * @param queue
 */
void TT_ATTRIBUTE_INNER ttLibC_FrameQueue_close(ttLibC_FrameQueue **queue) {
	ttLibC_FrameQueue_ *target = (ttLibC_FrameQueue_ *)*queue;
	if(target == NULL) {
		return;
	}
	for(int i = 0;i < target->stack_count;++ i) {
		ttLibC_Frame_close(&target->frame_array[i]);
	}
	ttLibC_free(target->frame_array);
	ttLibC_free(target);
	*queue = NULL;
}

typedef struct FrameQueue_ReadyFrameCounter {
	uint32_t count;
	uint32_t result;
} FrameQueue_ReadyFrameCounter;

static bool FrameQueue_checkReadyFrameCallback(void *ptr, ttLibC_Frame *frame) {
	FrameQueue_ReadyFrameCounter *counter = (FrameQueue_ReadyFrameCounter *)ptr;
	counter->count ++;
	if(!ttLibC_ContainerWriter_isReadyFrame(frame)) {
		return true;
	}
	counter->result = counter->count;
	return true;
}

uint32_t TT_ATTRIBUTE_INNER ttLibC_FrameQueue_getReadyFrameCount(ttLibC_FrameQueue *queue) {
	FrameQueue_ReadyFrameCounter counter;
	counter.count = 0;
	counter.result = 0;
	ttLibC_FrameQueue_ref(queue, FrameQueue_checkReadyFrameCallback, &counter);
	return counter.result;
}
