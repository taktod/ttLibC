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
#include "../_log.h"
#include "../allocator.h"
#include <stdlib.h>

bool TT_ATTRIBUTE_API ttLibC_ImageResampler_ToBgr(
		ttLibC_Bgr   *dest_frame,
		ttLibC_Video *src_frame) {
	if(src_frame == NULL || dest_frame == NULL) {
		return false;
	}
	if(dest_frame->inherit_super.width  != src_frame->width
	|| dest_frame->inherit_super.height != src_frame->height) {
		return false;
	}
	uint8_t *b_dest = NULL;
	uint8_t *g_dest = NULL;
	uint8_t *r_dest = NULL;
	uint8_t *a_dest = NULL;
	switch(dest_frame->type) {
	case BgrType_bgr:
		b_dest = dest_frame->data;
		g_dest = dest_frame->data + 1;
		r_dest = dest_frame->data + 2;
		break;
	case BgrType_bgra:
		b_dest = dest_frame->data;
		g_dest = dest_frame->data + 1;
		r_dest = dest_frame->data + 2;
		a_dest = dest_frame->data + 3;
		break;
	case BgrType_abgr:
		a_dest = dest_frame->data;
		b_dest = dest_frame->data + 1;
		g_dest = dest_frame->data + 2;
		r_dest = dest_frame->data + 3;
		break;
	case BgrType_rgb:
		r_dest = dest_frame->data;
		g_dest = dest_frame->data + 1;
		b_dest = dest_frame->data + 2;
		break;
	case BgrType_rgba:
		r_dest = dest_frame->data;
		g_dest = dest_frame->data + 1;
		b_dest = dest_frame->data + 2;
		a_dest = dest_frame->data + 3;
		break;
	case BgrType_argb:
		a_dest = dest_frame->data;
		r_dest = dest_frame->data + 1;
		g_dest = dest_frame->data + 2;
		b_dest = dest_frame->data + 3;
		break;
	default:
		return false;
	}
	switch(src_frame->inherit_super.type) {
	case frameType_bgr:
		{
			ttLibC_Bgr *src = (ttLibC_Bgr *)src_frame;
			uint8_t *b_src = NULL;
			uint8_t *g_src = NULL;
			uint8_t *r_src = NULL;
			uint8_t *a_src = NULL;
			switch(src->type) {
			case BgrType_bgr:
				b_src = src->data;
				g_src = src->data + 1;
				r_src = src->data + 2;
				break;
			case BgrType_bgra:
				b_src = src->data;
				g_src = src->data + 1;
				r_src = src->data + 2;
				a_src = src->data + 3;
				break;
			case BgrType_abgr:
				a_src = src->data;
				b_src = src->data + 1;
				g_src = src->data + 2;
				r_src = src->data + 3;
				break;
			case BgrType_rgb:
				r_src = src->data;
				g_src = src->data + 1;
				b_src = src->data + 2;
				break;
			case BgrType_rgba:
				r_src = src->data;
				g_src = src->data + 1;
				b_src = src->data + 2;
				a_src = src->data + 3;
				break;
			case BgrType_argb:
				a_src = src->data;
				r_src = src->data + 1;
				g_src = src->data + 2;
				b_src = src->data + 3;
				break;
			default:
				return false;
			}
			for(int i = 0;i < src_frame->height;++ i) {
				uint8_t *ad = a_dest;
				uint8_t *bd = b_dest;
				uint8_t *gd = g_dest;
				uint8_t *rd = r_dest;
				uint8_t *as = a_src;
				uint8_t *bs = b_src;
				uint8_t *gs = g_src;
				uint8_t *rs = r_src;
				for(int j = 0;j < src_frame->width;++ j) {
					*bd = *bs;
					*gd = *gs;
					*rd = *rs;
					bd += dest_frame->unit_size;
					gd += dest_frame->unit_size;
					rd += dest_frame->unit_size;
					bs += src->unit_size;
					gs += src->unit_size;
					rs += src->unit_size;
					if(ad != NULL) {
						if(as != NULL) {
							*ad = *as;
							as += src->unit_size;
						}
						else {
							*ad = 0xFF;
						}
						ad += dest_frame->unit_size;
					}
				}
				if(a_src != NULL) {
					a_src += src->width_stride;
				}
				b_src += src->width_stride;
				g_src += src->width_stride;
				r_src += src->width_stride;
				if(a_dest != NULL) {
					a_dest += dest_frame->width_stride;
				}
				b_dest += dest_frame->width_stride;
				g_dest += dest_frame->width_stride;
				r_dest += dest_frame->width_stride;
			}
			return true;
		}
		break;
	case frameType_yuv420:
		{
			ttLibC_Yuv420 *src = (ttLibC_Yuv420 *)src_frame;
			uint8_t *y_src = src->y_data;
			uint8_t *u_src = src->u_data;
			uint8_t *v_src = src->v_data;
			for(int i = 0;i < src_frame->height;++ i) {
				uint8_t *ad = a_dest;
				uint8_t *bd = b_dest;
				uint8_t *gd = g_dest;
				uint8_t *rd = r_dest;
				uint8_t *ys = y_src;
				uint8_t *us = u_src;
				uint8_t *vs = v_src;
				for(int j = 0;j < src_frame->width;++ j) {
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
					if(ad != NULL) {
						*ad = 255;
					}
					if((j & 1) == 1) {
						us += src->u_step;
						vs += src->v_step;
					}
					ys += src->y_step;
					rd += dest_frame->unit_size;
					gd += dest_frame->unit_size;
					bd += dest_frame->unit_size;
					if(ad != NULL) {
						ad += dest_frame->unit_size;
					}
				}
				if((i & 1) == 1) {
					u_src += src->u_stride;
					v_src += src->v_stride;
				}
				y_src += src->y_stride;
				r_dest += dest_frame->width_stride;
				g_dest += dest_frame->width_stride;
				b_dest += dest_frame->width_stride;
				if(a_dest != NULL) {
					a_dest += dest_frame->width_stride;
				}
			}
			return true;
 		}
		break;
	default:
		break;
	}
	return false;
}

