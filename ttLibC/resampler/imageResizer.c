/**
 * @file  imageResizer.c
 * @brief library or image resizing.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2016/9/10
 */

#include "../allocator.h"
#include "../_log.h"
#include "imageResizer.h"

typedef struct ImageResize_point_t {
	uint32_t x;
	uint32_t y;
	uint32_t val;
} ImageResize_point_t;

/**
 * do for each plane.
 */
static bool ImageResizer_resizePlane(
		uint8_t *target_plane,
		uint32_t target_width,
		uint32_t target_height,
		uint32_t target_stride,
		uint32_t target_step,
		uint8_t *src_plane,
		uint32_t src_width,
		uint32_t src_height,
		uint32_t src_stride,
		uint32_t src_step,
		bool is_lefttop) {
	/*
	 * in case, target a x b src c x d
	 * image the field, ac x bd
	 * then calcurate point in the field from original pos.
	 * target: point for target
	 * lt:nearest left top
	 * lb:nearest left bottom
	 * rt:nearest right top
	 * rb:nearest right bottom
	 *
	 * from the distance estimate the color of point.
	 */
#define clearPoint(t) t.x = 0;t.y = 0;t.val = 0;
	ImageResize_point_t target;
	ImageResize_point_t lt, lb, rt, rb;
	clearPoint(target);
	clearPoint(lt);
	clearPoint(lb);
	clearPoint(rt);
	clearPoint(rb);
#undef clearPoint
	uint32_t lt_x = 0, lt_y = 0, rb_x = 0, rb_y = 0;
	int32_t xxx = 0, yyy = 0;
	int32_t hmax = ((src_height - 1) * target_height) << 1;
	int32_t wmax = ((src_width - 1) * target_width) << 1;
	for(uint32_t i = 0;i < target_height;++ i) { // for y
		target.y = src_height * (1 + (i << 1));
		yyy = target.y - target_height;
		if(yyy < 0) { // bound out on the top
			lt_y = 0;
			rb_y = 0;
			lt.y = 0;
			lb.y = target_height;
		}
		else if(yyy > hmax){ // bound out on the bottom
			lt_y = src_height - 1;
			rb_y = lt_y;
			lt.y = (2 * lt_y + 1) * target_height;
			lb.y = lt.y + target_height;
		}
		else { // inside
			lt_y = (yyy / target_height) >> 1;
			rb_y = lt_y + 1;
			lt.y = ((lt_y << 1) + 1) * target_height;
			lb.y = lt.y + (target_height << 1);
		}
		lt_y *= src_stride;
		rb_y *= src_stride;
		for(uint32_t j = 0, jstep = 0;j < target_width;++ j, jstep += target_step) { // for x
			target.x = src_width * (1 + (j << 1));
			xxx = target.x - target_width;
			if(xxx < 0) { // bound out on the left
				lt_x = 0;
				rb_x = 0;
				lt.x = 0;
				rt.x = target_width;
			}
			else if(xxx > wmax) { // bound out on the right
				lt_x = src_width - 1;
				rb_x = lt_x;
				lt.x = ((lt_x << 1) + 1) * target_width;
				rt.x = lt.x + target_width;
			}
			else { // inside
				lt_x = (xxx / target_width) >> 1;
				rb_x = lt_x + 1;
				lt.x = ((lt_x << 1) + 1) * target_width;
				rt.x = lt.x + (target_width << 1);
			}
			lt.val = (*(src_plane + lt_y + lt_x * src_step));
			if(is_lefttop) { // in the case to use left top value.
				(*(target_plane + jstep)) = lt.val;
			}
			else { // in the case to calculate.
				lb.val = (*(src_plane + rb_y + lt_x * src_step));
				rt.val = (*(src_plane + lt_y + rb_x * src_step));
				rb.val = (*(src_plane + rb_y + rb_x * src_step));

				// now calcurate
				uint32_t l_value = (lt.val * (lb.y - target.y) + lb.val * (target.y - lt.y)) / (lb.y - lt.y);
				uint32_t r_value = (rt.val * (lb.y - target.y) + rb.val * (target.y - lt.y)) / (lb.y - lt.y);
				(*(target_plane + jstep)) = (uint8_t)(
						(l_value * (rt.x - target.x) + r_value * (target.x - lt.x)) / (rt.x - lt.x));
			}
		}
		target_plane += target_stride;
	}
	return true;
}

