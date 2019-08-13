#include "test.hpp"
#include <ttLibC/allocator.h>

void TTTest::SetUp() {
  ttLibC_Allocator_init();
}
void TTTest::TearDown() {
  size_t value = ttLibC_Allocator_dump();
  EXPECT_EQ(value, 0);
  ttLibC_Allocator_close();
}

#pragma warning(disable: 4996)

TEST(System, fopen) {
  FILE *fp = fopen("test.txt", "w");
  if(fp) {
    fwrite("hello", 1, 5, fp);
    fclose(fp);
    return;
  }
  FAIL();
}
