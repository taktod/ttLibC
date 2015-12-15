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
#include <ttLibC/allocator.h>
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

#include <ttLibC/util/dynamicBufferUtil.h>
#include <ttLibC/util/byteUtil.h>
#include <ttLibC/util/linkedListUtil.h>
#include <ttLibC/util/stlListUtil.h>

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

static bool stlListTest_findItem(void *ptr, void *item) {
	LOG_PRINT("fi:%d", *((int *)item));
	return true;
}

static bool stlListTest_findItem2(void *ptr, void *item) {
	ttLibC_StlList *list = (ttLibC_StlList *)ptr;
	LOG_PRINT("fi2:%d", *((int *)item));
	if(*((int *)item) == 5) {
		LOG_PRINT("削除対象アイテムみつけた。:%d", *((int *)item));
		ttLibC_StlList_remove(list, item);
	}
	return true;
}

static void stlListTest() {
	LOG_PRINT("stlListTest");
	ttLibC_StlList *list = ttLibC_StlList_make();
	int val1 = 5;
	int val2 = 6;
	int val3 = 3;
	ttLibC_StlList_addFirst(list, &val1);
	ttLibC_StlList_addLast(list, &val2);
	ttLibC_StlList_addFirst(list, &val3);
	ttLibC_StlList_forEach(list, stlListTest_findItem, NULL);
	ttLibC_StlList_forEachReverse(list, stlListTest_findItem, NULL);
	ttLibC_StlList_forEach(list, stlListTest_findItem2, list);
	ttLibC_StlList_forEach(list, stlListTest_findItem, NULL);
	ttLibC_StlList_close(&list);
	ASSERT(ttLibC_Allocator_dump() == 0);
}

static bool findItem(void *ptr, void *item, size_t item_size) {
	// intの配列であると仮定させてやらないとだめなわけか・・・面倒だね。
	LOG_PRINT("value:%d", *((int *)item));
	if(*(int *)item == 5) {
		LOG_PRINT("削除すべきデータを見つけた。");
		ttLibC_LinkedList *linkedList = (ttLibC_LinkedList *)ptr;
		ttLibC_LinkedList_remove(linkedList, item);
	}
	return true;
}

static bool findItem2(void *ptr, void *item, size_t item_size) {
	// intの配列であると仮定させてやらないとだめなわけか・・・面倒だね。
	LOG_PRINT("value:%d", *((int *)item));
	return true;
}

static void linkedListTest() {
	LOG_PRINT("linkedListTest");
	ttLibC_LinkedList *linkedList = ttLibC_LinkedList_make();
	int val = 5;
	ttLibC_LinkedList_addLast(linkedList, &val, sizeof(val), false);
	val = 6;
	ttLibC_LinkedList_addLast(linkedList, &val, sizeof(val), false);
	val = 3;
	ttLibC_LinkedList_addFirst(linkedList, &val ,sizeof(val), false);
	ttLibC_LinkedList_forEach(linkedList, findItem, linkedList);
	ttLibC_LinkedList_forEach(linkedList, findItem2, NULL);
	ttLibC_LinkedList_close(&linkedList);
	ASSERT(ttLibC_Allocator_dump() == 0);
}