bool TT_ATTRIBUTE_API ttLibC_ImageResampler_ToYuv420(
		ttLibC_Yuv420 *dest_frame,
		ttLibC_Video  *src_frame) {
	if(src_frame == NULL || dest_frame == NULL) {
		return false;
	}
	if(dest_frame->inherit_super.width  != src_frame->width
	|| dest_frame->inherit_super.height != src_frame->height) {
		return false;
	}
	uint8_t *y_dest = dest_frame->y_data;
	uint8_t *u_dest = dest_frame->u_data;
	uint8_t *v_dest = dest_frame->v_data;
	switch(src_frame->inherit_super.type) {
	case frameType_bgr:
		{
			ttLibC_Bgr *src = (ttLibC_Bgr *)src_frame;
			uint8_t *b_src = NULL;
			uint8_t *g_src = NULL;
			uint8_t *r_src = NULL;
			switch(src->type) {
			case BgrType_bgr:
				b_src = src->data;
				g_src = src->data + 1;
				r_src = src->data + 2;
				break;
			case BgrType_bgra:
				b_src = src->data;
				g_src = src->data + 1;
				r_src = src->data + 2;
				break;
			case BgrType_abgr:
				b_src = src->data + 1;
				g_src = src->data + 2;
				r_src = src->data + 3;
				break;
			case BgrType_rgb:
				r_src = src->data;
				g_src = src->data + 1;
				b_src = src->data + 2;
				break;
			case BgrType_rgba:
				r_src = src->data;
				g_src = src->data + 1;
				b_src = src->data + 2;
				break;
			case BgrType_argb:
				r_src = src->data + 1;
				g_src = src->data + 2;
				b_src = src->data + 3;
				break;
			default:
				return false;
			}
			for(int i = 0;i < src_frame->height;++ i) {
				uint8_t *yd = y_dest;
				uint8_t *ud = u_dest;
				uint8_t *vd = v_dest;
				uint8_t *bs = b_src;
				uint8_t *gs = g_src;
				uint8_t *rs = r_src;
				for(int j = 0;j < src_frame->width;++ j) {
					int y = ((66 * (*rs) + 129 * (*gs) + 25 * (*bs)) >> 8) + 16;
					*yd = y > 235 ? 235 : y < 16 ? 16 : y;
					if((i & 1) == 0 && (j & 1) == 0) {
						int u = ((-38 * (*rs) + -74 * (*gs) + 112 * (*bs)) >> 8) + 128;
						int v = ((112 * (*rs) + -94 * (*gs) + -18 * (*bs)) >> 8) + 128;
						*ud = u > 240 ? 240 : u < 16 ? 16 : u;
						*vd = v > 240 ? 240 : v < 16 ? 16 : v;
						ud += dest_frame->u_step;
						vd += dest_frame->v_step;
					}
					yd += dest_frame->y_step;
					rs += src->unit_size;
					gs += src->unit_size;
					bs += src->unit_size;
				}
				if((i & 1) == 1) {
					u_dest += dest_frame->u_stride;
					v_dest += dest_frame->v_stride;
				}
				y_dest += dest_frame->y_stride;
				r_src += src->width_stride;
				g_src += src->width_stride;
				b_src += src->width_stride;
			}
			return true;
		}
		break;
	case frameType_yuv420:
		{
			ttLibC_Yuv420 *src = (ttLibC_Yuv420 *)src_frame;
			uint8_t *y_src = src->y_data;
			uint8_t *u_src = src->u_data;
			uint8_t *v_src = src->v_data;
			for(int i = 0;i < src_frame->height;++ i) {
				uint8_t *yd = y_dest;
				uint8_t *ud = u_dest;
				uint8_t *vd = v_dest;
				uint8_t *ys = y_src;
				uint8_t *us = u_src;
				uint8_t *vs = v_src;
				for(int j = 0;j < src_frame->width;++ j) {
					*yd = *ys;
					if((i & 1) == 0 && (j & 1) == 0) {
						*ud = *us;
						*vd = *vs;
						us += src->u_step;
						vs += src->v_step;
						ud += dest_frame->u_step;
						vd += dest_frame->v_step;
					}
					ys += src->y_step;
					yd += dest_frame->y_step;
				}
				if((i & 1) == 1) {
					u_src += src->u_stride;
					v_src += src->v_stride;
					u_dest += dest_frame->u_stride;
					v_dest += dest_frame->v_stride;
				}
				y_src += src->y_stride;
				y_dest += dest_frame->y_stride;
			}
			return true;
		}
		break;
	default:
		break;
	}
	return false;
}

