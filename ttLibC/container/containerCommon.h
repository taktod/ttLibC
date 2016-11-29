/**
 * @file   containerCommon.h
 * @brief  container support. common header for inner use.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/11/07
 */

#ifndef TTLIBC_CONTAINER_CONTAINERCOMMON_H_
#define TTLIBC_CONTAINER_CONTAINERCOMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "container.h"
#include "misc.h"
#include "../util/stlMapUtil.h"

/**
 * definition of bit type. (just for information.)
 */
typedef uint8_t ttLibC_uint1_t;
typedef uint8_t ttLibC_uint2_t;
typedef uint8_t ttLibC_uint3_t;
typedef uint8_t ttLibC_uint4_t;
typedef uint8_t ttLibC_uint5_t;
typedef uint8_t ttLibC_uint6_t;
typedef uint8_t ttLibC_uint7_t;
typedef uint8_t ttLibC_uint8_t;
typedef uint16_t ttLibC_uint9_t;
typedef uint16_t ttLibC_uint10_t;
typedef uint16_t ttLibC_uint11_t;
typedef uint16_t ttLibC_uint12_t;
typedef uint16_t ttLibC_uint13_t;
typedef uint16_t ttLibC_uint14_t;
typedef uint16_t ttLibC_uint15_t;
typedef uint16_t ttLibC_uint16_t;

typedef uint32_t ttLibC_uint17_t;
typedef uint32_t ttLibC_uint18_t;
typedef uint32_t ttLibC_uint19_t;
typedef uint32_t ttLibC_uint20_t;
typedef uint32_t ttLibC_uint21_t;
typedef uint32_t ttLibC_uint22_t;
typedef uint32_t ttLibC_uint23_t;
typedef uint32_t ttLibC_uint24_t;
typedef uint32_t ttLibC_uint25_t;
typedef uint32_t ttLibC_uint26_t;
typedef uint32_t ttLibC_uint27_t;
typedef uint32_t ttLibC_uint28_t;
typedef uint32_t ttLibC_uint29_t;
typedef uint32_t ttLibC_uint30_t;
typedef uint32_t ttLibC_uint31_t;
typedef uint32_t ttLibC_uint32_t;

typedef enum ttLibC_ContainerWriter_Status {
	status_init_check,
	status_make_init,
	status_target_check,
	status_data_check,
	status_make_data,
	status_update,
	status_video_check,
	status_video_add,
	status_audio_check,
	status_audio_add,
	status_current_update,
} ttLibC_ContainerWriter_Status;

/**
 * common function for container make.
 * @param prev_container reuse container object.
 * @param container_size memory allocate size for container object.
 * @param container_type target container_type
 * @param data           data
 * @param data_size      data_size
 * @param non_copy_mode  true:hold the pointer / false:copy memory data.
 * @param pts            pts for container
 * @param timebase       timebase for pts.
 * @return container object.
 */
ttLibC_Container *ttLibC_Container_make(
		ttLibC_Container *prev_container,
		size_t container_size,
		ttLibC_Container_Type container_type,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase);

// void ttLibC_Container_close(ttLibC_Container **container);

/**
 * common work for containerReader make.
 * use inner only.
 * @param container_type target container type.
 * @param reader_size    sizeof object.
 * @return reader object.
 */
ttLibC_ContainerReader *ttLibC_ContainerReader_make(
		ttLibC_Container_Type container_type,
		size_t reader_size);

/**
 * common work for containerWriter make
 * use inner only.
 * @param container_type target container type.
 * @param writer_size    sizeof object.
 * @param timebase       timebase for writer.
 * @return writer object.
 */
ttLibC_ContainerWriter *ttLibC_ContainerWriter_make(
		ttLibC_Container_Type container_type,
		size_t writer_size,
		uint32_t timebase);

typedef struct ttLibC_ContainerWriter_WriteTrack {
	ttLibC_FrameQueue          *frame_queue;
	ttLibC_Frame               *h26x_configData;
	ttLibC_Frame_Type           frame_type;
	uint32_t                    counter;
	bool                        is_appending;
	ttLibC_ContainerWriter_Mode enable_mode;
	ttLibC_ContainerWriter_Mode use_mode;
} ttLibC_ContainerWriter_WriteTrack;

// ContainerWriterの詳細
typedef struct ttLibC_ContainerWriter_ {
	ttLibC_ContainerWriter     inherit_super;
	ttLibC_StlMap             *track_list;
	ttLibC_ContainerWriteFunc  callback;
	void                      *ptr;

	bool                          is_first;
	ttLibC_ContainerWriter_Status status;
	uint64_t                      current_pts_pos;
	uint64_t                      target_pos;
	uint32_t                      unit_duration;
} ttLibC_ContainerWriter_;

/*
 * trackのclose
 * writeの動作の補助部
 * appendQueueの動作(内部) // ここの動作がinit系の動作に影響するのか・・・うーん。
 * 必要なデータを参照する動作をつくっておきたい・・・けど・・・
 * theoraのデータみたいに、合算してとらない方がいいものもあるわけだけど・・・
 * まぁ一気にとってもいいんだが・・・
 * 一度しかしないので、bufferで応答してもいいかな・・・
 * まぁ、よく考えよう。
 * primaryVideoTrackCheck
 * このあたりは抜き出しておきたいね。
 */

ttLibC_ContainerWriter_WriteTrack *ttLibC_ContainerWriteTrack_make(
		size_t            track_size,
		uint32_t          track_id,
		ttLibC_Frame_Type frame_type);

bool ttLibC_ContainerWriteTrack_appendQueue(
		ttLibC_ContainerWriter_WriteTrack *track,
		ttLibC_Frame                      *frame,
		uint32_t                           timebase,
		ttLibC_ContainerWriter_Mode        enable_mode);

// これは共通した方がいいけど、writerの動作調整しなければならないわけだが・・・
// これはあとでする。
//ttLibC_ContainerWriteTrack_primaryVideoTrackCheck();

void ttLibC_ContainerWriteTrack_close(ttLibC_ContainerWriter_WriteTrack **track);

// 内部で利用するcontainerWriterのmake動作
ttLibC_ContainerWriter *ttLibC_ContainerWriter_make_(
		ttLibC_Container_Type container_type,
		size_t                writer_size,
		uint32_t              timebase,
		size_t                track_size,
		uint32_t              track_base_id,
		ttLibC_Frame_Type    *target_frame_types,
		uint32_t              types_num,
		uint32_t              unit_duration);

void ttLibC_ContainerWriter_close_(ttLibC_ContainerWriter_ **writer);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_CONTAINERCOMMON_H_ */
