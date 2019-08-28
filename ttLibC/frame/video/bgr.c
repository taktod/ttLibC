/*
 * @file   bgr.c
 * @brief  bgr image frame information
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/18
 */

#include <string.h>
#include <stdlib.h>

#include "bgr.h"
#include "../../_log.h"
#include "../../allocator.h"

typedef struct {
	ttLibC_Bgr inherit_super;
	uint32_t width;
	uint32_t height;
	uint8_t *data;
	uint32_t width_stride;
} ttLibC_Frame_Video_Bgr_;

typedef ttLibC_Frame_Video_Bgr_ ttLibC_Bgr_;

/*
 * make bgr frame
 * @param prev_frame    reuse frame object. if NULL, create new one.
 * @param type          type of bgr
 * @param width         width of image
 * @param height        height of image
 * @param width_stride  width stride bytes size
 * @param data          bgr data
 * @param data_size     bgr data size
 * @param non_copy_mode true:hold the data pointer. false:data will copy.
 * @param pts           pts for image.
 * @param timebase      timebase number for pts.
 * @return bgr object.
 */
ttLibC_Bgr TT_ATTRIBUTE_API *ttLibC_Bgr_make(
		ttLibC_Bgr *prev_frame,
		ttLibC_Bgr_Type type,
		uint32_t width,
		uint32_t height,
		uint32_t width_stride,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	if(prev_frame != NULL && prev_frame->inherit_super.inherit_super.type != frameType_bgr) {
		ERR_PRINT("reuse with incompatible frame.");
		return NULL;
	}
	if(!non_copy_mode) {
		ttLibC_Bgr *bgr = ttLibC_Bgr_makeEmptyFrame2(
				prev_frame,
				type,
				width,
				height);
		if(bgr == NULL) {
			ERR_PRINT("failed to make empty frame.");
			return NULL;
		}
		uint8_t *data_dst = bgr->data;
		uint8_t *data_src = (uint8_t *)data;
		for(uint32_t i = 0;i < height;++ i) {
			memcpy(data_dst, data_src, width * bgr->unit_size);
			data_dst += bgr->width_stride;
			data_src += width_stride;
		}
		bgr->inherit_super.inherit_super.pts = pts;
		bgr->inherit_super.inherit_super.timebase = timebase;
		return bgr;
	}
	ttLibC_Bgr *bgr = prev_frame;
	size_t data_size_ = data_size;
	size_t buffer_size_ = data_size;
	uint32_t unit_size = 3;
	switch(type) {
	case BgrType_bgr:
	case BgrType_rgb:
		break;
	case BgrType_abgr:
	case BgrType_bgra:
	case BgrType_argb:
	case BgrType_rgba:
		unit_size = 4;
		break;
	default:
		ERR_PRINT("unknown bgr type.%d", type);
		return NULL;
	}
	if(bgr == NULL) {
		bgr = (ttLibC_Bgr *)ttLibC_malloc(sizeof(ttLibC_Bgr_));
		if(bgr == NULL) {
			ERR_PRINT("failed to allocate memory for bgr frame.");
			return NULL;
		}
		bgr->inherit_super.inherit_super.data = NULL;
	}
	else {
		if(!bgr->inherit_super.inherit_super.is_non_copy) {
			if(non_copy_mode || bgr->inherit_super.inherit_super.data_size < data_size_) {
				ttLibC_free(bgr->inherit_super.inherit_super.data);
				bgr->inherit_super.inherit_super.data = NULL;
			}
			else {
				data_size_ = bgr->inherit_super.inherit_super.data_size;
			}
		}
	}
	bgr->width_stride                            = width_stride;
	bgr->unit_size                               = unit_size;
	bgr->type                                    = type;
	bgr->inherit_super.width                     = width;
	bgr->inherit_super.height                    = height;
	bgr->inherit_super.type                      = videoType_key;
	bgr->inherit_super.inherit_super.is_non_copy = non_copy_mode;
	bgr->inherit_super.inherit_super.pts         = pts;
	bgr->inherit_super.inherit_super.dts         = 0;
	bgr->inherit_super.inherit_super.timebase    = timebase;
	bgr->inherit_super.inherit_super.type        = frameType_bgr;
	bgr->inherit_super.inherit_super.data_size   = data_size_;
	bgr->inherit_super.inherit_super.buffer_size = buffer_size_;
	bgr->inherit_super.inherit_super.data = data;
	bgr->data = bgr->inherit_super.inherit_super.data;
	ttLibC_Bgr_ *bgr_ = (ttLibC_Bgr_ *)bgr;
	bgr_->width        = bgr->inherit_super.width;
	bgr_->height       = bgr->inherit_super.height;
	bgr_->data         = bgr->data;
	bgr_->width_stride = bgr->width_stride;
	return bgr;
}

