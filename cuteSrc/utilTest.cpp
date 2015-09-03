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

#include <ttLibC/util/amfUtil.h>

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

bool amfTest_write(void *ptr, void *data, size_t data_size) {
	LOG_DUMP(data, data_size, true);
	return true;
}

bool amfTest_read(void *ptr, ttLibC_Amf0Object *amf0_obj) {
	switch(amf0_obj->type) {
	case amf0Type_Number:
		LOG_PRINT("number %f", *((double *)amf0_obj->object));
		return true;
	case amf0Type_Boolean:
		if(*((uint8_t *)amf0_obj->object) == 1) {
			LOG_PRINT("bool true");
		}
		else {
			LOG_PRINT("bool false");
		}
		return true;
	case amf0Type_String:
		LOG_PRINT("string %s", (char *)amf0_obj->object);
		return true;
	case amf0Type_Object:
		LOG_PRINT("object:");
		{
			// try to dump data.
			ttLibC_Amf0MapObject *map_objects = (ttLibC_Amf0MapObject *)amf0_obj->object;
			for(int i = 0;map_objects[i].key != NULL && map_objects[i].amf0_obj != NULL;++ i) {
				LOG_PRINT("key:%s", map_objects[i].key);
				amfTest_read(ptr, map_objects[i].amf0_obj);
			}
		}
		return true;
	case amf0Type_MovieClip:
	case amf0Type_Null:
	case amf0Type_Undefined:
	case amf0Type_Reference:
		break;
	case amf0Type_Map:
		{
			// try to dump data.
			ttLibC_Amf0MapObject *map_objects = (ttLibC_Amf0MapObject *)amf0_obj->object;
			for(int i = 0;map_objects[i].key != NULL && map_objects[i].amf0_obj != NULL;++ i) {
				LOG_PRINT("key:%s", map_objects[i].key);
				amfTest_read(ptr, map_objects[i].amf0_obj);
			}
		}
		return true;
	case amf0Type_ObjectEnd:
	case amf0Type_Array:
	case amf0Type_Date:
	case amf0Type_LongString:
	case amf0Type_Unsupported:
	case amf0Type_RecordSet:
	case amf0Type_XmlDocument:
	case amf0Type_TypedObject:
	case amf0Type_Amf3Object:
	default:
		break;
	}
	return false;
}

static void amfTest() {
	LOG_PRINT("amfTest");
	uint8_t buf[1024];
	LOG_PRINT("read amf0 test a(string)");
	uint32_t size = ttLibC_HexUtil_makeBuffer("02000A6F6E4D65746144617461", buf, 1024);
	ttLibC_Amf0_read(buf, size, amfTest_read, NULL);

	LOG_PRINT("read amf0 test b(map)");
	size = ttLibC_HexUtil_makeBuffer("080000000D00086475726174696F6E0040607547AE147AE1000577696474680040840000000000000006686569676874004076800000000000000D766964656F646174617261746500000000000000000000096672616D657261746500403DF853E2556B28000C766964656F636F6465636964004000000000000000000D617564696F6461746172617465000000000000000000000F617564696F73616D706C65726174650040E5888000000000000F617564696F73616D706C6573697A65004030000000000000000673746572656F0101000C617564696F636F64656369640040240000000000000007656E636F64657202000D4C61766635362E33362E313030000866696C6573697A65004162D2F860000000000009", buf, 1024);
	ttLibC_Amf0_read(buf, size, amfTest_read, NULL);

	LOG_PRINT("read amf0 test c(object)");
	size = ttLibC_HexUtil_makeBuffer("0300036170700200046C6976650008666C61736856657202000E57494E2031352C302C302C3232330005746355726C02001D72746D703A2F2F34392E3231322E33392E31373A313933352F6C6976650004667061640100000B617564696F436F646563730040AFCE0000000000000B766964656F436F6465637300406F800000000000000E6F626A656374456E636F64696E67000000000000000000000C6361706162696C697469657300402E000000000000000D766964656F46756E6374696F6E003FF0000000000000000009", buf, 1024);
	ttLibC_Amf0_read(buf, size, amfTest_read, NULL);

	LOG_PRINT("generate map");
	ttLibC_Amf0Object *map = ttLibC_Amf0_map((ttLibC_Amf0MapObject [])
		{
			{"test1", ttLibC_Amf0_number(16)},
			{"test2", ttLibC_Amf0_string("hogehoge")},
			{"test3", ttLibC_Amf0_boolean(true)},
			{NULL, NULL},
		});
	ttLibC_Amf0Object *amf0_obj = ttLibC_Amf0_getElement(map, "test1");
	if(amf0_obj != NULL) {
		amfTest_read(NULL, amf0_obj);
	}
	amfTest_read(NULL, map);
	ttLibC_Amf0_write(map, amfTest_write, NULL);
	ttLibC_Amf0_close(&map);

	LOG_PRINT("generate number");
	ttLibC_Amf0Object *number = ttLibC_Amf0_number(15);
	amfTest_read(NULL, number);
	ttLibC_Amf0_write(number, amfTest_write, NULL);
	ttLibC_Amf0_close(&number);
}

static void crc32Test() {
	LOG_PRINT("crc32Test");
	ttLibC_Crc32 *crc32 = ttLibC_Crc32_make(0xFFFFFFFFL);
	uint8_t buf[256];
	// 2AB104B2
	uint32_t size = ttLibC_HexUtil_makeBuffer("00B00D0001C100000001F000", buf, sizeof(buf));
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
	s.push_back(CUTE(amfTest));
	s.push_back(CUTE(crc32Test));
	s.push_back(CUTE(ioTest));
	s.push_back(CUTE(httpClientTest));
	s.push_back(CUTE(hexUtilTest));
	s.push_back(CUTE(opencvUtilTest));
	s.push_back(CUTE(openalUtilTest));
	return s;
}
