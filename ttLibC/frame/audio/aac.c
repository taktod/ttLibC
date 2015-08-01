/*
 * @file   aac.c
 * @brief  aac frame information
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/23
 * TODO need to make adts analyze task.
 * TODO need to make dsi analyze task. (needed for flv.)
 */

#include "aac.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../log.h"

typedef ttLibC_Frame_Audio_Aac ttLibC_Aac_;

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
		uint32_t timebase) {
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
		aac->inherit_super.inherit_super.data = NULL;
	}
	else {
		if(!aac->inherit_super.inherit_super.is_non_copy) {
			if(non_copy_mode || aac->inherit_super.inherit_super.data_size < data_size) {
				free(aac->inherit_super.inherit_super.data);
				aac->inherit_super.inherit_super.data = NULL;
			}
			else {
				data_size_ = aac->inherit_super.inherit_super.data_size;
			}
		}
	}
	aac->type                                    = type;
	aac->inherit_super.channel_num               = channel_num;
	aac->inherit_super.sample_rate               = sample_rate;
	aac->inherit_super.sample_num                = sample_num;
	aac->inherit_super.inherit_super.buffer_size = buffer_size_;
	aac->inherit_super.inherit_super.data_size   = data_size_;
	aac->inherit_super.inherit_super.is_non_copy = non_copy_mode;
	aac->inherit_super.inherit_super.pts         = pts;
	aac->inherit_super.inherit_super.timebase    = timebase;
	aac->inherit_super.inherit_super.type        = frameType_aac;
	if(non_copy_mode) {
		aac->inherit_super.inherit_super.data = data;
	}
	else {
		if(aac->inherit_super.inherit_super.data == NULL) {
			aac->inherit_super.inherit_super.data = malloc(data_size);
			if(aac->inherit_super.inherit_super.data == NULL) {
				ERR_PRINT("failed to allocate memory for data.");
				if(prev_frame == NULL) {
					free(aac);
				}
				return NULL;
			}
		}
		memcpy(aac->inherit_super.inherit_super.data, data, data_size);
	}
	return (ttLibC_Aac *)aac;
}

/*
 * analyze aac frame and make data.
 * @param prev_frame reuse frame
 * @param data       aac binary data.
 * @param data_size  data size
 * @param pts        pts for aac frame.
 * @param timebase   timebase for pts.
 * @return aac object
 */
ttLibC_Aac *ttLibC_Aac_makeFrame(
		ttLibC_Aac *prev_frame,
		void *data,
		size_t data_size,
		uint64_t pts,
		uint32_t timebase) {
	// TODO make this.
	return NULL;
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
	if(target->inherit_super.inherit_super.type != frameType_aac) {
		ERR_PRINT("found non aac frame in aac_close.");
		return;
	}
	if(!target->inherit_super.inherit_super.is_non_copy) {
		free(target->inherit_super.inherit_super.data);
	}
	free(target);
	*frame = NULL;
}
