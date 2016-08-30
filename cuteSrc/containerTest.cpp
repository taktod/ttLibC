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
#include <ttLibC/allocator.h>
#include <ttLibC/util/hexUtil.h>
#include <stdio.h>

#include <ttLibC/container/flv.h>
#include <ttLibC/container/mpegts.h>
#include <ttLibC/container/mp3.h>
#include <ttLibC/container/mp4.h>
#include <ttLibC/container/mkv.h>

typedef struct {
	ttLibC_ContainerReader *reader;
	ttLibC_ContainerWriter *writer;
	FILE *fp_in;
	FILE *fp_out;
	size_t write_size;
} containerTest_t;

static bool mp4Test_writeDataCallback(void *ptr, void *data, size_t data_size) {
	containerTest_t *testData = (containerTest_t *)ptr;
	testData->write_size += data_size;
	if(testData->fp_out) {
		fwrite(data, 1, data_size, testData->fp_out);
	}
	return true;
}

static bool mp4Test_getFrameCallback(void *ptr, ttLibC_Frame *frame) {
	containerTest_t *testData = (containerTest_t *)ptr;
	switch(frame->type) {
	case frameType_h264:
		{
			ttLibC_Mp4Writer_write((ttLibC_Mp4Writer *)testData->writer, frame, mp4Test_writeDataCallback, ptr);
			// make clone frame for 2nd video track.
			ttLibC_Frame *cloned_frame = ttLibC_Frame_clone(NULL, frame);
			cloned_frame->id = 3;
			ttLibC_Mp4Writer_write((ttLibC_Mp4Writer *)testData->writer, cloned_frame, mp4Test_writeDataCallback, ptr);
			ttLibC_Frame_close(&cloned_frame);
		}
		return true;
	case frameType_aac:
		return ttLibC_Mp4Writer_write((ttLibC_Mp4Writer *)testData->writer, frame, mp4Test_writeDataCallback, ptr);
	default:
		break;
	}
	return true;
}

static bool mp4Test_getMp4Callback(void *ptr, ttLibC_Mp4 *mp4) {
	return ttLibC_Mp4_getFrame(mp4, mp4Test_getFrameCallback, ptr);
}

static void mp4Test() {
	LOG_PRINT("mp4Test");
	containerTest_t testData;
	testData.write_size = 0;
	testData.reader = (ttLibC_ContainerReader *)ttLibC_Mp4Reader_make();
	ttLibC_Frame_Type frameTypes[] = {
			frameType_h264,
			frameType_aac,
			frameType_h264
	};
	testData.writer = (ttLibC_ContainerWriter *)ttLibC_Mp4Writer_make(frameTypes, 3);
	testData.fp_in = fopen("test.mp4", "rb");
//	testData.fp_in = fopen("test_mod.mp4", "rb");
//	testData.fp_in = fopen("test.fmp4", "rb");
	testData.fp_out = fopen("test_out.mp4", "wb");
	do {
		if(!testData.fp_in || !testData.fp_out) {
			break;
		}
		uint8_t buffer[65536];
		size_t read_size = fread(buffer, 1, 65536, testData.fp_in);
		if(!ttLibC_Mp4Reader_read((ttLibC_Mp4Reader *)testData.reader, buffer, read_size, mp4Test_getMp4Callback, &testData)) {
			ERR_PRINT("error occured!");
			break;
		}
	} while(!feof(testData.fp_in));
	ttLibC_ContainerReader_close(&testData.reader);
	ttLibC_ContainerWriter_close(&testData.writer);
	if(testData.fp_in)  {fclose(testData.fp_in); testData.fp_in  = NULL;}
	if(testData.fp_out) {fclose(testData.fp_out);testData.fp_out = NULL;}
	ASSERT(ttLibC_Allocator_dump() == 0);
}

