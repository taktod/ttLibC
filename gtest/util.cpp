#include "test.hpp"
#include <ttLibC/util/crc32Util.h>
#include <ttLibC/util/hexUtil.h>
#include <cstring>

class UtilTest : public TTTest{

};

TEST_F(UtilTest, crc32) {
  uint8_t buf[] = {1,2,3,4,5,6,7,8,9,10};
  ttLibC_Crc32 *crc32 = ttLibC_Crc32_make(0);
  for(int i = 0;i < 10;++ i) {
    ttLibC_Crc32_update(crc32, buf[i]);
  }
  uint32_t value = ttLibC_Crc32_getValue(crc32);
  ttLibC_Crc32_close(&crc32);
  EXPECT_EQ(value, 366250939);
}

TEST_F(UtilTest, hexString) {
  uint8_t buffer[256];
  uint32_t length = ttLibC_HexUtil_makeBuffer("000102"
    "030405", buffer, 256);
  uint8_t buf[] = {0, 1, 2, 3, 4, 5};
  EXPECT_EQ(std::memcmp((const char *)buffer, (const char *)buf, length), 0);
}