/**
 * make clone frame.
 * always make copy buffer on it.
 * @param prev_frame reuse frame object.
 * @param src_frame  source of clone.
 */
ttLibC_Bgr TT_ATTRIBUTE_API *ttLibC_Bgr_clone(
		ttLibC_Bgr *prev_frame,
		ttLibC_Bgr *src_frame) {
	if(src_frame == NULL) {
		return NULL;
	}
	return ttLibC_Bgr_make(
			prev_frame,
			src_frame->type,
			src_frame->inherit_super.width,
			src_frame->inherit_super.height,
			src_frame->width_stride,
			src_frame->data,
			src_frame->inherit_super.height * src_frame->width_stride,
			false,
			src_frame->inherit_super.inherit_super.pts,
			src_frame->inherit_super.inherit_super.timebase);
}

/*
 * close frame
 * @param frame
 */
void TT_ATTRIBUTE_API ttLibC_Bgr_close(ttLibC_Bgr **frame) {
	ttLibC_Bgr *target = (*frame);
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.inherit_super.type != frameType_bgr) {
		ERR_PRINT("found non bgr frame in bgr_close.");
		return;
	}
	if(!target->inherit_super.inherit_super.is_non_copy) {
		ttLibC_free(target->inherit_super.inherit_super.data);
	}
	ttLibC_free(target);
	*frame = NULL;
}

ttLibC_Bgr TT_ATTRIBUTE_API *ttLibC_Bgr_makeEmptyFrame(
		ttLibC_Bgr_Type sub_type,
		uint32_t        width,
		uint32_t        height) {
	return ttLibC_Bgr_makeEmptyFrame2(NULL, sub_type, width, height);
}

/**
 * generate empty frame
 * @param prev_frame reuse frame object.
 * @param sub_type type of bgr
 * @param width    width of image
 * @param height   height of image
 */
