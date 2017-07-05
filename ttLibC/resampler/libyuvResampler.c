/*
 * libyuvResampler.c
 *
 *  Created on: 2017/04/27
 *      Author: taktod
 */

#ifdef __ENABLE_LIBYUV__

#include "libyuvResampler.h"
#include "../allocator.h"

#include <libyuv.h>

ttLibC_Yuv420 *ttLibC_LibyuvResampler_resize(
		ttLibC_Yuv420 *prev_frame,
		uint32_t width,
		uint32_t height,
		ttLibC_Yuv420 *src_frame,
		ttLibC_LibyuvFilter_Mode y_mode,
		ttLibC_LibyuvFilter_Mode u_mode,
		ttLibC_LibyuvFilter_Mode v_mode) {
	if(src_frame == NULL) {
		return NULL;
	}
	if(prev_frame != NULL && prev_frame->inherit_super.inherit_super.type != frameType_yuv420) {
		ttLibC_Frame_close((ttLibC_Frame **)&prev_frame);
	}
	ttLibC_Yuv420 *yuv = prev_frame;
	uint32_t f_stride = ((((width - 1) >> 4) + 1) << 4);
	uint32_t h_stride = (((((width >> 1) - 1) >> 4) + 1) << 4);
	uint32_t y_size = f_stride * height;
	uint32_t u_size = h_stride * (height >> 1);
	uint32_t v_size = h_stride * (height >> 1);
	uint32_t data_size = y_size + u_size + v_size;
	bool alloc_flag = false;
	uint8_t *data = NULL;
	if(yuv != NULL) {
		if(!yuv->inherit_super.inherit_super.is_non_copy) {
			if(yuv->inherit_super.inherit_super.data_size >= data_size) {
				data = yuv->inherit_super.inherit_super.data;
				data_size = yuv->inherit_super.inherit_super.data_size;
			}
			else {
				ttLibC_free(yuv->inherit_super.inherit_super.data);
			}
		}
		if(data == NULL) {
			yuv->inherit_super.inherit_super.data = NULL;
			yuv->inherit_super.inherit_super.data_size = 0;
		}
		yuv->inherit_super.inherit_super.is_non_copy = true;
	}
	if(data == NULL) {
		data = ttLibC_malloc(data_size);
		if(data == NULL) {
			return NULL;
		}
		alloc_flag = true;
	}
	FilterModeEnum filter = kFilterNone;
	switch(y_mode) {
	default:
	case LibyuvFilter_None:
		break;
	case LibyuvFilter_Linear:
		filter = kFilterLinear;
		break;
	case LibyuvFilter_Bilinear:
		filter = kFilterBilinear;
		break;
	case LibyuvFilter_Box:
		filter = kFilterBox;
		break;
	}
	ScalePlane(src_frame->y_data,
			src_frame->y_stride,
			src_frame->inherit_super.width,
			src_frame->inherit_super.height,
			data,
			f_stride,
			width,
			height,
			filter);
	switch(u_mode) {
	default:
	case LibyuvFilter_None:
		filter = kFilterNone;
		break;
	case LibyuvFilter_Linear:
		filter = kFilterLinear;
		break;
	case LibyuvFilter_Bilinear:
		filter = kFilterBilinear;
		break;
	case LibyuvFilter_Box:
		filter = kFilterBox;
		break;
	}
	ScalePlane(src_frame->u_data,
			src_frame->u_stride,
			src_frame->inherit_super.width  / 2,
			src_frame->inherit_super.height / 2,
			data + y_size,
			h_stride,
			width / 2,
			height / 2,
			filter);
	switch(v_mode) {
	default:
	case LibyuvFilter_None:
		filter = kFilterNone;
		break;
	case LibyuvFilter_Linear:
		filter = kFilterLinear;
		break;
	case LibyuvFilter_Bilinear:
		filter = kFilterBilinear;
		break;
	case LibyuvFilter_Box:
		filter = kFilterBox;
		break;
	}
	ScalePlane(src_frame->v_data,
			src_frame->v_stride,
			src_frame->inherit_super.width  / 2,
			src_frame->inherit_super.height / 2,
			data + y_size + u_size,
			h_stride,
			width / 2,
			height / 2,
			filter);
	yuv = ttLibC_Yuv420_make(
			yuv,
			Yuv420Type_planar,
			width,
			height,
			data,
			data_size,
			data,
			f_stride,
			data + y_size,
			h_stride,
			data + y_size + u_size,
			h_stride,
			true,
			src_frame->inherit_super.inherit_super.pts,
			src_frame->inherit_super.inherit_super.timebase);
	if(yuv == NULL) {
		if(alloc_flag) {
			ttLibC_free(data);
		}
		return NULL;
	}
	yuv->inherit_super.inherit_super.is_non_copy = false;
	return yuv;
}

