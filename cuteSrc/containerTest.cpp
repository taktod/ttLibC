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

#include <ttLibC/frame/audio/audio.h>
#include <ttLibC/frame/video/h264.h>

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
	case frameType_h265:
		LOG_PRINT("h265:%f", 1.0 * frame->pts / frame->timebase);
		break;
	case frameType_h264:
		LOG_PRINT("h264:%f", 1.0 * frame->pts / frame->timebase);
		break;
	case frameType_jpeg:
		LOG_PRINT("jpeg:%f", 1.0 * frame->pts / frame->timebase);
		break;
	case frameType_aac:
		LOG_PRINT("aac:%f", 1.0 * frame->pts / frame->timebase);
		break;
	case frameType_vorbis:
		LOG_PRINT("vorbis:%f", 1.0 * frame->pts / frame->timebase);
		break;
	case frameType_mp3:
		LOG_PRINT("mp3:%f", 1.0 * frame->pts / frame->timebase);
		break;
	default:
		LOG_PRINT("frame:%f", 1.0 * frame->pts / frame->timebase);
		return true;
	}
	if(testData->writer != NULL) {
		return ttLibC_Mp4Writer_write((ttLibC_Mp4Writer *)testData->writer, frame, mp4Test_writeDataCallback, ptr);
	}
	return true;
}

static bool mp4Test_getMp4Callback(void *ptr, ttLibC_Mp4 *mp4) {
	return ttLibC_Mp4_getFrame(mp4, mp4Test_getFrameCallback, ptr);
}

