/*
 * @file   pcmf32.c
 * @brief  pcmf32 frame information
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/08
 */

#include "pcmf32.h"
#include "../../log.h"
#include "../../allocator.h"

typedef ttLibC_Frame_Audio_PcmF32 ttLibC_PcmF32_;

/*
 * make pcmf32 frame
 * @param prev_frame    reuse frame. if NULL, make new frame object.
 * @param type          type of pcmf32
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
 * @return pcmf32 object.
 */
ttLibC_PcmF32 *ttLibC_PcmF32_make(
		ttLibC_PcmF32 *prev_frame,
		ttLibC_PcmF32_Type type,
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
	uint32_t l_step = 4;
	uint32_t r_step = 0;
	switch(type) {
	case PcmF32Type_interleave:
		if(channel_num == 2) {
			l_step = 8;
			r_step = 8;
		}
		break;
	case PcmF32Type_planar:
		if(channel_num == 2) {
			r_step = 4;
		}
		break;
	default:
		ERR_PRINT("unknown pcmf32 type.%d", type);
	}
	ttLibC_PcmF32_ *pcmf32 = (ttLibC_PcmF32_ *)ttLibC_Audio_make(
			(ttLibC_Audio *)prev_frame,
			sizeof(ttLibC_PcmF32_),
			frameType_pcmF32,
			sample_rate,
			sample_num,
			channel_num,
			data,
			data_size,
			non_copy_mode,
			pts,
			timebase);
	if(pcmf32 != NULL) {
		pcmf32->type     = type;
		pcmf32->l_stride = l_stride;
		pcmf32->l_step   = l_step;
		pcmf32->r_stride = r_stride;
		pcmf32->r_step   = r_step;
		if(pcmf32->inherit_super.inherit_super.is_non_copy) {
			pcmf32->l_data = l_data;
			pcmf32->r_data = r_data;
		}
		else {
			if(l_data != NULL) {
				pcmf32->l_data = pcmf32->inherit_super.inherit_super.data + (l_data - data);
			}
			if(r_data != NULL) {
				pcmf32->r_data = pcmf32->inherit_super.inherit_super.data + (r_data - data);
			}
		}
	}
	return (ttLibC_PcmF32 *)pcmf32;
}

static ttLibC_PcmF32 *PcmF32_cloneInterleave(
		ttLibC_PcmF32 *prev_frame,
		ttLibC_PcmF32 *src_frame) {
	uint8_t *data = NULL;
	uint32_t data_size = src_frame->inherit_super.sample_num * src_frame->inherit_super.channel_num * 4;
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
	ttLibC_PcmF32 *cloned_frame = ttLibC_PcmF32_make(
			prev_frame,
			PcmF32Type_interleave,
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

static ttLibC_PcmF32 *PcmF32_clonePlanar(
		ttLibC_PcmF32 *prev_frame,
		ttLibC_PcmF32 *src_frame) {
	uint8_t *data = NULL;
	uint32_t plane_size = src_frame->inherit_super.sample_num * 4;
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
	ttLibC_PcmF32 *cloned_frame = ttLibC_PcmF32_make(
			prev_frame,
			PcmF32Type_planar,
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

/**
 * make clone frame.
 * always make copy buffer on it.
 * @param prev_frame reuse frame object.
 * @param src_frame  source of clone.
 */
ttLibC_PcmF32 *ttLibC_PcmF32_clone(
		ttLibC_PcmF32 *prev_frame,
		ttLibC_PcmF32 *src_frame) {
	if(src_frame == NULL) {
		return NULL;
	}
	if(src_frame->inherit_super.inherit_super.type != frameType_pcmF32) {
		ERR_PRINT("try to clone non pcmf32 frame.");
		return NULL;
	}
	if(prev_frame != NULL && prev_frame->inherit_super.inherit_super.type != frameType_pcmF32) {
		ERR_PRINT("try to use non pcmf32 frame for reuse.");
		return NULL;
	}
	switch(src_frame->type) {
	case PcmF32Type_interleave:
		return PcmF32_cloneInterleave(prev_frame, src_frame);
	case PcmF32Type_planar:
		return PcmF32_clonePlanar(prev_frame, src_frame);
	default:
		break;
	}
	return NULL;
}
/*
 * close frame
 * @param frame
 */
void ttLibC_PcmF32_close(ttLibC_PcmF32 **frame) {
	ttLibC_Audio_close_((ttLibC_Audio **)frame);
}