static void byteUtilTest() {
	LOG_PRINT("byteUtilTest");
	uint8_t buffer[256];
	uint32_t size = ttLibC_HexUtil_makeBuffer("01024304403132333430", buffer, 256);
	ttLibC_ByteReader *reader = ttLibC_ByteReader_make(buffer, size, ByteUtilType_default);
	LOG_PRINT("4bit val:%llx", ttLibC_ByteReader_bit(reader, 4));
	LOG_PRINT("4bit val:%llx", ttLibC_ByteReader_bit(reader, 4));
	LOG_PRINT("4bit val:%llx", ttLibC_ByteReader_bit(reader, 4));
	LOG_PRINT("4bit val:%llx", ttLibC_ByteReader_bit(reader, 4));
	LOG_PRINT("ebml val:%llx", ttLibC_ByteReader_ebml(reader, false));
	char buf[256];
	ttLibC_ByteReader_string(reader, buf, 256, 5);
	LOG_PRINT("string:%s", buf);
	LOG_PRINT("4bit val:%llx", ttLibC_ByteReader_bit(reader, 8));
	ttLibC_ByteReader_close(&reader);
	ttLibC_ByteConnector *connector = ttLibC_ByteConnector_make(buffer, 256, ByteUtilType_default);
	ttLibC_ByteConnector_ebml(connector, 0x02);
	ttLibC_ByteConnector_ebml(connector, 0xFF);
	ttLibC_ByteConnector_ebml(connector, 0xFFFF);
	ttLibC_ByteConnector_ebml(connector, 0xFFFFFF);
	ttLibC_ByteConnector_ebml(connector, 0xFFFFFFFFL);
	ttLibC_ByteConnector_ebml(connector, 0xFFFFFFFFFFL);
	ttLibC_ByteConnector_ebml(connector, 0xFFFFFFFFFFFFL);
	ttLibC_ByteConnector_ebml(connector, 0xFFFFFFFFFFFFFFL);
	LOG_DUMP(buffer, connector->write_size, true);
	ttLibC_ByteConnector_close(&connector);
	connector = ttLibC_ByteConnector_make(buffer, 256, ByteUtilType_default);
	ttLibC_ByteConnector_bit(connector, 0xFEDCB, 20);
	ttLibC_ByteConnector_bit(connector, 3, 4);
	ttLibC_ByteConnector_bit(connector, 5, 3);
	ttLibC_ByteConnector_bit(connector, 6, 5);
	ttLibC_ByteConnector_string(connector, "test1234", 8);
	LOG_DUMP(buffer, connector->write_size, true);
	ttLibC_ByteConnector_close(&connector);
	ASSERT(ttLibC_Allocator_dump() == 0);
}

static void connectorTest() {
	LOG_PRINT("connectorTest");
	uint8_t buffer[256];
	size_t buffer_size = 256;
	ttLibC_ByteConnector *connector = ttLibC_ByteConnector_make(buffer, buffer_size, ByteUtilType_default);
	ttLibC_ByteConnector_ebml(connector, 0x02);
	ttLibC_ByteConnector_ebml(connector, 0xFF);
	ttLibC_ByteConnector_ebml(connector, 0xFFFF);
	ttLibC_ByteConnector_ebml(connector, 0xFFFFFF);
	ttLibC_ByteConnector_ebml(connector, 0xFFFFFFFFL);
	ttLibC_ByteConnector_ebml(connector, 0xFFFFFFFFFFL);
	ttLibC_ByteConnector_ebml(connector, 0xFFFFFFFFFFFFL);
	ttLibC_ByteConnector_ebml(connector, 0xFFFFFFFFFFFFFFL);
	LOG_DUMP(buffer, connector->write_size, true);
	ttLibC_ByteConnector_close(&connector);
	connector = ttLibC_ByteConnector_make(buffer, buffer_size, ByteUtilType_default);
	ttLibC_ByteConnector_bit(connector, 0xFEDCB, 20);
	ttLibC_ByteConnector_bit(connector, 3, 4);
	ttLibC_ByteConnector_bit(connector, 5, 3);
	ttLibC_ByteConnector_bit(connector, 6, 5);
	LOG_DUMP(buffer, connector->write_size, true);
	ttLibC_ByteConnector_close(&connector);
	ASSERT(ttLibC_Allocator_dump() == 0);
}

