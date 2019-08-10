cmake_minimum_required(VERSION 2.8)

project(ttLibC-test)

add_subdirectory(lib/googletest)

if(WIN32)
  set(EXECUTABLE_OUTPUT_PATH "..")
  set(CMAKE_CXX_FLAGS_RELEASE "/MT")
  set(CMAKE_CXX_FLAGS_DEBUG "/MTd")
endif(WIN32)

include_directories("..")
add_executable(ttLibC-test
  test.cpp
  frame.cpp
  resampler.cpp
  resampler/image.cpp
  resampler/imageResizer.cpp
  resampler/libyuv.cpp
  resampler/swscale.cpp
  util.cpp)
target_link_libraries(ttLibC-test
  gtest_main
  ttLibC)

enable_testing()
add_test(NAME frame COMMAND ttLibC-test --gtest_filter=FrameTest.*)
add_test(NAME resampler COMMAND ttLibC-test --gtest_filter=ResamplerTest.*)
add_test(NAME util COMMAND ttLibC-test --gtest_filter=UtilTest.*)
