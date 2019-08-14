#include <gtest/gtest.h>

#pragma warning(disable: 4996)

class TTTest : public testing::Test {
protected:
  void SetUp();
  void TearDown();
};
