/**
 * @file   containerTest.cpp
 * @brief  container test code.
 *
 * this code is under GPLv3 license.
 *
 * @author taktod
 * @date   2015/07/18
 */

#include <cute.h>
#include <array>
#include <ttLibC/log.h>
#include <ttLibC/util/hexUtil.h>
#include <stdio.h>

#include <ttLibC/container/flv.h>
#include <ttLibC/container/mpegts.h>

typedef struct {
	ttLibC_ContainerReader *reader;
	ttLibC_ContainerWriter *writer;
	FILE *fp_in;
	FILE *fp_out;
} containerTest_t;

bool mpegtsTest_writePacketCallback(void *ptr, void *data, size_t data_size) {
	containerTest_t *testData = (containerTest_t *)ptr;
	if(testData->fp_out) {
		fwrite(data, 1, data_size, testData->fp_out);
	}
	return true;
}

bool mpegtsTest_getFrameCallback(void *ptr, ttLibC_Frame *frame) {
	containerTest_t *testData = (containerTest_t *)ptr;
	switch(frame->type) {
	case frameType_h264:
		return ttLibC_MpegtsWriter_write((ttLibC_MpegtsWriter *)testData->writer, false, 0x100, frame, mpegtsTest_writePacketCallback, ptr);
	case frameType_aac:
		return ttLibC_MpegtsWriter_write((ttLibC_MpegtsWriter *)testData->writer, false, 0x101, frame, mpegtsTest_writePacketCallback, ptr);
	default:
		LOG_PRINT("unexpect frame type is found.");
		return false;
	}
}

bool mpegtsTest_getMpegtsCallback(void *ptr, ttLibC_Mpegts *packet) {
	return ttLibC_Mpegts_getFrame(packet, mpegtsTest_getFrameCallback, ptr);
}

static void mpegtsTest() {
	LOG_PRINT("mpegtsTest");

	containerTest_t testData;
	testData.reader = (ttLibC_ContainerReader *)ttLibC_MpegtsReader_make();
	ttLibC_Frame_Type frameTypes[] = {
			frameType_h264,
			frameType_aac
	};
	testData.writer = (ttLibC_ContainerWriter *)ttLibC_MpegtsWriter_make(frameTypes, 2);
	testData.fp_in = fopen("test.ts", "rb");
	testData.fp_out = fopen("test_out.ts", "wb");
	do {
		uint8_t buffer[65536];
		size_t read_size = fread(buffer, 1, 65536, testData.fp_in);
		if(!ttLibC_MpegtsReader_read((ttLibC_MpegtsReader *)testData.reader, buffer, read_size, mpegtsTest_getMpegtsCallback, &testData)) {
			ERR_PRINT("error occured!");
			break;
		}
	}while(!feof(testData.fp_in));
	ttLibC_ContainerReader_close(&testData.reader);
	ttLibC_ContainerWriter_close(&testData.writer);
	if(testData.fp_in)  {fclose(testData.fp_in); testData.fp_in  = NULL;}
	if(testData.fp_out) {fclose(testData.fp_out);testData.fp_out = NULL;}
}

bool flvTest_writeTagCallback(void *ptr, void *data, size_t data_size) {
	containerTest_t *testData = (containerTest_t *)ptr;
	if(testData->fp_out) {
		fwrite(data, 1, data_size, testData->fp_out);
	}
	return true;
}

bool flvTest_getFrameCallback(void *ptr, ttLibC_Frame *frame) {
	containerTest_t *testData = (containerTest_t *)ptr;
	return ttLibC_FlvWriter_write((ttLibC_FlvWriter*)testData->writer, frame, flvTest_writeTagCallback, ptr);
}

bool flvTest_getFlvCallback(void *ptr, ttLibC_Flv *flv) {
	return ttLibC_Flv_getFrame(flv, flvTest_getFrameCallback, ptr);
}

static void flvTest() {
	LOG_PRINT("flvTest");

	containerTest_t testData;
	testData.reader = (ttLibC_ContainerReader *)ttLibC_FlvReader_make();
	testData.writer = (ttLibC_ContainerWriter *)ttLibC_FlvWriter_make(frameType_h264, frameType_aac);
	testData.fp_in = fopen("test.flv", "rb");
	testData.fp_out = fopen("test_out.flv", "wb");
	do {
		uint8_t buffer[65536];
		size_t read_size = fread(buffer, 1, 65536, testData.fp_in);
		if(!ttLibC_FlvReader_read((ttLibC_FlvReader *)testData.reader, buffer, read_size, flvTest_getFlvCallback, &testData)) {
			ERR_PRINT("error occured!");
			break;
		}
	}while(!feof(testData.fp_in));
	ttLibC_ContainerReader_close(&testData.reader);
	ttLibC_ContainerWriter_close(&testData.writer);
	if(testData.fp_in)  {fclose(testData.fp_in); testData.fp_in  = NULL;}
	if(testData.fp_out) {fclose(testData.fp_out);testData.fp_out = NULL;}
}

/**
 * define all test for container package.
 * @param s cute::suite obj
 * @return cute::suite obj
 */
cute::suite containerTests(cute::suite s) {
	s.clear();
	// now these test is works only with h264/aac data.
//	s.push_back(CUTE(mpegtsTest));
//	s.push_back(CUTE(flvTest));
	return s;
}

