/**
 * @file  imageResizer.c
 * @brief library or image resizing.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2016/9/10
 */

#include "../ttLibC_predef.h"
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

/**
 * resize yuv image.
 * @param prev_frame reuse image object
 * @param type       target yuv420 image type.
 * @param width      target width
 * @param height     target height
 * @param src_frame
 * @return scaled yuv image.
 */
ttLibC_Yuv420 TT_VISIBILITY_DEFAULT *ttLibC_ImageResizer_resizeYuv420(
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
			src_frame->inherit_super.width >> 1,
			src_frame->inherit_super.height >> 1,
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
			src_frame->inherit_super.width >> 1,
			src_frame->inherit_super.height >> 1,
			src_frame->v_stride,
			src_frame->v_step,
			true);
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
ttLibC_Bgr TT_VISIBILITY_DEFAULT *ttLibC_ImageResizer_resizeBgr(
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
	return bgr;
}
