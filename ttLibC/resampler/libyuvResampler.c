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

bool TT_ATTRIBUTE_API ttLibC_LibyuvResampler_resize(
		ttLibC_Video *dest,
		ttLibC_Video *src,
		ttLibC_LibyuvFilter_Mode mode,
		ttLibC_LibyuvFilter_Mode sub_mode) {
	if(dest == NULL
	|| src  == NULL) {
		return false;
	}
	if(src->inherit_super.type != dest->inherit_super.type) {
		return false;
	}
	FilterModeEnum fmode;
	FilterModeEnum fsubmode;
	switch(mode) {
	case LibyuvFilter_None:
	default:
		fmode = kFilterNone;
		break;
	case LibyuvFilter_Linear:
		fmode = kFilterLinear;
		break;
	case LibyuvFilter_Bilinear:
		fmode = kFilterBilinear;
		break;
	case LibyuvFilter_Box:
		fmode = kFilterBox;
		break;
	}
	switch(sub_mode) {
	case LibyuvFilter_None:
	default:
		fsubmode = kFilterNone;
		break;
	case LibyuvFilter_Linear:
		fsubmode = kFilterLinear;
		break;
	case LibyuvFilter_Bilinear:
		fsubmode = kFilterBilinear;
		break;
	case LibyuvFilter_Box:
		fsubmode = kFilterBox;
		break;
	}
	if(src->inherit_super.type == frameType_yuv420) {
		ttLibC_Yuv420 *src_  = (ttLibC_Yuv420 *)src;
		ttLibC_Yuv420 *dest_ = (ttLibC_Yuv420 *)dest;
		if((src_->type == Yuv420Type_planar
		||  src_->type == Yvu420Type_planar)
		&& (dest_->type == Yuv420Type_planar
		||  dest_->type == Yvu420Type_planar)) {
			ScalePlane(
				src_->y_data,
				src_->y_stride,
				src->width,
				src->height,
				dest_->y_data,
				dest_->y_stride,
				dest->width,
				dest->height,
				fmode);
			ScalePlane(
				src_->u_data,
				src_->u_stride,
				(src->width + 1) >> 1,
				(src->height + 1) >> 1,
				dest_->u_data,
				dest_->u_stride,
				(dest->width + 1) >> 1,
				(dest->height + 1) >> 1,
				fsubmode);
			ScalePlane(
				src_->v_data,
				src_->v_stride,
				(src->width + 1) >> 1,
				(src->height + 1) >> 1,
				dest_->v_data,
				dest_->v_stride,
				(dest->width + 1) >> 1,
				(dest->height + 1) >> 1,
				fsubmode);
			return true;
		}
		if(src_->type  == Yuv420Type_semiPlanar
		&& dest_->type == Yuv420Type_semiPlanar) {
			ScalePlane(
				src_->y_data,
				src_->y_stride,
				src->width,
				src->height,
				dest_->y_data,
				dest_->y_stride,
				dest->width,
				dest->height,
				fmode);
			ScalePlane_16(
				(uint16_t *)src_->u_data,
				src_->u_stride,
				((src->width + 1) >> 1),
				((src->height + 1) >> 1),
				(uint16_t *)dest_->u_data,
				dest_->u_stride,
				((dest->width + 1) >> 1),
				((dest->height + 1) >> 1),
				fsubmode);
			return true;
		}
		if(src_->type  == Yvu420Type_semiPlanar
		&& dest_->type == Yvu420Type_semiPlanar) {
			ScalePlane(
				src_->y_data,
				src_->y_stride,
				src->width,
				src->height,
				dest_->y_data,
				dest_->y_stride,
				dest->width,
				dest->height,
				fmode);
			ScalePlane_16(
				(uint16_t *)src_->v_data,
				src_->v_stride,
				((src->width + 1) >> 1),
				((src->height + 1) >> 1),
				(uint16_t *)dest_->v_data,
				dest_->v_stride,
				((dest->width + 1) >> 1),
				((dest->height + 1) >> 1),
				fsubmode);
			return true;
		}
	}
	else if(src->inherit_super.type == frameType_bgr) {
		ttLibC_Bgr *src_ = (ttLibC_Bgr *)src;
		ttLibC_Bgr *dest_ = (ttLibC_Bgr *)dest;
		if(src_->type == dest_->type) {
			if(src_->type == BgrType_abgr
			|| src_->type == BgrType_bgra
			|| src_->type == BgrType_argb
			|| src_->type == BgrType_rgba) {
				ARGBScale(
					src_->data,
					src_->width_stride,
					src->width,
					src->height,
					dest_->data,
					dest_->width_stride,
					dest->width,
					dest->height,
					fmode);
				return true;
			}
		}
	}
	return false;
}

