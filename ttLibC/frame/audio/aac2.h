/**
 * @file   aac2.h
 * @brief  aac frame information
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2019/11/19
 */

#ifndef TTLIBC_FRAME_AUDIO_AAC2_H_
#define TTLIBC_FRAME_AUDIO_AAC2_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../../ttLibC_predef.h"
#include "audio.h"

typedef enum ttLibC_Aac2_Type {
  /** with global header. */
  Aac2Type_raw,
  /** global header for aac frame */
  Aac2Type_asi
} ttLibC_Aac2_Type;

// no info for object-type

typedef struct ttLibC_Frame_Audio_Aac2 {
  ttLibC_Audio     inherit_super;
  ttLibC_Aac2_Type type;
  uint32_t         object_type;
} ttLibC_Frame_Audio_Aac2;

typedef ttLibC_Frame_Audio_Aac2 ttLibC_Aac2;

ttLibC_Aac2 TT_ATTRIBUTE_API *ttLibC_Aac2_make(
    ttLibC_Aac2 *prev_frame,
    ttLibC_Aac2_Type type,
		uint32_t sample_rate,
		uint32_t sample_num,
		uint32_t channel_num,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		uint32_t object_type);

ttLibC_Aac2 TT_ATTRIBUTE_API *ttLibC_Aac2_clone(
    ttLibC_Aac2 *prev_frame,
    ttLibC_Aac2 *src_frame);

ttLibC_Aac2 TT_ATTRIBUTE_API *ttLibC_Aac2_getFrame(
    ttLibC_Aac2 *prev_frame,
    void *data,
    size_t data_size,
    bool non_copy_mode,
    uint64_t pts,
    uint32_t timebase);

void TT_ATTRIBUTE_API ttLibC_Aac2_close(ttLibC_Aac2 **frame);


size_t TT_ATTRIBUTE_API ttLibC_Aac2_makeAdtsHeader(
    ttLibC_Aac2 *target,
    void *data,
    size_t data_size);

size_t TT_ATTRIBUTE_API ttLibC_Aac2_makeAsiHeader(
    ttLibC_Aac2 *target,
    void *data,
    size_t data_size);

size_t TT_ATTRIBUTE_API ttLibC_Aac2_makeAsiHeaderWithParams(
    uint32_t object_type,
    uint32_t sample_rate,
    uint32_t channel_num,
    void *data,
    size_t data_size);

uint32_t TT_ATTRIBUTE_API ttLibC_Aac2_getConfigCrc32(ttLibC_Aac2 *aac);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
