/*
 * @file   aac.c
 * @brief  aac frame information
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/23
 * TODO need to make dsi analyze task. (needed for flv.)
 */

#include "aac.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../log.h"

#include "../../util/bitUtil.h"

typedef struct {
	ttLibC_Aac inherit_super;
	uint64_t dsi_info; // for raw, need to have dsi_information.
} ttLibC_Frame_Audio_Aac_;

typedef ttLibC_Frame_Audio_Aac_ ttLibC_Aac_;

static uint32_t sample_rate_table[] = {
		96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000
};


/*
 * make aac frame.
 * @param prev_frame    reuse frame.
 * @param type          type of aac
 * @param sample_rate   sample rate of data
 * @param sample_num    sample num of data(1024 fixed?)
 * @param channel_num   channel number of data
 * @param data          aac data
 * @param data_size     aac data size
 * @param non_copy_mode true:hold the data pointer. false:data will copy.
 * @param pts           pts for aac data.
 * @param timebase      timebase number for pts.
 * @return aac object.
 */
ttLibC_Aac *ttLibC_Aac_make(
		ttLibC_Aac *prev_frame,
		ttLibC_Aac_Type type,
		uint32_t sample_rate,
		uint32_t sample_num,
		uint32_t channel_num,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		uint64_t dsi_info) {
	ttLibC_Aac_ *aac = (ttLibC_Aac_ *)prev_frame;
	size_t data_size_ = data_size;
	size_t buffer_size_ = data_size;
	switch(type) {
	case AacType_adts:
	case AacType_raw:
		break;
	default:
		ERR_PRINT("unknown aac type.%d", type);
		return NULL;
	}
	if(aac == NULL) {
		aac = malloc(sizeof(ttLibC_Aac_));
		if(aac == NULL) {
			ERR_PRINT("failed to allocate memory for aac frame.");
			return NULL;
		}
		aac->inherit_super.inherit_super.inherit_super.data = NULL;
	}
	else {
		if(!aac->inherit_super.inherit_super.inherit_super.is_non_copy) {
			if(non_copy_mode || aac->inherit_super.inherit_super.inherit_super.data_size < data_size) {
				free(aac->inherit_super.inherit_super.inherit_super.data);
				aac->inherit_super.inherit_super.inherit_super.data = NULL;
			}
			else {
				data_size_ = aac->inherit_super.inherit_super.inherit_super.data_size;
			}
		}
	}
	aac->dsi_info                                              = dsi_info;
	aac->inherit_super.type                                    = type;
	aac->inherit_super.inherit_super.channel_num               = channel_num;
	aac->inherit_super.inherit_super.sample_rate               = sample_rate;
	aac->inherit_super.inherit_super.sample_num                = sample_num;
	aac->inherit_super.inherit_super.inherit_super.buffer_size = buffer_size_;
	aac->inherit_super.inherit_super.inherit_super.data_size   = data_size_;
	aac->inherit_super.inherit_super.inherit_super.is_non_copy = non_copy_mode;
	aac->inherit_super.inherit_super.inherit_super.pts         = pts;
	aac->inherit_super.inherit_super.inherit_super.timebase    = timebase;
	aac->inherit_super.inherit_super.inherit_super.type        = frameType_aac;
	if(non_copy_mode) {
		aac->inherit_super.inherit_super.inherit_super.data = data;
	}
	else {
		if(aac->inherit_super.inherit_super.inherit_super.data == NULL) {
			aac->inherit_super.inherit_super.inherit_super.data = malloc(data_size);
			if(aac->inherit_super.inherit_super.inherit_super.data == NULL) {
				ERR_PRINT("failed to allocate memory for data.");
				if(prev_frame == NULL) {
					free(aac);
				}
				return NULL;
			}
		}
		memcpy(aac->inherit_super.inherit_super.inherit_super.data, data, data_size);
	}
	return (ttLibC_Aac *)aac;
}

/*
 * make clone frame.
 * always make copy buffer on it.
 * @param prev_frame reuse frame object.
 * @param src_frame  source of clone.
 */
