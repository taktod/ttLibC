#ifndef TTLIBC_RESAMPLER_MSIMAGERESAMPLER_H_
#define TTLIBC_RESAMPLER_MSIMAGERESAMPLER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../ttLibC_predef.h"
#include "../frame/video/bgr.h"
#include "../frame/video/yuv420.h"

typedef void *ttLibC_Resampler_MsImageResampler;

typedef ttLibC_Resampler_MsImageResampler ttLibC_MsImageResampler;

ttLibC_MsImageResampler TT_ATTRIBUTE_API *ttLibC_MsImageResampler_make();

bool TT_ATTRIBUTE_API ttLibC_MsImageResampler_ToBgr(
  ttLibC_MsImageResampler *resampler,
  ttLibC_Bgr   *dest_frame,
  ttLibC_Video *src_frame);

bool TT_ATTRIBUTE_API ttLibC_MsImageResampler_ToYuv420(
  ttLibC_MsImageResampler *resampler,
  ttLibC_Yuv420 *dest_frame,
  ttLibC_Video  *src_frame);

void TT_ATTRIBUTE_API ttLibC_MsImageResampler_close(ttLibC_MsImageResampler **resampler);

#ifdef __cplusplus
}
#endif


#endif