ttLibC_Bgr TT_ATTRIBUTE_API *ttLibC_Bgr_makeEmptyFrame2(
		ttLibC_Bgr     *prev_frame,
		ttLibC_Bgr_Type sub_type,
		uint32_t        width,
		uint32_t        height) {
#ifndef  GET_ALIGNED_STRIDE
#	define GET_ALIGNED_STRIDE(w) (((((w) - 1) >> 4) + 1) << 4)
	ttLibC_Bgr *bgr = NULL;
	uint32_t stride = GET_ALIGNED_STRIDE(width * 3);
	uint32_t hstride = GET_ALIGNED_STRIDE(height);
	uint32_t buffer_size = stride * height;
	uint32_t data_size = stride * hstride;
	switch(sub_type) {
	case BgrType_abgr:
	case BgrType_bgra:
	case BgrType_argb:
	case BgrType_rgba:
		stride = GET_ALIGNED_STRIDE(width * 4);
		buffer_size = stride * height;
		data_size = stride * hstride;
		break;
	case BgrType_bgr:
	case BgrType_rgb:
		break;
	default:
		return NULL;
	}

	if(prev_frame != NULL && prev_frame->inherit_super.inherit_super.type != frameType_bgr) {
		ERR_PRINT("prev_frame with incompatible frame.");
		ttLibC_Frame_close((ttLibC_Frame **)&prev_frame);
	}
	bgr = prev_frame;
	if(bgr == NULL) {
		bgr = (ttLibC_Bgr *)ttLibC_malloc(sizeof(ttLibC_Bgr_));
		if(bgr == NULL) {
			ERR_PRINT("failed to allocate memory for bgr frame.");
			return NULL;
		}
		bgr->inherit_super.inherit_super.data = NULL;
	}
	else {
		if(!bgr->inherit_super.inherit_super.is_non_copy) {
			if(bgr->inherit_super.inherit_super.data_size < data_size) {
				ttLibC_free(bgr->inherit_super.inherit_super.data);
				bgr->inherit_super.inherit_super.data = NULL;
			}
			else {
				data_size = bgr->inherit_super.inherit_super.data_size;
			}
		}
	}
	if(bgr->inherit_super.inherit_super.data == NULL) {
		bgr->inherit_super.inherit_super.data = ttLibC_malloc(data_size);
		if(bgr->inherit_super.inherit_super.data == NULL) {
			ERR_PRINT("failed to allocate memory for data.");
			if(prev_frame == NULL) {
				ttLibC_free(bgr);
			}
			return NULL;
		}
	}
	bgr->data         = bgr->inherit_super.inherit_super.data;
	bgr->type         = sub_type;
	bgr->width_stride = stride;
	switch(sub_type) {
	case BgrType_abgr:
	case BgrType_bgra:
	case BgrType_argb:
	case BgrType_rgba:
		bgr->unit_size = 4;
		break;
	case BgrType_bgr:
	case BgrType_rgb:
		bgr->unit_size = 3;
		break;
	}
	bgr->inherit_super.width  = width;
	bgr->inherit_super.height = height;
	bgr->inherit_super.type   = videoType_key;
	bgr->inherit_super.inherit_super.is_non_copy = false;
	bgr->inherit_super.inherit_super.pts         = 0;
	bgr->inherit_super.inherit_super.dts         = 0;
	bgr->inherit_super.inherit_super.timebase    = 1000;
	bgr->inherit_super.inherit_super.type        = frameType_bgr;
	bgr->inherit_super.inherit_super.buffer_size = buffer_size;
	bgr->inherit_super.inherit_super.data_size   = data_size;
	ttLibC_Bgr_ *bgr_ = (ttLibC_Bgr_ *)bgr;
	bgr_->width        = bgr->inherit_super.width;
	bgr_->height       = bgr->inherit_super.height;
	bgr_->data         = bgr->data;
	bgr_->width_stride = bgr->width_stride;
	return bgr;
#	undef  GET_ALIGNED_STRIDE
#endif
}

/*
 * get minimum size of binary string.
 * @param bgr      target bgr frame
 * @param callback binary buffer callback
 * @param ptr      user def pointer.
 * @return true / false
 */
bool TT_ATTRIBUTE_API ttLibC_Bgr_getMinimumBinaryBuffer(
		ttLibC_Bgr *bgr,
		ttLibC_FrameBinaryFunc callback,
		void *ptr) {
	if(bgr == NULL) {
		return false;
	}
	uint8_t *data = NULL;
	size_t data_size = 0;
	uint32_t stride = 0;
	switch(bgr->type) {
	case BgrType_abgr:
	case BgrType_argb:
	case BgrType_bgra:
	case BgrType_rgba:
		stride = bgr->inherit_super.width * 4;
		data_size = stride * bgr->inherit_super.height;
		break;
	case BgrType_bgr:
	case BgrType_rgb:
		stride = bgr->inherit_super.width * 3;
		data_size = stride * bgr->inherit_super.height;
		break;
	default:
		ERR_PRINT("unknown bgr type.");
		return false;
	}
	data = ttLibC_malloc(data_size);
	if(data == NULL) {
		ERR_PRINT("failed to allocate memory.");
		return false;
	}
	uint8_t *data_src = bgr->data;
	uint8_t *data_dst = (uint8_t *)data;
	for(uint32_t i = 0;i < bgr->inherit_super.height;++ i) {
		memcpy(data_dst, data_src, bgr->inherit_super.width * bgr->unit_size);
		data_src += bgr->width_stride;
		data_dst += stride;
	}
	bool result = false;
	if(callback != NULL) {
		result = callback(ptr, data, data_size);
	}
	ttLibC_free(data);
 	return result;
}

/*
 * reset changed data.
 * @param bgr target bgr frame.
 */
void TT_ATTRIBUTE_API ttLibC_Bgr_resetData(ttLibC_Bgr *bgr) {
	if(bgr == NULL) {
		return;
	}
	ttLibC_Bgr_ *bgr_ = (ttLibC_Bgr_ *)bgr;
	bgr->inherit_super.width  = bgr_->width;
	bgr->inherit_super.height = bgr_->height;
	bgr->data         = bgr_->data;
	bgr->width_stride = bgr_->width_stride;
}
