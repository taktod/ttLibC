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
	queue->inherit_super.isBframe_fixed = false;
	queue->frame_cache[0] = NULL;
	queue->frame_cache[1] = NULL;
	queue->frame_cache[2] = NULL;
	queue->pts_cache[0] = 0;
	queue->pts_cache[1] = 0;
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
					switch(h->frame_type) {
					case H264FrameType_I:
					case H264FrameType_B:
					case H264FrameType_P:
						break;
					default:
						{
							ERR_PRINT("support only i b p frame.");
						}
						return false;
					}
					if(queue_->frame_cache[0] == NULL) {
						queue_->frame_cache[0] = f;
					}
					else {
						uint64_t a_pts, b_pts, c_pts, d_pts, e_pts;
						a_pts = f->pts;
						b_pts = queue_->frame_cache[0]->pts;
						c_pts = (queue_->frame_cache[1] == NULL ? 0 : queue_->frame_cache[1]->pts);
						d_pts = queue_->pts_cache[0];
						e_pts = queue_->pts_cache[1];
						ttLibC_H264 *bh = (ttLibC_H264 *)queue_->frame_cache[0];
						switch(queue_->status) {
						default:
						case frameStatus_normal:
							switch(h->frame_type) {
							case H264FrameType_I:
							case H264FrameType_P:
								switch(bh->frame_type) {
								case H264FrameType_I:
								case H264FrameType_P:
									{
										queue_->frame_cache[0]->dts = b_pts;
										queue_->inherit_super.pts = b_pts;
										queue_->frame_cache[0] = f;
										queue_->frame_cache[1] = NULL;
										queue_->pts_cache[0] = b_pts;
										queue_->pts_cache[1] = 0;
									}
									break;
								case H264FrameType_B:
									{
										uint64_t min = a_pts;
										if(min > b_pts) {
											min = b_pts;
										}
										if(min > c_pts) {
											min = c_pts;
										}
										queue_->pts_cache[0] = b_pts;
										queue_->pts_cache[1] = c_pts;
										queue_->frame_cache[1]->dts = (min + 2 * d_pts) / 3;
										queue_->frame_cache[0]->dts = (min * 2 + d_pts) / 3;
										queue_->inherit_super.pts = queue_->frame_cache[0]->dts;
										if(queue_->inherit_super.isBframe_fixed) {
											f->dts = min;
											queue_->inherit_super.pts = min;
										}
										queue_->frame_cache[1] = NULL;
										queue_->frame_cache[0] = f;
										queue_->status = frameStatus_inBframe;
									}
									break;
								default:
									break;
								}
								break;
							case H264FrameType_B:
								switch(bh->frame_type) {
								case H264FrameType_I:
								case H264FrameType_P:
									{
										queue_->frame_cache[1] = queue_->frame_cache[0];
										queue_->frame_cache[0] = f;
									}
									break;
								case H264FrameType_B:
									{
										uint64_t min = a_pts;
										if(min > b_pts) {
											min = b_pts;
										}
										if(min > c_pts) {
											min = c_pts;
										}
										queue_->pts_cache[0] = b_pts;
										queue_->pts_cache[1] = c_pts;
										queue_->frame_cache[1]->dts = (min + 2 * d_pts) / 3;
										queue_->frame_cache[0]->dts = (min * 2 + d_pts) / 3;
										f->dts = min;
										queue_->inherit_super.pts = min;
										queue_->frame_cache[1] = NULL;
										queue_->frame_cache[0] = f;
										queue_->status = frameStatus_inBframe;
									}
									break;
								default:
									break;
								}
								break;
							default:
								break;
							}
							break;
						case frameStatus_inBframe:
							switch(h->frame_type) {
							case H264FrameType_I:
							case H264FrameType_P:
								if(!queue_->inherit_super.isBframe_fixed) {
									switch(bh->frame_type) {
									case H264FrameType_I:
									case H264FrameType_P:
										{
											queue_->frame_cache[0]->dts = queue_->frame_cache[0]->pts;
											queue_->inherit_super.pts = queue_->frame_cache[0]->dts;
											queue_->pts_cache[0] = queue_->frame_cache[0]->pts;
											queue_->pts_cache[1] = 0;
											queue_->frame_cache[0] = f;
											queue_->frame_cache[1] = NULL;
											queue_->status = frameStatus_normal;
										}
										break;
									case H264FrameType_B:
										{
											queue_->frame_cache[0] = f;
										}
										break;
									default:
										break;
									}
									break;
								}
								/* no break */
							case H264FrameType_B:
								switch(bh->frame_type) {
								case H264FrameType_I:
								case H264FrameType_P:
									if(!queue_->inherit_super.isBframe_fixed)
									{
										uint64_t min = b_pts;
										if(min > d_pts) {
											min = d_pts;
										}
										if(min > e_pts) {
											min = e_pts;
										}
										uint64_t *pcache = queue_->pts_cache;
										if(b_pts != min) {
											*pcache = b_pts;
											++ pcache;
										}
										if(d_pts != min) {
											*pcache = d_pts;
											++ pcache;
										}
										if(e_pts != min) {
											*pcache = e_pts;
											++ pcache;
										}
										queue_->frame_cache[0]->dts = min;
										min = a_pts;
										d_pts = queue_->pts_cache[0];
										e_pts = queue_->pts_cache[1];
										if(min > d_pts) {
											min = d_pts;
										}
										if(min > e_pts) {
											min = e_pts;
										}
										pcache = queue_->pts_cache;
										if(a_pts != min) {
											*pcache = a_pts;
											++ pcache;
										}
										if(d_pts != min) {
											*pcache = d_pts;
											++ pcache;
										}
										if(e_pts != min) {
											*pcache = e_pts;
											++ pcache;
										}
										f->dts = min;
										queue_->inherit_super.pts = min;
										queue_->frame_cache[0] = f;
										break;
									}
									/* no break */
								case H264FrameType_B:
									{
										uint64_t min = a_pts;
										if(min > d_pts) {
											min = d_pts;
										}
										if(min > e_pts) {
											min = e_pts;
										}
										uint64_t *pcache = queue_->pts_cache;
										if(a_pts != min) {
											*pcache = a_pts;
											++ pcache;
										}
										if(d_pts != min) {
											*pcache = d_pts;
											++ pcache;
										}
										if(e_pts != min) {
											*pcache = e_pts;
											++ pcache;
										}
										f->dts = min;
										queue_->inherit_super.pts = min;
										queue_->frame_cache[0] = f;
										queue_->frame_cache[1] = NULL;
									}
									break;
								default:
									break;
								}
								break;
							default:
								break;
							}
							break;
						}
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
					switch(h->frame_type) {
					case H265FrameType_I:
					case H265FrameType_B:
					case H265FrameType_P:
						break;
					default:
						{
							ERR_PRINT("support only i b p frame.");
						}
						return false;
					}
					if(queue_->frame_cache[0] == NULL) {
						queue_->frame_cache[0] = f;
					}
					else {
						uint64_t a_pts, b_pts, c_pts, d_pts, e_pts;
						a_pts = f->pts;
						b_pts = queue_->frame_cache[0]->pts;
						c_pts = (queue_->frame_cache[1] == NULL ? 0 : queue_->frame_cache[1]->pts);
						d_pts = queue_->pts_cache[0];
						e_pts = queue_->pts_cache[1];
						ttLibC_H265 *bh = (ttLibC_H265 *)queue_->frame_cache[0];
						switch(queue_->status) {
						default:
						case frameStatus_normal:
							switch(h->frame_type) {
							case H265FrameType_I:
							case H265FrameType_P:
								switch(bh->frame_type) {
								case H265FrameType_I:
								case H265FrameType_P:
									{
										queue_->frame_cache[0]->dts = b_pts;
										queue_->inherit_super.pts = b_pts;
										queue_->frame_cache[0] = f;
										queue_->frame_cache[1] = NULL;
										queue_->pts_cache[0] = b_pts;
										queue_->pts_cache[1] = 0;
									}
									break;
								case H265FrameType_B:
									{
										uint64_t min = a_pts;
										if(min > b_pts) {
											min = b_pts;
										}
										if(min > c_pts) {
											min = c_pts;
										}
										queue_->pts_cache[0] = b_pts;
										queue_->pts_cache[1] = c_pts;
										queue_->frame_cache[1]->dts = (min + 2 * d_pts) / 3;
										queue_->frame_cache[0]->dts = (min * 2 + d_pts) / 3;
										queue_->inherit_super.pts = queue_->frame_cache[0]->dts;
										queue_->frame_cache[1] = NULL;
										queue_->frame_cache[0] = f;
										queue_->status = frameStatus_inBframe;
									}
									break;
								default:
									break;
								}
								break;
							case H265FrameType_B:
								switch(bh->frame_type) {
								case H265FrameType_I:
								case H265FrameType_P:
									{
										queue_->frame_cache[1] = queue_->frame_cache[0];
										queue_->frame_cache[0] = f;
									}
									break;
								case H265FrameType_B:
									{
										uint64_t min = a_pts;
										if(min > b_pts) {
											min = b_pts;
										}
										if(min > c_pts) {
											min = c_pts;
										}
										queue_->pts_cache[0] = b_pts;
										queue_->pts_cache[1] = c_pts;
										queue_->frame_cache[1]->dts = (min + 2 * d_pts) / 3;
										queue_->frame_cache[0]->dts = (min * 2 + d_pts) / 3;
										f->dts = min;
										queue_->inherit_super.pts = min;
										queue_->frame_cache[1] = NULL;
										queue_->frame_cache[0] = f;
										queue_->status = frameStatus_inBframe;
									}
									break;
								default:
									break;
								}
								break;
							default:
								break;
							}
							break;
						case frameStatus_inBframe:
							switch(h->frame_type) {
							case H265FrameType_I:
							case H265FrameType_P:
								switch(bh->frame_type) {
								case H265FrameType_I:
								case H265FrameType_P:
									{
										queue_->frame_cache[0]->dts = queue_->frame_cache[0]->pts;
										queue_->inherit_super.pts = queue_->frame_cache[0]->dts;
										queue_->pts_cache[0] = queue_->frame_cache[0]->pts;
										queue_->pts_cache[1] = 0;
										queue_->frame_cache[0] = f;
										queue_->frame_cache[1] = NULL;
										queue_->status = frameStatus_normal;
									}
									break;
								case H265FrameType_B:
									{
										queue_->frame_cache[0] = f;
									}
									break;
								default:
									break;
								}
								break;
							case H265FrameType_B:
								switch(bh->frame_type) {
								case H265FrameType_I:
								case H265FrameType_P:
									{
										uint64_t min = b_pts;
										if(min > d_pts) {
											min = d_pts;
										}
										if(min > e_pts) {
											min = e_pts;
										}
										uint64_t *pcache = queue_->pts_cache;
										if(b_pts != min) {
											*pcache = b_pts;
											++ pcache;
										}
										if(d_pts != min) {
											*pcache = d_pts;
											++ pcache;
										}
										if(e_pts != min) {
											*pcache = e_pts;
											++ pcache;
										}
										queue_->frame_cache[0]->dts = min;
										min = a_pts;
										d_pts = queue_->pts_cache[0];
										e_pts = queue_->pts_cache[1];
										if(min > d_pts) {
											min = d_pts;
										}
										if(min > e_pts) {
											min = e_pts;
										}
										pcache = queue_->pts_cache;
										if(a_pts != min) {
											*pcache = a_pts;
											++ pcache;
										}
										if(d_pts != min) {
											*pcache = d_pts;
											++ pcache;
										}
										if(e_pts != min) {
											*pcache = e_pts;
											++ pcache;
										}
										f->dts = min;
										queue_->inherit_super.pts = min;
										queue_->frame_cache[0] = f;
									}
									break;
								case H265FrameType_B:
									{
										uint64_t min = a_pts;
										if(min > d_pts) {
											min = d_pts;
										}
										if(min > e_pts) {
											min = e_pts;
										}
										uint64_t *pcache = queue_->pts_cache;
										if(a_pts != min) {
											*pcache = a_pts;
											++ pcache;
										}
										if(d_pts != min) {
											*pcache = d_pts;
											++ pcache;
										}
										if(e_pts != min) {
											*pcache = e_pts;
											++ pcache;
										}
										f->dts = min;
										queue_->inherit_super.pts = min;
										queue_->frame_cache[0] = f;
										queue_->frame_cache[1] = NULL;
									}
									break;
								default:
									break;
								}
								break;
							default:
								break;
							}
							break;
						}
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
