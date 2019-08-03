#include "gtest/gtest.h"
#include <ttLibC/ttLibC.h>

TEST(hoge, base) {
  puts(ttLibC_getVersion());
  EXPECT_EQ(1,0);
}
