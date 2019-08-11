#include "test.hpp"

#include <iostream>
#include <ttLibC/frame/video/yuv420.h>

using namespace std;

class EncoderTest : public TTTest {
public:
  ttLibC_Yuv420 *makeGradateYuv(ttLibC_Yuv420_Type type, uint32_t width, uint32_t height);
};