static void dynamicBufferTest() {
	LOG_PRINT("dynamicBufferTest");
	ttLibC_DynamicBuffer *buffer = ttLibC_DynamicBuffer_make();
	uint8_t data[256];
	size_t size = ttLibC_HexUtil_makeBuffer("010203040506", data, 256);
	LOG_PRINT("size:%llu", size);
	// データを追記する。
	ttLibC_DynamicBuffer_append(buffer, data, size);
	// 中身確認
	LOG_DUMP(ttLibC_DynamicBuffer_refData(buffer),
			ttLibC_DynamicBuffer_refSize(buffer), true);
	LOG_PRINT("bs:%llu, ts:%llu", buffer->buffer_size, buffer->target_size);
	// 5byte処理済みにする。
	ttLibC_DynamicBuffer_markAsRead(buffer, 5);
	// データを追記する。
	size = ttLibC_HexUtil_makeBuffer("AABBCCDD", data, 256);
	ttLibC_DynamicBuffer_append(buffer, data, size);
	LOG_PRINT("bs:%llu, ts:%llu", buffer->buffer_size, buffer->target_size);
	// 中身確認
	LOG_DUMP(ttLibC_DynamicBuffer_refData(buffer),
			ttLibC_DynamicBuffer_refSize(buffer), true);
	// 読み込み済みデータをクリアする。
	ttLibC_DynamicBuffer_reset(buffer);
	// さらに中身確認
	LOG_DUMP(ttLibC_DynamicBuffer_refData(buffer),
			ttLibC_DynamicBuffer_refSize(buffer), true);
	// 4byte処理済みにする。
	ttLibC_DynamicBuffer_markAsRead(buffer, 4);
	// さらに中身確認
	LOG_DUMP(ttLibC_DynamicBuffer_refData(buffer),
			ttLibC_DynamicBuffer_refSize(buffer), true);
	// 読み込み済みデータをクリアする。
	ttLibC_DynamicBuffer_clear(buffer);
	// さらに中身確認
	LOG_DUMP(ttLibC_DynamicBuffer_refData(buffer),
			ttLibC_DynamicBuffer_refSize(buffer), true);
	size = ttLibC_HexUtil_makeBuffer("EEFF", data, 256);
	ttLibC_DynamicBuffer_append(buffer, data, size);
	LOG_PRINT("bs:%llu, ts:%llu", buffer->buffer_size, buffer->target_size);
	LOG_DUMP(ttLibC_DynamicBuffer_refData(buffer),
			ttLibC_DynamicBuffer_refSize(buffer), true);
	ttLibC_DynamicBuffer_close(&buffer);
	ASSERT(ttLibC_Allocator_dump() == 0);
}

bool amfTest_write(void *ptr, void *data, size_t data_size) {
	LOG_DUMP(data, data_size, true);
	return true;
}

bool amfTest_read(void *ptr, ttLibC_Amf0Object *amf0_obj) {
	switch(amf0_obj->type) {
	case amf0Type_Number:
		LOG_PRINT("number %f", *((double *)(amf0_obj->object)));
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
	ASSERT(ttLibC_Allocator_dump() == 0);
}

static void crc32Test() {
	LOG_PRINT("crc32Test");
	ttLibC_Crc32 *crc32 = ttLibC_Crc32_make(0xFFFFFFFFL);
	uint8_t buf[256] = "123456789";
	// 2AB104B2
//	uint32_t size = ttLibC_HexUtil_makeBuffer("00B00D0001C100000001F000", buf, sizeof(buf));
	uint32_t size = 9;
	uint8_t *data = buf;
	for(int i = 0;i < size;++ i, ++data) {
		LOG_PRINT("%c", *data);
		ttLibC_Crc32_update(crc32, *data);
	}
	LOG_PRINT("result:%llu", ttLibC_Crc32_getValue(crc32));
	ASSERTM("FAILED", ttLibC_Crc32_getValue(crc32) == 0x2AB104B2);
	ttLibC_Crc32_close(&crc32);
	ASSERT(ttLibC_Allocator_dump() == 0);
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
	ASSERT(ttLibC_Allocator_dump() == 0);
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
	ASSERT(ttLibC_Allocator_dump() == 0);
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
	ASSERT(ttLibC_Allocator_dump() == 0);
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
	ASSERT(ttLibC_Allocator_dump() == 0);
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
	ASSERT(ttLibC_Allocator_dump() == 0);
}

/**
 * define all test for util package.
 * @param s cute::suite obj
 * @return cute::suite obj
 */
cute::suite utilTests(cute::suite s) {
	s.clear();
	s.push_back(CUTE(stlListTest));
	s.push_back(CUTE(linkedListTest));
	s.push_back(CUTE(byteUtilTest));
	s.push_back(CUTE(connectorTest));
	s.push_back(CUTE(dynamicBufferTest));
	s.push_back(CUTE(amfTest));
	s.push_back(CUTE(crc32Test));
	s.push_back(CUTE(ioTest));
	s.push_back(CUTE(httpClientTest));
	s.push_back(CUTE(hexUtilTest));
	s.push_back(CUTE(opencvUtilTest));
	s.push_back(CUTE(openalUtilTest));
	return s;
}
