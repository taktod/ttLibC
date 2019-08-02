cmake_minimum_required(VERSION 2.8)

project(ttLibC-test)

add_subdirectory(lib/googletest)

add_executable(ttLibC-test test.cpp)
target_link_libraries(ttLibC-test gtest_main)
