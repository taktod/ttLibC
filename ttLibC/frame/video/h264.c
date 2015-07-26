/*
 * @file   h264.c
 * @brief  h264 image frame information
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/24
 */

#include "h264.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../util/hexUtil.h"
#include "../../log.h"

/*
 * h264 frame definition(detail)
 */
typedef struct {
	/** inherit data from ttLibC_H264 */
	ttLibC_H264 inherit_super;
} ttLibC_Frame_Video_H264_;

typedef ttLibC_Frame_Video_H264_ ttLibC_H264_;

/*
 * make h264 frame
 * @param prev_frame    reuse frame.
 * @param type          type of h264
 * @param width         width of picture
 * @param height        height of picture
 * @param data          h264 data
 * @param data_size     h264 data size
 * @param non_copy_mode true:hold the data pointer. false:data will copy.
 * @param pts           pts for h264 data.
 * @param timebase      timebase number for pts.
 */
ttLibC_H264 *ttLibC_H264_make(
		ttLibC_H264 *prev_frame,
		ttLibC_H264_Type type,
		uint32_t width,
		uint32_t height,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	ttLibC_H264_ *h264 = (ttLibC_H264_ *)prev_frame;
	switch(type) {
	case H264Type_configData:
	case H264Type_slice:
	case H264Type_sliceIDR:
		break;
	case H264Type_unknown:
		ERR_PRINT("unknown is defined.");
		return NULL;
	default:
		ERR_PRINT("unknown h264 type.%d", type);
		return NULL;
	}
	if(h264 == NULL) {
		h264 = malloc(sizeof(ttLibC_H264_));
		if(h264 == NULL) {
			ERR_PRINT("failed to allocate memory for h264 frame.");
			return NULL;
		}
		h264->inherit_super.inherit_super.inherit_super.data = NULL;
	}
	else {
		if(!h264->inherit_super.inherit_super.inherit_super.is_non_copy) {
			if(non_copy_mode || h264->inherit_super.inherit_super.inherit_super.data_size < data_size) {
				free(h264->inherit_super.inherit_super.inherit_super.data);
				h264->inherit_super.inherit_super.inherit_super.data = NULL;
			}
		}
	}
	h264->inherit_super.type                 = type;
	h264->inherit_super.inherit_super.width  = width;
	h264->inherit_super.inherit_super.height = height;
	switch(type) {
	case H264Type_configData:
		h264->inherit_super.inherit_super.type = videoType_info;
		break;
	case H264Type_slice:
		h264->inherit_super.inherit_super.type = videoType_inter;
		break;
	case H264Type_sliceIDR:
		h264->inherit_super.inherit_super.type = videoType_key;
		break;
	case H264Type_unknown:
		if(prev_frame == NULL) {
			free(h264);
		}
		return NULL;
	}
	h264->inherit_super.inherit_super.inherit_super.buffer_size = data_size;
	h264->inherit_super.inherit_super.inherit_super.data_size   = data_size;
	h264->inherit_super.inherit_super.inherit_super.is_non_copy = non_copy_mode;
	h264->inherit_super.inherit_super.inherit_super.pts         = pts;
	h264->inherit_super.inherit_super.inherit_super.timebase    = timebase;
	h264->inherit_super.inherit_super.inherit_super.type        = frameType_h264;
	if(non_copy_mode) {
		h264->inherit_super.inherit_super.inherit_super.data = data;
	}
	else {
		if(h264->inherit_super.inherit_super.inherit_super.data == NULL) {
			h264->inherit_super.inherit_super.inherit_super.data = malloc(data_size);
			if(h264->inherit_super.inherit_super.inherit_super.data == NULL) {
				ERR_PRINT("failed to allocate memory for data.");
				if(prev_frame == NULL) {
					free(h264);
				}
				return NULL;
			}
		}
		memcpy(h264->inherit_super.inherit_super.inherit_super.data, data, data_size);
	}
	return (ttLibC_H264 *)h264;
}

/*
 * analyze info of one nal.
 * @param info      pointer for info data.(update with data.)
 * @param data      data for analyze
 * @param data_size data size
 */
bool ttLibC_H264_getNalInfo(ttLibC_H264_NalInfo* info, uint8_t *data, size_t data_size) {
	if(info == NULL) {
		return false;
	}
//	info->pos           = 0;
	info->data_pos      = 0;
	info->nal_unit_type = H264NalType_error;
	info->nal_size      = 0;
	size_t pos = 0;
	for(size_t i = 0;i < data_size; ++ i, ++ data) {
		if((*data) != 0) {
			if(i - pos == 2 || i - pos == 3) {
				if((*data) == 1) {
					if(info->nal_unit_type != H264NalType_error) {
						// 次のnalが見つかったということは必要なデータがみつかったということ。
//						info->nal_size = pos - info->pos;
						info->nal_size = pos;
						break;
					}
//					info->pos = pos;
					info->data_pos = i + 1;
				}
			}
			else if(info->nal_unit_type == H264NalType_error && info->data_pos != 0) {
				if(((*data) & 0x80) != 0) {
					ERR_PRINT("forbidden zero bit is not zero.");
					return false;
				}
				else {
					info->nal_unit_type = (*data) & 0x1F;
				}
			}
			pos = i + 1;
		}
	}
	if(info->nal_size == 0) {
		info->nal_size = pos;
	}
	return true;
}

/*
 * close frame
 * @param frame
 */
void ttLibC_H264_close(ttLibC_H264 **frame) {
	ttLibC_H264_ *target = (ttLibC_H264_ *)*frame;
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.inherit_super.inherit_super.type != frameType_h264) {
		ERR_PRINT("found non h264 frame in h264_close.");
		return;
	}
	if(!target->inherit_super.inherit_super.inherit_super.is_non_copy) {
		free(target->inherit_super.inherit_super.inherit_super.data);
	}
	free(target);
	*frame = NULL;
}
