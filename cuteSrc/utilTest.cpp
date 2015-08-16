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
#include <ttLibC/util/ioUtil.h>

#ifdef __ENABLE_OPENCV__
#	include <ttLibC/util/opencvUtil.h>
#endif

#ifdef __ENABLE_OPENAL__
#	include <ttLibC/util/openalUtil.h>
#endif

#ifdef __ENABLE_FILE__
#	include <ttLibC/util/httpUtil.h>
#endif

#include <ttLibC/util/crc32Util.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/param.h>
#include <sys/uio.h>
#include <unistd.h>

static void crc32Test() {
	LOG_PRINT("crc32Test");
	ttLibC_Crc32 *crc32 = ttLibC_Crc32_make(0xFFFFFFFFL);
	uint8_t buf[32];
	// 2AB104B2
	uint32_t size = ttLibC_HexUtil_makeBuffer("00B00D0001C100000001F000", buf, 32);
	uint8_t *data = buf;
	for(int i = 0;i < size;++ i, ++data) {
		ttLibC_Crc32_update(crc32, *data);
	}
	LOG_PRINT("result:%x", ttLibC_Crc32_getValue(crc32));
	ASSERTM("FAILED", ttLibC_Crc32_getValue(crc32) == 0x2AB104B2);
	ttLibC_Crc32_close(&crc32);
}

static void ioTest() {
	LOG_PRINT("ioTest");
	uint64_t num = 0x12345678;
	uint64_t result = num;
	ttLibC_HexUtil_dump(&num, 8, true);
	result = le_uint16_t(num);
	ttLibC_HexUtil_dump(&result, 8, true);
	result = be_uint16_t(num);
	ttLibC_HexUtil_dump(&result, 8, true);
	result = le_uint24_t(num);
	ttLibC_HexUtil_dump(&result, 8, true);
	result = be_uint24_t(num);
	ttLibC_HexUtil_dump(&result, 8, true);
	result = le_uint32_t(num);
	ttLibC_HexUtil_dump(&result, 8, true);
	result = be_uint32_t(num);
	ttLibC_HexUtil_dump(&result, 8, true);
	result = le_uint64_t(num);
	ttLibC_HexUtil_dump(&result, 8, true);
	result = be_uint64_t(num);
	ttLibC_HexUtil_dump(&result, 8, true);

	num = 0x81828384;
	int64_t iresult = le_int16_t(num);
	LOG_PRINT("val:%lld", iresult);
	iresult = be_int16_t(num);
	LOG_PRINT("val:%lld", iresult);
	iresult = le_int32_t(num);
	LOG_PRINT("val:%lld", iresult);
	iresult = be_int32_t(num);
	LOG_PRINT("val:%lld", iresult);
	iresult = le_int64_t(num);
	LOG_PRINT("val:%lld", iresult);
	iresult = be_int64_t(num);
	LOG_PRINT("val:%lld", iresult);
}

#ifdef __ENABLE_FILE__
bool httpClientCallback(void *ptr, ttLibC_HttpClient *client, void *data, size_t data_size) {
	LOG_PRINT("callback is called.");
	ttLibC_HexUtil_dump(data, data_size, true);
	return true;
}
#endif

static void httpClientTest() {
	LOG_PRINT("httpClientTest");
#ifdef __ENABLE_FILE__
	ttLibC_HttpClient *client = ttLibC_HttpClient_make(256, 10);
	ttLibC_HttpClient_getRange(client, "http://www.google.com/", 0, 500, false, httpClientCallback, NULL);
	ttLibC_HttpClient_close(&client);
#endif
}

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
	ttLibC_CvCapture *capture = ttLibC_CvCapture_make(0, 320, 240);
	ttLibC_CvWindow *window = ttLibC_CvWindow_make("opencvTest");
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
	for(int i = 0;i < 50;i ++) {
//		ttLibC_PcmS16 *p = ttLibC_BeepGenerator_makeBeepByMiliSec(generator, pcm, 1000);
		ttLibC_PcmS16 *p = ttLibC_BeepGenerator_makeBeepBySampleNum(generator, pcm, 1024);
		if(p == NULL) {
			break;
		}
		pcm = p;
		LOG_PRINT("pts:%lld timebase:%d", pcm->inherit_super.inherit_super.pts, pcm->inherit_super.inherit_super.timebase);
		ttLibC_AlDevice_queue(device, pcm);
//		ttLibC_AlDevice_proceed(device, 10);
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
	s.push_back(CUTE(crc32Test));
	s.push_back(CUTE(ioTest));
	s.push_back(CUTE(httpClientTest));
	s.push_back(CUTE(hexUtilTest));
	s.push_back(CUTE(opencvUtilTest));
	s.push_back(CUTE(openalUtilTest));
	return s;
}
