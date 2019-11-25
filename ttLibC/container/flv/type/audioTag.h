/**
 * @file   audioTag.h
 * @brief  flvTag for audio
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/12
 */

#ifndef TTLIBC_CONTAINER_FLV_TYPE_AUDIOTAG_H_
#define TTLIBC_CONTAINER_FLV_TYPE_AUDIOTAG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../../../ttLibC_predef.h"
#include "../flvTag.h"
#include "../flvWriter.h"

typedef struct ttLibC_Container_Flv_FlvAudioTag {
	ttLibC_FlvTag inherit_super;
	uint8_t codec_id;
	uint32_t sample_rate;
	uint32_t bit_count;
	uint32_t channel_num;
	ttLibC_Frame_Type frame_type;
//	uint64_t aac_dsi_info; // aac dsi info will be copy from prev tag.
} ttLibC_Container_Flv_FlvAudioTag;

typedef ttLibC_Container_Flv_FlvAudioTag ttLibC_FlvAudioTag;

ttLibC_FlvAudioTag TT_ATTRIBUTE_INNER *ttLibC_FlvAudioTag_make(
		ttLibC_FlvTag *prev_tag,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		uint32_t track_id,
		uint8_t codec_id,
		uint8_t sample_rate_flag,
		uint8_t bit_count_flag,
		uint8_t channel_flag);

ttLibC_FlvAudioTag TT_ATTRIBUTE_INNER *ttLibC_FlvAudioTag_getTag(
		ttLibC_FlvTag *prev_tag,
		uint8_t *data,
		size_t data_size) ;

bool TT_ATTRIBUTE_INNER ttLibC_FlvAudioTag_getFrame(
		ttLibC_FlvAudioTag *audio_tag,
		ttLibC_getFrameFunc callback,
		void *ptr);

bool TT_ATTRIBUTE_INNER ttLibC_FlvAudioTag_writeTag(
		ttLibC_FlvWriter_ *writer,
		ttLibC_Frame *frame,
		ttLibC_ContainerWriteFunc callback,
		void *ptr);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_FLV_TYPE_AUDIOTAG_H_ */
