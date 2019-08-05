#include "test.hpp"
#include <ttLibC/frame/frame.h>
#include <ttLibC/frame/video/bgr.h>

class FrameTest : public TTTest{
};

TEST_F(FrameTest, bgr) {
  ttLibC_Bgr *bgr = ttLibC_Bgr_make(NULL, BgrType_abgr, 320, 240, 1280, NULL, 0, true, 0, 1000);
  EXPECT_TRUE(bgr != NULL);
  ttLibC_Bgr_close(&bgr);
}

TEST(frame, bgrMakeEmpty) {
  ttLibC_Bgr *bgr = ttLibC_Bgr_makeEmptyFrame(BgrType_bgr, 320, 240);
  EXPECT_TRUE(bgr != NULL);
  ttLibC_Bgr_close(&bgr);
}
