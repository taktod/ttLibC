/**
 * @file   flvReader.h
 * @brief  
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/12
 */

#ifndef TTLIBC_CONTAINER_FLV_FLVREADER_H_
#define TTLIBC_CONTAINER_FLV_FLVREADER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../flv.h"

#include "flvTag.h"
#include "type/audioTag.h"
#include "type/headerTag.h"
#include "type/metaTag.h"
#include "type/videoTag.h"

typedef enum {
	size,
	body
} ttLibC_FlvReader_Status;

typedef struct {
	ttLibC_FlvReader inherit_super;

	// 使い回すtagオブジェクト
	ttLibC_FlvTag *flv_tag; // reuse tag
	ttLibC_FlvAudioTag *audio_tag;
	ttLibC_FlvVideoTag *video_tag;

	// 処理関連
	ttLibC_Flv_Type type;
	ttLibC_FlvReader_Status status;
	size_t target_size; // 読み込むべきデータサイズ

	// 一時バッファ関連
	uint8_t *tmp_buffer;
	size_t tmp_buffer_size;
	size_t tmp_buffer_next_pos;
} ttLibC_ContainerReader_FlvReader_;

typedef ttLibC_ContainerReader_FlvReader_ ttLibC_FlvReader_;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_FLV_FLVREADER_H_ */