ttLibC_Aac *ttLibC_Aac_clone(
		ttLibC_Aac *prev_frame,
		ttLibC_Aac *src_frame) {
	if(src_frame == NULL) {
		return NULL;
	}
	if(src_frame->inherit_super.inherit_super.type != frameType_aac) {
		ERR_PRINT("try to clone non aac frame.");
		return NULL;
	}
	if(prev_frame != NULL && prev_frame->inherit_super.inherit_super.type != frameType_aac) {
		ERR_PRINT("try to use non aac frame for reuse.");
		return NULL;
	}
	ttLibC_Aac_ *src_frame_ = (ttLibC_Aac_ *)src_frame;
	return ttLibC_Aac_make(
			prev_frame,
			src_frame->type,
			src_frame->inherit_super.sample_rate,
			src_frame->inherit_super.sample_num,
			src_frame->inherit_super.channel_num,
			src_frame->inherit_super.inherit_super.data,
			src_frame->inherit_super.inherit_super.buffer_size,
			false,
			src_frame->inherit_super.inherit_super.pts,
			src_frame->inherit_super.inherit_super.timebase,
			src_frame_->dsi_info);
}

/*
 * analyze aac frame and make data.
 * only support adts.
 * @param prev_frame reuse frame
 * @param data       aac binary data.
 * @param data_size  data size
 * @param pts        pts for aac frame.
 * @param timebase   timebase for pts.
 * @return aac object
 */
ttLibC_Aac *ttLibC_Aac_getFrame(
		ttLibC_Aac *prev_frame,
		void *data,
		size_t data_size,
		uint64_t pts,
		uint32_t timebase) {
	/*
	 * bit memo for adts aac.
	 * 12bit syncbit fill with 1
	 * 1bit id
	 * 2bit layer
	 * 1bit protection absent
	 * 2bit profile
	 * 4bit sampling frequence index
	 * 1bit private bit
	 * 3bit channel configuration
	 * 1bit original flag
	 * 1bit home
	 * 1bit copyright identification bit
	 * 1bit copyright identification start
	 * 13bit frame size
	 * 11bit adts buffer full ness
	 * 2bit no raw data blocks in frame.
	 */
	ttLibC_BitReader *reader = ttLibC_BitReader_make(data, data_size, BitReaderType_default);
	if(ttLibC_BitReader_bit(reader, 12) != 0xFFF) {
		ERR_PRINT("sync bit is invalid.");
		ttLibC_BitReader_close(&reader);
		return NULL;
	}
	ttLibC_BitReader_bit(reader, 1);
	ttLibC_BitReader_bit(reader, 2);
	ttLibC_BitReader_bit(reader, 1);
	ttLibC_BitReader_bit(reader, 2);
	uint32_t sample_rate_index = ttLibC_BitReader_bit(reader, 4);
	uint32_t sample_rate = sample_rate_table[sample_rate_index];
	ttLibC_BitReader_bit(reader, 1);
	uint32_t channel_num = ttLibC_BitReader_bit(reader, 3);
	ttLibC_BitReader_bit(reader, 1);
	ttLibC_BitReader_bit(reader, 1);
	ttLibC_BitReader_bit(reader, 1);
	ttLibC_BitReader_bit(reader, 1);
	uint32_t frame_size = ttLibC_BitReader_bit(reader, 13);
	ttLibC_BitReader_bit(reader, 11);
	ttLibC_BitReader_bit(reader, 2);
	ttLibC_BitReader_close(&reader);
	// this frame_size includes the adts header.
	return ttLibC_Aac_make(
			prev_frame,
			AacType_adts,
			sample_rate,
			1024,
			channel_num,
			data,
			frame_size,
			true,
			pts,
			timebase,
			0);
}

/*
 * close frame
 * @param frame
 */
void ttLibC_Aac_close(ttLibC_Aac **frame) {
	ttLibC_Aac_ *target = (ttLibC_Aac_ *)*frame;
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.inherit_super.inherit_super.type != frameType_aac) {
		ERR_PRINT("found non aac frame in aac_close.");
		return;
	}
	if(!target->inherit_super.inherit_super.inherit_super.is_non_copy) {
		free(target->inherit_super.inherit_super.inherit_super.data);
	}
	free(target);
	*frame = NULL;
}