bool TT_ATTRIBUTE_API ttLibC_LibyuvResampler_rotate(
		ttLibC_Video *dest,
		ttLibC_Video *src,
		ttLibC_LibyuvRotate_Mode mode) {
	if(dest == NULL
	|| src  == NULL) {
		return false;
	}
	if(src->inherit_super.type != dest->inherit_super.type) {
		return false;
	}
	RotationModeEnum mode_;
	switch(mode) {
	case LibyuvRotate_0:
		mode_ = kRotate0;
		if(src->width  != dest->width
		|| src->height != dest->height) {
			return false;
		}
		break;
	case LibyuvRotate_180:
		mode_ = kRotate180;
		if(src->width  != dest->width
		|| src->height != dest->height) {
			return false;
		}
		break;
	case LibyuvRotate_90:
		mode_ = kRotate90;
		if(src->width  != dest->height
		|| src->height != dest->width) {
			return false;
		}
		break;
	case LibyuvRotate_270:
		mode_ = kRotate270;
		if(src->width  != dest->height
		|| src->height != dest->width) {
			return false;
		}
		break;
	}
	if(src->inherit_super.type  == frameType_yuv420) {
		ttLibC_Yuv420 *src_ = (ttLibC_Yuv420 *)src;
		ttLibC_Yuv420 *dest_ = (ttLibC_Yuv420 *)dest;
		// yuv planar -> yuv planar
		if((src_->type == Yuv420Type_planar
		||  src_->type == Yvu420Type_planar)
		&& (dest_->type == Yuv420Type_planar
		||  dest_->type == Yvu420Type_planar)) {
			I420Rotate(
				src_->y_data,
				src_->y_stride,
				src_->u_data,
				src_->u_stride,
				src_->v_data,
				src_->v_stride,
				dest_->y_data,
				dest_->y_stride,
				dest_->u_data,
				dest_->u_stride,
				dest_->v_data,
				dest_->v_stride,
				src->width,
				src->height,
				mode_);
			return true;
		}
		// nv12 -> yuv planar
		if(src_->type == Yuv420Type_semiPlanar
		&& (dest_->type == Yuv420Type_planar
		||  dest_->type == Yvu420Type_planar)) {
			NV12ToI420Rotate(
				src_->y_data,
				src_->y_stride,
				src_->u_data,
				src_->u_stride,
				dest_->y_data,
				dest_->y_stride,
				dest_->u_data,
				dest_->u_stride,
				dest_->v_data,
				dest_->v_stride,
				src->width,
				src->height,
				mode_);
			return true;
		}
	}
	else if(src->inherit_super.type  == frameType_bgr) {
		// bgra -> bgra
		ttLibC_Bgr *src_  = (ttLibC_Bgr *)src;
		ttLibC_Bgr *dest_ = (ttLibC_Bgr *)dest;
		if((src_->type == BgrType_bgra && dest_->type == BgrType_bgra)
		|| (src_->type == BgrType_rgba && dest_->type == BgrType_rgba)
		|| (src_->type == BgrType_abgr && dest_->type == BgrType_abgr)
		|| (src_->type == BgrType_argb && dest_->type == BgrType_argb)) {
			ARGBRotate(
				src_->data,
				src_->width_stride,
				dest_->data,
				dest_->width_stride,
				src->width,
				src->height,
				mode_);
			return true;
		}
	}
	return false;
}

