/*
 * @file   misc2.c
 * @brief  bisc for container.(use StlList instead of array.)
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/23
 */

#include "misc.h"
#include "../_log.h"
#include "../allocator.h"
#include "../frame/video/video.h"
#include <stdlib.h>
#include "../util/stlListUtil.h"

typedef enum {
	frameStatus_preprocess1,
	frameStatus_preprocess2,
	frameStatus_setupInitial1,
	frameStatus_setupInitial2,
	frameStatus_nonPiramidal,
	frameStatus_piramidal
} frameStatus;

/*
 * detail definition for FrameQueue (stlList version.)
 */
typedef struct ttLibC_Container_Misc_FrameQueue2_{
	/** inherit ttLibC_FrameQueue object. */
	ttLibC_FrameQueue inherit_super;
	/** queue frame list */
	ttLibC_StlList *frame_list;
	/** for reuse frame */
	ttLibC_StlList *used_frame_list;
	/** callback */
	ttLibC_FrameQueueFunc callback;
	/** ptr */
	void *ptr;
	// data for h26x dts calculate.
	/** frame cache */
	ttLibC_Frame *frame_cache[3];
	/** for calcurate */
	uint64_t pts_cache[2];
	/** process status */
	frameStatus status;
} ttLibC_Container_Misc_FrameQueue2_;

typedef ttLibC_Container_Misc_FrameQueue2_ ttLibC_FrameQueue2_;

/*
 * make frame queue
 * @param track_id track_id for container.
 * @param max_size max number of holding frame object.
 * @return frame queue object.
 */
ttLibC_FrameQueue *ttLibC_FrameQueue_make(
		uint32_t track_id,
		uint32_t max_size) {
	(void)max_size;
	ttLibC_FrameQueue2_ *queue = ttLibC_malloc(sizeof(ttLibC_FrameQueue2_));
	if(queue == NULL) {
		ERR_PRINT("failed to allocate queue object.");
		return NULL;
	}
	queue->frame_list = ttLibC_StlList_make();
	if(queue->frame_list == NULL) {
		ERR_PRINT("failed to allocate frame list.");
		ttLibC_free(queue);
		return NULL;
	}
	queue->used_frame_list = ttLibC_StlList_make();
	if(queue->used_frame_list == NULL) {
		ttLibC_StlList_close(&queue->frame_list);
		ttLibC_free(queue);
		return NULL;
	}
	queue->callback = NULL;
	queue->ptr = NULL;
	queue->inherit_super.track_id = track_id;
	queue->inherit_super.timebase = 1000;
	queue->inherit_super.pts = 0;
	queue->frame_cache[0] = NULL;
	queue->frame_cache[1] = NULL;
	queue->frame_cache[2] = NULL;
	queue->pts_cache[0] = 0;
	queue->pts_cache[1] = 0;
	queue->status = frameStatus_preprocess1;
	return (ttLibC_FrameQueue *)queue;
}

static bool FrameQueue_refCallback(void *ptr, void *item) {
	ttLibC_FrameQueue2_ *queue = (ttLibC_FrameQueue2_ *)ptr;
	if(queue->callback != NULL) {
		if(!queue->callback(queue->ptr, (ttLibC_Frame *)item)) {
			return false;
		}
	}
	return true;
}

/*
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
		void *ptr) {
	ttLibC_FrameQueue2_ *queue_ = (ttLibC_FrameQueue2_ *)queue;
	if(queue_ == NULL) {
		return false;
	}
	queue_->callback = callback;
	queue_->ptr = ptr;
	ttLibC_StlList_forEach(queue_->frame_list, FrameQueue_refCallback, queue_);
	queue_->callback = NULL;
	queue_->ptr = NULL;
	return true;
}

/*
 * ref the first object only.
 * @param queue target queue object.
 * @return frame object.
 */
