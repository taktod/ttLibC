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
#include <ttLibC/frame/audio/pcmf32.h>
#include <ttLibC/util/beepUtil.h>
#include <ttLibC/util/ioUtil.h>
#include <ttLibC/resampler/audioResampler.h>

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
#include <ttLibC/util/stlMapUtil.h>

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

#ifdef __ENABLE_APPLE__
#	include <ttLibC/util/audioUnitUtil.h>
#endif

#ifdef __ENABLE_MP3LAME_ENCODE__
#	include <ttLibC/encoder/mp3lameEncoder.h>
#endif

#ifdef __ENABLE_SOUNDTOUCH__
#	include <soundtouch/SoundTouch.h>
#	include <ttLibC/resampler/soundtouchResampler.h>
#endif

#ifdef __ENABLE_SPEEXDSP__
#	include <ttLibC/resampler/speexdspResampler.h>
#endif

#if defined(__ENABLE_SPEEXDSP__) && defined(__ENABLE_APPLE__)
#endif

static void speexdspPlayTest() {
	LOG_PRINT("speexdspResampleTest");
#if defined(__ENABLE_SPEEXDSP__) && defined(__ENABLE_APPLE__)
	uint32_t sampleRate = 44100;
	uint32_t outputSampleRate = 48000;
	uint32_t channelNum = 2;
	ttLibC_BeepGenerator *generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian_planar, 440, sampleRate, channelNum);
	ttLibC_AuPlayer *auPlayer = ttLibC_AuPlayer_make(outputSampleRate, channelNum, AuPlayerType_DefaultOutput);
	ttLibC_SpeexdspResampler *resampler = ttLibC_SpeexdspResampler_make(channelNum, sampleRate, outputSampleRate, 8);
	ttLibC_PcmS16 *pcm = NULL, *p, *rpcm = NULL, *ipcm = NULL;
	for(int i = 0;i < 10;++ i) {
		// generator pcm
		p = ttLibC_BeepGenerator_makeBeepByMiliSec(generator, pcm, 500);
		if(p == NULL) {
			break;
		}
		// resample sampleRate
		pcm = p;
		if(sampleRate != outputSampleRate) {
			p = ttLibC_SpeexdspResampler_resample(resampler, rpcm, pcm);
			if(p == NULL) {
				break;
			}
			rpcm = p;
		}
		else {
			rpcm = pcm;
		}
		// change pcm type to littleEndian interleave
		if(rpcm->type != PcmS16Type_littleEndian) {
			p = (ttLibC_PcmS16 *)ttLibC_AudioResampler_convertFormat((ttLibC_Audio *)ipcm, frameType_pcmS16, PcmS16Type_littleEndian, channelNum, (ttLibC_Audio *)rpcm);
			if(p == NULL) {
				break;
			}
			ipcm = p;
		}
		else {
			ipcm = rpcm;
		}
		// play with audioUnit.
		while(!ttLibC_AuPlayer_queue(auPlayer, (ttLibC_PcmS16 *)ipcm)) {
			usleep(100);
		}
	}
	ttLibC_AuPlayer_close(&auPlayer);
	if(ipcm != rpcm) {
		ttLibC_PcmS16_close(&ipcm);
	}
	if(rpcm != pcm) {
		ttLibC_PcmS16_close(&rpcm);
 	}
	ttLibC_PcmS16_close(&pcm);
	ttLibC_SpeexdspResampler_close(&resampler);
	ttLibC_BeepGenerator_close(&generator);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

#if defined(__ENABLE_SOUNDTOUCH__) && defined(__ENABLE_APPLE__)

static bool soundtouchPlayTestCallback(void *ptr, ttLibC_Audio *pcm) {
	if(pcm->inherit_super.type != frameType_pcmS16) {
		return true;
	}
	ttLibC_AuPlayer *auPlayer = (ttLibC_AuPlayer *)ptr;
	while(!ttLibC_AuPlayer_queue(auPlayer, (ttLibC_PcmS16 *)pcm)) {
		usleep(100); // ...
	}// */
	return true;
}

#endif

