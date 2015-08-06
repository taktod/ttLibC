/*
 * @file   video.c
 * @brief  video frame information.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/18
 */

#include "video.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "bgr.h"
#include "yuv420.h"
#include "h264.h"
#include "vp8.h"
#include "../../log.h"

/*
 * make video frame
 * @param prev_frame    reuse frame.
 * @param frame_size    allocate frame size.
 * @param frame_type    type of frame.
 * @param type          type of videoframe
 * @param width         width
 * @param height        height
 * @param data          data
 * @param data_size     data size
 * @param non_copy_mode true: hold the data pointer. false:copy the data.
 * @param pts           pts for data.
 * @param timebase      timebase number for pts.
 * @return video frame object.
 */
ttLibC_Video *ttLibC_Video_make(
		ttLibC_Video *prev_frame,
		size_t frame_size,
		ttLibC_Frame_Type frame_type,
		ttLibC_Video_Type type,
		uint32_t width,
		uint32_t height,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	ttLibC_Video *video = prev_frame;
	size_t buffer_size_ = data_size;
	size_t data_size_ = data_size;
	switch(frame_type) {
	case frameType_bgr:
	case frameType_flv1:
	case frameType_h264:
	case frameType_vp8:
	case frameType_wmv1:
	case frameType_yuv420:
		break;
	default:
		ERR_PRINT("unknown video frame type.%d", frame_type);
		return NULL;
	}
	switch(type) {
	case videoType_info:
	case videoType_inter:
	case videoType_key:
		break;
	default:
		ERR_PRINT("unknown video type.%d", frame_type);
		return NULL;
	}
	if(frame_size == 0) {
		frame_size = sizeof(ttLibC_Video);
	}
	if(video == NULL) {
		video = malloc(frame_size);
		if(video == NULL) {
			ERR_PRINT("failed to allocate memory for video frame.");
			return NULL;
		}
		video->inherit_super.data = NULL;
	}
	else {
		if(!video->inherit_super.is_non_copy) {
			if(non_copy_mode || video->inherit_super.data_size < data_size) {
				free(video->inherit_super.data);
				video->inherit_super.data = NULL;
			}
			else {
				data_size_ = video->inherit_super.data_size;
			}
		}
	}
	video->type                      = type;
	video->width                     = width;
	video->height                    = height;
	video->inherit_super.buffer_size = buffer_size_;
	video->inherit_super.data_size   = data_size_;
	video->inherit_super.is_non_copy = non_copy_mode;
	video->inherit_super.pts         = pts;
	video->inherit_super.timebase    = timebase;
	video->inherit_super.type        = frame_type;
	if(non_copy_mode) {
		video->inherit_super.data = data;
	}
	else {
		if(video->inherit_super.data == NULL) {
			video->inherit_super.data = malloc(data_size);
			if(video->inherit_super.data == NULL) {
				ERR_PRINT("failed to allocate memory for data.");
				if(prev_frame == NULL) {
					free(video);
				}
				return NULL;
			}
		}
		memcpy(video->inherit_super.data, data, data_size);
	}
	return video;
}

/**
 * close frame(use internal)
 * @param frame
 */
void ttLibC_Video_close_(ttLibC_Video **frame) {
	ttLibC_Video *target = *frame;
	if(target == NULL) {
		return;
	}
	if(!target->inherit_super.is_non_copy) {
		free(target->inherit_super.data);
	}
	free(target);
	*frame = NULL;
}

/*
 * close frame
 * @param frame
 */
void ttLibC_Video_close(ttLibC_Video **frame) {
	ttLibC_Video *target = *frame;
	if(target == NULL) {
		return;
	}
	switch(target->inherit_super.type) {
	case frameType_bgr:
		ttLibC_Bgr_close((ttLibC_Bgr **)frame);
		break;
	case frameType_h264:
		ttLibC_H264_close((ttLibC_H264 **)frame);
		break;
	case frameType_yuv420:
		ttLibC_Yuv420_close((ttLibC_Yuv420 **)frame);
		break;
	case frameType_vp8:
		ttLibC_Vp8_close((ttLibC_Vp8 **)frame);
		break;
	case frameType_flv1:
	case frameType_wmv1:
		{
			ttLibC_Video_close_(frame);
		}
		break;
	default:
		ERR_PRINT("unknown type:%d", target->inherit_super.type);
		break;
	}
}

