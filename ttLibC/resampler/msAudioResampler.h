#ifndef TTLIBC_RESAMPLER_MSAUDIORESAMPLER_H_
#define TTLIBC_RESAMPLER_MSAUDIORESAMPLER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../ttLibC_predef.h"
#include "../frame/audio/pcms16.h"
#include "../frame/audio/pcmf32.h"

typedef void *ttLibC_Resampler_MsAudioResampler;
typedef ttLibC_Resampler_MsAudioResampler ttLibC_MsAudioResampler;

typedef bool (* ttLibC_getMsAudioResamplerFrameFunc)(void *ptr, ttLibC_Audio *audio);

ttLibC_MsAudioResampler TT_ATTRIBUTE_API *ttLibC_MsAudioResampler_make(
  ttLibC_Frame_Type in_type,
  uint32_t in_sub_type,
  uint32_t in_sample_rate,
  uint32_t in_channel_num,
  ttLibC_Frame_Type out_type,
  uint32_t out_sub_type,
  uint32_t out_sample_rate,
  uint32_t out_channel_num);

bool TT_ATTRIBUTE_API ttLibC_MsAudioResampler_resample(
  ttLibC_MsAudioResampler *resampler,
  ttLibC_Audio *src_frame,
  ttLibC_getMsAudioResamplerFrameFunc callback,
  void *ptr);

void TT_ATTRIBUTE_API ttLibC_MsAudioResampler_close(ttLibC_MsAudioResampler **resampler);

#ifdef __cplusplus
}
#endif

#endif
