/*
 * @file   pcms16.c
 * @brief  pcms16 frame information.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/19
 */

#include <string.h>
#include <stdlib.h>

#include "pcms16.h"
#include "../../log.h"
#include "../../allocator.h"

/*
 * make pcms16 frame
 * @param prev_frame    reuse frame. if NULL, make new frame object.
 * @param type          type of pcms16
 * @param sample_rate   sample rate of data.
 * @param sample_num    sample number of data.
 * @param channel_num   channel number of data. 1:monoral 2:stereo
 * @param data          pcm data
 * @param data_size     pcm data size
 * @param l_data        pointer for l_data
 * @param l_stride      stride(data_size) for l_data
 * @param r_data        pointer for r_data
 * @param r_stride      stride(data_size) for r_data
 * @param non_copy_mode true:hold the data pointer. false:data will copy.
 * @param pts           pts for pcm data.
 * @param timebase      timebase number for pts.
 * @return pcms16 object.
 */
ttLibC_PcmS16 *ttLibC_PcmS16_make(
		ttLibC_PcmS16 *prev_frame,
		ttLibC_PcmS16_Type type,
		uint32_t sample_rate,
		uint32_t sample_num,
		uint32_t channel_num,
		void *data,
		size_t data_size,
		void *l_data,
		uint32_t l_stride,
		void *r_data,
		uint32_t r_stride,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	ttLibC_PcmS16 *pcms16 = prev_frame;
	size_t data_size_ = data_size;
	size_t buffer_size_ = data_size;
	uint32_t l_step = 2;
	uint32_t r_step = 0;
	switch(type) {
	case PcmS16Type_littleEndian:
	case PcmS16Type_bigEndian:
		if(channel_num == 2) {
			l_step = 4;
			r_step = 4;
		}
		break;
	case PcmS16Type_bigEndian_planar:
	case PcmS16Type_littleEndian_planar:
		if(channel_num == 2) {
			r_step = 2;
		}
		break;
	default:
		ERR_PRINT("unknown pcmS16 type.%d", type);
		return NULL;
	}
	if(pcms16 == NULL) {
		pcms16 = (ttLibC_PcmS16 *)ttLibC_malloc(sizeof(ttLibC_PcmS16));
		if(pcms16 == NULL) {
			ERR_PRINT("failed to allocate memory for pcms16 frame.");
			return NULL;
		}
		pcms16->inherit_super.inherit_super.data = NULL;
	}
	else {
		if(!pcms16->inherit_super.inherit_super.is_non_copy) {
			if(non_copy_mode || pcms16->inherit_super.inherit_super.data_size < data_size) {
				ttLibC_free(pcms16->inherit_super.inherit_super.data);
				pcms16->inherit_super.inherit_super.data = NULL;
			}
			else {
				data_size_ = pcms16->inherit_super.inherit_super.data_size;
			}
		}
	}
	pcms16->type                                    = type;
	pcms16->l_data                                  = l_data;
	pcms16->l_stride                                = l_stride;
	pcms16->l_step                                  = l_step;
	pcms16->r_data                                  = r_data;
	pcms16->r_stride                                = r_stride;
	pcms16->r_step                                  = r_step;
	pcms16->inherit_super.channel_num               = channel_num;
	pcms16->inherit_super.sample_num                = sample_num;
	pcms16->inherit_super.sample_rate               = sample_rate;
	pcms16->inherit_super.inherit_super.buffer_size = channel_num * sample_num * 2;
	pcms16->inherit_super.inherit_super.data_size   = data_size_;
	pcms16->inherit_super.inherit_super.is_non_copy = non_copy_mode;
	pcms16->inherit_super.inherit_super.pts         = pts;
	pcms16->inherit_super.inherit_super.dts         = 0;
	pcms16->inherit_super.inherit_super.timebase    = timebase;
	pcms16->inherit_super.inherit_super.type        = frameType_pcmS16;
	if(non_copy_mode) {
		pcms16->inherit_super.inherit_super.data = data;
	}
	else {
		if(pcms16->inherit_super.inherit_super.data == NULL) {
			pcms16->inherit_super.inherit_super.data = ttLibC_malloc(data_size);
			if(pcms16->inherit_super.inherit_super.data == NULL) {
				ERR_PRINT("failed to allocate memory for data.");
				if(prev_frame == NULL) {
					ttLibC_free(pcms16);
				}
				return NULL;
			}
		}
		memcpy(pcms16->inherit_super.inherit_super.data, data, data_size);
		if(l_data != NULL) {
			pcms16->l_data = pcms16->inherit_super.inherit_super.data + (l_data - data);
		}
		if(r_data != NULL) {
			pcms16->r_data = pcms16->inherit_super.inherit_super.data + (r_data - data);
		}
	}
	return pcms16;
}

static ttLibC_PcmS16 *PcmS16_cloneBigEndian(
		ttLibC_PcmS16 *prev_frame,
		ttLibC_PcmS16 *src_frame) {
	ERR_PRINT("clone bigendian is not created");
	return NULL;
}