static void mp4CodecTest() {
	LOG_PRINT("mp4CodecTest");
	containerTest_t testData;
	char file[256];
	ttLibC_Frame_Type types[2];

	LOG_PRINT("h264 / aac");
	testData.reader = (ttLibC_ContainerReader *)ttLibC_Mp4Reader_make();
	types[0] = frameType_h264;
	types[1] = frameType_aac;
	testData.writer = (ttLibC_ContainerWriter *)ttLibC_Mp4Writer_make(types, 2);
	testData.writer->mode = containerWriter_enable_dts;
	sprintf(file, "%s/tools/data/source/test.h264.aac.mp4", getenv("HOME"));
	testData.fp_in = fopen(file, "rb");
	sprintf(file, "%s/tools/data/c_out/test.h264.aac.mp4", getenv("HOME"));
	testData.fp_out = fopen(file, "wb");
	do {
		uint8_t buffer[65536];
		if(!testData.fp_in) {
			break;
		}
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
	// */

	LOG_PRINT("h265 / mp3");
	testData.reader = (ttLibC_ContainerReader *)ttLibC_Mp4Reader_make();
	types[0] = frameType_h265;
	types[1] = frameType_mp3;
	testData.writer = (ttLibC_ContainerWriter *)ttLibC_Mp4Writer_make(types, 2);
	testData.writer->mode = containerWriter_enable_dts;
	sprintf(file, "%s/tools/data/source/test.h265.mp3.mp4", getenv("HOME"));
	testData.fp_in = fopen(file, "rb");
	sprintf(file, "%s/tools/data/c_out/test.h265b.mp3.mp4", getenv("HOME"));
	testData.fp_out = fopen(file, "wb");
	do {
		uint8_t buffer[65536];
		if(!testData.fp_in) {
			break;
		}
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
	// */

	LOG_PRINT("mjpeg / vorbis");
	testData.reader = (ttLibC_ContainerReader *)ttLibC_Mp4Reader_make();
	types[0] = frameType_jpeg;
	types[1] = frameType_vorbis;
	testData.writer = (ttLibC_ContainerWriter *)ttLibC_Mp4Writer_make(types, 2);
	sprintf(file, "%s/tools/data/source/test.mjpeg.vorbis.mp4", getenv("HOME"));
	testData.fp_in = fopen(file, "rb");
	sprintf(file, "%s/tools/data/c_out/test.mjpeg.vorbis.mp4", getenv("HOME"));
	testData.fp_out = fopen(file, "wb");
	do {
		uint8_t buffer[65536];
		if(!testData.fp_in) {
			break;
		}
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
	// */
}

static void mp4Test() {
	LOG_PRINT("mp4Test");
	containerTest_t testData;
	testData.write_size = 0;
	testData.reader = (ttLibC_ContainerReader *)ttLibC_Mp4Reader_make();
	ttLibC_Frame_Type frameTypes[] = {
			frameType_h264,
			frameType_aac,
	};
	testData.writer = (ttLibC_ContainerWriter *)ttLibC_Mp4Writer_make(frameTypes, 2);
	testData.fp_in = fopen("test.mp4", "rb");
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

static bool mkvTest_writeDataCallback(void *ptr, void *data, size_t data_size) {
	containerTest_t *testData = (containerTest_t *)ptr;
	testData->write_size += data_size;
	if(testData->fp_out) {
		fwrite(data, 1, data_size, testData->fp_out);
	}
	return true;
}

bool mkvTest_getFrameCallback(void *ptr, ttLibC_Frame *frame) {
	containerTest_t *testData = (containerTest_t *)ptr;
	switch(frame->type) {
	case frameType_h265:
		LOG_PRINT("h265:%f", 1.0 * frame->pts / frame->timebase);
		break;
	case frameType_h264:
		LOG_PRINT("h264:%f", 1.0 * frame->pts / frame->timebase);
		break;
	case frameType_jpeg:
		LOG_PRINT("jpeg:%f", 1.0 * frame->pts / frame->timebase);
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
	case frameType_adpcm_ima_wav:
		LOG_PRINT("adpcmImaWav:%f", 1.0 * frame->pts / frame->timebase);
		break;
	case frameType_opus:
		LOG_PRINT("opus:%f", 1.0 * frame->pts / frame->timebase);
		break;
	case frameType_vorbis:
		LOG_PRINT("vorbis:%f", 1.0 * frame->pts / frame->timebase);
		break;
	case frameType_mp3:
		LOG_PRINT("mp3:%f", 1.0 * frame->pts / frame->timebase);
		break;
	case frameType_speex:
		LOG_PRINT("speex:%f", 1.0 *frame->pts / frame->timebase);
		break;
	default:
		LOG_PRINT("frame:%f", 1.0 * frame->pts / frame->timebase);
		return true;
	}
	if(testData->writer != NULL) {
		return ttLibC_MkvWriter_write((ttLibC_MkvWriter *)testData->writer, frame, mkvTest_writeDataCallback, ptr);
	}
	return true;
}

bool mkvTest_getMkvCallback(void *ptr, ttLibC_Mkv *mkv) {
	return ttLibC_Mkv_getFrame(mkv, mkvTest_getFrameCallback, ptr);
}

static void mkvCodecTest() {
	LOG_PRINT("mkvCodecTest");
	containerTest_t testData;
	char file[256];
	ttLibC_Frame_Type types[2];

	LOG_PRINT("h264 / aac");
	testData.reader = (ttLibC_ContainerReader *)ttLibC_MkvReader_make();
	types[0] = frameType_h264;
	types[1] = frameType_aac;
	testData.writer = (ttLibC_ContainerWriter *)ttLibC_MkvWriter_make(types, 2);
	sprintf(file, "%s/tools/data/source/test.h264.aac.mkv", getenv("HOME"));
	testData.fp_in = fopen(file, "rb");
	sprintf(file, "%s/tools/data/c_out/test.h264.aac.mkv", getenv("HOME"));
	testData.fp_out = fopen(file, "wb");
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

	LOG_PRINT("h265 / mp3");
	testData.reader = (ttLibC_ContainerReader *)ttLibC_MkvReader_make();
	types[0] = frameType_h265;
	types[1] = frameType_mp3;
	testData.writer = (ttLibC_ContainerWriter *)ttLibC_MkvWriter_make(types, 2);
	sprintf(file, "%s/tools/data/source/test.h265.mp3.mkv", getenv("HOME"));
	testData.fp_in = fopen(file, "rb");
	sprintf(file, "%s/tools/data/c_out/test.h265.mp3.mkv", getenv("HOME"));
	testData.fp_out = fopen(file, "wb");
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

	LOG_PRINT("theora / speex");
	testData.reader = (ttLibC_ContainerReader *)ttLibC_MkvReader_make();
	types[0] = frameType_theora;
	types[1] = frameType_speex;
	testData.writer = (ttLibC_ContainerWriter *)ttLibC_MkvWriter_make(types, 2);
	sprintf(file, "%s/tools/data/source/test.theora.speex.mkv", getenv("HOME"));
	testData.fp_in = fopen(file, "rb");
	sprintf(file, "%s/tools/data/c_out/test.theora.speex.mkv", getenv("HOME"));
	testData.fp_out = fopen(file, "wb");
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

	LOG_PRINT("mjpeg / adpcmimawav");
	testData.reader = (ttLibC_ContainerReader *)ttLibC_MkvReader_make();
	types[0] = frameType_jpeg;
	types[1] = frameType_adpcm_ima_wav;
	testData.writer = (ttLibC_ContainerWriter *)ttLibC_MkvWriter_make(types, 2);
	sprintf(file, "%s/tools/data/source/test.mjpeg.adpcmimawav.mkv", getenv("HOME"));
	testData.fp_in = fopen(file, "rb");
	sprintf(file, "%s/tools/data/c_out/test.mjpeg.adpcmimawav.mkv", getenv("HOME"));
	testData.fp_out = fopen(file, "wb");
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

	LOG_PRINT("vp8 / vorbis");
	testData.reader = (ttLibC_ContainerReader *)ttLibC_MkvReader_make();
	types[0] = frameType_vp8;
	types[1] = frameType_vorbis;
	testData.writer = (ttLibC_ContainerWriter *)ttLibC_MkvWriter_make(types, 2);
	((ttLibC_MkvWriter *)testData.writer)->is_webm = true;
	sprintf(file, "%s/tools/data/source/test.vp8.vorbis.webm", getenv("HOME"));
	testData.fp_in = fopen(file, "rb");
	sprintf(file, "%s/tools/data/c_out/test.vp8.vorbis.webm", getenv("HOME"));
	testData.fp_out = fopen(file, "wb");
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

	LOG_PRINT("vp9 / opus");
	testData.reader = (ttLibC_ContainerReader *)ttLibC_MkvReader_make();
	types[0] = frameType_vp9;
	types[1] = frameType_opus;
	testData.writer = (ttLibC_ContainerWriter *)ttLibC_MkvWriter_make(types, 2);
	((ttLibC_MkvWriter *)testData.writer)->is_webm = true;
	sprintf(file, "%s/tools/data/source/test.vp9.opus.webm", getenv("HOME"));
	testData.fp_in = fopen(file, "rb");
	sprintf(file, "%s/tools/data/c_out/test.vp9.opus.webm", getenv("HOME"));
	testData.fp_out = fopen(file, "wb");
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

static void webmTest() {
	LOG_PRINT("webmTest");

	containerTest_t testData;
	testData.write_size = 0;
	testData.reader = (ttLibC_ContainerReader *)ttLibC_MkvReader_make();
	ttLibC_Frame_Type frameTypes[] = {
			frameType_vp8,
			frameType_opus
	};
	testData.writer = (ttLibC_ContainerWriter *)ttLibC_MkvWriter_make(
			frameTypes, 2);
	((ttLibC_MkvWriter *)testData.writer)->is_webm = true;
	testData.fp_in = fopen("test.webm", "rb");
	testData.fp_out = fopen("test_out.webm", "wb");
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

static void mkvTest() {
	LOG_PRINT("mkvTest");

	containerTest_t testData;
	testData.write_size = 0;
	testData.reader = (ttLibC_ContainerReader *)ttLibC_MkvReader_make();
	ttLibC_Frame_Type frameTypes[] = {
			frameType_h264,
			frameType_aac
	};
	testData.writer = (ttLibC_ContainerWriter *)ttLibC_MkvWriter_make(
			frameTypes, 2);
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
	LOG_PRINT("write size is %llu", testData.write_size);
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
		{
			ttLibC_H264 *h264 = (ttLibC_H264 *)frame;
			if(h264->type == H264Type_unknown) {
				return true;
			}
//			LOG_PRINT("h264:%x %f %llu type:%d disposable:%d", frame->id, 1.0 * frame->pts / frame->timebase, frame->pts, h264->frame_type, h264->is_disposable);
		}
		break;
	case frameType_aac:
//		LOG_PRINT("aac:%x %f %llu", frame->id, 1.0 * frame->pts / frame->timebase, frame->pts);
		break;
	case frameType_mp3:
//		LOG_PRINT("mp3:%x %f", frame->id, 1.0 * frame->pts / frame->timebase);
		break;
	default:
		LOG_PRINT("unexpect frame type is found.");		return false;
	}
	if(testData->writer != NULL) {
		return ttLibC_MpegtsWriter_write((ttLibC_MpegtsWriter *)testData->writer, frame, mpegtsTest_writePacketCallback, ptr);;
	}
	return true;
}

bool mpegtsTest_getMpegtsCallback(void *ptr, ttLibC_Mpegts *packet) {
	return ttLibC_Mpegts_getFrame(packet, mpegtsTest_getFrameCallback, ptr);
}

static void mpegtsCodecTest() {
	LOG_PRINT("mpegtsCodecTest");
	containerTest_t testData;
	char file[256];
	ttLibC_Frame_Type types[2];

	LOG_PRINT("h264 / aac");
	testData.reader = (ttLibC_ContainerReader *)ttLibC_MpegtsReader_make();
	types[0] = frameType_h264;
	types[1] = frameType_aac;
	testData.writer = (ttLibC_ContainerWriter *)ttLibC_MpegtsWriter_make(types, 2);
	testData.writer->mode = containerWriter_innerFrame_split | containerWriter_allKeyFrame_split;
	sprintf(file, "%s/tools/data/source/test.h264.aac.ts", getenv("HOME"));
	testData.fp_in = fopen(file, "rb");
	sprintf(file, "%s/tools/data/c_out/test.h264.aac.ts", getenv("HOME"));
	testData.fp_out = fopen(file, "wb");
	do {
		uint8_t buffer[65536];
		if(!testData.fp_in) {
			break;
		}
		size_t read_size = fread(buffer, 1, 65536, testData.fp_in);
		if(!ttLibC_MpegtsReader_read((ttLibC_MpegtsReader *)testData.reader, buffer, read_size, mpegtsTest_getMpegtsCallback, &testData)) {
			ERR_PRINT("error occured!");
			break;
		}
	} while(!feof(testData.fp_in));
	ttLibC_ContainerReader_close(&testData.reader);
	ttLibC_ContainerWriter_close(&testData.writer);
	if(testData.fp_in)  {fclose(testData.fp_in); testData.fp_in  = NULL;}
	if(testData.fp_out) {fclose(testData.fp_out);testData.fp_out = NULL;}
	ASSERT(ttLibC_Allocator_dump() == 0);
	// */

	LOG_PRINT("h264 / mp3");
	testData.reader = (ttLibC_ContainerReader *)ttLibC_MpegtsReader_make();
	types[0] = frameType_h264;
	types[1] = frameType_mp3;
	testData.writer = (ttLibC_ContainerWriter *)ttLibC_MpegtsWriter_make(types, 2);
	testData.writer->mode = containerWriter_enable_dts | containerWriter_allKeyFrame_split;
	sprintf(file, "%s/tools/data/source/test.h264.mp3.ts", getenv("HOME"));
	testData.fp_in = fopen(file, "rb");
	sprintf(file, "%s/tools/data/c_out/test.h264.mp3.ts", getenv("HOME"));
	testData.fp_out = fopen(file, "wb");
	do {
		uint8_t buffer[65536];
		if(!testData.fp_in) {
			break;
		}
		size_t read_size = fread(buffer, 1, 65536, testData.fp_in);
		if(!ttLibC_MpegtsReader_read((ttLibC_MpegtsReader *)testData.reader, buffer, read_size, mpegtsTest_getMpegtsCallback, &testData)) {
			ERR_PRINT("error occured!");
			break;
		}
	} while(!feof(testData.fp_in));
	ttLibC_ContainerReader_close(&testData.reader);
	ttLibC_ContainerWriter_close(&testData.writer);
	if(testData.fp_in)  {fclose(testData.fp_in); testData.fp_in  = NULL;}
	if(testData.fp_out) {fclose(testData.fp_out);testData.fp_out = NULL;}
	ASSERT(ttLibC_Allocator_dump() == 0);
	// */

	LOG_PRINT("h264b / mp3");
	testData.reader = (ttLibC_ContainerReader *)ttLibC_MpegtsReader_make();
	types[0] = frameType_h264;
	types[1] = frameType_mp3;
	testData.writer = (ttLibC_ContainerWriter *)ttLibC_MpegtsWriter_make(types, 2);
	testData.writer->mode = containerWriter_enable_dts | containerWriter_allKeyFrame_split;
	sprintf(file, "%s/tools/data/source/test.h264b.mp3.ts", getenv("HOME"));
	testData.fp_in = fopen(file, "rb");
	sprintf(file, "%s/tools/data/c_out/test.h264b.mp3.ts", getenv("HOME"));
	testData.fp_out = fopen(file, "wb");
	do {
		uint8_t buffer[65536];
		if(!testData.fp_in) {
			break;
		}
		size_t read_size = fread(buffer, 1, 65536, testData.fp_in);
		if(!ttLibC_MpegtsReader_read((ttLibC_MpegtsReader *)testData.reader, buffer, read_size, mpegtsTest_getMpegtsCallback, &testData)) {
			ERR_PRINT("error occured!");
			break;
		}
	} while(!feof(testData.fp_in));
	ttLibC_ContainerReader_close(&testData.reader);
	ttLibC_ContainerWriter_close(&testData.writer);
	if(testData.fp_in)  {fclose(testData.fp_in); testData.fp_in  = NULL;}
	if(testData.fp_out) {fclose(testData.fp_out);testData.fp_out = NULL;}
	ASSERT(ttLibC_Allocator_dump() == 0);
	// */

	LOG_PRINT("h264b / aac");
	testData.reader = (ttLibC_ContainerReader *)ttLibC_MpegtsReader_make();
	types[0] = frameType_h264;
	types[1] = frameType_aac;
	testData.writer = (ttLibC_ContainerWriter *)ttLibC_MpegtsWriter_make_ex(types, 2, 15000);
	testData.writer->mode = containerWriter_innerFrame_split | containerWriter_allKeyFrame_split;
	sprintf(file, "%s/tools/data/source/test.h264b.aac.ts", getenv("HOME"));
//	sprintf(file, "%s/tools/data/c_out/test.h264b.aac.ts", getenv("HOME"));
//	sprintf(file, "%s/tools/data/test.h264b.aac.phpOutput.ts", getenv("HOME"));
	testData.fp_in = fopen(file, "rb");
	sprintf(file, "%s/tools/data/c_out/test.h264b.aac.ts", getenv("HOME"));
	testData.fp_out = fopen(file, "wb");
//	testData.fp_out = NULL;
	do {
		uint8_t buffer[65536];
		if(!testData.fp_in) {
			break;
		}
		size_t read_size = fread(buffer, 1, 65536, testData.fp_in);
		if(!ttLibC_MpegtsReader_read((ttLibC_MpegtsReader *)testData.reader, buffer, read_size, mpegtsTest_getMpegtsCallback, &testData)) {
			ERR_PRINT("error occured!");
			break;
		}
	} while(!feof(testData.fp_in));
	ttLibC_ContainerReader_close(&testData.reader);
	ttLibC_ContainerWriter_close(&testData.writer);
	if(testData.fp_in)  {fclose(testData.fp_in); testData.fp_in  = NULL;}
	if(testData.fp_out) {fclose(testData.fp_out);testData.fp_out = NULL;}
	ASSERT(ttLibC_Allocator_dump() == 0);
	// */
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
	s.push_back(CUTE(mp4Test)); // h264/aac
	s.push_back(CUTE(webmTest)); // vp8/opus
	s.push_back(CUTE(mkvTest)); // h264/aac
	s.push_back(CUTE(mp3Test)); // none/mp3
	s.push_back(CUTE(mpegtsTest)); // h264/aac
	s.push_back(CUTE(flvTest)); // h264/aac

	s.push_back(CUTE(mp4CodecTest));
	s.push_back(CUTE(mkvCodecTest));
	s.push_back(CUTE(mpegtsCodecTest));
	return s;
}