bool mkvTest_getFrameCallback(void *ptr, ttLibC_Frame *frame) {
	switch(frame->type) {
	case frameType_h264:
		LOG_PRINT("h264:%f", 1.0 * frame->pts / frame->timebase);
		break;
	case frameType_vp8:
		LOG_PRINT("vp8:%f", 1.0 * frame->pts / frame->timebase);
		break;
	case frameType_vp9:
		LOG_PRINT("vp9:%f", 1.0 * frame->pts / frame->timebase);
		break;
	case frameType_theora:
		LOG_PRINT("theora:%f", 1.0 * frame->pts / frame->timebase);
		break;
	case frameType_aac:
		LOG_PRINT("aac:%f", 1.0 * frame->pts / frame->timebase);
		break;
	case frameType_opus:
		LOG_PRINT("opus:%f", 1.0 * frame->pts / frame->timebase);
		break;
	case frameType_vorbis:
		LOG_PRINT("vorbis:%f", 1.0 * frame->pts / frame->timebase);
		break;
	default:
		LOG_PRINT("frame:%f", 1.0 * frame->pts / frame->timebase);
		break;
	}
	return true;
}

bool mkvTest_getMkvCallback(void *ptr, ttLibC_Mkv *mkv) {
	return ttLibC_Mkv_getFrame(mkv, mkvTest_getFrameCallback, ptr);
}

static void mkvTest() {
	LOG_PRINT("mkvTest");

	containerTest_t testData;
	testData.write_size = 0;
	testData.reader = (ttLibC_ContainerReader *)ttLibC_MkvReader_make();
	testData.writer = (ttLibC_ContainerWriter *)ttLibC_MkvWriter_make();
	testData.fp_in = fopen("test.mkv", "rb");
	testData.fp_out = fopen("test_out.mkv", "wb");
	do {
		uint8_t buffer[65536];
		if(!testData.fp_in) {
			break;
		}
		size_t read_size = fread(buffer, 1, 65536, testData.fp_in);
		if(!ttLibC_MkvReader_read((ttLibC_MkvReader *)testData.reader, buffer, read_size, mkvTest_getMkvCallback, &testData)) {
			ERR_PRINT("error occured!");
			break;
		}
	} while(!feof(testData.fp_in));
	ttLibC_ContainerReader_close(&testData.reader);
	ttLibC_ContainerWriter_close(&testData.writer);
	if(testData.fp_in)  {fclose(testData.fp_in); testData.fp_in  = NULL;}
	if(testData.fp_out) {fclose(testData.fp_out);testData.fp_out = NULL;}
	ASSERT(ttLibC_Allocator_dump() == 0);
}

bool mpegtsToFlvTest_writeFuncCallback(void *ptr, void *data, size_t data_size) {
	containerTest_t *testData = (containerTest_t *)ptr;
	testData->write_size += data_size;
	if(testData->fp_out) {
		fwrite(data, 1, data_size, testData->fp_out);
	}
	return true;
}

bool mpegtsToFlvTest_getFrameCallback(void *ptr, ttLibC_Frame *frame) {
	containerTest_t *testData = (containerTest_t *)ptr;
	return ttLibC_FlvWriter_write(
			(ttLibC_FlvWriter *)testData->writer,
			frame,
			mpegtsToFlvTest_writeFuncCallback,
			ptr);
}

bool mpegtsToFlvTest_getMpegtsPacketCallback(void *ptr, ttLibC_Mpegts *mpegts) {
	return ttLibC_Mpegts_getFrame(
			mpegts,
			mpegtsToFlvTest_getFrameCallback,
			ptr);
}

