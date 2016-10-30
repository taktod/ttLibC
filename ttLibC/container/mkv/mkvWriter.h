/**
 * @file   mkvWriter.h
 * @brief  mkv container writer.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/09/04
 */

#ifndef TTLIBC_CONTAINER_MKV_MKVWRITER_H_
#define TTLIBC_CONTAINER_MKV_MKVWRITER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../mkv.h"
#include "../misc.h"
#include "../containerCommon.h"

#include "../../util/stlMapUtil.h"
#include "../../util/dynamicBufferUtil.h"

typedef struct ttLibC_MkvWriteTrack {
	ttLibC_FrameQueue *frame_queue;
	ttLibC_Frame *h26x_configData;
	// まぁ、h264やh265のconfigDataが中途でかわってしまう場合はだめで判定しなければいけない。(エラー扱いにしないとだめ)という意味では、別になっててもいいか・・・
	ttLibC_Frame_Type frame_type;
	bool is_appending; // 追加中であるかフラグ
	uint32_t counter; // 追加フレーム数をカウントするカウンター theoraやvorbisで使う。
} ttLibC_MkvWriteTrack;


typedef struct ttLibC_ContainerWriter_MkvWriter_ {
	ttLibC_MkvWriter inherit_super;
	ttLibC_StlMap *track_list;
	ttLibC_ContainerWriter_Status status;
	ttLibC_ContainerWriteFunc callback;
	void *ptr;
	uint32_t max_unit_duration;

	bool is_first;
	uint64_t current_pts_pos; // written data pts.
	uint64_t target_pos; // chunk target pts.
} ttLibC_ContainerWriter_MkvWriter_;

typedef ttLibC_ContainerWriter_MkvWriter_ ttLibC_MkvWriter_;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MKV_MKVWRITER_H_ */
