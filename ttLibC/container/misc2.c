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
#include "../ttLibC_predef.h"
#include "../_log.h"
#include "../allocator.h"
#include "../frame/video/video.h"
#include "../frame/video/h264.h"
#include "../frame/video/h265.h"
#include <stdlib.h>
#include "../util/stlListUtil.h"
#include "containerCommon.h"

typedef enum {
	frameStatus_normal,
	frameStatus_inBframe
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
	uint64_t pts_cache[3];
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
ttLibC_FrameQueue TT_VISIBILITY_HIDDEN *ttLibC_FrameQueue_make(
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
	queue->inherit_super.isBframe_fixed = false;
	queue->frame_cache[0] = NULL;
	queue->frame_cache[1] = NULL;
	queue->frame_cache[2] = NULL;
	queue->pts_cache[0] = 0;
	queue->pts_cache[1] = 0;
	queue->pts_cache[2] = 0;
	queue->status = frameStatus_normal;
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
bool TT_VISIBILITY_HIDDEN ttLibC_FrameQueue_ref(
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
ttLibC_Frame TT_VISIBILITY_HIDDEN *ttLibC_FrameQueue_ref_first(ttLibC_FrameQueue *queue) {
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
bool TT_VISIBILITY_HIDDEN ttLibC_FrameQueue_dequeue(
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
ttLibC_Frame TT_VISIBILITY_HIDDEN *ttLibC_FrameQueue_dequeue_first(ttLibC_FrameQueue *queue) {
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

/**
 * check frame cache status.
 * @param queue
 * @return true:filled by cache, false:have more slot.
 */
static bool FrameQueue_checkFrameQueueIsFull(ttLibC_FrameQueue2_ *queue) {
	return queue->frame_cache[0] != NULL
		&& queue->frame_cache[1] != NULL
		&& queue->frame_cache[2] != NULL;
}

/**
 * get the lowest pts among frames.
 * @param frame 
 * @param queue
 * @param check_pts_cache
 * @return lowest pts
 */
static uint64_t FrameQueue_getLowestPts(
		ttLibC_Frame *frame,
		ttLibC_FrameQueue2_ *queue,
		bool check_pts_cache) {
	uint64_t lowest_pts = frame->pts;
	for(int i = 0;i < 3;++ i) {
		if(lowest_pts > queue->frame_cache[i]->pts) {
			lowest_pts = queue->frame_cache[i]->pts;
		}
	}
	if(check_pts_cache) {
		for(int i = 0;i < 3;++ i) {
			if(queue->pts_cache[i] == 0) {
				continue;
			}
			if(lowest_pts > queue->pts_cache[i]) {
				lowest_pts = queue->pts_cache[i];
			}
		}
	}
	return lowest_pts;
}

/**
 * update pts cache in the queue
 * @param queue
 * @param pts
 */
static void FrameQueue_updatePtsCache(
		ttLibC_FrameQueue2_ *queue,
		uint64_t pts) {
	for(int i = 0;i < 3;++ i) {
		if(queue->pts_cache[i] == 0) {
			queue->pts_cache[i] = pts;
			return;
		}
	}
}

/**
 * update frame with pts cache
 * @param queue
 * @param lowest_pts
 * @return true:cache is update false:nothing is changed.
 */
static bool FrameQueue_updateFrameCache(
		ttLibC_FrameQueue2_ *queue,
		uint64_t lowest_pts) {
	for(int i = 0;i < 3;++ i) {
		if(queue->pts_cache[i] != 0) {
			if(queue->pts_cache[i] == lowest_pts) {
				queue->frame_cache[2]->dts = lowest_pts;
				queue->pts_cache[i] = queue->frame_cache[2]->pts;
				queue->frame_cache[2] = NULL;
				return true;
			}
		}
	}
	return false;
}

/**
 * update frame with frame cache
 * @param queue
 * @param lowest_pts
 * @return true:frame cache is update false:nothing is changed.
 */
static bool FrameQueue_updateFrameQueue(
		ttLibC_FrameQueue2_ *queue,
		uint64_t lowest_pts) {
	if(queue->frame_cache[2]->pts == lowest_pts) {
		queue->frame_cache[2]->dts = lowest_pts;
		queue->frame_cache[2] = NULL;
		return true;
	}
	if(queue->frame_cache[1]->pts == lowest_pts) {
		uint64_t diff = (lowest_pts - queue->inherit_super.pts) / 2;
		queue->frame_cache[2]->dts = queue->inherit_super.pts + diff;
		FrameQueue_updatePtsCache(queue, queue->frame_cache[2]->pts);
		queue->frame_cache[2] = NULL;
		queue->frame_cache[1]->dts = lowest_pts;
		queue->frame_cache[1] = NULL;
		return true;
	}
	if(queue->frame_cache[0]->pts == lowest_pts) {
		uint64_t diff = (lowest_pts - queue->inherit_super.pts) / 3;
		queue->frame_cache[2]->dts = queue->inherit_super.pts + diff;
		FrameQueue_updatePtsCache(queue, queue->frame_cache[2]->pts);
		queue->frame_cache[2] = NULL;
		queue->frame_cache[1]->dts = queue->inherit_super.pts + 2 * diff;
		FrameQueue_updatePtsCache(queue, queue->frame_cache[1]->pts);
		queue->frame_cache[1] = NULL;
		queue->frame_cache[0]->dts = lowest_pts;
		queue->frame_cache[0] = NULL;
		return true;
	}
	return false;
}

/**
 * update frame
 * @param frame
 * @param queue
 * @param lowests_pts
 * @return true:frame is update, false:nothing change(no way to here.)
 */
static bool FrameQueue_updateFrame(
		ttLibC_Frame *frame,
		ttLibC_FrameQueue2_ *queue,
		uint64_t lowest_pts) {
	if(frame->pts != lowest_pts) {
		ERR_PRINT("unexpected. lowest pts is not match any frame.");
		return false;
	}
	uint64_t diff = (lowest_pts - queue->inherit_super.pts) / 4;
	queue->frame_cache[2]->dts = queue->inherit_super.pts + diff;
	FrameQueue_updatePtsCache(queue, queue->frame_cache[2]->pts);
	queue->frame_cache[2] = NULL;
	queue->frame_cache[1]->dts = queue->inherit_super.pts + 2 * diff;
	FrameQueue_updatePtsCache(queue, queue->frame_cache[1]->pts);
	queue->frame_cache[1] = NULL;
	queue->frame_cache[0]->dts = queue->inherit_super.pts + 3 * diff;
	FrameQueue_updatePtsCache(queue, queue->frame_cache[0]->pts);
	queue->frame_cache[0] = NULL;
	frame->dts = lowest_pts;
	frame = NULL;
	return true;
}

/*
 * add frame on queue.
 * @param queue target queue object.
 * @param frame add frame object.
 * @return true:success false:error.
 */
bool TT_VISIBILITY_HIDDEN ttLibC_FrameQueue_queue(
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
			{
				ttLibC_H264 *h = (ttLibC_H264 *)f;
				switch(h->type) {
				default:
				case H264Type_configData:
				case H264Type_unknown:
					f->dts = f->pts;
					break;
				case H264Type_slice:
				case H264Type_sliceIDR:
					{
						ttLibC_Frame *ff = f;
						// if queue is full, need to calcurate dts
						if(FrameQueue_checkFrameQueueIsFull(queue_)) {
							// in the case of bframe fixed(flv or rtmp)
							if(queue_->inherit_super.isBframe_fixed) {
								uint64_t lowest_pts = FrameQueue_getLowestPts(
									ff,
									queue_,
									true);
								// check with pts_cache
								if(!FrameQueue_updateFrameCache(
									queue_,
									lowest_pts)) {
									// check with frame_cache
									if(!FrameQueue_updateFrameQueue(
										queue_,
										lowest_pts)) {
										// check with frame
										FrameQueue_updateFrame(
											ff,
											queue_,
											lowest_pts);
										ff = NULL;
									}
								}
								queue_->inherit_super.pts = lowest_pts;
							}
							else {
								uint64_t lowest_pts = FrameQueue_getLowestPts(
									ff,
									queue_,
									false);
								// check with frame_cache
								if(!FrameQueue_updateFrameQueue(
									queue_,
									lowest_pts)) {
									// check with frame
									FrameQueue_updateFrame(
										ff,
										queue_,
										lowest_pts);
									ff = NULL;
								}
								// clear pts_cache (in this mode, not use.)
								queue_->pts_cache[0] = 0;
								queue_->pts_cache[1] = 0;
								queue_->pts_cache[2] = 0;
								queue_->inherit_super.pts = lowest_pts;
							}
						}
						// rotate frame cache
						queue_->frame_cache[2] = queue_->frame_cache[1];
						queue_->frame_cache[1] = queue_->frame_cache[0];
						queue_->frame_cache[0] = ff;
					}
					break;
				}
			}
			break;
		case frameType_h265:
			{
				ttLibC_H265 *h = (ttLibC_H265 *)f;
				switch(h->type) {
				default:
				case H265Type_configData:
				case H265Type_unknown:
					f->dts = f->pts;
					break;
				case H265Type_slice:
				case H265Type_sliceIDR:
					{
						ttLibC_Frame *ff = f;
						// if queue is full, need to calcurate dts
						if(FrameQueue_checkFrameQueueIsFull(queue_)) {
							// in the case of bframe fixed(flv or rtmp)
							if(queue_->inherit_super.isBframe_fixed) {
								uint64_t lowest_pts = FrameQueue_getLowestPts(
									ff,
									queue_,
									true);
								// check with pts_cache
								if(!FrameQueue_updateFrameCache(
									queue_,
									lowest_pts)) {
									// check with frame_cache
									if(!FrameQueue_updateFrameQueue(
										queue_,
										lowest_pts)) {
										// check with frame
										FrameQueue_updateFrame(
											ff,
											queue_,
											lowest_pts);
										ff = NULL;
									}
								}
								queue_->inherit_super.pts = lowest_pts;
							}
							else {
								uint64_t lowest_pts = FrameQueue_getLowestPts(
									ff,
									queue_,
									false);
								// check with frame_cache
								if(!FrameQueue_updateFrameQueue(
									queue_,
									lowest_pts)) {
									// check with frame
									FrameQueue_updateFrame(
										ff,
										queue_,
										lowest_pts);
									ff = NULL;
								}
								// clear pts_cache (in this mode, not use.)
								queue_->pts_cache[0] = 0;
								queue_->pts_cache[1] = 0;
								queue_->pts_cache[2] = 0;
								queue_->inherit_super.pts = lowest_pts;
							}
						}
						// rotate frame cache
						queue_->frame_cache[2] = queue_->frame_cache[1];
						queue_->frame_cache[1] = queue_->frame_cache[0];
						queue_->frame_cache[0] = ff;
					}
					break;
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
void TT_VISIBILITY_HIDDEN ttLibC_FrameQueue_close(ttLibC_FrameQueue **queue) {
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

uint32_t TT_VISIBILITY_HIDDEN ttLibC_FrameQueue_getReadyFrameCount(ttLibC_FrameQueue *queue) {
	FrameQueue_ReadyFrameCounter counter;
	counter.count = 0;
	counter.result = 0;
	ttLibC_FrameQueue_ref(queue, FrameQueue_checkReadyFrameCallback, &counter);
	return counter.result;
}
