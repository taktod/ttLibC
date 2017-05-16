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
#include "../log.h"
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
ttLibC_Yuv420 *ttLibC_ImageResizer_resizeYuv420(
		ttLibC_Yuv420 *prev_frame,
		ttLibC_Yuv420_Type type,
		uint32_t width,
		uint32_t height,
		ttLibC_Yuv420 *src_frame,
		bool is_quick) {
	if(src_frame == NULL) {
		return NULL;
	}
	if(prev_frame != NULL && prev_frame->inherit_super.inherit_super.type != frameType_yuv420) {
		ttLibC_Frame_close((ttLibC_Frame **)&prev_frame);
	}
	uint32_t src_uv_step = 1;
	switch(src_frame->type) {
	case Yuv420Type_planar:
	case Yvu420Type_planar:
		break;
	case Yuv420Type_semiPlanar:
	case Yvu420Type_semiPlanar:
		src_uv_step = 2;
		break;
	default:
		return false;
	}

	ttLibC_Yuv420 *yuv = prev_frame;
	uint32_t wh = width * height;
	size_t data_size = wh + (wh >> 1);
	bool alloc_flag = false;
	uint8_t *data = NULL;
	if(yuv != NULL) {
		// yuv420->inherit_super.inherit_super.data_size >= data_size;
		if(!yuv->inherit_super.inherit_super.is_non_copy) {
			// buffer can reuse.
			if(yuv->inherit_super.inherit_super.data_size >= data_size) {
				// size is ok for reuse.
				data = yuv->inherit_super.inherit_super.data;
				data_size = yuv->inherit_super.inherit_super.data_size;
			}
			else {
				// size is small for reuse.
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
	uint8_t *y_data = NULL;
	uint8_t *u_data = NULL;
	uint8_t *v_data = NULL;
	uint32_t y_step = 1;
	uint32_t u_step = 1;
	uint32_t v_step = 1;
	uint32_t y_stride = width;
	uint32_t u_stride = width;
	uint32_t v_stride = width;
	switch(type) {
	default:
	case Yuv420Type_planar:
		y_data = data;
		u_data = data + wh;
		v_data = data + wh + (wh >> 2);
		y_step = 1;
		u_step = 1;
		v_step = 1;
		y_stride = width;
		u_stride = (width >> 1);
		v_stride = (width >> 1);
		break;
	case Yuv420Type_semiPlanar:
		y_data = data;
		u_data = data + wh;
		v_data = data + wh + 1;
		y_step = 1;
		u_step = 2;
		v_step = 2;
		y_stride = width;
		u_stride = width;
		v_stride = width;
		break;
	case Yvu420Type_planar:
		y_data = data;
		u_data = data + wh + (wh >> 2);
		v_data = data + wh;
		y_step = 1;
		u_step = 1;
		v_step = 1;
		y_stride = width;
		u_stride = (width >> 1);
		v_stride = (width >> 1);
		break;
	case Yvu420Type_semiPlanar:
		y_data = data;
		u_data = data + wh + 1;
		v_data = data + wh;
		y_step = 1;
		u_step = 2;
		v_step = 2;
		y_stride = width;
		u_stride = width;
		v_stride = width;
		break;
	}
	ImageResizer_resizePlane(
			y_data,
			width,
			height,
			y_stride,
			y_step,
			src_frame->y_data,
			src_frame->inherit_super.width,
			src_frame->inherit_super.height,
			src_frame->y_stride,
			1,
			is_quick);
	ImageResizer_resizePlane(
			u_data,
			width >> 1,
			height >> 1,
			u_stride,
			u_step,
			src_frame->u_data,
			src_frame->inherit_super.width >> 1,
			src_frame->inherit_super.height >> 1,
			src_frame->u_stride,
			src_uv_step,
			true);
	ImageResizer_resizePlane(
			v_data,
			width >> 1,
			height >> 1,
			v_stride,
			v_step,
			src_frame->v_data,
			src_frame->inherit_super.width >> 1,
			src_frame->inherit_super.height >> 1,
			src_frame->v_stride,
			src_uv_step,
			true);
	yuv = ttLibC_Yuv420_make(
			yuv,
			type,
			width,
			height,
			data,
			data_size,
			y_data,
			y_stride,
			u_data,
			u_stride,
			v_data,
			v_stride,
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

/**
 * resize bgr image.
 * @param prev_frame
 * @param type
 * @param width
 * @param height
 * @param src_frame
 * @return scaled bgr image.
 */
ttLibC_Bgr *ttLibC_ImageResizer_resizeBgr(
		ttLibC_Bgr *prev_frame,
		ttLibC_Bgr_Type type,
		uint32_t width,
		uint32_t height,
		ttLibC_Bgr *src_frame) {
	if(src_frame == NULL) {
		return NULL;
	}
	if(prev_frame != NULL && prev_frame->inherit_super.inherit_super.type != frameType_bgr) {
		ttLibC_Frame_close((ttLibC_Frame **)&prev_frame);
	}
	ttLibC_Bgr *bgr = prev_frame;
	uint32_t wh = width * height;
	size_t data_size = wh + (wh << 1);
	bool alloc_flag = false;
	switch(type) {
	case BgrType_bgr:
		break;
	case BgrType_abgr:
	case BgrType_bgra:
		data_size = (wh << 2);
		break;
	default:
		ERR_PRINT("target bgr type is unknown:%d", type);
		return NULL;
	}
	uint8_t *data = NULL;
	if(bgr != NULL) {
		if(!bgr->inherit_super.inherit_super.is_non_copy) {
			// buffer can reuse.
			if(bgr->inherit_super.inherit_super.data_size >= data_size) {
				// data size is enough.
				data = bgr->inherit_super.inherit_super.data;
				data_size = bgr->inherit_super.inherit_super.data_size;
			}
			else {
				// size is too short.
				ttLibC_free(bgr->inherit_super.inherit_super.data);
			}
		}
		if(data == NULL) {
			bgr->inherit_super.inherit_super.data = NULL;
			bgr->inherit_super.inherit_super.data_size = 0;
		}
		bgr->inherit_super.inherit_super.is_non_copy = true;
	}
	if(data == NULL) {
		data = ttLibC_malloc(data_size);
		if(data == NULL) {
			return NULL;
		}
		alloc_flag = true;
	}
	uint8_t *b_data = NULL;
	uint8_t *g_data = NULL;
	uint8_t *r_data = NULL;
	uint8_t *a_data = NULL;
	uint32_t step = 3;
	uint32_t stride = width + (width << 1);
	switch(type) {
	default:
		if(alloc_flag) {
			ttLibC_free(data);
		}
		return NULL;
	case BgrType_bgr:
		b_data = data;
		g_data = data + 1;
		r_data = data + 2;
		break;
	case BgrType_abgr:
		a_data = data;
		b_data = data + 1;
		g_data = data + 2;
		r_data = data + 3;
		step = 4;
		stride = (width << 2);
		break;
	case BgrType_bgra:
		b_data = data;
		g_data = data + 1;
		r_data = data + 2;
		a_data = data + 3;
		step = 4;
		stride = (width << 2);
		break;
	}
	uint8_t *src_b_data = NULL;
	uint8_t *src_g_data = NULL;
	uint8_t *src_r_data = NULL;
	uint8_t *src_a_data = NULL;
	uint32_t src_step = 3;
	uint32_t src_stride = src_frame->width_stride;
	switch(src_frame->type) {
	default:
		if(alloc_flag) {
			ttLibC_free(data);
		}
		return NULL;
	case BgrType_bgr:
		src_b_data = src_frame->data;
		src_g_data = src_b_data + 1;
		src_r_data = src_b_data + 2;
		break;
	case BgrType_abgr:
		src_a_data = src_frame->data;
		src_b_data = src_a_data + 1;
		src_g_data = src_a_data + 2;
		src_r_data = src_a_data + 3;
		src_step = 4;
		break;
	case BgrType_bgra:
		src_b_data = src_frame->data;
		src_g_data = src_b_data + 1;
		src_r_data = src_b_data + 2;
		src_a_data = src_b_data + 3;
		src_step = 4;
		break;
	}
	ImageResizer_resizePlane(
			b_data,
			width,
			height,
			stride,
			step,
			src_b_data,
			src_frame->inherit_super.width,
			src_frame->inherit_super.height,
			src_stride,
			src_step,
			false);
	ImageResizer_resizePlane(
			g_data,
			width,
			height,
			stride,
			step,
			src_g_data,
			src_frame->inherit_super.width,
			src_frame->inherit_super.height,
			src_stride,
			src_step,
			false);
	ImageResizer_resizePlane(
			r_data,
			width,
			height,
			stride,
			step,
			src_r_data,
			src_frame->inherit_super.width,
			src_frame->inherit_super.height,
			src_stride,
			src_step,
			false);
	if(a_data != NULL) {
		// fill alpha = 255
		for(uint32_t i = 0;i < height;++ i) {
			uint8_t *a_buf = a_data;
			for(uint32_t j = 0;j < width;++ j) {
				*a_buf = 255;
				++ a_buf;
			}
			a_data += stride;
		}
	}
	bgr = ttLibC_Bgr_make(
			bgr,
			type,
			width,
			height,
			stride,
			data,
			data_size,
			true,
			src_frame->inherit_super.inherit_super.pts,
			src_frame->inherit_super.inherit_super.timebase);
	if(bgr == NULL) {
		if(alloc_flag) {
			ttLibC_free(data);
		}
		return NULL;
	}
	bgr->inherit_super.inherit_super.is_non_copy = false;
	return bgr;
}