static ttLibC_PcmS16 *PcmS16_cloneLittleEndian(
		ttLibC_PcmS16 *prev_frame,
		ttLibC_PcmS16 *src_frame) {
	uint8_t *data = NULL;
	uint32_t data_size = src_frame->inherit_super.sample_num * src_frame->inherit_super.channel_num * 2;
	uint32_t buffer_size = data_size;
	bool allocflag = false;
	if(prev_frame != NULL) {
		if(!prev_frame->inherit_super.inherit_super.is_non_copy) {
			if(prev_frame->inherit_super.inherit_super.data_size >= data_size) {
				data = prev_frame->inherit_super.inherit_super.data;
				data_size = prev_frame->inherit_super.inherit_super.data_size;
			}
			else {
				ttLibC_free(prev_frame->inherit_super.inherit_super.data);
			}
			prev_frame->inherit_super.inherit_super.data = NULL;
			prev_frame->inherit_super.inherit_super.is_non_copy = true;
		}
	}
	if(data == NULL) {
		data = ttLibC_malloc(buffer_size);
		if(data == NULL) {
			ERR_PRINT("failed to allocate buffer for pcms16 clone.");
			return NULL;
		}
		allocflag = true;
	}
	memcpy(data, src_frame->l_data, buffer_size);
	ttLibC_PcmS16 *cloned_frame = ttLibC_PcmS16_make(
			prev_frame,
			PcmS16Type_littleEndian,
			src_frame->inherit_super.sample_rate,
			src_frame->inherit_super.sample_num,
			src_frame->inherit_super.channel_num,
			data,
			data_size,
			data,
			buffer_size,
			NULL,
			0,
			true,
			src_frame->inherit_super.inherit_super.pts,
			src_frame->inherit_super.inherit_super.timebase);
	if(cloned_frame != NULL) {
		cloned_frame->inherit_super.inherit_super.is_non_copy = false;
	}
	else {
		if(allocflag) {
			ttLibC_free(data);
		}
	}
	return cloned_frame;
}

static ttLibC_PcmS16 *PcmS16_cloneBigEndianPlanar(
		ttLibC_PcmS16 *prev_frame,
		ttLibC_PcmS16 *src_frame) {
	ERR_PRINT("clone bigendian planar is not created");
	return NULL;
}

static ttLibC_PcmS16 *PcmS16_cloneLittleEndianPlanar(
		ttLibC_PcmS16 *prev_frame,
		ttLibC_PcmS16 *src_frame) {
	uint8_t *data = NULL;
	uint32_t plane_size = src_frame->inherit_super.sample_num * 2;
	uint32_t data_size = plane_size * src_frame->inherit_super.channel_num;
	uint32_t buffer_size = data_size;
	bool allocflag = false;
	if(prev_frame != NULL) {
		if(!prev_frame->inherit_super.inherit_super.is_non_copy) {
			if(prev_frame->inherit_super.inherit_super.data_size >= data_size) {
				data = prev_frame->inherit_super.inherit_super.data;
				data_size = prev_frame->inherit_super.inherit_super.data_size;
			}
			else {
				ttLibC_free(prev_frame->inherit_super.inherit_super.data);
			}
			prev_frame->inherit_super.inherit_super.data = NULL;
			prev_frame->inherit_super.inherit_super.is_non_copy = true;
		}
	}
	if(data == NULL) {
		data = ttLibC_malloc(buffer_size);
		if(data == NULL) {
			ERR_PRINT("failed to allocate buffer for pcms16 clone.");
			return NULL;
		}
		allocflag = true;
	}
	memcpy(data, src_frame->l_data, plane_size);
	if(src_frame->r_data != NULL) {
		memcpy(data + plane_size, src_frame->r_data, plane_size);
	}
	ttLibC_PcmS16 *cloned_frame = ttLibC_PcmS16_make(
			prev_frame,
			PcmS16Type_littleEndian_planar,
			src_frame->inherit_super.sample_rate,
			src_frame->inherit_super.sample_num,
			src_frame->inherit_super.channel_num,
			data,
			data_size,
			data,
			plane_size,
			data + plane_size,
			plane_size,
			true,
			src_frame->inherit_super.inherit_super.pts,
			src_frame->inherit_super.inherit_super.timebase);
	if(cloned_frame != NULL) {
		cloned_frame->inherit_super.inherit_super.is_non_copy = false;
	}
	else {
		if(allocflag) {
			ttLibC_free(data);
		}
	}
	return cloned_frame;
}

/*
 * make clone frame.
 * always make copy buffer on it.
 * @param prev_frame reuse frame object.
 * @param src_frame  source of clone.
 */
ttLibC_PcmS16 *ttLibC_PcmS16_clone(
		ttLibC_PcmS16 *prev_frame,
		ttLibC_PcmS16 *src_frame) {
	if(src_frame == NULL) {
		return NULL;
	}
	if(src_frame->inherit_super.inherit_super.type != frameType_pcmS16) {
		ERR_PRINT("try to clone non pcms16 frame.");
		return NULL;
	}
	if(prev_frame != NULL && prev_frame->inherit_super.inherit_super.type != frameType_pcmS16) {
		ERR_PRINT("try to use non pcms16 frame for reuse.");
		return NULL;
	}
	switch(src_frame->type) {
	case PcmS16Type_bigEndian:
		return PcmS16_cloneBigEndian(prev_frame, src_frame);
	case PcmS16Type_littleEndian:
		return PcmS16_cloneLittleEndian(prev_frame, src_frame);
	case PcmS16Type_bigEndian_planar:
		return PcmS16_cloneBigEndianPlanar(prev_frame, src_frame);
	case PcmS16Type_littleEndian_planar:
		return PcmS16_cloneLittleEndianPlanar(prev_frame, src_frame);
	default:
		break;
	}
	return NULL;
}

/*
 * close frame
 * @param frame
 */
void ttLibC_PcmS16_close(ttLibC_PcmS16 **frame) {
	ttLibC_PcmS16 *target = (*frame);
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.inherit_super.type != frameType_pcmS16) {
		ERR_PRINT("found non pcmS16 frame in pcmS16_close.");
		return;
	}
	if(!target->inherit_super.inherit_super.is_non_copy) {
		ttLibC_free(target->inherit_super.inherit_super.data);
	}
	ttLibC_free(target);
	*frame = NULL;
}