ttLibC_Frame *ttLibC_FrameQueue_ref_first(ttLibC_FrameQueue *queue) {
	ttLibC_FrameQueue2_ *queue_ = (ttLibC_FrameQueue2_ *)queue;
	if(queue_ == NULL) {
		return NULL;
	}
	return (ttLibC_Frame *)ttLibC_StlList_refFirst(queue_->frame_list);
}

/*
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
		void *ptr) {
	ttLibC_FrameQueue2_ *queue_ = (ttLibC_FrameQueue2_ *)queue;
	if(queue_ == NULL) {
		return false;
	}
	ttLibC_Frame *frame = NULL;
	while((frame = (ttLibC_Frame *)ttLibC_StlList_refFirst(queue_->frame_list)) != NULL) {
		if(!callback(ptr, frame)) {
			return false;
		}
		ttLibC_StlList_remove(queue_->frame_list, frame);
		ttLibC_StlList_addLast(queue_->used_frame_list, frame);
	}
	return true;
}

/*
 * dequeue the first object only.
 * return frame is removed from queue.
 * @param queue target queue object
 * @return frame object.
 */
ttLibC_Frame *ttLibC_FrameQueue_dequeue_first(ttLibC_FrameQueue *queue) {
	ttLibC_FrameQueue2_ *queue_ = (ttLibC_FrameQueue2_ *)queue;
	if(queue_ == NULL) {
		return NULL;
	}
	ttLibC_Frame *frame = (ttLibC_Frame *)ttLibC_StlList_refFirst(queue_->frame_list);
	if(frame == NULL) {
		return NULL;
	}
	ttLibC_StlList_remove(queue_->frame_list, frame);
	ttLibC_StlList_addLast(queue_->used_frame_list, frame);
	return frame;
}

/*
 * add frame on queue.
 * @param queue target queue object.
 * @param frame add frame object.
 * @return true:success false:error.
 */
