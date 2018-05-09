/*
 * @file   imageResampler.c
 * @brief  library for image resample.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/19
 */

#include "imageResampler.h"
#include "../ttLibC_predef.h"
#include "../_log.h"
#include "../allocator.h"
#include <stdlib.h>

/*
 * make yuv420 frame from bgr frame.
 * @param prev_frame reuse frame.
 * @param type       yuv420 type.
 * @param src_frame  src bgr frame.
 */
ttLibC_Yuv420 TT_VISIBILITY_DEFAULT *ttLibC_ImageResampler_makeYuv420FromBgr(
		ttLibC_Yuv420 *prev_frame,
		ttLibC_Yuv420_Type type,
		ttLibC_Bgr *src_frame) {
	ttLibC_Yuv420 *yuv420 = prev_frame;
	if(src_frame == NULL) {
		return NULL;
	}
	uint32_t width = src_frame->inherit_super.width;
	uint32_t height = src_frame->inherit_super.height;
	uint32_t f_stride = ((((width - 1) >> 4) + 1) << 4);
	uint32_t h_stride = (((((width >> 1) - 1) >> 4) + 1) << 4);
	uint32_t y_size = f_stride * height;
	uint32_t u_size = h_stride * (height >> 1);
	uint32_t v_size = h_stride * (height >> 1);
	uint32_t wh = width * height;

	size_t data_size = y_size + u_size + v_size;
	bool alloc_flag = false;
	switch(type) {
	case Yuv420Type_planar:
	case Yuv420Type_semiPlanar:
	case Yvu420Type_planar:
	case Yvu420Type_semiPlanar:
		break;
	default:
		ERR_PRINT("target yuv420 type is unknown:%d", type);
		return NULL;
	}
	uint8_t *data = NULL;
	if(yuv420 != NULL) {
		// yuv420->inherit_super.inherit_super.data_size >= data_size;
		if(!yuv420->inherit_super.inherit_super.is_non_copy) {
			// buffer can reuse.
			if(yuv420->inherit_super.inherit_super.data_size >= data_size) {
				// size is ok for reuse.
				data = yuv420->inherit_super.inherit_super.data;
				data_size = yuv420->inherit_super.inherit_super.data_size;
			}
			else {
				// size is small for reuse.
				ttLibC_free(yuv420->inherit_super.inherit_super.data);
			}
		}
		if(data == NULL) {
			yuv420->inherit_super.inherit_super.data = NULL;
			yuv420->inherit_super.inherit_super.data_size = 0;
		}
		yuv420->inherit_super.inherit_super.is_non_copy = true;
	}
	if(data == NULL) {
		data = ttLibC_malloc(data_size);
		if(data == NULL) {
			return NULL;
		}
		alloc_flag = true;
	}
	// now start to convert.
	uint8_t *y_data = NULL;
	uint8_t *u_data = NULL;
	uint8_t *v_data = NULL;
	uint32_t y_step = 1;
	uint32_t u_step = 1;
	uint32_t v_step = 1;
	uint32_t y_stride = f_stride;
	uint32_t u_stride = h_stride;
	uint32_t v_stride = h_stride;
	switch(type) {
	default:
	case Yuv420Type_planar:
		y_data = data;
		u_data = data + y_size;
		v_data = data + y_size + u_size;
		y_step = 1;
		u_step = 1;
		v_step = 1;
		y_stride = f_stride;
		u_stride = h_stride;
		v_stride = h_stride;
		break;
	case Yuv420Type_semiPlanar:
		y_data = data;
		u_data = data + y_size;
		v_data = data + y_size + 1;
		y_step = 1;
		u_step = 2;
		v_step = 2;
		y_stride = f_stride;
		u_stride = f_stride;
		v_stride = f_stride;
		break;
	case Yvu420Type_planar:
		y_data = data;
		u_data = data + y_size + v_size;
		v_data = data + y_size;
		y_step = 1;
		u_step = 1;
		v_step = 1;
		y_stride = f_stride;
		u_stride = h_stride;
		v_stride = h_stride;
		break;
	case Yvu420Type_semiPlanar:
		y_data = data;
		u_data = data + y_size + 1;
		v_data = data + y_size;
		y_step = 1;
		u_step = 2;
		v_step = 2;
		y_stride = f_stride;
		u_stride = f_stride;
		v_stride = f_stride;
		break;
	}
	// rgb data
	uint8_t *src_data = src_frame->data;
	uint8_t *src_b_data = NULL;
	uint8_t *src_g_data = NULL;
	uint8_t *src_r_data = NULL;
	uint32_t src_step = 3;
	switch(src_frame->type) {
	case BgrType_bgr:
		src_step = 3;
		src_b_data = src_data;
		src_g_data = src_data + 1;
		src_r_data = src_data + 2;
		break;
	case BgrType_abgr:
		src_step = 4;
		src_b_data = src_data + 1;
		src_g_data = src_data + 2;
		src_r_data = src_data + 3;
		break;
	case BgrType_bgra:
		src_step = 4;
		src_b_data = src_data;
		src_g_data = src_data + 1;
		src_r_data = src_data + 2;
		break;
	case BgrType_rgb:
		src_step = 3;
		src_b_data = src_data + 2;
		src_g_data = src_data + 1;
		src_r_data = src_data;
		break;
	case BgrType_argb:
		src_step = 4;
		src_b_data = src_data + 3;
		src_g_data = src_data + 2;
		src_r_data = src_data + 1;
		break;
	case BgrType_rgba:
		src_step = 4;
		src_b_data = src_data + 2;
		src_g_data = src_data + 1;
		src_r_data = src_data;
		break;
	default:
		ERR_PRINT("found unknown bgr type for src_frame:%d", src_frame->type);
		if(alloc_flag) {
			ttLibC_free(data);
		}
		return NULL;
	}
	uint32_t dst_y_stride_diff = y_stride - width * y_step;
	uint32_t dst_u_stride_diff = u_stride - (width >> 1) * u_step;
	uint32_t dst_v_stride_diff = v_stride - (width >> 1) * v_step;
	uint32_t src_width_stride_diff = src_frame->width_stride - src_step * width;
	uint8_t r,g,b;
	uint32_t y,u,v;
	for(uint32_t i = 0;i < height;++ i) {
		for(uint32_t j = 0;j < width;++ j) {
			b = (*src_b_data);
			src_b_data += src_step;
			g = (*src_g_data);
			src_g_data += src_step;
			r = (*src_r_data);
			src_r_data += src_step;
			y = ((66 * r + 129 * g + 25 * b) >> 8) + 16;
			y = y > 235 ? 235 : y < 16 ? 16 : y;
			(*y_data) = y;
			y_data += y_step;
			if((i & 1) == 0 && (j & 1) == 0) {
				u = ((-38*r + -74*g + 112*b) >> 8) + 128;
				v = ((112*r + -94*g + -18*b) >> 8) + 128;
				u = u > 240 ? 240 : u < 16 ? 16 : u;
				v = v > 240 ? 240 : v < 16 ? 16 : v;
				(*u_data) = u;
				(*v_data) = v;
				u_data += u_step;
				v_data += v_step;
			}
		}
		if(dst_y_stride_diff > 0) {
			y_data += dst_y_stride_diff;
		}
		if(dst_u_stride_diff > 0) {
			u_data += dst_u_stride_diff;
		}
		if(dst_v_stride_diff > 0) {
			v_data += dst_v_stride_diff;
		}
		if(src_width_stride_diff > 0) {
			src_b_data += src_width_stride_diff;
			src_g_data += src_width_stride_diff;
			src_r_data += src_width_stride_diff;
		}
	}
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
	yuv420 = ttLibC_Yuv420_make(yuv420, type, width, height, data, data_size, y_data, y_stride, u_data, u_stride, v_data, v_stride, true, src_frame->inherit_super.inherit_super.pts, src_frame->inherit_super.inherit_super.timebase);
	if(yuv420 == NULL) {
		if(alloc_flag) {
			ttLibC_free(data);
		}
		return NULL;
	}
	yuv420->inherit_super.inherit_super.is_non_copy = false;
	return yuv420;
}