ttLibC_Yuv420 *ttLibC_LibyuvResampler_rotate(
		ttLibC_Yuv420 *prev_frame,
		ttLibC_Yuv420 *src_frame,
		ttLibC_LibyuvRotate_Mode mode) {
	if(src_frame == NULL) {
		return NULL;
	}
	if(prev_frame != NULL && prev_frame->inherit_super.inherit_super.type != frameType_yuv420) {
		ttLibC_Frame_close((ttLibC_Frame **)&prev_frame);
	}
	ttLibC_Yuv420 *yuv = prev_frame;
	uint32_t width  = src_frame->inherit_super.width;
	uint32_t height = src_frame->inherit_super.height;
	RotationModeEnum rotate = kRotate0;
	switch(mode) {
	default:
	case LibyuvRotate_0:
		break;
	case LibyuvRotate_90:
		rotate =kRotate90;
		width = src_frame->inherit_super.height;
		height = src_frame->inherit_super.width;
		break;
	case LibyuvRotate_180:
		rotate =kRotate180;
		break;
	case LibyuvRotate_270:
		rotate =kRotate270;
		width = src_frame->inherit_super.height;
		height = src_frame->inherit_super.width;
		break;
	}
	uint32_t f_stride = ((((width - 1) >> 4) + 1) << 4);
	uint32_t h_stride = (((((width >> 1) - 1) >> 4) + 1) << 4);
	uint32_t y_size = f_stride * height;
	uint32_t u_size = h_stride * (height >> 1);
	uint32_t v_size = h_stride * (height >> 1);

	uint32_t data_size = y_size + u_size + v_size;
	bool alloc_flag = false;
	uint8_t *data = NULL;
	if(yuv != NULL) {
		if(!yuv->inherit_super.inherit_super.is_non_copy) {
			if(yuv->inherit_super.inherit_super.data_size >= data_size) {
				data = yuv->inherit_super.inherit_super.data;
				data_size = yuv->inherit_super.inherit_super.data_size;
			}
			else {
				ttLibC_free(yuv->inherit_super.inherit_super.data);
			}
		}
		if(data == NULL) {
			yuv->inherit_super.inherit_super.data = NULL;
			yuv->inherit_super.inherit_super.data_size = 0;
		}
		yuv->inherit_super.inherit_super.is_non_copy = true;
	}
	if(data == NULL) {
		data = ttLibC_malloc(data_size);
		if(data == NULL) {
			return NULL;
		}
		alloc_flag = true;
	}
	I420Rotate(src_frame->y_data,
			src_frame->y_stride,
			src_frame->u_data,
			src_frame->u_stride,
			src_frame->v_data,
			src_frame->v_stride,
			data,
			f_stride,
			data + y_size,
			h_stride,
			data + y_size + u_size,
			h_stride,
			src_frame->inherit_super.width,
			src_frame->inherit_super.height,
			rotate);
	yuv = ttLibC_Yuv420_make(
			yuv,
			Yuv420Type_planar,
			width,
			height,
			data,
			data_size,
			data,
			f_stride,
			data + y_size,
			h_stride,
			data + y_size + u_size,
			h_stride,
			true,
			src_frame->inherit_super.inherit_super.pts,
			src_frame->inherit_super.inherit_super.timebase);
	if(yuv == NULL) {
		if(alloc_flag) {
			ttLibC_free(data);
		}
		return NULL;
	}
	yuv->inherit_super.inherit_super.is_non_copy = false;
	return yuv;
}

#endif
