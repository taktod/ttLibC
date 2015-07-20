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
 * @param y_step        step for each element of y_data
 * @param u_data        pointer for u_data
 * @param u_stride      stride for each line for u_data
 * @param u_step        step for each element of u_data
 * @param v_data        pointer for v_data
 * @param v_stride      stride for each line for v_data
 * @param v_step        step for each element of v_data
 * @param non_copy_mode true:hold the data pointer. false:data will copy
 * @param pts           pts for image
 * @param timebase      timebase number for pts.
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
		yuv420 = (ttLibC_Yuv420 *)malloc(sizeof(ttLibC_Yuv420));
		if(yuv420 == NULL) {
			ERR_PRINT("failed to allocate memory for yuv420 frame");
			return NULL;
		}
	}
	else {
		if(!yuv420->inherit_super.inherit_super.is_non_copy) {
			free(yuv420->inherit_super.inherit_super.data);
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
	yuv420->inherit_super.inherit_super.data_size   = data_size;
	yuv420->inherit_super.inherit_super.buffer_size = data_size;

	if(non_copy_mode) {
		yuv420->inherit_super.inherit_super.data = data;
	}
	else {
		yuv420->inherit_super.inherit_super.data = malloc(data_size);
		if(yuv420->inherit_super.inherit_super.data == NULL) {
			ERR_PRINT("failed to allocate memory for data.");
			if(prev_frame == NULL) {
				free(yuv420);
			}
			return NULL;
		}
		memcpy(yuv420->inherit_super.inherit_super.data, data, data_size);
	}
	return yuv420;
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
		free(target->inherit_super.inherit_super.data);
	}
	free(target);
	*frame = NULL;
}