bool TT_ATTRIBUTE_API ttLibC_ImageResizer_resize(
		ttLibC_Video *dest,
		ttLibC_Video *src,
		bool is_quick) {
	if(dest == NULL
	|| src  == NULL) {
		return false;
	}
	if(src->inherit_super.type != dest->inherit_super.type) {
		return false;
	}
	if(src->inherit_super.type == frameType_yuv420) {
		ttLibC_Yuv420 *src_  = (ttLibC_Yuv420 *)src;
		ttLibC_Yuv420 *dest_ = (ttLibC_Yuv420 *)dest;
		ImageResizer_resizePlane(
			dest_->y_data,
			dest_->inherit_super.width,
			dest_->inherit_super.height,
			dest_->y_stride,
			dest_->y_step,
			src_->y_data,
			src_->inherit_super.width,
			src_->inherit_super.height,
			src_->y_stride,
			src_->y_step,
			is_quick);
		ImageResizer_resizePlane(
			dest_->u_data,
			(dest_->inherit_super.width + 1) >> 1,
			(dest_->inherit_super.height + 1) >> 1,
			dest_->u_stride,
			dest_->u_step,
			src_->u_data,
			(src_->inherit_super.width + 1) >> 1,
			(src_->inherit_super.height + 1) >> 1,
			src_->u_stride,
			src_->u_step,
			true);
		ImageResizer_resizePlane(
			dest_->v_data,
			(dest_->inherit_super.width + 1) >> 1,
			(dest_->inherit_super.height + 1) >> 1,
			dest_->v_stride,
			dest_->v_step,
			src_->v_data,
			(src_->inherit_super.width + 1) >> 1,
			(src_->inherit_super.height + 1) >> 1,
			src_->v_stride,
			src_->v_step,
			true);
		return true;
	}
	else if(src->inherit_super.type == frameType_bgr) {
		ttLibC_Bgr *src_ = (ttLibC_Bgr *)src;
		ttLibC_Bgr *dest_ = (ttLibC_Bgr *)dest;

		uint8_t *bs_ = NULL;
		uint8_t *gs_ = NULL;
		uint8_t *rs_ = NULL;
		uint8_t *as_ = NULL;

		uint8_t *bd_ = NULL;
		uint8_t *gd_ = NULL;
		uint8_t *rd_ = NULL;
		uint8_t *ad_ = NULL;

		switch(src_->type) {
		case BgrType_bgr:
			bs_ = src_->data;
			gs_ = src_->data + 1;
			rs_ = src_->data + 2;
			break;
		case BgrType_bgra:
			bs_ = src_->data;
			gs_ = src_->data + 1;
			rs_ = src_->data + 2;
			as_ = src_->data + 3;
			break;
		case BgrType_abgr:
			as_ = src_->data;
			bs_ = src_->data + 1;
			gs_ = src_->data + 2;
			rs_ = src_->data + 3;
			break;
		case BgrType_rgb:
			rs_ = src_->data;
			gs_ = src_->data + 1;
			bs_ = src_->data + 2;
			break;
		case BgrType_rgba:
			rs_ = src_->data;
			gs_ = src_->data + 1;
			bs_ = src_->data + 2;
			as_ = src_->data + 3;
			break;
		case BgrType_argb:
			as_ = src_->data;
			rs_ = src_->data + 1;
			gs_ = src_->data + 2;
			bs_ = src_->data + 3;
			break;
		default:
			return false;
		}
		switch(dest_->type) {
		case BgrType_bgr:
			bd_ = dest_->data;
			gd_ = dest_->data + 1;
			rd_ = dest_->data + 2;
			break;
		case BgrType_bgra:
			bd_ = dest_->data;
			gd_ = dest_->data + 1;
			rd_ = dest_->data + 2;
			ad_ = dest_->data + 3;
			break;
		case BgrType_abgr:
			ad_ = dest_->data;
			bd_ = dest_->data + 1;
			gd_ = dest_->data + 2;
			rd_ = dest_->data + 3;
			break;
		case BgrType_rgb:
			rd_ = dest_->data;
			gd_ = dest_->data + 1;
			bd_ = dest_->data + 2;
			break;
		case BgrType_rgba:
			rd_ = dest_->data;
			gd_ = dest_->data + 1;
			bd_ = dest_->data + 2;
			ad_ = dest_->data + 3;
			break;
		case BgrType_argb:
			ad_ = dest_->data;
			rd_ = dest_->data + 1;
			gd_ = dest_->data + 2;
			bd_ = dest_->data + 3;
			break;
		default:
			return false;
		}
		// then copy.
		ImageResizer_resizePlane(
			bd_,
			dest_->inherit_super.width,
			dest_->inherit_super.height,
			dest_->width_stride,
			dest_->unit_size,
			bs_,
			src_->inherit_super.width,
			src_->inherit_super.height,
			src_->width_stride,
			src_->unit_size,
			is_quick);
		ImageResizer_resizePlane(
			gd_,
			dest_->inherit_super.width,
			dest_->inherit_super.height,
			dest_->width_stride,
			dest_->unit_size,
			gs_,
			src_->inherit_super.width,
			src_->inherit_super.height,
			src_->width_stride,
			src_->unit_size,
			is_quick);
		ImageResizer_resizePlane(
			rd_,
			dest_->inherit_super.width,
			dest_->inherit_super.height,
			dest_->width_stride,
			dest_->unit_size,
			rs_,
			src_->inherit_super.width,
			src_->inherit_super.height,
			src_->width_stride,
			src_->unit_size,
			is_quick);
		if(ad_ != NULL) {
			if(as_ == NULL) {
				for(int i = 0;i < dest_->inherit_super.height;++ i) {
					uint8_t *ad__ = ad_;
					for(int j = 0;j < dest_->inherit_super.width;++ j) {
						*ad__ = 0xFF;
						ad__ += dest_->unit_size;
					}
					ad_ += dest_->width_stride;
				}
			}
			else {
				ImageResizer_resizePlane(
					ad_,
					dest_->inherit_super.width,
					dest_->inherit_super.height,
					dest_->width_stride,
					dest_->unit_size,
					as_,
					src_->inherit_super.width,
					src_->inherit_super.height,
					src_->width_stride,
					src_->unit_size,
					is_quick);
			}
		}
		return true;
	}
	return false;
}