static void soundtouchPlayTest() {
	LOG_PRINT("soundtouchPlayTest");
#if defined(__ENABLE_SOUNDTOUCH__) && defined(__ENABLE_APPLE__)
	ttLibC_BeepGenerator *generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian, 440, 44100, 2);
	ttLibC_AuPlayer *auPlayer = ttLibC_AuPlayer_make(44100, 2, AuPlayerType_DefaultOutput);
	ttLibC_Soundtouch *soundtouch = ttLibC_Soundtouch_make(44100, 2);
	ttLibC_Soundtouch_setPitchSemiTones(soundtouch, 1.5);
	ttLibC_PcmS16 *pcm = NULL;
	for(int i = 0;i < 10;i ++) {
		ttLibC_PcmS16 *p = ttLibC_BeepGenerator_makeBeepByMiliSec(generator, pcm, 1000);
		if(p == NULL) {
			break;
		}
		pcm = p;
		ttLibC_Soundtouch_resample(soundtouch, (ttLibC_Audio *)pcm, soundtouchPlayTestCallback, auPlayer);
	}
	ttLibC_Soundtouch_resample(soundtouch, NULL, soundtouchPlayTestCallback, auPlayer);
	ttLibC_PcmS16_close(&pcm);
	ttLibC_Soundtouch_close(&soundtouch);
	ttLibC_AuPlayer_close(&auPlayer);
	ttLibC_BeepGenerator_close(&generator);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

static void soundtouchPlayTest_test() {
	LOG_PRINT("soundtouchPlayTest");
#if defined(__ENABLE_APPLE__) && defined(__ENABLE_SOUNDTOUCH__)
/*	ttLibC_BeepGenerator *generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian, 440, 44100, 2);
	ttLibC_AuPlayer *auPlayer = ttLibC_AuPlayer_make(44100, 2, AuPlayerType_DefaultOutput);
	ttLibC_PcmS16 *pcm = NULL;
	soundtouch::SoundTouch *st = new soundtouch::SoundTouch();
	st->setSampleRate(44100);
	st->setChannels(2);
	st->setPitchSemiTones(1.5);
	ttLibC_PcmF32 *fpcm = NULL;
	ttLibC_PcmF32 *ffpcm = NULL;
	ttLibC_PcmS16 *ppcm = NULL;
	for(int i = 0;i < 10;i ++) {
		ttLibC_PcmS16 *p = ttLibC_BeepGenerator_makeBeepByMiliSec(generator, pcm, 1000);
		if(p == NULL) {
			break;
		}
		pcm = p;
		ttLibC_PcmF32 *f = ttLibC_AudioResampler_makePcmF32FromPcmS16(fpcm, PcmF32Type_interleave, pcm);
		if(f == NULL) {
			break;
		}
		fpcm = f;
		st->putSamples((const float *)fpcm->l_data, fpcm->inherit_super.sample_num);

		int nSamples = 0;
		do {
			float buf[1024];
			nSamples = st->receiveSamples(buf, 1024 / 2);
			if(nSamples == 0) {
				break;
			}
			f = ttLibC_PcmF32_make(ffpcm, PcmF32Type_interleave, 44100, nSamples, 2, buf, nSamples * 4 * 2, buf, nSamples, NULL, 0, true, 0, 1000);
			if(f == NULL) {
				break;
			}
			ffpcm = f;
			p = ttLibC_AudioResampler_makePcmS16FromPcmF32(ppcm, PcmS16Type_littleEndian, ffpcm);
			if(p == NULL) {
				break;
			}
			ppcm = p;
			while(!ttLibC_AuPlayer_queue(auPlayer, ppcm)) {
				usleep(100); // ...
			}// * /
		} while(nSamples != 0);
	}
	delete st;
	ttLibC_PcmS16_close(&pcm);
	ttLibC_PcmF32_close(&fpcm);
	ttLibC_PcmS16_close(&ppcm);
	ttLibC_PcmF32_close(&ffpcm);
	ttLibC_AuPlayer_close(&auPlayer);
	ttLibC_BeepGenerator_close(&generator);*/
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}


