/**
 * @file   utilTest.cpp
 * @brief  util test code.
 *
 * this code is under GPLv3 license.
 *
 * @author taktod
 * @date   2015/07/18
 */

#include <cute.h>

#include <ttLibC/ttLibC.h>
#include <ttLibC/log.h>
#include <array>

#include <ttLibC/util/hexUtil.h>

#include <ttLibC/frame/video/bgr.h>
#include <ttLibC/frame/audio/pcms16.h>
#include <ttLibC/util/beepUtil.h>

#ifdef __ENABLE_OPENCV__
#	include <ttLibC/util/opencvUtil.h>
#endif

#ifdef __ENABLE_OPENAL__
#	include <ttLibC/util/openalUtil.h>
#endif

/**
 * how to use hexUtil
 */
static void hexUtilTest() {
	LOG_PRINT("hexUtilTest");
	std::array<char, 5> buf = {0, 1, 10, 20, 30};
	ttLibC_HexUtil_dump(buf.data(), buf.size(), true);

	char data[] = {0, 1, 10, 20, 30};
	LOG_DUMP(data, 5, true);

	char buffer[256] = {0};
	uint32_t length = ttLibC_HexUtil_makeBuffer("010203abcd", buffer, sizeof(buffer));
	ttLibC_HexUtil_dump(buffer, length, true);
}

/**
 * how to use opencv window and camera capture.
 */
static void opencvUtilTest() {
	LOG_PRINT("opencvUtilTest");
#ifdef __ENABLE_OPENCV__
	ttLibC_CvCapture *capture = ttLibC_CvCapture_makeCapture(0, 320, 240);
	ttLibC_CvWindow *window = ttLibC_CvWindow_makeWindow("opencvTest");
	ttLibC_Bgr *bgr = NULL;
	while(true) {
		ttLibC_Bgr *b = ttLibC_CvCapture_queryFrame(capture, bgr);
		if(b == NULL) {
			break;
		}
		bgr = b;
		ttLibC_CvWindow_showBgr(window, bgr);
		uint8_t key = ttLibC_CvWindow_waitForKeyInput(10);
		if(key == Keychar_Esc) {
			break;
		}
	}
	ttLibC_Bgr_close(&bgr);
	ttLibC_CvWindow_close(&window);
	ttLibC_CvCapture_close(&capture);
#endif
}

/**
 * how to use openalUtil and beepGenerator.
 */
static void openalUtilTest() {
	LOG_PRINT("openalUtilTest");
#ifdef __ENABLE_OPENAL__
	ttLibC_BeepGenerator *generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian, 440, 44100, 2);
	ttLibC_PcmS16 *pcm = NULL;
	ttLibC_AlDevice *device = ttLibC_AlDevice_make(100);
	for(int i = 0;i < 10;i ++) {
		ttLibC_PcmS16 *p = ttLibC_BeepGenerator_makeBeepByMiliSec(generator, pcm, 1000);
		if(p == NULL) {
			break;
		}
		pcm = p;
		LOG_PRINT("pts:%lld timebase:%d", pcm->inherit_super.inherit_super.pts, pcm->inherit_super.inherit_super.timebase);
		ttLibC_AlDevice_queue(device, pcm);
		ttLibC_AlDevice_proceed(device, 100);
	}
	ttLibC_AlDevice_proceed(device, -1);
	ttLibC_AlDevice_proceed(device, -1);
	ttLibC_PcmS16_close(&pcm);
	ttLibC_AlDevice_close(&device);
	ttLibC_BeepGenerator_close(&generator);
#endif
}

/**
 * define all test for util package.
 * @param s cute::suite obj
 * @return cute::suite obj
 */
cute::suite utilTests(cute::suite s) {
	s.clear();
	s.push_back(CUTE(hexUtilTest));
	s.push_back(CUTE(opencvUtilTest));
	s.push_back(CUTE(openalUtilTest));
	return s;
}