/**
 * resize yuv image.
 * @param prev_frame reuse image object
 * @param type       target yuv420 image type.
 * @param width      target width
 * @param height     target height
 * @param src_frame
 * @return scaled yuv image.
 */
ttLibC_Yuv420 TT_ATTRIBUTE_API *ttLibC_ImageResizer_resizeYuv420(
		ttLibC_Yuv420 *prev_frame,
		ttLibC_Yuv420_Type type,
		uint32_t width,
		uint32_t height,
		ttLibC_Yuv420 *src_frame,
		bool is_quick) {
	if(src_frame == NULL) {
		return NULL;
	}
	ttLibC_Yuv420 *yuv = ttLibC_Yuv420_makeEmptyFrame2(
			prev_frame,
			type,
			width,
			height);
	if(yuv == NULL) {
		ERR_PRINT("failed to make dst frame.");
		return NULL;
	}
	yuv->inherit_super.inherit_super.pts = src_frame->inherit_super.inherit_super.pts;
	yuv->inherit_super.inherit_super.timebase = src_frame->inherit_super.inherit_super.timebase;
	ImageResizer_resizePlane(
			yuv->y_data,
			width,
			height,
			yuv->y_stride,
			yuv->y_step,
			src_frame->y_data,
			src_frame->inherit_super.width,
			src_frame->inherit_super.height,
			src_frame->y_stride,
			src_frame->y_step,
			is_quick);
	ImageResizer_resizePlane(
			yuv->u_data,
			(width + 1) >> 1,
			(height + 1) >> 1,
			yuv->u_stride,
			yuv->u_step,
			src_frame->u_data,
			(src_frame->inherit_super.width  + 1) >> 1,
			(src_frame->inherit_super.height + 1) >> 1,
			src_frame->u_stride,
			src_frame->u_step,
			true);
	ImageResizer_resizePlane(
			yuv->v_data,
			(width + 1) >> 1,
			(height + 1) >> 1,
			yuv->v_stride,
			yuv->v_step,
			src_frame->v_data,
			(src_frame->inherit_super.width  + 1) >> 1,
			(src_frame->inherit_super.height + 1) >> 1,
			src_frame->v_stride,
			src_frame->v_step,
			true);
	yuv->inherit_super.inherit_super.id = src_frame->inherit_super.inherit_super.id;
	return yuv;
}

/**
 * resize bgr image.
 * @param prev_frame
 * @param type
 * @param width
 * @param height
 * @param src_frame
 * @return scaled bgr image.
 */
