#include "../util.hpp"
#include <ttLibC/util/msGlobalUtil.h>
#include <ttLibC/frame/video/bgr.h>
#include <ttLibC/util/hexUtil.h>

using namespace std;

#ifdef __ENABLE_WIN32__
#define MSGLOBAL(A, B) TEST_F(UtilTest, MsGlobal##A){B();}
#else
#define MSGLOBAL(A, B) TEST_F(UtilTest, DISABLED_MsGlobal##A){}
#endif

MSGLOBAL(Bmp24Save, [this](){
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
  ttLibC_MsGlobal_WriteBitmap("test24.bmp", bgr);
  ttLibC_Bgr_close(&bgr);
});

MSGLOBAL(Bmp32Save, [this](){
auto bgr = ttLibC_Bgr_makeEmptyFrame(BgrType_bgra, 640, 360);
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
      *(p + 2) = (uint8_t)(0xFF);
      *(p + 3) = (uint8_t)(0xFF);
      p += bgr->unit_size;
    }
    ps += bgr->width_stride;
  }
  ttLibC_MsGlobal_WriteBitmap("test32.bmp", bgr);
  ttLibC_Bgr_close(&bgr);
});

MSGLOBAL(UnicodeToSjis, [this](){
  // convert japanese katakana aiueo
  uint8_t src[256];
  uint32_t src_length = ttLibC_HexUtil_makeBuffer("A2 30 A4 30 A6 30 A8 30 AA 30 00 00 ", src, 256);
  uint8_t dest[256];
  uint8_t target[256];
  uint32_t target_length = ttLibC_HexUtil_makeBuffer("83 41 83 43 83 45 83 47 83 49 00 ", target, 256);
  ttLibC_MsGlobal_unicodeToSjis((const wchar_t *)src, (char *)dest, 256);
  EXPECT_EQ(std::memcmp((const char *)dest, (const char *)target, target_length), 0);
});

MSGLOBAL(UnicodeToUtf8, [this](){
  // convert japanese katakana aiueo
  uint8_t src[256];
  uint32_t src_length = ttLibC_HexUtil_makeBuffer("A2 30 A4 30 A6 30 A8 30 AA 30 00 00 ", src, 256);
  uint8_t dest[256];
  uint8_t target[256];
  uint32_t target_length = ttLibC_HexUtil_makeBuffer("E3 82 A2 E3 82 A4 E3 82 A6 E3 82 A8 E3 82 AA 00 ", target, 256);
  ttLibC_MsGlobal_unicodeToUtf8((const wchar_t *)src, (char *)dest, 256);
  EXPECT_EQ(std::memcmp((const char *)dest, (const char *)target, target_length), 0);
});

MSGLOBAL(SjisToUnicode, [this](){
  // convert japanese katakana aiueo
  uint8_t src[256];
  uint32_t src_length = ttLibC_HexUtil_makeBuffer("83 41 83 43 83 45 83 47 83 49 00 ", src, 256);
  uint8_t dest[256];
  uint8_t target[256];
  uint32_t target_length = ttLibC_HexUtil_makeBuffer("A2 30 A4 30 A6 30 A8 30 AA 30 00 00 ", target, 256);
  ttLibC_MsGlobal_sjisToUnicode((const char *)src, (wchar_t *)dest, 256);
  EXPECT_EQ(std::memcmp((const char *)dest, (const char *)target, target_length), 0);
});

MSGLOBAL(Utf8ToUnicode, [this](){
  // convert japanese katakana aiueo
  uint8_t src[256];
  uint32_t src_length = ttLibC_HexUtil_makeBuffer("E3 82 A2 E3 82 A4 E3 82 A6 E3 82 A8 E3 82 AA 00 ", src, 256);
  uint8_t dest[256];
  uint8_t target[256];
  uint32_t target_length = ttLibC_HexUtil_makeBuffer("A2 30 A4 30 A6 30 A8 30 AA 30 00 00 ", target, 256);
  ttLibC_MsGlobal_utf8ToUnicode((const char *)src, (wchar_t *)dest, 256);
  EXPECT_EQ(std::memcmp((const char *)dest, (const char *)target, target_length), 0);
});

