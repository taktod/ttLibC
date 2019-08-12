#include "../util.hpp"
#include <ttLibC/util/hexUtil.h>

using namespace std;

#define HEX(A, B) TEST_F(UtilTest, Hex##A){B();}

HEX(String, [this]() {
  uint8_t buffer[256];
  uint32_t length = ttLibC_HexUtil_makeBuffer("000102"
    "030405", buffer, 256);
  uint8_t buf[6];
  buf[0] = 0;
  buf[1] = 1;
  buf[2] = 2;
  buf[3] = 3;
  buf[4] = 4;
  buf[5] = 5;
  EXPECT_EQ(std::memcmp((const char*)buffer, (const char*)buf, length), 0);
});

#undef HEX