#if defined(__ENABLE_APPLE__) && defined(__ENABLE_MP3LAME_ENCODE__)
typedef struct audioUnitRecordTest_t {
	ttLibC_AuRecorder *recorder;
	ttLibC_Mp3lameEncoder *encoder;
	ttLibC_PcmS16 *resampled_pcm;
	FILE *fp;
	ttLibC_StlList *frame_list;
	ttLibC_StlList *used_frame_list;
} audioUnitRecordTest_t;

static bool audioUnitRecordTest_mp3EncodeCallback(void *ptr, ttLibC_Mp3 *mp3) {
	audioUnitRecordTest_t *testData = (audioUnitRecordTest_t *)ptr;
	if(testData->fp != NULL) {
		fwrite(
				mp3->inherit_super.inherit_super.data,
				1,
				mp3->inherit_super.inherit_super.data_size,
				testData->fp);
	}
	return true;
}

static bool audioUnitRecordTest_makePcmCallback(void *ptr, ttLibC_Audio *audio) {
	audioUnitRecordTest_t *testData = (audioUnitRecordTest_t *)ptr;
	if(audio->inherit_super.type != frameType_pcmS16) {
		return false;
	}
	ttLibC_Frame *prev_frame = NULL;
	if(testData->used_frame_list->size > 5) {
		prev_frame = (ttLibC_Frame *)ttLibC_StlList_refFirst(testData->used_frame_list);
		if(prev_frame != NULL) {
			ttLibC_StlList_remove(testData->used_frame_list, prev_frame);
		}
	}
	ttLibC_Frame *cloned_frame = ttLibC_Frame_clone(
			prev_frame,
			(ttLibC_Frame *)audio);
	if(cloned_frame == NULL) {
		return false;
	}
	ttLibC_StlList_addLast(testData->frame_list, cloned_frame);
	return true;
}

static bool audioUnitRecordTest_closeAllFrame(void *ptr, void *item) {
	if(item != NULL) {
		ttLibC_Frame_close((ttLibC_Frame **)&item);
	}
	return true;
}
#endif

static void audioUnitRecordTest() {
	LOG_PRINT("audioUnitRecordTest");
#if defined(__ENABLE_APPLE__) && defined(__ENABLE_MP3LAME_ENCODE__)
	audioUnitRecordTest_t testData;
	uint32_t sample_rate = 44100;
	uint32_t channel_num = 2;
	testData.fp = fopen("output.mp3", "wb");
	testData.resampled_pcm = NULL;
	testData.recorder = ttLibC_AuRecorder_make(
			sample_rate,
			channel_num,
			AuRecorderType_DefaultInput,
			0);
	testData.encoder = ttLibC_Mp3lameEncoder_make(sample_rate, channel_num, 2);
	testData.frame_list = ttLibC_StlList_make();
	testData.used_frame_list = ttLibC_StlList_make();
	ttLibC_AuRecorder_start(testData.recorder, audioUnitRecordTest_makePcmCallback, &testData);

	for(int i = 0;i < 400;i ++) {
		usleep(20000); // wait a little for recording.
		if(!testData.recorder->is_recording) {
			break;
		}
		// get frame from list.
		ttLibC_Frame *frame = NULL;
		while((frame = (ttLibC_Frame *)ttLibC_StlList_refFirst(testData.frame_list)) != NULL) {
			// for example encode to mp3 and save.
			ttLibC_Mp3lameEncoder_encode(
					testData.encoder,
					(ttLibC_PcmS16 *)frame,
					audioUnitRecordTest_mp3EncodeCallback,
					&testData);
			// used frame is moved to used_frame_list.
			ttLibC_StlList_remove(testData.frame_list, frame);
			ttLibC_StlList_addLast(testData.used_frame_list, frame);
		}
	}
	ttLibC_AuRecorder_stop(testData.recorder);
	// once freeze here. why? need sleep for recorder thread?
	ttLibC_Mp3lameEncoder_close(&testData.encoder);
	ttLibC_StlList_forEach(testData.frame_list, audioUnitRecordTest_closeAllFrame, NULL);
	ttLibC_StlList_close(&testData.frame_list);
	ttLibC_StlList_forEach(testData.used_frame_list, audioUnitRecordTest_closeAllFrame, NULL);
	ttLibC_StlList_close(&testData.used_frame_list);
	ttLibC_AuRecorder_close(&testData.recorder);
	ttLibC_PcmS16_close(&testData.resampled_pcm);
	if(testData.fp) {
		fclose(testData.fp);
	}
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

static void audioUnitPlayTest() {
	LOG_PRINT("audioUnitPlayTest");
#ifdef __ENABLE_APPLE__
	ttLibC_BeepGenerator *generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian, 440, 44100, 2);
	ttLibC_AuPlayer *auPlayer = ttLibC_AuPlayer_make(44100, 2, AuPlayerType_DefaultOutput);
	ttLibC_PcmS16 *pcm = NULL;
	for(int i = 0;i < 10;i ++) {
		ttLibC_PcmS16 *p = ttLibC_BeepGenerator_makeBeepByMiliSec(generator, pcm, 1000);
		if(p == NULL) {
			break;
		}
		pcm = p;
		while(!ttLibC_AuPlayer_queue(auPlayer, pcm)) {
 			usleep(100); // ...
		}
	}
	ttLibC_PcmS16_close(&pcm);
	ttLibC_AuPlayer_close(&auPlayer);
	ttLibC_BeepGenerator_close(&generator);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}


