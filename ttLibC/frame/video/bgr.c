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
#include "../../ttLibC_predef.h"
#include "../../_log.h"
#include "../../allocator.h"

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
ttLibC_Bgr TT_VISIBILITY_DEFAULT *ttLibC_Bgr_make(
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
		bgr = (ttLibC_Bgr *)ttLibC_malloc(sizeof(ttLibC_Bgr));
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
	if(non_copy_mode) {
		bgr->inherit_super.inherit_super.data = data;
	}
	else {
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
		memcpy(bgr->inherit_super.inherit_super.data, data, data_size);
	}
	bgr->data = bgr->inherit_super.inherit_super.data;
	return bgr;
}

/**
 * make clone frame.
 * always make copy buffer on it.
 * @param prev_frame reuse frame object.
 * @param src_frame  source of clone.
 */
ttLibC_Bgr TT_VISIBILITY_DEFAULT *ttLibC_Bgr_clone(
		ttLibC_Bgr *prev_frame,
		ttLibC_Bgr *src_frame) {
	if(src_frame == NULL) {
		return NULL;
	}
	if(src_frame->inherit_super.inherit_super.type != frameType_bgr) {
		ERR_PRINT("try to clone non bgr frame.");
		return NULL;
	}
	if(prev_frame != NULL && prev_frame->inherit_super.inherit_super.type != frameType_bgr) {
		ERR_PRINT("try to use non bgr frame for reuse.");
		return NULL;
	}
	uint32_t bgr_size = src_frame->inherit_super.height * src_frame->width_stride;
	switch(src_frame->type) {
	case BgrType_abgr:
	case BgrType_bgra:
	case BgrType_argb:
	case BgrType_rgba:
		bgr_size = bgr_size * 4;
		break;
	case BgrType_bgr:
	case BgrType_rgb:
		bgr_size = bgr_size * 3;
		break;
	default:
		return NULL;
	}
	bool allocflag = false;
	size_t buffer_size = bgr_size;
	uint8_t *buffer = NULL;
	if(prev_frame != NULL) {
		if(!prev_frame->inherit_super.inherit_super.is_non_copy) {
			if(prev_frame->inherit_super.inherit_super.data_size >= buffer_size) {
				buffer = prev_frame->inherit_super.inherit_super.data;
				buffer_size = prev_frame->inherit_super.inherit_super.data_size;
			}
			else {
				ttLibC_free(prev_frame->inherit_super.inherit_super.data);
			}
			prev_frame->inherit_super.inherit_super.data = NULL;
			prev_frame->inherit_super.inherit_super.is_non_copy = true;
		}
	}
	if(buffer == NULL) {
		buffer = ttLibC_malloc(buffer_size);
		if(buffer == NULL) {
			ERR_PRINT("failed to allocate buffer for yuv420 clone.");
			return NULL;
		}
		allocflag = true;
	}
	memcpy(buffer, src_frame->data, bgr_size);
	ttLibC_Bgr *bgr = ttLibC_Bgr_make(
			prev_frame,
			src_frame->type,
			src_frame->inherit_super.width,
			src_frame->inherit_super.height,
			src_frame->width_stride,
			buffer,
			buffer_size,
			false,
			src_frame->inherit_super.inherit_super.pts,
			src_frame->inherit_super.inherit_super.timebase);
	if(bgr == NULL) {
		if(allocflag) {
			ttLibC_free(buffer);
		}
		return NULL;
	}
	bgr->inherit_super.inherit_super.id = src_frame->inherit_super.inherit_super.id;
	return bgr;
}

/*
 * close frame
 * @param frame
 */
void TT_VISIBILITY_DEFAULT ttLibC_Bgr_close(ttLibC_Bgr **frame) {
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

ttLibC_Bgr TT_VISIBILITY_DEFAULT *ttLibC_Bgr_makeEmptyFrame(
		ttLibC_Bgr_Type sub_type,
		uint32_t        width,
		uint32_t        height) {
	uint8_t    *data = NULL;
	ttLibC_Bgr *bgr  = NULL;
	uint32_t memory_size = 0;
	uint32_t stride      = 0;
	switch(sub_type) {
	case BgrType_abgr:
	case BgrType_bgra:
	case BgrType_argb:
	case BgrType_rgba:
		stride = ((((width - 1) >> 4) + 1) << 4) * 4;
		memory_size = stride * height;
		break;
	case BgrType_bgr:
	case BgrType_rgb:
		stride = ((((width - 1) >> 4) + 1) << 4) * 3;
		memory_size = stride * height;
		break;
	default:
		return NULL;
	}
	data = ttLibC_malloc(memory_size);
	if(data == NULL) {
		return NULL;
	}
	memset(data, 0, memory_size);
	bgr = ttLibC_Bgr_make(
		NULL,
		sub_type,
		width,
		height,
		stride,
		data,
		memory_size,
		true,
		0,
		1000);
	if(bgr == NULL) {
		ttLibC_free(data);
		return NULL;
	}
	bgr->data = data;
	bgr->inherit_super.inherit_super.is_non_copy = false;
	return bgr;
}

