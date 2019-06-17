/*
 * libyuvResampler.c
 *
 *  Created on: 2017/04/27
 *      Author: taktod
 */

#ifdef __ENABLE_LIBYUV__

#include "libyuvResampler.h"
#include "../allocator.h"
#include "../_log.h"

#include <libyuv.h>

ttLibC_Yuv420 TT_ATTRIBUTE_API *ttLibC_LibyuvResampler_resize(
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
	ttLibC_Yuv420 *yuv = ttLibC_Yuv420_makeEmptyFrame2(
			prev_frame,
			Yuv420Type_planar,
			width,
			height);
	if(yuv == NULL) {
		ERR_PRINT("failed to make dst frame.");
		return NULL;
	}
	yuv->inherit_super.inherit_super.pts = src_frame->inherit_super.inherit_super.pts;
	yuv->inherit_super.inherit_super.timebase = src_frame->inherit_super.inherit_super.timebase;
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
	ScalePlane(
			src_frame->y_data,
			src_frame->y_stride,
			src_frame->inherit_super.width,
			src_frame->inherit_super.height,
			yuv->y_data,
			yuv->y_stride,
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
	ScalePlane(
			src_frame->u_data,
			src_frame->u_stride,
			(src_frame->inherit_super.width  + 1) >> 1,
			(src_frame->inherit_super.height + 1) >> 1,
			yuv->u_data,
			yuv->u_stride,
			(width  + 1) >> 1,
			(height + 1) >> 1,
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
	ScalePlane(
			src_frame->v_data,
			src_frame->v_stride,
			(src_frame->inherit_super.width  + 1) >> 1,
			(src_frame->inherit_super.height + 1) >> 1,
			yuv->v_data,
			yuv->v_stride,
			(width  + 1) >> 1,
			(height + 1) >> 1,
			filter);
	return yuv;
}

ttLibC_Yuv420 TT_ATTRIBUTE_API *ttLibC_LibyuvResampler_rotate(
		ttLibC_Yuv420 *prev_frame,
		ttLibC_Yuv420 *src_frame,
		ttLibC_LibyuvRotate_Mode mode) {
	if(src_frame == NULL) {
		return NULL;
	}
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
	ttLibC_Yuv420 *yuv = ttLibC_Yuv420_makeEmptyFrame2(
			prev_frame,
			Yuv420Type_planar,
			width,
			height);
	if(yuv == NULL) {
		ERR_PRINT("failed to make dst frame.");
		return NULL;
	}
	yuv->inherit_super.inherit_super.pts = src_frame->inherit_super.inherit_super.pts;
	yuv->inherit_super.inherit_super.timebase = src_frame->inherit_super.inherit_super.timebase;
	I420Rotate(
			src_frame->y_data,
			src_frame->y_stride,
			src_frame->u_data,
			src_frame->u_stride,
			src_frame->v_data,
			src_frame->v_stride,
			yuv->y_data,
			yuv->y_stride,
			yuv->u_data,
			yuv->u_stride,
			yuv->v_data,
			yuv->v_stride,
			src_frame->inherit_super.width,
			src_frame->inherit_super.height,
			rotate);
	return yuv;
}

#endif
