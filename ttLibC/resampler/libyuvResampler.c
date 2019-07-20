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
	yuv->inherit_super.inherit_super.id = src_frame->inherit_super.inherit_super.id;
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
	yuv->inherit_super.inherit_super.id = src_frame->inherit_super.inherit_super.id;
	return yuv;
}

ttLibC_Bgr TT_ATTRIBUTE_API *ttLibC_LibyuvResampler_ToBgr(
		ttLibC_Bgr *prev_frame,
		ttLibC_Yuv420 *src_frame,
		ttLibC_Bgr_Type bgr_type) {
	if(src_frame == NULL) {
		return NULL;
	}
	switch(src_frame->type) {
	case Yuv420Type_planar:
	case Yvu420Type_planar:
		switch(bgr_type) {
		case BgrType_bgra:
		case BgrType_argb:
		case BgrType_rgba:
		case BgrType_abgr:
		case BgrType_bgr:
			break;
		default:
			return NULL;
		}
		break;
	case Yuv420Type_semiPlanar:
	case Yvu420Type_semiPlanar:
		switch(bgr_type) {
		case BgrType_bgra:
		case BgrType_rgba:
			break;
		default:
			return NULL;
		}
		break;
	}
	ttLibC_Bgr *bgr = ttLibC_Bgr_makeEmptyFrame2(
		prev_frame,
		bgr_type,
		src_frame->inherit_super.width,
		src_frame->inherit_super.height);
	if(bgr == NULL) {
		return NULL;
	}
	switch(src_frame->type) {
	case Yvu420Type_planar:
	case Yuv420Type_planar:
		switch(bgr_type) {
		case BgrType_bgra:
			I420ToARGB(
				src_frame->y_data,
				src_frame->y_stride,
				src_frame->u_data,
				src_frame->u_stride,
				src_frame->v_data,
				src_frame->v_stride,
				bgr->data,
				bgr->width_stride,
				src_frame->inherit_super.width,
				src_frame->inherit_super.height);
			return bgr;
		case BgrType_argb:
			I420ToBGRA(
				src_frame->y_data,
				src_frame->y_stride,
				src_frame->u_data,
				src_frame->u_stride,
				src_frame->v_data,
				src_frame->v_stride,
				bgr->data,
				bgr->width_stride,
				src_frame->inherit_super.width,
				src_frame->inherit_super.height);
			return bgr;
		case BgrType_rgba:
			I420ToABGR(
				src_frame->y_data,
				src_frame->y_stride,
				src_frame->u_data,
				src_frame->u_stride,
				src_frame->v_data,
				src_frame->v_stride,
				bgr->data,
				bgr->width_stride,
				src_frame->inherit_super.width,
				src_frame->inherit_super.height);
			return bgr;
		case BgrType_abgr:
			I420ToRGBA(
				src_frame->y_data,
				src_frame->y_stride,
				src_frame->u_data,
				src_frame->u_stride,
				src_frame->v_data,
				src_frame->v_stride,
				bgr->data,
				bgr->width_stride,
				src_frame->inherit_super.width,
				src_frame->inherit_super.height);
			return bgr;
		case BgrType_bgr:
		default:
			break;
		}
		break;
	case Yuv420Type_semiPlanar:
		switch(bgr_type) {
		case BgrType_bgra:
			NV12ToARGB(
				src_frame->y_data,
				src_frame->y_stride,
				src_frame->u_data,
				src_frame->u_stride,
				bgr->data,
				bgr->width_stride,
				src_frame->inherit_super.width,
				src_frame->inherit_super.height);
			return bgr;
		case BgrType_rgba:
			NV12ToABGR(
				src_frame->y_data,
				src_frame->y_stride,
				src_frame->u_data,
				src_frame->u_stride,
				bgr->data,
				bgr->width_stride,
				src_frame->inherit_super.width,
				src_frame->inherit_super.height);
			return bgr;
		default:
			break;
		}
		break;
	case Yvu420Type_semiPlanar:
		switch(bgr_type) {
		case BgrType_bgra:
			NV21ToARGB(
				src_frame->y_data,
				src_frame->y_stride,
				src_frame->v_data,
				src_frame->v_stride,
				bgr->data,
				bgr->width_stride,
				src_frame->inherit_super.width,
				src_frame->inherit_super.height);
			return bgr;
		case BgrType_rgba:
			NV21ToABGR(
				src_frame->y_data,
				src_frame->y_stride,
				src_frame->v_data,
				src_frame->v_stride,
				bgr->data,
				bgr->width_stride,
				src_frame->inherit_super.width,
				src_frame->inherit_super.height);
			return bgr;
		default:
			break;
		}
		break;
	}
	ttLibC_Bgr_close(&bgr);
	return NULL;
}

