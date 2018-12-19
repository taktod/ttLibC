/*
 * @file   yuv420.c
 * @brief  yuv420 image frame information.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/18
 */

#include <string.h>
#include <stdlib.h>

#include "yuv420.h"
#include "../../ttLibC_predef.h"
#include "../../_log.h"
#include "../../allocator.h"

/*
 * make yuv420 frame
 * @param prev_frame    reuse frame object. if NULL, create new one.
 * @param type          type of yuv420
 * @param width         width of image
 * @param height        height of image
 * @param data          yuv420 data
 * @param data_size     data size
 * @param y_data        pointer for y_data
 * @param y_stride      stride for each line for y_data
 * @param u_data        pointer for u_data
 * @param u_stride      stride for each line for u_data
 * @param v_data        pointer for v_data
 * @param v_stride      stride for each line for v_data
 * @param non_copy_mode true:hold the data pointer. false:data will copy
 * @param pts           pts for image
 * @param timebase      timebase number for pts.
 * @return yuv420 object.
 */
ttLibC_Yuv420 TT_VISIBILITY_DEFAULT *ttLibC_Yuv420_make(
		ttLibC_Yuv420 *prev_frame,
		ttLibC_Yuv420_Type type,
		uint32_t width,
		uint32_t height,
		void *data,
		size_t data_size,
		void *y_data,
		uint32_t y_stride,
		void *u_data,
		uint32_t u_stride,
		void *v_data,
		uint32_t v_stride,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	if(prev_frame != NULL && prev_frame->inherit_super.inherit_super.type != frameType_yuv420) {
		ERR_PRINT("reuse with incompatible frame.");
		return NULL;
	}
	if(!non_copy_mode) {
		// make empty frame and copy. 
		ttLibC_Yuv420 *yuv = ttLibC_Yuv420_makeEmptyFrame2(
				prev_frame,
				type,
				width,
				height);
		if(yuv == NULL) {
			ERR_PRINT("failed to make empty frame.");
			return NULL;
		}
		uint8_t *y_dst = yuv->y_data;
		uint8_t *u_dst = yuv->u_data;
		uint8_t *v_dst = yuv->v_data;
		uint8_t *y_src = (uint8_t *)y_data;
		uint8_t *u_src = (uint8_t *)u_data;
		uint8_t *v_src = (uint8_t *)v_data;
		for(uint32_t i = 0;i < height;++ i) {
			uint8_t *yd = y_dst;
			uint8_t *ud = u_dst;
			uint8_t *vd = v_dst;
			uint8_t *ys = y_src;
			uint8_t *us = u_src;
			uint8_t *vs = v_src;
			for(uint32_t j = 0;j < width;++ j) {
				*yd = *ys;
				yd += yuv->y_step;
				ys += yuv->y_step;
				if((i & 1) == 0 && (j & 1) == 0) {
					*ud = *us;
					*vd = *vs;
					ud += yuv->u_step;
					us += yuv->u_step;
					vd += yuv->v_step;
					vs += yuv->v_step;
				}
			}
			if((i & 1) == 0) {
				u_dst += yuv->u_stride;
				v_dst += yuv->v_stride;
				u_src += u_stride;
				v_src += v_stride;
			}
			y_dst += yuv->y_stride;
			y_src += y_stride;
		}
		return yuv;
	}
	ttLibC_Yuv420 *yuv420 = prev_frame;
	size_t buffer_size_ = data_size;
	size_t data_size_ = data_size;
	uint32_t y_step = 1;
	uint32_t u_step = 1;
	uint32_t v_step = 1;
	switch(type) {
	case Yuv420Type_planar:
	case Yvu420Type_planar:
		break;
	case Yuv420Type_semiPlanar:
	case Yvu420Type_semiPlanar:
		u_step = 2;
		v_step = 2;
		break;
	default:
		ERR_PRINT("unknown yuv420 type.%d", type);
		return NULL;
	}
	if(yuv420 == NULL) {
		yuv420 = (ttLibC_Yuv420 *)ttLibC_malloc(sizeof(ttLibC_Yuv420));
		if(yuv420 == NULL) {
			ERR_PRINT("failed to allocate memory for yuv420 frame");
			return NULL;
		}
		yuv420->inherit_super.inherit_super.data = NULL;
	}
	else {
		if(!yuv420->inherit_super.inherit_super.is_non_copy) {
			if(non_copy_mode || yuv420->inherit_super.inherit_super.data_size < data_size_) {
				ttLibC_free(yuv420->inherit_super.inherit_super.data);
				yuv420->inherit_super.inherit_super.data = NULL;
			}
			else {
				data_size_ = yuv420->inherit_super.inherit_super.data_size;
			}
		}
	}
	yuv420->inherit_super.inherit_super.data        = NULL;
	yuv420->inherit_super.inherit_super.data_size   = 0;
	yuv420->type                                    = type;
	yuv420->y_data                                  = y_data;
	yuv420->y_stride                                = y_stride;
	yuv420->y_step                                  = y_step;
	yuv420->u_data                                  = u_data;
	yuv420->u_stride                                = u_stride;
	yuv420->u_step                                  = u_step;
	yuv420->v_data                                  = v_data;
	yuv420->v_stride                                = v_stride;
	yuv420->v_step                                  = v_step;
	yuv420->inherit_super.type                      = videoType_key;
	yuv420->inherit_super.width                     = width;
	yuv420->inherit_super.height                    = height;
	yuv420->inherit_super.inherit_super.is_non_copy = non_copy_mode;
	yuv420->inherit_super.inherit_super.pts         = pts;
	yuv420->inherit_super.inherit_super.dts         = 0;
	yuv420->inherit_super.inherit_super.timebase    = timebase;
	yuv420->inherit_super.inherit_super.type        = frameType_yuv420;
	yuv420->inherit_super.inherit_super.data_size   = data_size_;
	yuv420->inherit_super.inherit_super.buffer_size = buffer_size_;

	yuv420->inherit_super.inherit_super.data = data;
	return yuv420;
}

