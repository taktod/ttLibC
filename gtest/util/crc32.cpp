#include "../util.hpp"
#include <ttLibC/util/crc32Util.h>

using namespace std;

#define CRC32(A, B) TEST_F(UtilTest, Crc32##A){B();}

CRC32(Calcurate, [this](){
  uint8_t buf[10];
  buf[0] = 1;
  buf[1] = 2;
  buf[2] = 3;
  buf[3] = 4;
  buf[4] = 5;
  buf[5] = 6;
  buf[6] = 7;
  buf[7] = 8;
  buf[8] = 9;
  buf[9] = 10;
  ttLibC_Crc32 *crc32 = ttLibC_Crc32_make(0);
  for(int i = 0;i < 10;++ i) {
    ttLibC_Crc32_update(crc32, buf[i]);
  }
  uint32_t value = ttLibC_Crc32_getValue(crc32);
  ttLibC_Crc32_close(&crc32);
  EXPECT_EQ(value, 366250939);
});

#undef CRC32