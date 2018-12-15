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
	if(src_frame == NULL) {
		return NULL;
	}
	uint8_t *r_src = NULL;
	uint8_t *g_src = NULL;
	uint8_t *b_src = NULL;
	switch(src_frame->type) {
	case BgrType_abgr:
		r_src = src_frame->data + 3;
		g_src = src_frame->data + 2;
		b_src = src_frame->data + 1;
		break;
	case BgrType_argb:
		r_src = src_frame->data + 1;
		g_src = src_frame->data + 2;
		b_src = src_frame->data + 3;
		break;
	case BgrType_bgr:
	case BgrType_bgra:
		r_src = src_frame->data + 2;
		g_src = src_frame->data + 1;
		b_src = src_frame->data;
		break;
	case BgrType_rgb:
	case BgrType_rgba:
		r_src = src_frame->data;
		g_src = src_frame->data + 1;
		b_src = src_frame->data + 2;
		break;
	default:
		ERR_PRINT("unknown bgr frame type:%d", src_frame->type);
		return NULL;
	}

	ttLibC_Yuv420 *yuv = ttLibC_Yuv420_makeEmptyFrame2(
			prev_frame,
			type,
			src_frame->inherit_super.width,			
			src_frame->inherit_super.height);
	if(yuv == NULL) {
		ERR_PRINT("failed to make dest frame.");
		return NULL;
	}
	uint8_t *y_dst = yuv->y_data;
	uint8_t *u_dst = yuv->u_data;
	uint8_t *v_dst = yuv->v_data;
	for(int i = 0;i < src_frame->inherit_super.height;++ i) {
		uint8_t *yd = y_dst;
		uint8_t *ud = u_dst;
		uint8_t *vd = v_dst;
		uint8_t *rs = r_src;
		uint8_t *gs = g_src;
		uint8_t *bs = b_src;
		for(int j = 0;j < src_frame->inherit_super.width;++ j) {
			int y = ((66 * (*rs) + 129 * (*gs) + 25 * (*bs)) >> 8) + 16;
			*yd = y > 235 ? 235 : y < 16 ? 16 : y;
			if((i & 1) == 0 && (j & 1) == 0) {
				int u = ((-38 * (*rs) + -74 * (*gs) + 112 * (*bs)) >> 8) + 128;
				int v = ((112 * (*rs) + -94 * (*gs) + -18 * (*bs)) >> 8) + 128;
				*ud = u > 240 ? 240 : u < 16 ? 16 : u;
				*vd = v > 240 ? 240 : v < 16 ? 16 : v;
				ud += yuv->u_step;
				vd += yuv->v_step;
			}
			yd += yuv->y_step;
			rs += src_frame->unit_size;
			gs += src_frame->unit_size;
			bs += src_frame->unit_size;
		}
		if((i & 1) == 1) {
			u_dst += yuv->u_stride;
			v_dst += yuv->v_stride;
		}
		y_dst += yuv->y_stride;
		r_src += src_frame->width_stride;
		g_src += src_frame->width_stride;
		b_src += src_frame->width_stride;
	}
	return yuv;
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
	if(src_frame == NULL) {
		return NULL;
	}
	uint8_t *y_src = src_frame->y_data;
	uint8_t *u_src = src_frame->u_data;
	uint8_t *v_src = src_frame->v_data;

	switch(type) {
	case BgrType_abgr:
	case BgrType_bgr:
	case BgrType_bgra:
	case BgrType_argb:
	case BgrType_rgb:
	case BgrType_rgba:
		break;
	default:
		ERR_PRINT("unknown bgr frame type:%d, type");
		return NULL;
	}
	ttLibC_Bgr *bgr = ttLibC_Bgr_makeEmptyFrame2(
			prev_frame,
			type,
			src_frame->inherit_super.width,
			src_frame->inherit_super.height);
	if(bgr == NULL) {
		ERR_PRINT("failed to make dest frame.");
		return NULL;
	}
	uint8_t *r_dst = NULL;
	uint8_t *g_dst = NULL;
	uint8_t *b_dst = NULL;
	uint8_t *a_dst = NULL;
	switch(type) {
	case BgrType_abgr:
		r_dst = bgr->data + 3;
		g_dst = bgr->data + 2;
		b_dst = bgr->data + 1;
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
	case BgrType_argb:
		r_dst = bgr->data + 1;
		g_dst = bgr->data + 2;
		b_dst = bgr->data + 3;
		a_dst = bgr->data;
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
		ERR_PRINT("unreachable.");
		if(prev_frame != NULL) {
			ttLibC_Bgr_close(&bgr);
		}
		return NULL;
	}
	for(int i = 0;i < src_frame->inherit_super.height;++ i) {
		uint8_t *rd = r_dst;
		uint8_t *gd = g_dst;
		uint8_t *bd = b_dst;
		uint8_t *ad = a_dst;
		uint8_t *ys = y_src;
		uint8_t *us = u_src;
		uint8_t *vs = v_src;
		for(int j = 0;j < src_frame->inherit_super.width;++ j) {
			int y1192 = 1192 * ((*ys) - 16);
			int u = *us - 128;
			int v = *vs - 128;
			int r = (y1192 + 1634 * v);
			int g = (y1192 - 833 * v - 400 * u);
			int b = (y1192 + 2066 * u);
			r = r < 0 ? 0 : r > 0x3FFFF ? 0x3FFFF : r;
			g = g < 0 ? 0 : g > 0x3FFFF ? 0x3FFFF : g;
			b = b < 0 ? 0 : b > 0x3FFFF ? 0x3FFFF : b;
			*rd = 0xFF & (r >> 10);
			*gd = 0xFF & (g >> 10);
			*bd = 0xFF & (b >> 10);
			if(a_dst != NULL) {
				*ad = 255;
			}
			if((j & 1) == 1) {
				us += src_frame->u_step;
				vs += src_frame->v_step;
			}
			ys += src_frame->y_step;
			rd += bgr->unit_size;
			gd += bgr->unit_size;
			bd += bgr->unit_size;
			if(a_dst != NULL) {
				ad += bgr->unit_size;
			}
		}
		if((i & 1) == 1) {
			u_src += src_frame->u_stride;
			v_src += src_frame->v_stride;
		}
		y_src += src_frame->y_stride;
		r_dst += bgr->width_stride;
		g_dst += bgr->width_stride;
		b_dst += bgr->width_stride;
		if(a_dst != NULL) {
			a_dst += bgr->width_stride;
		}
	}
	return bgr;
}



