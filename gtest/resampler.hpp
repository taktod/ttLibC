#include "test.hpp"

#include <ttLibC/util/hexUtil.h>
#include <ttLibC/allocator.h>
#include <ttLibC/frame/video/yuv420.h>
#include <ttLibC/frame/video/bgr.h>

using namespace std;
#include <iostream>

class ResamplerTest : public TTTest {
public:
  ttLibC_Yuv420 *makeYuv(string hex, ttLibC_Yuv420_Type type,
      uint32_t width, uint32_t height);
  ttLibC_Bgr *makeBgr(string hex, ttLibC_Bgr_Type type,
      uint32_t width, uint32_t height);
  void binaryEq(string hex, ttLibC_Video *video);
  void binaryEq(string hex, ttLibC_Yuv420 *yuv);
  void binaryEq(string hex, ttLibC_Bgr *bgr);
  ttLibC_Yuv420 *makeAlignedYuv(string hex, ttLibC_Yuv420_Type type,
    uint32_t width, uint32_t height);
  ttLibC_Bgr *makeAlignedBgr(string hex, ttLibC_Bgr_Type type,
    uint32_t width, uint32_t height);
};