/*
 * make bgr frame from yuv420 frame.
 * @param prev_frame reuse frame.
 * @param type       bgr type.
 * @param src_frame  src yuv420 frame.
 */
ttLibC_Bgr TT_VISIBILITY_DEFAULT *ttLibC_ImageResampler_makeBgrFromYuv420(
		ttLibC_Bgr *prev_frame,
		ttLibC_Bgr_Type type,
		ttLibC_Yuv420 *src_frame) {
	ttLibC_Bgr *bgr = prev_frame;
	if(src_frame == NULL) {
		return NULL;
	}
	uint32_t width = src_frame->inherit_super.width;
	uint32_t height = src_frame->inherit_super.height;
	uint32_t bgr_stride = ((((width - 1) >> 4) + 1) << 4);
	uint32_t wh = bgr_stride * height;
	size_t data_size = wh + (wh << 1);
	bool alloc_flag = false;
	switch(type) {
	case BgrType_bgr:
		bgr_stride = bgr_stride * 3;
		break;
	case BgrType_abgr:
	case BgrType_bgra:
		bgr_stride = bgr_stride * 4;
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
	// now start to convert.
	uint8_t *b_data = NULL;
	uint8_t *g_data = NULL;
	uint8_t *r_data = NULL;
	uint8_t *a_data = NULL;
	uint32_t step = 3;
	switch(type) {
	default:
		if(alloc_flag) {
			ttLibC_free(data);
			return NULL;
		}
		break;
	case BgrType_bgr:
		b_data = data;
		g_data = data + 1;
		r_data = data + 2;
		a_data = NULL;
		step = 3;
		break;
	case BgrType_abgr:
		a_data = data;
		b_data = data + 1;
		g_data = data + 2;
		r_data = data + 3;
		step = 4;
		break;
	case BgrType_bgra:
		b_data = data;
		g_data = data + 1;
		r_data = data + 2;
		a_data = data + 3;
		step = 4;
		break;
	case BgrType_rgb:
		b_data = data + 2;
		g_data = data + 1;
		r_data = data;
		a_data = NULL;
		step = 3;
		break;
	case BgrType_argb:
		a_data = data;
		b_data = data + 3;
		g_data = data + 2;
		r_data = data + 1;
		step = 4;
		break;
	case BgrType_rgba:
		b_data = data + 2;
		g_data = data + 1;
		r_data = data;
		a_data = data + 3;
		step = 4;
		break;
	}
	// yuv420 data
	uint8_t *src_y_data = src_frame->y_data;
	uint8_t *src_u_data = src_frame->u_data;
	uint8_t *src_v_data = src_frame->v_data;
	uint32_t src_y_step = src_frame->y_step;
	uint32_t src_u_step = src_frame->u_step;
	uint32_t src_v_step = src_frame->v_step;
	uint32_t src_y_stride_diff = src_frame->y_stride - width * src_y_step;
	uint32_t src_u_stride_diff = src_frame->u_stride - (width >> 1) * src_u_step;
	uint32_t src_v_stride_diff = src_frame->v_stride - (width >> 1) * src_v_step;
	uint32_t dst_bgr_stride_diff = bgr_stride - width * step;

	uint32_t y1192 = 0;
	int32_t y = 0;
	int32_t u = 0;
	int32_t v = 0;
	int32_t b = 0;
	int32_t g = 0;
	int32_t r = 0;
	for(uint32_t i = 0;i < height;++ i) {
		if((i & 1) == 1) {
			src_u_data -= src_frame->u_stride;
			src_v_data -= src_frame->v_stride;
		}
		u = 0;
		v = 0;
		for(uint32_t j = 0;j < width;++ j) {
			y = (*src_y_data) - 16;
			src_y_data += src_y_step;
			if((j & 1) == 0) {
				u = (*src_u_data) - 128;
				v = (*src_v_data) - 128;
				src_u_data += src_u_step;
				src_v_data += src_v_step;
			}
			y1192 = 1192 * y;
			r = (y1192 + 1634 * v);
			g = (y1192 - 833 * v - 400 * u);
			b = (y1192 + 2066 * u);
			r = r < 0 ? 0 : r > 0x3FFFF ? 0x3FFFF : r;
			g = g < 0 ? 0 : g > 0x3FFFF ? 0x3FFFF : g;
			b = b < 0 ? 0 : b > 0x3FFFF ? 0x3FFFF : b;
			(*b_data) = 0xFF & (b >> 10);
			(*g_data) = 0xFF & (g >> 10);
			(*r_data) = 0xFF & (r >> 10);
			b_data += step;
			g_data += step;
			r_data += step;
			if(a_data != NULL) {
				(*a_data) = 0xFF;
				a_data += step;
			}
		}
		if(src_y_stride_diff > 0) {
			src_y_data += src_y_stride_diff;
		}
		if(src_u_stride_diff > 0) {
			src_u_data += src_u_stride_diff;
		}
		if(src_v_stride_diff > 0) {
			src_v_data += src_v_stride_diff;
		}
		if(dst_bgr_stride_diff > 0) {
			b_data += dst_bgr_stride_diff;
			g_data += dst_bgr_stride_diff;
			r_data += dst_bgr_stride_diff;
			if(a_data != NULL) {
				a_data += dst_bgr_stride_diff;
			}
		}
	}
	bgr = ttLibC_Bgr_make(bgr, type, width, height, bgr_stride, data, data_size, true, src_frame->inherit_super.inherit_super.pts, src_frame->inherit_super.inherit_super.timebase);
	if(bgr == NULL) {
		if(alloc_flag) {
			ttLibC_free(data);
		}
		return NULL;
	}
	bgr->inherit_super.inherit_super.is_non_copy = false;
	return bgr;
}



