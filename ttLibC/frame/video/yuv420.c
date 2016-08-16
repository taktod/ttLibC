/*
 * @file   yuv420.c
 * @brief  yuv420 image frame information.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/18
 */

#include <string.h>
#include <stdlib.h>

#include "yuv420.h"
#include "../../log.h"
#include "../../allocator.h"

/*
 * make yuv420 frame
 * @param prev_frame    reuse frame object. if NULL, create new one.
 * @param type          type of yuv420
 * @param width         width of image
 * @param height        height of image
 * @param data          yuv420 data
 * @param data_size     data size
 * @param y_data        pointer for y_data
 * @param y_stride      stride for each line for y_data
 * @param u_data        pointer for u_data
 * @param u_stride      stride for each line for u_data
 * @param v_data        pointer for v_data
 * @param v_stride      stride for each line for v_data
 * @param non_copy_mode true:hold the data pointer. false:data will copy
 * @param pts           pts for image
 * @param timebase      timebase number for pts.
 * @return yuv420 object.
 */
ttLibC_Yuv420 *ttLibC_Yuv420_make(
		ttLibC_Yuv420 *prev_frame,
		ttLibC_Yuv420_Type type,
		uint32_t width,
		uint32_t height,
		void *data,
		size_t data_size,
		void *y_data,
		uint32_t y_stride,
		void *u_data,
		uint32_t u_stride,
		void *v_data,
		uint32_t v_stride,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	ttLibC_Yuv420 *yuv420 = prev_frame;
	size_t buffer_size_ = data_size;
	size_t data_size_ = data_size;
	uint32_t y_step = 1;
	uint32_t u_step = 1;
	uint32_t v_step = 1;
	switch(type) {
	case Yuv420Type_planar:
	case Yvu420Type_planar:
		break;
	case Yuv420Type_semiPlanar:
	case Yvu420Type_semiPlanar:
		u_step = 2;
		v_step = 2;
		break;
	default:
		ERR_PRINT("unknown yuv420 type.%d", type);
		return NULL;
	}
	if(yuv420 == NULL) {
		yuv420 = (ttLibC_Yuv420 *)ttLibC_malloc(sizeof(ttLibC_Yuv420));
		if(yuv420 == NULL) {
			ERR_PRINT("failed to allocate memory for yuv420 frame");
			return NULL;
		}
		yuv420->inherit_super.inherit_super.data = NULL;
	}
	else {
		if(!yuv420->inherit_super.inherit_super.is_non_copy) {
			if(non_copy_mode || yuv420->inherit_super.inherit_super.data_size < data_size_) {
				ttLibC_free(yuv420->inherit_super.inherit_super.data);
				yuv420->inherit_super.inherit_super.data = NULL;
			}
			else {
				data_size_ = yuv420->inherit_super.inherit_super.data_size;
			}
		}
	}
	yuv420->inherit_super.inherit_super.data = NULL;
	yuv420->inherit_super.inherit_super.data_size = 0;
	yuv420->type                                    = type;
	yuv420->y_data                                  = y_data;
	yuv420->y_stride                                = y_stride;
	yuv420->y_step                                  = y_step;
	yuv420->u_data                                  = u_data;
	yuv420->u_stride                                = u_stride;
	yuv420->u_step                                  = u_step;
	yuv420->v_data                                  = v_data;
	yuv420->v_stride                                = v_stride;
	yuv420->v_step                                  = v_step;
	yuv420->inherit_super.type                      = videoType_key;
	yuv420->inherit_super.width                     = width;
	yuv420->inherit_super.height                    = height;
	yuv420->inherit_super.inherit_super.is_non_copy = non_copy_mode;
	yuv420->inherit_super.inherit_super.pts         = pts;
	yuv420->inherit_super.inherit_super.timebase    = timebase;
	yuv420->inherit_super.inherit_super.type        = frameType_yuv420;
	yuv420->inherit_super.inherit_super.data_size   = data_size_;
	yuv420->inherit_super.inherit_super.buffer_size = buffer_size_;

	if(non_copy_mode) {
		yuv420->inherit_super.inherit_super.data = data;
	}
	else {
		if(yuv420->inherit_super.inherit_super.data == NULL) {
			yuv420->inherit_super.inherit_super.data = ttLibC_malloc(data_size);
			if(yuv420->inherit_super.inherit_super.data == NULL) {
				ERR_PRINT("failed to allocate memory for data.");
				if(prev_frame == NULL) {
					ttLibC_free(yuv420);
				}
				return NULL;
			}
		}
		memcpy(yuv420->inherit_super.inherit_super.data, data, data_size);
	}
	return yuv420;
}

