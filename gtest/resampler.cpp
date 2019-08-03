#include "test.hpp"

class ResamplerTest : public TTTest {

};

// test for resamplers.
#ifdef __ENABLE_LIBYUV__
#define LIBYUV(A) libyuv##A
#else
#define LIBYUV(A) DISABLED_libyuv##A
#endif
TEST(ResamplerTest, LIBYUV(Rotate)) {
  EXPECT_EQ(1,1);
}