/*
 * make clone frame.
 * always make copy buffer on it.
 * @param prev_frame reuse frame object.
 * @param src_frame  source of clone.
 */
ttLibC_Yuv420 TT_VISIBILITY_DEFAULT *ttLibC_Yuv420_clone(
		ttLibC_Yuv420 *prev_frame,
		ttLibC_Yuv420 *src_frame) {
	if(src_frame == NULL) {
		return NULL;
	}
	return ttLibC_Yuv420_make(
			prev_frame,
			src_frame->type,
			src_frame->inherit_super.width,
			src_frame->inherit_super.height,
			NULL,
			0,
			src_frame->y_data,
			src_frame->y_stride,
			src_frame->u_data,
			src_frame->u_stride,
			src_frame->v_data,
			src_frame->v_stride,
			false,
			src_frame->inherit_super.inherit_super.pts,
			src_frame->inherit_super.inherit_super.timebase);
}
/*
 * close frame
 * @param frame
 */
void TT_VISIBILITY_DEFAULT ttLibC_Yuv420_close(ttLibC_Yuv420 **frame) {
	ttLibC_Yuv420 *target = (*frame);
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.inherit_super.type != frameType_yuv420) {
		ERR_PRINT("found non yuv420 frame in yuv420_close.");
	}
	if(!target->inherit_super.inherit_super.is_non_copy) {
		ttLibC_free(target->inherit_super.inherit_super.data);
	}
	ttLibC_free(target);
	*frame = NULL;
}

ttLibC_Yuv420 TT_VISIBILITY_DEFAULT *ttLibC_Yuv420_makeEmptyFrame(
		ttLibC_Yuv420_Type sub_type,
		uint32_t           width,
		uint32_t           height) {
	return ttLibC_Yuv420_makeEmptyFrame2(NULL, sub_type, width, height);
}

