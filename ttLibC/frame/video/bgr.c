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
#include "../../log.h"

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
 */
ttLibC_Bgr *ttLibC_Bgr_make(
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
	ttLibC_Bgr *bgr = prev_frame;
	uint32_t unit_size = 3;
	switch(type) {
	case BgrType_bgr:
		break;
	case BgrType_abgr:
	case BgrType_bgra:
		unit_size = 4;
		break;
	default:
		ERR_PRINT("unknown bgr type.%d", type);
		return NULL;
	}
	if(bgr == NULL) {
		bgr = (ttLibC_Bgr *)malloc(sizeof(ttLibC_Bgr));
		if(bgr == NULL) {
			ERR_PRINT("failed to allocate memory for bgr frame.");
			return NULL;
		}
	}
	else {
		if(!bgr->inherit_super.inherit_super.is_non_copy) {
			free(bgr->inherit_super.inherit_super.data);
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
	bgr->inherit_super.inherit_super.timebase    = timebase;
	bgr->inherit_super.inherit_super.type        = frameType_bgr;
	bgr->inherit_super.inherit_super.data_size   = data_size;
	bgr->inherit_super.inherit_super.buffer_size = data_size;
	if(non_copy_mode) {
		bgr->inherit_super.inherit_super.data = data;
	}
	else {
		bgr->inherit_super.inherit_super.data = malloc(data_size);
		if(bgr->inherit_super.inherit_super.data == NULL) {
			ERR_PRINT("failed to allocate memory for data.");
			if(prev_frame == NULL) {
				free(bgr);
			}
			return NULL;
		}
		memcpy(bgr->inherit_super.inherit_super.data, data, data_size);
	}
	return bgr;
}

/*
 * close frame
 * @param frame
 */
void ttLibC_Bgr_close(ttLibC_Bgr **frame) {
	ttLibC_Bgr *target = (*frame);
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.inherit_super.type != frameType_bgr) {
		ERR_PRINT("found non bgr frame in bgr_close.");
		return;
	}
	if(!target->inherit_super.inherit_super.is_non_copy) {
		free(target->inherit_super.inherit_super.data);
	}
	free(target);
	*frame = NULL;
}