static bool stlMapTest_findItem(void *ptr, void *key, void *item) {
	LOG_PRINT("key:%s val:%s", (const char *)key, (const char *)item);
	return true;
}

static bool stlMapTest_findItem2(void *ptr, void *key, void *item) {
	LOG_PRINT("key:%s val:%s", (const char *)key, (const char *)item);
	if(strcmp((const char *)key, "hogehoge_k") == 0) {
		ttLibC_StlMap *map = (ttLibC_StlMap *)ptr;
		ttLibC_StlMap_remove(map, key);
	}
	return true;
}

static void stlMapTest() {
	LOG_PRINT("stlMapTest");
	ttLibC_StlMap *map = ttLibC_StlMap_make();
	char *a_k = "hogehoge_k";
	char *a_i = "hogehoge";
	char *b_k = "hello_k";
	char *b_i = "hello";
	ttLibC_StlMap_put(map, a_k, a_i);
	ttLibC_StlMap_put(map, b_k, b_i);
	ttLibC_StlMap_forEach(map, stlMapTest_findItem, map);
	ttLibC_StlMap_forEach(map, stlMapTest_findItem2, map);
	ttLibC_StlMap_forEach(map, stlMapTest_findItem, map);
	ttLibC_StlMap_close(&map);
	ASSERT(ttLibC_Allocator_dump() == 0);
}

static bool stlListTest_findItem(void *ptr, void *item) {
	LOG_PRINT("fi:%d", *((int *)item));
	return true;
}