ttLibC_Bgr TT_ATTRIBUTE_API *ttLibC_ImageResizer_resizeBgr(
		ttLibC_Bgr *prev_frame,
		ttLibC_Bgr_Type type,
		uint32_t width,
		uint32_t height,
		ttLibC_Bgr *src_frame) {
	if(src_frame == NULL) {
		return NULL;
	}
	uint8_t *r_src = NULL;
	uint8_t *g_src = NULL;
	uint8_t *b_src = NULL;
	uint8_t *a_src = NULL;
	switch(src_frame->type) {
	case BgrType_abgr:
		r_src = src_frame->data + 3;
		g_src = src_frame->data + 2;
		b_src = src_frame->data + 1;
		a_src = src_frame->data;
		break;
	case BgrType_argb:
		r_src = src_frame->data + 1;
		g_src = src_frame->data + 2;
		b_src = src_frame->data + 3;
		a_src = src_frame->data;
		break;
	case BgrType_bgr:
		r_src = src_frame->data + 2;
		g_src = src_frame->data + 1;
		b_src = src_frame->data;
		break;
	case BgrType_bgra:
		r_src = src_frame->data + 2;
		g_src = src_frame->data + 1;
		b_src = src_frame->data;
		a_src = src_frame->data + 3;
		break;
	case BgrType_rgb:
		r_src = src_frame->data;
		g_src = src_frame->data + 1;
		b_src = src_frame->data + 2;
		break;
	case BgrType_rgba:
		r_src = src_frame->data;
		g_src = src_frame->data + 1;
		b_src = src_frame->data + 2;
		a_src = src_frame->data + 3;
		break;
	default:
		ERR_PRINT("src bgr_type is invalid.:%d", src_frame->type);
		return NULL;
	}
	switch(type) {
	case BgrType_abgr:
	case BgrType_argb:
	case BgrType_bgr:
	case BgrType_bgra:
	case BgrType_rgb:
	case BgrType_rgba:
		break;
	default:
		ERR_PRINT("bgr type is invalid.:%d", type);
		return NULL;
	}
	ttLibC_Bgr *bgr = ttLibC_Bgr_makeEmptyFrame2(
			prev_frame,
			type,
			width,
			height);
	if(bgr == NULL) {
		ERR_PRINT("failed to make dst frame.");
		return NULL;
	}
	bgr->inherit_super.inherit_super.pts = src_frame->inherit_super.inherit_super.pts;
	bgr->inherit_super.inherit_super.timebase = src_frame->inherit_super.inherit_super.timebase;
	uint8_t *r_dst = NULL;
	uint8_t *g_dst = NULL;
	uint8_t *b_dst = NULL;
	uint8_t *a_dst = NULL;
	switch(bgr->type) {
	case BgrType_abgr:
		r_dst = bgr->data + 3;
		g_dst = bgr->data + 2;
		b_dst = bgr->data + 1;
		a_dst = bgr->data;
		break;
	case BgrType_argb:
		r_dst = bgr->data + 1;
		g_dst = bgr->data + 2;
		b_dst = bgr->data + 3;
		a_dst = bgr->data;
		break;
	case BgrType_bgr:
		r_dst = bgr->data + 2;
		g_dst = bgr->data + 1;
		b_dst = bgr->data;
		break;
	case BgrType_bgra:
		r_dst = bgr->data + 2;
		g_dst = bgr->data + 1;
		b_dst = bgr->data;
		a_dst = bgr->data + 3;
		break;
	case BgrType_rgb:
		r_dst = bgr->data;
		g_dst = bgr->data + 1;
		b_dst = bgr->data + 2;
		break;
	case BgrType_rgba:
		r_dst = bgr->data;
		g_dst = bgr->data + 1;
		b_dst = bgr->data + 2;
		a_dst = bgr->data + 3;
		break;
	default:
		ERR_PRINT("src bgr_type is invalid.:%d", src_frame->type);
		return NULL;
	}
	ImageResizer_resizePlane(
			b_dst,
			width,
			height,
			bgr->width_stride,
			bgr->unit_size,
			b_src,
			src_frame->inherit_super.width,
			src_frame->inherit_super.height,
			src_frame->width_stride,
			src_frame->unit_size,
			false);
	ImageResizer_resizePlane(
			g_dst,
			width,
			height,
			bgr->width_stride,
			bgr->unit_size,
			g_src,
			src_frame->inherit_super.width,
			src_frame->inherit_super.height,
			src_frame->width_stride,
			src_frame->unit_size,
			false);
	ImageResizer_resizePlane(
			r_dst,
			width,
			height,
			bgr->width_stride,
			bgr->unit_size,
			r_src,
			src_frame->inherit_super.width,
			src_frame->inherit_super.height,
			src_frame->width_stride,
			src_frame->unit_size,
			false);
	if(a_dst != NULL) {
		if(a_src == NULL) {
			for(uint32_t i = 0;i < height; ++ i) {
				uint8_t *as = a_src;
				for(uint32_t j = 0;j < width;++ j) {
					*as = 255;
					as += bgr->unit_size;
				}
				a_src += bgr->width_stride;
			}
		}
		else {
			ImageResizer_resizePlane(
					a_dst,
					width,
					height,
					bgr->width_stride,
					bgr->unit_size,
					a_src,
					src_frame->inherit_super.width,
					src_frame->inherit_super.height,
					src_frame->width_stride,
					src_frame->unit_size,
					false);
		}
	}
	bgr->inherit_super.inherit_super.id = src_frame->inherit_super.inherit_super.id;
	return bgr;
}
