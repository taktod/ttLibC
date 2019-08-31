#ifndef TTLIBC_RESAMPLER_MSIMAGERESAMPLER_HPP_
#define TTLIBC_RESAMPLER_MSIMAGERESAMPLER_HPP_

#include <iostream>
#include <functional>

#include "../ttLibC_predef.h"
#include "../frame/video/bgr.h"
#include "../frame/video/yuv420.h"

namespace ttLibC {
  class TT_ATTRIBUTE_API MsImageResampler {
  public:
    MsImageResampler();
    bool toBgr(
      ttLibC_Bgr   *dest_frame,
      ttLibC_Video *src_frame);
    bool toYuv420(
      ttLibC_Yuv420 *dest_frame,
      ttLibC_Video  *src_frame);
    ~MsImageResampler();
    bool isInitialized;
  private:
    void *_instance;
  };
}

#endif