ttLibC_Yuv420 TT_ATTRIBUTE_API *ttLibC_LibyuvResampler_ToYuv420(
		ttLibC_Yuv420 *prev_frame,
		ttLibC_Bgr *src_frame,
		ttLibC_Yuv420_Type yuv420_type) {
	if(src_frame == NULL) {
		return NULL;
	}
	switch(src_frame->type) {
	case BgrType_bgra:
		break;
	case BgrType_rgba:
	case BgrType_abgr:
	case BgrType_argb:
	case BgrType_bgr:
		switch(yuv420_type) {
		case Yuv420Type_planar:
		case Yvu420Type_planar:
			break;
		case Yuv420Type_semiPlanar:
		case Yvu420Type_semiPlanar:
		default:
			return NULL;
		}
		break;
		break;
	case BgrType_rgb:
		return NULL;
	default:
		break;
	}
	ttLibC_Yuv420 *yuv = ttLibC_Yuv420_makeEmptyFrame2(
		prev_frame,
		yuv420_type,
		src_frame->inherit_super.width,
		src_frame->inherit_super.height);
	if(yuv == NULL) {
		return NULL;
	}
	switch(src_frame->type) {
	case BgrType_bgra:
		switch(yuv420_type) {
		case Yuv420Type_planar:
		case Yvu420Type_planar:
			ARGBToI420(
				src_frame->data,
				src_frame->width_stride,
				yuv->y_data,
				yuv->y_stride,
				yuv->u_data,
				yuv->u_stride,
				yuv->v_data,
				yuv->v_stride,
				src_frame->inherit_super.width,
				src_frame->inherit_super.height);
			return yuv;
		case Yuv420Type_semiPlanar:
			ARGBToNV12(
				src_frame->data,
				src_frame->width_stride,
				yuv->y_data,
				yuv->y_stride,
				yuv->u_data,
				yuv->u_stride,
				src_frame->inherit_super.width,
				src_frame->inherit_super.height);
			return yuv;
		case Yvu420Type_semiPlanar:
			ARGBToNV21(
				src_frame->data,
				src_frame->width_stride,
				yuv->y_data,
				yuv->y_stride,
				yuv->v_data,
				yuv->v_stride,
				src_frame->inherit_super.width,
				src_frame->inherit_super.height);
			return yuv;
		default:
			break;
		}
		break;
	case BgrType_rgba:
		switch(yuv420_type) {
		case Yuv420Type_planar:
		case Yvu420Type_planar:
			ABGRToI420(
				src_frame->data,
				src_frame->width_stride,
				yuv->y_data,
				yuv->y_stride,
				yuv->u_data,
				yuv->u_stride,
				yuv->v_data,
				yuv->v_stride,
				src_frame->inherit_super.width,
				src_frame->inherit_super.height);
			return yuv;
		case Yuv420Type_semiPlanar:
		case Yvu420Type_semiPlanar:
		default:
			break;
		}
		break;
	case BgrType_abgr:
		switch(yuv420_type) {
		case Yuv420Type_planar:
		case Yvu420Type_planar:
			RGBAToI420(
				src_frame->data,
				src_frame->width_stride,
				yuv->y_data,
				yuv->y_stride,
				yuv->u_data,
				yuv->u_stride,
				yuv->v_data,
				yuv->v_stride,
				src_frame->inherit_super.width,
				src_frame->inherit_super.height);
			return yuv;
		case Yuv420Type_semiPlanar:
		case Yvu420Type_semiPlanar:
		default:
			break;
		}
		break;
	case BgrType_argb:
		switch(yuv420_type) {
		case Yuv420Type_planar:
		case Yvu420Type_planar:
			BGRAToI420(
				src_frame->data,
				src_frame->width_stride,
				yuv->y_data,
				yuv->y_stride,
				yuv->u_data,
				yuv->u_stride,
				yuv->v_data,
				yuv->v_stride,
				src_frame->inherit_super.width,
				src_frame->inherit_super.height);
			return yuv;
		case Yuv420Type_semiPlanar:
		case Yvu420Type_semiPlanar:
		default:
			break;
		}
		break;
	case BgrType_bgr:
		switch(yuv420_type) {
		case Yuv420Type_planar:
		case Yvu420Type_planar:
			RGB24ToI420(
				src_frame->data,
				src_frame->width_stride,
				yuv->y_data,
				yuv->y_stride,
				yuv->u_data,
				yuv->u_stride,
				yuv->v_data,
				yuv->v_stride,
				src_frame->inherit_super.width,
				src_frame->inherit_super.height);
			return yuv;
		case Yuv420Type_semiPlanar:
		case Yvu420Type_semiPlanar:
		default:
			break;
		}
		break;
	case BgrType_rgb:
	default:
		break;
	}
	ttLibC_Yuv420_close(&yuv);
	return NULL;
}

#endif
