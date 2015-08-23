/**
 * @file   flvWriter.h
 * @brief  
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/12
 */

#ifndef TTLIBC_CONTAINER_FLV_FLVWRITER_H_
#define TTLIBC_CONTAINER_FLV_FLVWRITER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../flv.h"
#include "flvTag.h"

#include "../misc.h"

typedef struct {
	ttLibC_FrameQueue *frame_queue;
	ttLibC_Frame_Type frame_type;
	uint32_t crc32; // crc32のみ保持しておいて、configDataはcrc32が一致する場合は同じものであるとしようか・・
	// これならaacのdsiでも使えるかも・・・
} ttLibC_FlvTrack;

typedef struct {
	ttLibC_FlvWriter inherit_super;
	ttLibC_FlvTrack video_track;
	ttLibC_FlvTrack audio_track;
	// h264の場合は、configDataをベット保持しておくという動作がほしいところ。
	// 通常のデータの場合でもqueueの形でちょっとだけ、データを保持しておかないとだめだと思われる。
	// うけとってすぐに書き出すとすると、audioとvideoの連続がずれることがあるだろう。
	bool is_first; // 初書き込み時のフラグ(flvHeaderを書き込むか決める的なやつ。)
} ttLibC_ContainerWriter_FlvWriter_;

typedef ttLibC_ContainerWriter_FlvWriter_ ttLibC_FlvWriter_;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_FLV_FLVWRITER_H_ */