static void mpegtsToFlvTest() {
	LOG_PRINT("mpegtsToFlvTest");
	containerTest_t testData;
	testData.write_size = 0;
	testData.reader = (ttLibC_ContainerReader *)ttLibC_MpegtsReader_make();
	testData.writer = (ttLibC_ContainerWriter *)ttLibC_FlvWriter_make(frameType_h264, frameType_aac);
	testData.fp_in = fopen("test.ts", "rb");
	testData.fp_out = fopen("test_ts_out.flv", "wb");
	do {
		uint8_t buffer[65536];
		size_t read_size = fread(buffer, 1, 65536, testData.fp_in);
		if(!ttLibC_MpegtsReader_read(
				(ttLibC_MpegtsReader *)testData.reader,
				buffer,
				read_size,
				mpegtsToFlvTest_getMpegtsPacketCallback,
				&testData)) {
			break;
		}
	}while(!feof(testData.fp_in));
	ttLibC_ContainerReader_close(&testData.reader);
	ttLibC_ContainerWriter_close(&testData.writer);
	if(testData.fp_in)  {fclose(testData.fp_in); testData.fp_in  = NULL;}
	if(testData.fp_out) {fclose(testData.fp_out);testData.fp_out = NULL;}
	ASSERT(ttLibC_Allocator_dump() == 0);
}

bool mp3Test_writeFrameCallback(void *ptr, void *data, size_t data_size) {
	containerTest_t *testData = (containerTest_t *)ptr;
	testData->write_size += data_size;
	if(testData->fp_out) {
		fwrite(data, 1, data_size, testData->fp_out);
	}
	return true;
}

bool mp3Test_getFrameCallback(void *ptr, ttLibC_Frame *frame) {
	containerTest_t *testData = (containerTest_t *)ptr;
	return ttLibC_Mp3Writer_write((ttLibC_Mp3Writer *)testData->writer, frame, mp3Test_writeFrameCallback, ptr);
}

bool mp3Test_getMp3Callback(void *ptr, ttLibC_Container_Mp3 *mp3) {
	return ttLibC_Container_Mp3_getFrame(mp3, mp3Test_getFrameCallback, ptr);
}

static void mp3Test() {
	LOG_PRINT("mp3Test");
	containerTest_t testData;
	testData.write_size = 0;
	testData.reader = (ttLibC_ContainerReader *)ttLibC_Mp3Reader_make();
	testData.writer = (ttLibC_ContainerWriter *)ttLibC_Mp3Writer_make();
	testData.fp_in = fopen("test.mp3", "rb");
	testData.fp_out = fopen("test_out.mp3", "wb");
	do {
		uint8_t buffer[65536];
		if(!testData.fp_in) {
			break;
		}
		if(!testData.fp_out) {
			break;
		}
		size_t read_size = fread(buffer, 1, 65536, testData.fp_in);
		if(!ttLibC_Mp3Reader_read((ttLibC_Mp3Reader *)testData.reader, buffer, read_size, mp3Test_getMp3Callback, &testData)) {
			ERR_PRINT("error occured!");
			break;
		}
	}while(!feof(testData.fp_in));
	ttLibC_ContainerReader_close(&testData.reader);
	ttLibC_ContainerWriter_close(&testData.writer);
	if(testData.fp_in)  {fclose(testData.fp_in); testData.fp_in  = NULL;}
	if(testData.fp_out) {fclose(testData.fp_out);testData.fp_out = NULL;}
	LOG_PRINT("write size is %d", testData.write_size);
	ASSERT(ttLibC_Allocator_dump() == 0);
}