bool TT_ATTRIBUTE_API ttLibC_LibyuvResampler_ToBgr(
		ttLibC_Bgr   *dest,
		ttLibC_Video *src) {
	if(dest == NULL || src == NULL) {
		return false;
	}
	if(dest->inherit_super.width  != src->width
	|| dest->inherit_super.height != src->height) {
		return false;
	}
	if(src->inherit_super.type == frameType_bgr) {
		ttLibC_Bgr *src_ = (ttLibC_Bgr *)src;
		switch(src_->type) {
		case BgrType_bgra:
			switch(dest->type) {
			case BgrType_bgra:
				ARGBToARGB(
					src_->data,
					src_->width_stride,
					dest->data,
					dest->width_stride,
					src->width,
					src->height);
				return true;
			case BgrType_argb:
				ARGBToBGRA(
					src_->data,
					src_->width_stride,
					dest->data,
					dest->width_stride,
					src->width,
					src->height);
				return true;
			case BgrType_rgba:
				ARGBToABGR(
					src_->data,
					src_->width_stride,
					dest->data,
					dest->width_stride,
					src->width,
					src->height);
				return true;
			case BgrType_abgr:
				ARGBToRGBA(
					src_->data,
					src_->width_stride,
					dest->data,
					dest->width_stride,
					src->width,
					src->height);
				return true;
			case BgrType_bgr:
				ARGBToRGB24(
					src_->data,
					src_->width_stride,
					dest->data,
					dest->width_stride,
					src->width,
					src->height);
				return true;
			default:
				break;
			}
			break;
		case BgrType_argb:
			if(dest->type == BgrType_bgra) {
				BGRAToARGB(
					src_->data,
					src_->width_stride,
					dest->data,
					dest->width_stride,
					src->width,
					src->height);
				return true;
			}
			break;
		case BgrType_rgba:
			if(dest->type == BgrType_bgra) {
				ABGRToARGB(
					src_->data,
					src_->width_stride,
					dest->data,
					dest->width_stride,
					src->width,
					src->height);
				return true;
			}
			break;
		case BgrType_abgr:
			if(dest->type == BgrType_bgra) {
				RGBAToARGB(
					src_->data,
					src_->width_stride,
					dest->data,
					dest->width_stride,
					src->width,
					src->height);
				return true;
			}
			break;
		case BgrType_bgr:
			if(dest->type == BgrType_bgra) {
				RGB24ToARGB(
					src_->data,
					src_->width_stride,
					dest->data,
					dest->width_stride,
					src->width,
					src->height);
				return true;
			}
			break;
		default:
			break;
		}
	}
	else if(src->inherit_super.type == frameType_yuv420) {
		ttLibC_Yuv420 *src_ = (ttLibC_Yuv420 *)src;
		switch(src_->type) {
		case Yuv420Type_planar:
		case Yvu420Type_planar:
			switch(dest->type) {
			case BgrType_bgra:
				I420ToARGB(
					src_->y_data,
					src_->y_stride,
					src_->u_data,
					src_->u_stride,
					src_->v_data,
					src_->v_stride,
					dest->data,
					dest->width_stride,
					src->width,
					src->height);
				return true;
			case BgrType_argb:
				I420ToBGRA(
					src_->y_data,
					src_->y_stride,
					src_->u_data,
					src_->u_stride,
					src_->v_data,
					src_->v_stride,
					dest->data,
					dest->width_stride,
					src->width,
					src->height);
				return true;
			case BgrType_rgba:
				I420ToABGR(
					src_->y_data,
					src_->y_stride,
					src_->u_data,
					src_->u_stride,
					src_->v_data,
					src_->v_stride,
					dest->data,
					dest->width_stride,
					src->width,
					src->height);
				return true;
			case BgrType_abgr:
				I420ToRGBA(
					src_->y_data,
					src_->y_stride,
					src_->u_data,
					src_->u_stride,
					src_->v_data,
					src_->v_stride,
					dest->data,
					dest->width_stride,
					src->width,
					src->height);
				return true;
			case BgrType_bgr:
				I420ToRGB24(
					src_->y_data,
					src_->y_stride,
					src_->u_data,
					src_->u_stride,
					src_->v_data,
					src_->v_stride,
					dest->data,
					dest->width_stride,
					src->width,
					src->height);
				return true;
			default:
				break;
			}
			break;
		case Yuv420Type_semiPlanar:
			switch(dest->type) {
			case BgrType_bgra:
				NV12ToARGB(
					src_->y_data,
					src_->y_stride,
					src_->v_data,
					src_->v_stride,
					dest->data,
					dest->width_stride,
					src->width,
					src->height);
				return true;
			case BgrType_rgba:
				NV12ToABGR(
					src_->y_data,
					src_->y_stride,
					src_->v_data,
					src_->v_stride,
					dest->data,
					dest->width_stride,
					src->width,
					src->height);
				return true;
			default:
				break;
			}
			break;
		case Yvu420Type_semiPlanar:
			switch(dest->type) {
			case BgrType_bgra:
				NV21ToARGB(
					src_->y_data,
					src_->y_stride,
					src_->v_data,
					src_->v_stride,
					dest->data,
					dest->width_stride,
					src->width,
					src->height);
				return true;
			case BgrType_rgba:
				NV21ToABGR(
					src_->y_data,
					src_->y_stride,
					src_->v_data,
					src_->v_stride,
					dest->data,
					dest->width_stride,
					src->width,
					src->height);
				return true;
			default:
				break;
			}
			break;
		}
	}
	return false;
}

