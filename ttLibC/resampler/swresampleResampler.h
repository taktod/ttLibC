/**
 * @file   swresampleResampler.h
 * @brief  
 * @author taktod
 * @date   2017/04/28
 */

#ifndef TTLIBC_RESAMPLER_SWRESAMPLERESAMPLER_H_
#define TTLIBC_RESAMPLER_SWRESAMPLERESAMPLER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../ttLibC_predef.h"
#include "../frame/audio/pcms16.h"
#include "../frame/audio/pcmf32.h"
#include "../frame/frame.h"

typedef struct ttLibC_Resampler_SwresampleResampler {
  /** target sample_rate */
  uint32_t sample_rate;
  /** target channel_num */
  uint32_t channel_num;
} ttLibC_Resampler_SwresampleResampler;

typedef ttLibC_Resampler_SwresampleResampler ttLibC_SwresampleResampler;

typedef bool (* ttLibC_getSwresampleFrameFunc)(void *ptr, ttLibC_Frame *frame);

ttLibC_SwresampleResampler TT_ATTRIBUTE_API *ttLibC_SwresampleResampler_make(
    ttLibC_Frame_Type input_frame_type,
    uint32_t          input_sub_type,
    uint32_t          input_sample_rate,
    uint32_t          input_channel_num,
    ttLibC_Frame_Type output_frame_type,
    uint32_t          output_sub_type,
    uint32_t          output_sample_rate,
    uint32_t          output_channel_num);

bool TT_ATTRIBUTE_API ttLibC_SwresampleResampler_resample(
    ttLibC_SwresampleResampler *resampler,
    ttLibC_Frame *frame,
    ttLibC_getSwresampleFrameFunc callback,
    void *ptr);

void TT_ATTRIBUTE_API ttLibC_SwresampleResampler_close(ttLibC_SwresampleResampler **resampler);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_RESAMPLER_SWRESAMPLERESAMPLER_H_ */