bool ttLibC_FrameQueue_queue(
		ttLibC_FrameQueue *queue,
		ttLibC_Frame *frame) {
	ttLibC_FrameQueue2_ *queue_ = (ttLibC_FrameQueue2_ *)queue;
	if(queue_ == NULL) {
		return false;
	}
	ttLibC_Frame *prev_frame = NULL;
	if(queue_->used_frame_list->size > 3) {
		prev_frame = (ttLibC_Frame *)ttLibC_StlList_refFirst(queue_->used_frame_list);
	}
	ttLibC_Frame *f = ttLibC_Frame_clone(
			prev_frame,
			frame);
	if(f == NULL) {
		ERR_PRINT("failed to clone frame.");
		return false;
	}
	// let save dts information from frame.
	f->dts = frame->dts;
	ttLibC_StlList_remove(queue_->used_frame_list, prev_frame);
	if(f->dts == 0) {
		switch(f->type) {
		case frameType_h264:
		case frameType_h265:
			{
				ttLibC_Video *v = (ttLibC_Video *)f;
				if(v->type != videoType_info) {
					switch(queue_->status) {
					case frameStatus_preprocess1:
						queue_->frame_cache[1] = f;
						queue_->status = frameStatus_preprocess2;
						break;
					case frameStatus_preprocess2:
						queue_->frame_cache[0] = f;
						queue_->status = frameStatus_setupInitial1;
						break;
					case frameStatus_setupInitial1:
						if(queue_->frame_cache[1]->pts > queue_->frame_cache[0]->pts) {
							ERR_PRINT("unexpected data. preprocess frame is flipped.");
							return false;
						}
						else {
							// set the dts of [1].
							queue_->frame_cache[1]->dts = queue_->frame_cache[1]->pts;
							queue_->inherit_super.pts = queue_->frame_cache[1]->pts;
							if(queue_->frame_cache[0]->pts < f->pts) {
								// [1] < [0] < f
								// we don't need to deal with dts yet.
								queue_->frame_cache[1] = queue_->frame_cache[0];
								queue_->frame_cache[0] = f;
							}
							else {
								// now we need to deal with dts.
								queue_->frame_cache[2] = queue_->frame_cache[1];
								queue_->frame_cache[1] = queue_->frame_cache[0];
								queue_->frame_cache[0] = f;
								queue_->status = frameStatus_setupInitial2;
							}
						}
						break;
					case frameStatus_setupInitial2:
						if(queue_->frame_cache[0]->pts < f->pts) {
							// non pyramidal bframe?
							// [2] < [1] > [0] < f
							queue_->frame_cache[1]->dts = (queue_->frame_cache[0]->pts + queue_->frame_cache[2]->pts * 2) / 3;
							queue_->frame_cache[0]->dts = (queue_->frame_cache[0]->pts * 2 + queue_->frame_cache[2]->pts) / 3;
							f->dts = queue_->frame_cache[0]->pts;
							if(queue_->frame_cache[1]->pts < f->pts) {
								queue_->pts_cache[0] = f->pts;
								queue_->pts_cache[1] = queue_->frame_cache[1]->pts;
							}
							else {
								queue_->pts_cache[1] = f->pts;
								queue_->pts_cache[0] = queue_->frame_cache[1]->pts;
							}
							queue_->frame_cache[2] = NULL;
							queue_->frame_cache[1] = NULL;
							queue_->frame_cache[0] = NULL;
							queue_->status = frameStatus_piramidal;
						}
						else {
							// pyramidal bframe?
							queue_->frame_cache[1]->dts = (f->pts + queue_->frame_cache[2]->pts * 2) / 3;
							queue_->frame_cache[0]->dts = (f->pts * 2 + queue_->frame_cache[2]->pts) / 3;
							f->dts = f->pts;
							queue_->pts_cache[0] = queue_->frame_cache[1]->pts;
							queue_->pts_cache[1] = queue_->frame_cache[0]->pts;
							queue_->frame_cache[2] = NULL;
							queue_->frame_cache[1] = NULL;
							queue_->frame_cache[0] = NULL;
							queue_->status = frameStatus_piramidal;
						}
						queue_->inherit_super.pts = f->dts;
						break;
					case frameStatus_nonPiramidal:
						if(queue_->pts_cache[0] < f->pts) {
							f->dts = queue_->pts_cache[0];
							queue_->pts_cache[0] = f->pts;
						}
						else {
							f->dts = f->pts;
						}
						queue_->inherit_super.pts = f->dts;
						break;
					case frameStatus_piramidal:
						if(queue_->pts_cache[1] < f->pts) {
							f->dts = queue_->pts_cache[1];
							if(queue_->pts_cache[0] < f->pts) {
								queue_->pts_cache[1] = queue_->pts_cache[0];
								queue_->pts_cache[0] = f->pts;
							}
							else {
								queue_->pts_cache[1] = f->pts;
							}
						}
						else {
							f->dts = f->pts;
						}
						queue_->inherit_super.pts = f->dts;
						break;
					default:
						break;
					}
				}
			}
			break;
		default:
			f->dts = f->pts;
			queue_->inherit_super.pts = f->pts;
			break;
		}
	}
	else {
		queue_->inherit_super.pts = f->pts;
	}
	queue_->inherit_super.timebase = f->timebase;
	return ttLibC_StlList_addLast(queue_->frame_list, f);
}

static bool FrameQueue_frameClose(void *ptr, void *item) {
	(void)ptr;
	ttLibC_Frame_close((ttLibC_Frame **)&item);
	return true;
}

/*
 * close queue object
 * @param queue
 */
void ttLibC_FrameQueue_close(ttLibC_FrameQueue **queue) {
	ttLibC_FrameQueue2_ *target = (ttLibC_FrameQueue2_ *)*queue;
	if(target == NULL) {
		return;
	}
	*queue = NULL;
	ttLibC_StlList_forEach(target->used_frame_list, FrameQueue_frameClose, NULL);
	ttLibC_StlList_close(&target->used_frame_list);
	ttLibC_StlList_forEach(target->frame_list, FrameQueue_frameClose, NULL);
	ttLibC_StlList_close(&target->frame_list);
	ttLibC_free(target);
}