bool TT_ATTRIBUTE_API ttLibC_LibyuvResampler_ToYuv420(
		ttLibC_Yuv420 *dest,
		ttLibC_Video  *src) {
	if(dest == NULL || src == NULL) {
		return false;
	}
	if(dest->inherit_super.width  != src->width
	|| dest->inherit_super.height != src->height) {
		return false;
	}
	if(src->inherit_super.type == frameType_bgr) {
		ttLibC_Bgr *src_ = (ttLibC_Bgr *)src;
		switch(src_->type) {
		case BgrType_bgra:
			switch(dest->type) {
			case Yuv420Type_planar:
			case Yvu420Type_planar:
				ARGBToI420(
					src_->data,
					src_->width_stride,
					dest->y_data,
					dest->y_stride,
					dest->u_data,
					dest->u_stride,
					dest->v_data,
					dest->v_stride,
					src->width,
					src->height);
				return true;
			case Yuv420Type_semiPlanar:
				ARGBToNV12(
					src_->data,
					src_->width_stride,
					dest->y_data,
					dest->y_stride,
					dest->u_data,
					dest->u_stride,
					src->width,
					src->height);
				return true;
			case Yvu420Type_semiPlanar:
				ARGBToNV21(
					src_->data,
					src_->width_stride,
					dest->y_data,
					dest->y_stride,
					dest->v_data,
					dest->v_stride,
					src->width,
					src->height);
				return true;
			default:
				break;
			}
			break;
		case BgrType_argb:
			if(dest->type == Yuv420Type_planar
			|| dest->type == Yvu420Type_planar) {
				BGRAToI420(
					src_->data,
					src_->width_stride,
					dest->y_data,
					dest->y_stride,
					dest->u_data,
					dest->u_stride,
					dest->v_data,
					dest->v_stride,
					src->width,
					src->height);
				return true;
			}
			break;
		case BgrType_rgba:
			if(dest->type == Yuv420Type_planar
			|| dest->type == Yvu420Type_planar) {
				ABGRToI420(
					src_->data,
					src_->width_stride,
					dest->y_data,
					dest->y_stride,
					dest->u_data,
					dest->u_stride,
					dest->v_data,
					dest->v_stride,
					src->width,
					src->height);
				return true;
			}
			break;
		case BgrType_abgr:
			if(dest->type == Yuv420Type_planar
			|| dest->type == Yvu420Type_planar) {
				RGBAToI420(
					src_->data,
					src_->width_stride,
					dest->y_data,
					dest->y_stride,
					dest->u_data,
					dest->u_stride,
					dest->v_data,
					dest->v_stride,
					src->width,
					src->height);
				return true;
			}
			break;
		case BgrType_bgr:
			if(dest->type == Yuv420Type_planar
			|| dest->type == Yvu420Type_planar) {
				RGB24ToI420(
					src_->data,
					src_->width_stride,
					dest->y_data,
					dest->y_stride,
					dest->u_data,
					dest->u_stride,
					dest->v_data,
					dest->v_stride,
					src->width,
					src->height);
				return true;
			}
			break;
		default:
			break;
		}
	}
	else if(src->inherit_super.type == frameType_yuv420) {
		ttLibC_Yuv420 *src_ = (ttLibC_Yuv420 *)src;
		switch(src_->type) {
		case Yuv420Type_planar:
		case Yvu420Type_planar:
			switch(dest->type) {
			case Yuv420Type_planar:
			case Yvu420Type_planar:
				I420ToI420(
					src_->y_data,
					src_->y_stride,
					src_->u_data,
					src_->u_stride,
					src_->v_data,
					src_->v_stride,
					dest->y_data,
					dest->y_stride,
					dest->u_data,
					dest->u_stride,
					dest->v_data,
					dest->v_stride,
					src->width,
					src->height);
				return true;
			case Yuv420Type_semiPlanar:
				I420ToNV12(
					src_->y_data,
					src_->y_stride,
					src_->u_data,
					src_->u_stride,
					src_->v_data,
					src_->v_stride,
					dest->y_data,
					dest->y_stride,
					dest->u_data,
					dest->u_stride,
					src->width,
					src->height);
				return true;
			case Yvu420Type_semiPlanar:
				I420ToNV21(
					src_->y_data,
					src_->y_stride,
					src_->u_data,
					src_->u_stride,
					src_->v_data,
					src_->v_stride,
					dest->y_data,
					dest->y_stride,
					dest->v_data,
					dest->v_stride,
					src->width,
					src->height);
				return true;
			default:
				break;
			}
			break;
		case Yuv420Type_semiPlanar:
			if(dest->type == Yuv420Type_planar
			|| dest->type == Yvu420Type_planar) {
				NV12ToI420(
					src_->y_data,
					src_->y_stride,
					src_->u_data,
					src_->u_stride,
					dest->y_data,
					dest->y_stride,
					dest->u_data,
					dest->u_stride,
					dest->v_data,
					dest->v_stride,
					src->width,
					src->height);
				return true;
			}
			break;
		case Yvu420Type_semiPlanar:
			if(dest->type == Yuv420Type_planar
			|| dest->type == Yvu420Type_planar) {
				NV21ToI420(
					src_->y_data,
					src_->y_stride,
					src_->v_data,
					src_->v_stride,
					dest->y_data,
					dest->y_stride,
					dest->u_data,
					dest->u_stride,
					dest->v_data,
					dest->v_stride,
					src->width,
					src->height);
				return true;
			}
			break;
		default:
			break;
		}
	}
	return false;
}



