static bool stlListTest_findItem2(void *ptr, void *item) {
	ttLibC_StlList *list = (ttLibC_StlList *)ptr;
	LOG_PRINT("fi2:%d", *((int *)item));
	if(*((int *)item) == 5) {
		LOG_PRINT("find delete target:%d", *((int *)item));
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
	LOG_PRINT("value:%d", *((int *)item));
	if(*(int *)item == 5) {
		LOG_PRINT("find delete target.");
		ttLibC_LinkedList *linkedList = (ttLibC_LinkedList *)ptr;
		ttLibC_LinkedList_remove(linkedList, item);
	}
	return true;
}

static bool findItem2(void *ptr, void *item, size_t item_size) {
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

static void byteUtilH26XTest() {
	LOG_PRINT("byteUtilH26XTest");
	uint8_t buffer[256];
	uint32_t size = ttLibC_HexUtil_makeBuffer("0001E012345678", buffer, 256);
	ttLibC_ByteReader *reader = ttLibC_ByteReader_make(buffer, size, ByteUtilType_h26x);
	LOG_PRINT("%x", ttLibC_ByteReader_bit(reader, 16));
	LOG_PRINT("%x", ttLibC_ByteReader_bit(reader, 1));
	LOG_PRINT("%x", ttLibC_ByteReader_expGolomb(reader, false));
	LOG_PRINT("%x", ttLibC_ByteReader_expGolomb(reader, false));
	ttLibC_ByteReader_close(&reader);
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
	// add data.
	ttLibC_DynamicBuffer_append(buffer, data, size);
	// check data.
	LOG_DUMP(ttLibC_DynamicBuffer_refData(buffer),
			ttLibC_DynamicBuffer_refSize(buffer), true);
	LOG_PRINT("bs:%llu, ts:%llu", buffer->buffer_size, buffer->target_size);
	// set 5 byte is read.
	ttLibC_DynamicBuffer_markAsRead(buffer, 5);
	// add more data.
	size = ttLibC_HexUtil_makeBuffer("AABBCCDD", data, 256);
	ttLibC_DynamicBuffer_append(buffer, data, size);
	LOG_PRINT("bs:%llu, ts:%llu", buffer->buffer_size, buffer->target_size);
	// check data.
	LOG_DUMP(ttLibC_DynamicBuffer_refData(buffer),
			ttLibC_DynamicBuffer_refSize(buffer), true);
	// clear read data.
	ttLibC_DynamicBuffer_reset(buffer);
	// check data again.
	LOG_DUMP(ttLibC_DynamicBuffer_refData(buffer),
			ttLibC_DynamicBuffer_refSize(buffer), true);
	/// set 4 byte is read.
	ttLibC_DynamicBuffer_markAsRead(buffer, 4);
	// check data.
	LOG_DUMP(ttLibC_DynamicBuffer_refData(buffer),
			ttLibC_DynamicBuffer_refSize(buffer), true);
	// clear
	ttLibC_DynamicBuffer_clear(buffer);
	// check data.
	LOG_DUMP(ttLibC_DynamicBuffer_refData(buffer),
			ttLibC_DynamicBuffer_refSize(buffer), true);
	size = ttLibC_HexUtil_makeBuffer("EEFF", data, 256);
	ttLibC_DynamicBuffer_append(buffer, data, size);
	LOG_PRINT("bs:%llu, ts:%llu", buffer->buffer_size, buffer->target_size);
	LOG_DUMP(ttLibC_DynamicBuffer_refData(buffer),
			ttLibC_DynamicBuffer_refSize(buffer), true);
	ttLibC_DynamicBuffer_close(&buffer);

	LOG_PRINT("test no.2");
	buffer = ttLibC_DynamicBuffer_make();
	ttLibC_DynamicBuffer_alloc(buffer, 11);
	uint8_t *dd = ttLibC_DynamicBuffer_refData(buffer);
	dd[0] = 0x09;
	dd[1] = 0x01;
	size_t dd_size = ttLibC_DynamicBuffer_refSize(buffer);
	LOG_DUMP(ttLibC_DynamicBuffer_refData(buffer),
			ttLibC_DynamicBuffer_refSize(buffer), true);
	data[0] = 0;
	data[1] = 1;
	data[2] = 2;
	data[3] = 3;
	data[4] = 4;
	data[5] = 5;
	ttLibC_DynamicBuffer_append(buffer, data, 6);
	LOG_DUMP(ttLibC_DynamicBuffer_refData(buffer),
			ttLibC_DynamicBuffer_refSize(buffer), true);
	ttLibC_DynamicBuffer_alloc(buffer, 11);
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
	uint32_t size = ttLibC_HexUtil_makeBuffer("02 B0 1D 00 01 C1 00 00 E1 00 F0 00 1B E1 00 F0 00 0F E1 01 F0 06 0A 04 75 6E 64 00", buf, sizeof(buf));
	uint8_t *data = buf;
	for(int i = 0;i < size;++ i, ++data) {
		ttLibC_Crc32_update(crc32, *data);
	}
	LOG_PRINT("result:%llx", ttLibC_Crc32_getValue(crc32));
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
	ttLibC_CvWindow *window = ttLibC_CvWindow_make("original");
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
	s.push_back(CUTE(speexdspPlayTest));
	s.push_back(CUTE(soundtouchPlayTest));
	s.push_back(CUTE(audioUnitRecordTest));
	s.push_back(CUTE(audioUnitPlayTest));
	s.push_back(CUTE(stlMapTest));
	s.push_back(CUTE(stlListTest));
	s.push_back(CUTE(linkedListTest));
	s.push_back(CUTE(byteUtilTest));
	s.push_back(CUTE(byteUtilH26XTest));
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