bool mpegtsTest_writePacketCallback(void *ptr, void *data, size_t data_size) {
	containerTest_t *testData = (containerTest_t *)ptr;
	testData->write_size += data_size;
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
	case frameType_mp3:
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
	testData.write_size = 0;
	testData.reader = (ttLibC_ContainerReader *)ttLibC_MpegtsReader_make();
	ttLibC_Frame_Type frameTypes[] = {
			frameType_h264,
			frameType_aac
	};
	testData.writer = (ttLibC_ContainerWriter *)ttLibC_MpegtsWriter_make_ex(
			frameTypes,
			2,
			15000); // write task with 0.5 sec each.
	testData.fp_in = fopen("test.ts", "rb");
	testData.fp_out = fopen("test_out.ts", "wb");
	do {
		if(!testData.fp_in || !testData.fp_out) {
			break;
		}
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
	LOG_PRINT("write size:%lld", testData.write_size);
	ASSERT(ttLibC_Allocator_dump() == 0);
}

static void mpegtsH264Mp3Test() {
	LOG_PRINT("mpegtsH264Mp3Test");

	containerTest_t testData;
	testData.write_size = 0;
	testData.reader = (ttLibC_ContainerReader *)ttLibC_MpegtsReader_make();
	ttLibC_Frame_Type frameTypes[] = {
			frameType_h264,
			frameType_mp3
	};
	testData.writer = (ttLibC_ContainerWriter *)ttLibC_MpegtsWriter_make_ex(
			frameTypes,
			2,
			15000); // write task with 0.5 sec each.
	testData.fp_in = fopen("test_h264_mp3.ts", "rb");
	testData.fp_out = fopen("test_h264_mp3_out.ts", "wb");
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
	ASSERT(ttLibC_Allocator_dump() == 0);
}

static void mpegtsVlcTest() {
	LOG_PRINT("mpegtsVlcTest");

	containerTest_t testData;
	testData.write_size = 0;
	testData.reader = (ttLibC_ContainerReader *)ttLibC_MpegtsReader_make();
	ttLibC_Frame_Type frameTypes[] = {
			frameType_h264,
			frameType_aac
	};
	testData.writer = (ttLibC_ContainerWriter *)ttLibC_MpegtsWriter_make_ex(
			frameTypes,
			2,
			15000); // write task with 0.5 sec each.
	testData.fp_in = fopen("test_vlc_h264_aac.ts", "rb");
	testData.fp_out = fopen("test_vlc_h264_aac_out.ts", "wb");
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
	ASSERT(ttLibC_Allocator_dump() == 0);
}

bool flvTest_writeTagCallback(void *ptr, void *data, size_t data_size) {
	containerTest_t *testData = (containerTest_t *)ptr;
	testData->write_size += data_size;
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
	testData.write_size = 0;
	testData.reader = (ttLibC_ContainerReader *)ttLibC_FlvReader_make();
	testData.writer = (ttLibC_ContainerWriter *)ttLibC_FlvWriter_make(frameType_h264, frameType_aac);
	testData.fp_in = fopen("test.flv", "rb");
	testData.fp_out = fopen("test_out.flv", "wb");
	do {
		if(!testData.fp_in || !testData.fp_out) {
			break;
		}
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
	LOG_PRINT("write size:%lld", testData.write_size);
	ASSERT(ttLibC_Allocator_dump() == 0);
}

static void flvFlv1AacTest() {
	LOG_PRINT("flvTest");

	containerTest_t testData;
	testData.write_size = 0;
	testData.reader = (ttLibC_ContainerReader *)ttLibC_FlvReader_make();
	testData.writer = (ttLibC_ContainerWriter *)ttLibC_FlvWriter_make(frameType_flv1, frameType_aac);
	testData.fp_in = fopen("test_flv1_aac.flv", "rb");
	testData.fp_out = fopen("test_flv1_aac_out.flv", "wb");
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
	ASSERT(ttLibC_Allocator_dump() == 0);
}

/**
 * define all test for container package.
 * @param s cute::suite obj
 * @return cute::suite obj
 */
cute::suite containerTests(cute::suite s) {
	s.clear();
//	s.push_back(CUTE(mpegtsVlcTest));
//	s.push_back(CUTE(mpegtsH264Mp3Test));
//	s.push_back(CUTE(flvFlv1AacTest));
//	s.push_back(CUTE(mpegtsToFlvTest)); // h264/aac
	s.push_back(CUTE(mp4Test));
	s.push_back(CUTE(mkvTest));
	s.push_back(CUTE(mp3Test)); // none/mp3
	s.push_back(CUTE(mpegtsTest)); // h264/aac
	s.push_back(CUTE(flvTest)); // h264/aac
	return s;
}

