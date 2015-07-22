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

/*
 * make pcms16 frame
 * @param prev_frame    reuse frame. if NULL, make new frame object.
 * @param type          type of pcms16
 * @param sample_rate   sample rate of data.
 * @param sample_num    sample number of data.
 * @param channel_num   channel number of data. 1:monoral 2:stereo
 * @param data          pcm data
 * @param data_size     pcm data size
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
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	ttLibC_PcmS16 *pcms16 = prev_frame;
	switch(type) {
	case PcmS16Type_littleEndian:
	case PcmS16Type_bigEndian:
	case PcmS16Type_bigEndian_planar:
	case PcmS16Type_littleEndian_planar:
		break;
	default:
		ERR_PRINT("unknown pcmS16 type.%d", type);
		return NULL;
	}
	if(pcms16 == NULL) {
		pcms16 = (ttLibC_PcmS16 *)malloc(sizeof(ttLibC_PcmS16));
		if(pcms16 == NULL) {
			ERR_PRINT("failed to allocate memory for pcms16 frame.");
			return NULL;
		}
	}
	else {
		if(!pcms16->inherit_super.inherit_super.is_non_copy) {
			if(non_copy_mode || pcms16->inherit_super.inherit_super.data_size < data_size) {
				free(pcms16->inherit_super.inherit_super.data);
				pcms16->inherit_super.inherit_super.data = NULL;
			}
		}
	}
	pcms16->type                                    = type;
	pcms16->inherit_super.channel_num               = channel_num;
	pcms16->inherit_super.sample_num                = sample_num;
	pcms16->inherit_super.sample_rate               = sample_rate;
	pcms16->inherit_super.inherit_super.buffer_size = channel_num * sample_num * 2;
	pcms16->inherit_super.inherit_super.data_size   = data_size;
	pcms16->inherit_super.inherit_super.is_non_copy = non_copy_mode;
	pcms16->inherit_super.inherit_super.pts         = pts;
	pcms16->inherit_super.inherit_super.timebase    = timebase;
	pcms16->inherit_super.inherit_super.type        = frameType_pcmS16;
	if(non_copy_mode) {
		pcms16->inherit_super.inherit_super.data = data;
	}
	else {
		if(pcms16->inherit_super.inherit_super.data == NULL) {
			pcms16->inherit_super.inherit_super.data = malloc(data_size);
			if(pcms16->inherit_super.inherit_super.data == NULL) {
				ERR_PRINT("failed to allocate memory for data.");
				if(prev_frame == NULL) {
					free(pcms16);
				}
				return NULL;
			}
		}
		memcpy(pcms16->inherit_super.inherit_super.data, data, data_size);
	}
	return pcms16;
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
		free(target->inherit_super.inherit_super.data);
	}
	free(target);
	*frame = NULL;
}
