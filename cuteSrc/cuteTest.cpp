/**
 * @file   cuteTest.cpp
 * @brief  cuteTest entry program.
 *
 * you need to install standalone cute_lib.
 *   1. download 2.0.0 (http://cute-test.com/projects/cute/wiki/CUTE_standalone)
 *   2. copy cute_lib in /usr/local/include/
 *      expected /usr/local/include/cute_lib/cute.h
 *                                          /cute_base.h
 *                                          ...
 *
 * require boost library.
 *   for osx
 *   1. brew install boost
 *
 * this code is under GPLv3 license.
 *
 * @author taktod
 * @date   2015/07/12
 */

#include <stdio.h>

#include <cute.h>
#include <cute_runner.h>
#include <ide_listener.h>
#include <xml_listener.h>
#include <ttLibC/allocator.h>

// entry for xxxTest.cpp
cute::suite containerTests(cute::suite s);
cute::suite audioTests(cute::suite s);
cute::suite videoTests(cute::suite s);
cute::suite utilTests(cute::suite s);
cute::suite encoderDecoderTests(cute::suite s);
cute::suite avcodecTests(cute::suite s);

/**
 * main entry for cuteTest
 * @param argc
 * @param argv
 * @return exit code.
 */
int main(int argc, const char *argv[]) {
	ttLibC_Allocator_init();
	cute::suite s;
	cute::xml_file_opener xmlfile(argc, argv);
	cute::xml_listener<cute::ide_listener<> > lis(xmlfile.out);
	cute::makeRunner(lis, argc, argv)(containerTests(s),      "containerTests");
	cute::makeRunner(lis, argc, argv)(audioTests(s),          "audioTests");
	cute::makeRunner(lis, argc, argv)(videoTests(s),          "videoTests");
	cute::makeRunner(lis, argc, argv)(utilTests(s),           "utilTests");
	cute::makeRunner(lis, argc, argv)(encoderDecoderTests(s), "encoderDecoderTests");
	cute::makeRunner(lis, argc, argv)(avcodecTests(s),        "avcodecTests");
	ttLibC_Allocator_close();
	return 0;
}