ttLibC_Yuv420 TT_VISIBILITY_DEFAULT *ttLibC_Yuv420_makeEmptyFrame2(
		ttLibC_Yuv420     *prev_frame,
		ttLibC_Yuv420_Type sub_type,
		uint32_t           width,
		uint32_t           height) {
#ifndef  GET_ALIGNED_STRIDE
#	define GET_ALIGNED_STRIDE(w) (((((w) - 1) >> 4) + 1) << 4)

	ttLibC_Yuv420 *yuv420 = NULL;
	uint32_t full_stride = GET_ALIGNED_STRIDE(width);
	uint32_t half_stride = GET_ALIGNED_STRIDE((width + 1) >> 1);
	uint32_t full_wh = full_stride * height;
	uint32_t half_wh = half_stride * ((height + 1) >> 1);
	uint32_t buffer_size = full_wh + (half_wh << 1);
	switch(sub_type) {
	case Yuv420Type_planar:
	case Yvu420Type_planar:
		break;
	case Yuv420Type_semiPlanar:
	case Yvu420Type_semiPlanar:
		buffer_size = full_wh + full_stride * ((height + 1) >> 1);
		break;
	default:
		ERR_PRINT("unknown yuv420 type.%d", sub_type);
		return NULL;
	}
	uint32_t data_size = buffer_size;

	if(prev_frame != NULL && prev_frame->inherit_super.inherit_super.type != frameType_yuv420) {
		ERR_PRINT("prev_frame with incompatible frame.");
		ttLibC_Frame_close((ttLibC_Frame **)&prev_frame);
	}
	yuv420 = prev_frame;
	if(yuv420 == NULL) {
		yuv420 = (ttLibC_Yuv420 *)ttLibC_malloc(sizeof(ttLibC_Yuv420));
		if(yuv420 == NULL) {
			ERR_PRINT("failed to allocate memory for yuv420 frame.");
			return NULL;
		}
		yuv420->inherit_super.inherit_super.data = NULL;
	}
	else {
		if(!yuv420->inherit_super.inherit_super.is_non_copy) {
			if(yuv420->inherit_super.inherit_super.data_size < data_size) {
				// data is short. 
				ttLibC_free(yuv420->inherit_super.inherit_super.data);
				yuv420->inherit_super.inherit_super.data = NULL;
			}
			else {
				data_size = yuv420->inherit_super.inherit_super.data_size;
			}
		}
	}
	if(yuv420->inherit_super.inherit_super.data == NULL) {
		yuv420->inherit_super.inherit_super.data = ttLibC_malloc(data_size);
		if(yuv420->inherit_super.inherit_super.data == NULL) {
			ERR_PRINT("failed to allocate memory for data.");
			if(prev_frame == NULL) {
				ttLibC_free(yuv420);
			}
			return NULL;
		}
	}
	uint8_t *data = (uint8_t *)yuv420->inherit_super.inherit_super.data;
	yuv420->type = sub_type;
	yuv420->y_data = data;
	yuv420->y_stride = full_stride;
	yuv420->y_step = 1;
	switch(sub_type) {
	case Yuv420Type_planar:
		yuv420->u_data = data + full_wh;
		yuv420->u_stride = half_stride;
		yuv420->u_step = 1;
		yuv420->v_data = data + full_wh + half_wh;
		yuv420->v_stride = half_stride;
		yuv420->v_step = 1;
		break;
	case Yvu420Type_planar:
		yuv420->u_data = data + full_wh + half_wh;
		yuv420->u_stride = half_stride;
		yuv420->u_step = 1;
		yuv420->v_data = data + full_wh;
		yuv420->v_stride = half_stride;
		yuv420->v_step = 1;
		break;
	case Yuv420Type_semiPlanar:
		yuv420->u_data = data + full_wh;
		yuv420->u_stride = full_stride;
		yuv420->u_step = 2;
		yuv420->v_data = data + full_wh + 1;
		yuv420->v_stride = full_stride;
		yuv420->v_step = 2;
		break;
	case Yvu420Type_semiPlanar:
		yuv420->u_data = data + full_wh + 1;
		yuv420->u_stride = full_stride;
		yuv420->u_step = 2;
		yuv420->v_data = data + full_wh;
		yuv420->v_stride = full_stride;
		yuv420->v_step = 2;
		break;
	default:
		ERR_PRINT("unreachable.");
		return NULL;
	}
	yuv420->inherit_super.type                      = videoType_key;
	yuv420->inherit_super.width                     = width;
	yuv420->inherit_super.height                    = height;
	yuv420->inherit_super.inherit_super.is_non_copy = false;
	yuv420->inherit_super.inherit_super.pts         = 0;
	yuv420->inherit_super.inherit_super.dts         = 0;
	yuv420->inherit_super.inherit_super.timebase    = 1000;
	yuv420->inherit_super.inherit_super.type        = frameType_yuv420;
	yuv420->inherit_super.inherit_super.data_size   = data_size;
	yuv420->inherit_super.inherit_super.buffer_size = buffer_size;

#	undef  GET_ALIGNED_STRIDE
#endif
	return yuv420;
}