ttLibC_Yuv420 *Yuv420_clonePlanar(
		ttLibC_Yuv420 *prev_frame,
		ttLibC_Yuv420 *src_frame) {
	uint32_t y_size = src_frame->inherit_super.height * src_frame->y_stride;
	uint32_t u_size = src_frame->inherit_super.height / 2 * src_frame->u_stride;
	uint32_t v_size = src_frame->inherit_super.height / 2 * src_frame->v_stride;
	uint32_t buffer_size = y_size + u_size + v_size;
	uint8_t *buffer = ttLibC_malloc(buffer_size);
	if(buffer == NULL) {
		return NULL;
	}
	memcpy(buffer, src_frame->y_data, y_size);
	memcpy(buffer + y_size, src_frame->u_data, u_size);
	memcpy(buffer + y_size + u_size, src_frame->v_data, v_size);
	ttLibC_Yuv420 *cloned_frame = ttLibC_Yuv420_make(
			prev_frame,
			Yuv420Type_planar,
			src_frame->inherit_super.width,
			src_frame->inherit_super.height,
			buffer,
			buffer_size,
			buffer,
			src_frame->y_stride,
			buffer + y_size,
			src_frame->u_stride,
			buffer + y_size + u_size,
			src_frame->v_stride,
			true,
			src_frame->inherit_super.inherit_super.pts,
			src_frame->inherit_super.inherit_super.timebase);
	if(cloned_frame != NULL) {
		cloned_frame->inherit_super.inherit_super.is_non_copy = false;
	}
	return cloned_frame;
}

ttLibC_Yuv420 *Yuv420_cloneSemiPlanar(
		ttLibC_Yuv420 *prev_frame,
		ttLibC_Yuv420 *src_frame) {
	ERR_PRINT("clone for yuv420_semiPlanar is not created");
	return NULL;
}

ttLibC_Yuv420 *Yvu420_clonePlanar(
		ttLibC_Yuv420 *prev_frame,
		ttLibC_Yuv420 *src_frame) {
	ERR_PRINT("clone for yvu420_planar is not created");
	return NULL;
}

ttLibC_Yuv420 *Yvu420_cloneSemiPlanar(
		ttLibC_Yuv420 *prev_frame,
		ttLibC_Yuv420 *src_frame) {
	ERR_PRINT("clone for yvu420_semiPlanar is not created");
	return NULL;
}

/*
 * make clone frame.
 * always make copy buffer on it.
 * @param prev_frame reuse frame object.
 * @param src_frame  source of clone.
 */
ttLibC_Yuv420 *ttLibC_Yuv420_clone(
		ttLibC_Yuv420 *prev_frame,
		ttLibC_Yuv420 *src_frame) {
	ttLibC_Yuv420 *yuv = NULL;
	switch(src_frame->type) {
	case Yuv420Type_planar:
		yuv = Yuv420_clonePlanar(prev_frame, src_frame);
		break;
	case Yuv420Type_semiPlanar:
		yuv = Yuv420_cloneSemiPlanar(prev_frame, src_frame);
		break;
	case Yvu420Type_planar:
		yuv = Yvu420_clonePlanar(prev_frame, src_frame);
		break;
	case Yvu420Type_semiPlanar:
		yuv = Yvu420_cloneSemiPlanar(prev_frame, src_frame);
		break;
	default:
		return NULL;
	}
	if(yuv != NULL) {
		yuv->inherit_super.inherit_super.id = src_frame->inherit_super.inherit_super.id;
	}
	return yuv;
}

/*
 * close frame
 * @param frame
 */
void ttLibC_Yuv420_close(ttLibC_Yuv420 **frame) {
	ttLibC_Yuv420 *target = (*frame);
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.inherit_super.type != frameType_yuv420) {
		ERR_PRINT("found non yuv420 frame in yuv420_close.");
	}
	if(!target->inherit_super.inherit_super.is_non_copy) {
		ttLibC_free(target->inherit_super.inherit_super.data);
	}
	ttLibC_free(target);
	*frame = NULL;
}
