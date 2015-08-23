/**
 * @file   audioTag.h
 * @brief  
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

#include "../flvTag.h"
#include "../flvWriter.h"

typedef struct {
	ttLibC_FlvTag inherit_super;
	// typeはFlvTagTypeが保持してる。
	// ptsはContainerが保持している。
	uint8_t codec_id;
	uint32_t sample_rate;
	uint32_t bit_count;
	uint32_t channel_num;
	ttLibC_Frame_Type frame_type;
	uint64_t aac_dsi_info; // aacのdsiになるデータ12 10がデフォルト(よくあるやつ)
	// このデータは、はいっている場合は、次のaudioTagに伝搬させます。(aacのdsiをずっと共有するため。)
} ttLibC_Container_Flv_FlvAudioTag;

typedef ttLibC_Container_Flv_FlvAudioTag ttLibC_FlvAudioTag;

ttLibC_FlvAudioTag *ttLibC_FlvAudioTag_make(
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

ttLibC_FlvAudioTag *ttLibC_FlvAudioTag_getTag(
		ttLibC_FlvTag *prev_tag,
		uint8_t *data,
		size_t data_size) ;

bool ttLibC_FlvAudioTag_getFrame(
		ttLibC_FlvAudioTag *audio_tag,
		ttLibC_getFrameFunc callback,
		void *ptr);

bool ttLibC_FlvAudioTag_writeTag(
		ttLibC_FlvWriter_ *writer,
		ttLibC_Frame *frame,
		ttLibC_ContainerWriterFunc callback,
		void *ptr);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_FLV_TYPE_AUDIOTAG_H_ */