/*
 * get minimum size of binary buffer.
 * @param yuv      target yuv frame
 * @param callback binary buffer callback
 * @param ptr      user def pointer.
 * @return true / false
 */
bool ttLibC_Yuv420_getMinimumBinaryBuffer(
		ttLibC_Yuv420 *yuv,
		ttLibC_FrameBinaryFunc callback,
		void *ptr) {
	if(yuv == NULL) {
		return false;
	}
	uint32_t half_width  = (yuv->inherit_super.width  + 1) >> 1;
	uint32_t half_height = (yuv->inherit_super.height + 1) >> 1;
	uint8_t *data = NULL;
	size_t data_size = yuv->inherit_super.width * yuv->inherit_super.height + ((half_width * half_height) << 1);
	data = ttLibC_malloc(data_size);
	if(data == NULL) {
		ERR_PRINT("failed to allocate memory.");
		return false;
	}
	uint8_t *y_src = yuv->y_data;
	uint8_t *u_src = yuv->u_data;
	uint8_t *v_src = yuv->v_data;
	uint8_t *y_dst = NULL;
	uint8_t *u_dst = NULL;
	uint8_t *v_dst = NULL;
	uint32_t y_stride = 0;
	uint32_t u_stride = 0;
	uint32_t v_stride = 0;
	switch(yuv->type) {
	case Yuv420Type_planar:
		y_dst = data;
		u_dst = data + yuv->inherit_super.width * yuv->inherit_super.height;
		v_dst = u_dst + half_width * half_height;
		y_stride = yuv->inherit_super.width;
		u_stride = half_width;
		v_stride = half_width;
		break;
	case Yuv420Type_semiPlanar:
		y_dst = data;
		u_dst = data + yuv->inherit_super.width * yuv->inherit_super.height;
		v_dst = u_dst + 1;
		y_stride = yuv->inherit_super.width;
		u_stride = (half_width << 1);
		v_stride = (half_width << 1);
		break;
	case Yvu420Type_planar:
		y_dst = data;
		v_dst = data + yuv->inherit_super.width * yuv->inherit_super.height;
		u_dst = v_dst + half_width * half_height;
		y_stride = yuv->inherit_super.width;
		u_stride = half_width;
		v_stride = half_width;
		break;
	case Yvu420Type_semiPlanar:
		y_dst = data;
		v_dst = data + yuv->inherit_super.width * yuv->inherit_super.height;
		u_dst = v_dst + 1;
		y_stride = yuv->inherit_super.width;
		u_stride = (half_width << 1);
		v_stride = (half_width << 1);
		break;
	default:
		ERR_PRINT("unknown yuv type.:%d", yuv->type);
		ttLibC_free(data);
		return false;
	}
	for(uint32_t i = 0;i < yuv->inherit_super.height;++ i) {
		uint8_t *yd = y_dst;
		uint8_t *ud = u_dst;
		uint8_t *vd = v_dst;
		uint8_t *ys = y_src;
		uint8_t *us = u_src;
		uint8_t *vs = v_src;
		for(uint32_t j = 0;j < yuv->inherit_super.width;++ j) {
			*yd = *ys;
			yd += yuv->y_step;
			ys += yuv->y_step;
			if((i & 1) == 0 && (j & 1) == 0) {
				*ud = *us;
				*vd = *vs;
				ud += yuv->u_step;
				us += yuv->u_step;
				vd += yuv->v_step;
				vs += yuv->v_step;
			}
		}
		if((i & 1) == 0) {
			u_dst += u_stride;
			v_dst += v_stride;
			u_src += yuv->u_stride;
			v_src += yuv->v_stride;
		}
		y_dst += y_stride;
		y_src += yuv->y_stride;
	}
	bool result = false;
	if(callback != NULL) {
		result = callback(ptr, data, data_size);
	}
	ttLibC_free(data);
 	return result;
}