/*
 * make yuv420 frame from bgr frame.
 * @param prev_frame reuse frame.
 * @param type       yuv420 type.
 * @param src_frame  src bgr frame.
 */
ttLibC_Yuv420 TT_ATTRIBUTE_API *ttLibC_ImageResampler_makeYuv420FromBgr(
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
	yuv->inherit_super.inherit_super.pts = src_frame->inherit_super.inherit_super.pts;
	yuv->inherit_super.inherit_super.timebase = src_frame->inherit_super.inherit_super.timebase;
	uint8_t *y_dst = yuv->y_data;
	uint8_t *u_dst = yuv->u_data;
	uint8_t *v_dst = yuv->v_data;
	for(uint32_t i = 0;i < src_frame->inherit_super.height;++ i) {
		uint8_t *yd = y_dst;
		uint8_t *ud = u_dst;
		uint8_t *vd = v_dst;
		uint8_t *rs = r_src;
		uint8_t *gs = g_src;
		uint8_t *bs = b_src;
		for(uint32_t j = 0;j < src_frame->inherit_super.width;++ j) {
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
	yuv->inherit_super.inherit_super.id = src_frame->inherit_super.inherit_super.id;
	return yuv;
}

/*
 * make bgr frame from yuv420 frame.
 * @param prev_frame reuse frame.
 * @param type       bgr type.
 * @param src_frame  src yuv420 frame.
 */
ttLibC_Bgr TT_ATTRIBUTE_API *ttLibC_ImageResampler_makeBgrFromYuv420(
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
		ERR_PRINT("unknown bgr frame type:%d", type);
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
	bgr->inherit_super.inherit_super.pts = src_frame->inherit_super.inherit_super.pts;
	bgr->inherit_super.inherit_super.timebase = src_frame->inherit_super.inherit_super.timebase;
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
	for(uint32_t i = 0;i < src_frame->inherit_super.height;++ i) {
		uint8_t *rd = r_dst;
		uint8_t *gd = g_dst;
		uint8_t *bd = b_dst;
		uint8_t *ad = a_dst;
		uint8_t *ys = y_src;
		uint8_t *us = u_src;
		uint8_t *vs = v_src;
		for(uint32_t j = 0;j < src_frame->inherit_super.width;++ j) {
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
	bgr->inherit_super.inherit_super.id = src_frame->inherit_super.inherit_super.id;
	return bgr;
}



