#include "../util.hpp"
#include <ttLibC/util/msGlobalUtil.h>
#include <ttLibC/frame/video/bgr.h>

using namespace std;

#ifdef __ENABLE_WIN32__
#define MSGLOBAL(A, B) TEST_F(UtilTest, MsGlobal##A){B();}
#else
#define MSGLOBAL(A, B) TEST_F(UtilTest, DISABLED_MsGlobal##A){}
#endif

MSGLOBAL(BmpSave, [this](){
auto bgr = ttLibC_Bgr_makeEmptyFrame(BgrType_bgr, 640, 360);
  if(bgr == nullptr) {
    FAIL();
    return;
  }
  uint8_t *ps = bgr->data;
  for(int i = 0;i < bgr->inherit_super.height;++ i) {
    uint8_t *p = ps;
    for(int j = 0;j < bgr->inherit_super.width;++ j) {
      *p = (uint8_t)(j & 0xFF);
      *(p + 1) = (uint8_t)(j & 0xFF);
      *(p + 2) = (uint8_t)(j & 0xFF);
      p += bgr->unit_size;
    }
    ps += bgr->width_stride;
  }
  ttLibC_MsGlobal_WriteBitmap("test.bmp", bgr);
  ttLibC_Bgr_close(&bgr);
});
