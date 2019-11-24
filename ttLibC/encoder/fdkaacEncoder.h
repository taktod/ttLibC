/**
 * @file   fdkaacEncoder.h
 * @brief  encode aac with fdk-aac
 * 
 * this code is under LGPLv3 license
 * 
 * @author taktod
 * @date   2019/07/15
 */

#ifndef TTLIBC_ENCODER_FDKAACENCODER_H_
#define TTLIBC_ENCODER_FDKAACENCODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../ttLibC_predef.h"
#include "../frame/audio/aac2.h"
#include "../frame/audio/pcms16.h"

typedef struct ttLibC_Encoder_FdkaacEncoder {
  uint32_t sample_rate;
  uint32_t channel_num;
  uint32_t bitrate;
} ttLibC_Encoder_FdkaacEncoder;

typedef bool (* ttLibC_FdkaacEncodeFunc)(void *ptr, ttLibC_Aac2 *aac);

typedef ttLibC_Encoder_FdkaacEncoder ttLibC_FdkaacEncoder;

ttLibC_FdkaacEncoder TT_ATTRIBUTE_API *ttLibC_FdkaacEncoder_make(
  const char *aac_type,
  uint32_t sample_rate,
  uint32_t channel_num,
  uint32_t bitrate);

bool TT_ATTRIBUTE_API ttLibC_FdkaacEncoder_encode(
  ttLibC_FdkaacEncoder *encoder,
  ttLibC_PcmS16 *frame,
  ttLibC_FdkaacEncodeFunc callback,
  void *ptr);

void TT_ATTRIBUTE_API ttLibC_FdkaacEncoder_close(ttLibC_FdkaacEncoder **encoder);

bool TT_ATTRIBUTE_API ttLibC_FdkaacEncoder_setBitrate(ttLibC_FdkaacEncoder *encoder, uint32_t value);

#ifdef __cplusplus
}
#endif

#endif