ttLibC_Yuv420 TT_ATTRIBUTE_API *ttLibC_LibyuvResampler_resize_(
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

ttLibC_Yuv420 TT_ATTRIBUTE_API *ttLibC_LibyuvResampler_rotate_(
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
	ttLibC_Yuv420 *yuv = NULL;
	if(prev_frame != NULL
	&& prev_frame->inherit_super.width == width
	&& prev_frame->inherit_super.height == height) {
		yuv = prev_frame;
	}
	else {
		yuv = ttLibC_Yuv420_makeEmptyFrame2(
				prev_frame,
				Yuv420Type_planar,
				width,
				height);
	}
	if(yuv == NULL) {
		ERR_PRINT("failed to make dst frame.");
		return NULL;
	}
	yuv->inherit_super.inherit_super.id       = src_frame->inherit_super.inherit_super.id;
	yuv->inherit_super.inherit_super.pts      = src_frame->inherit_super.inherit_super.pts;
	yuv->inherit_super.inherit_super.timebase = src_frame->inherit_super.inherit_super.timebase;
	switch(src_frame->type) {
	case Yuv420Type_planar:
	case Yvu420Type_planar:
	default:
		switch(yuv->type) {
		case Yuv420Type_planar:
		case Yvu420Type_planar:
		default:
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
			break;
		case Yuv420Type_semiPlanar:
		case Yvu420Type_semiPlanar:
			switch(rotate) {
			case kRotate0:
			default:
				break;
			case kRotate90:
				break;
			case kRotate180:
			case kRotate270:
				break;
			}
			break;
		}
		break;
	case Yuv420Type_semiPlanar:
	case Yvu420Type_semiPlanar:
		switch(yuv->type) {
		case Yuv420Type_planar:
		case Yvu420Type_planar:
		default:
			break;
		case Yuv420Type_semiPlanar:
		case Yvu420Type_semiPlanar:
			switch(rotate) {
			case kRotate0:
			default:
				break;
			case kRotate90:
				RotatePlane90(
					src_frame->y_data,
					src_frame->y_stride,
					yuv->y_data,
					yuv->y_stride,
					src_frame->inherit_super.width,
					src_frame->inherit_super.height);
				RotateUV90(
					src_frame->u_data,
					src_frame->u_stride,
					yuv->v_data,
					yuv->v_stride,
					yuv->v_data,
					yuv->v_stride,
					src_frame->inherit_super.width / 2,
					src_frame->inherit_super.height / 2);
				break;
			case kRotate180:
			case kRotate270:
				break;
			}
			break;
		}
		break;
	}
	return yuv;
}

ttLibC_Bgr TT_ATTRIBUTE_API *ttLibC_LibyuvResampler_ToBgr_(
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

ttLibC_Yuv420 TT_ATTRIBUTE_API *ttLibC_LibyuvResampler_ToYuv420_(
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
