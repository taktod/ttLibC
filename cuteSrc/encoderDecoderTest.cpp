/**
 * @file   encoderDecoderTest.cpp
 * @brief  encoder, decoder, and resampler test code.
 *
 * this code is under GPLv3 license.
 *
 * @author taktod
 * @date   2015/07/18
 */

#include <cute.h>

#include <ttLibC/log.h>
#include <ttLibC/allocator.h>

#ifdef __ENABLE_OPENCV__
#	include <ttLibC/util/opencvUtil.h>
#endif

#ifdef __ENABLE_OPENAL__
#	include <ttLibC/util/openalUtil.h>
#endif

#ifdef __ENABLE_MP3LAME_ENCODE__
#	include <ttLibC/encoder/mp3lameEncoder.h>
#endif

#ifdef __ENABLE_MP3LAME_DECODE__
#	include <ttLibC/decoder/mp3lameDecoder.h>
#endif

#ifdef __ENABLE_FAAC_ENCODE__
#	include <ttLibC/encoder/faacEncoder.h>
#endif

#ifdef __ENABLE_OPENH264__
#	include <ttLibC/encoder/openh264Encoder.h>
#	include <ttLibC/decoder/openh264Decoder.h>
#endif

#ifdef __ENABLE_SPEEXDSP__
#	include <ttLibC/resampler/speexdspResampler.h>
#endif

#ifdef __ENABLE_SPEEX__
#	include <ttLibC/encoder/speexEncoder.h>
#	include <ttLibC/decoder/speexDecoder.h>
#endif

#ifdef __ENABLE_OPUS__
#	include <ttLibC/encoder/opusEncoder.h>
#	include <ttLibC/decoder/opusDecoder.h>
#endif

#ifdef __ENABLE_JPEG__
#	include <ttLibC/encoder/jpegEncoder.h>
#	include <ttLibC/decoder/jpegDecoder.h>
#endif

#ifdef __ENABLE_X264__
#	include <ttLibC/encoder/x264Encoder.h>
#endif

#include <ttLibC/util/beepUtil.h>
#include <ttLibC/frame/audio/pcms16.h>
#include <ttLibC/frame/audio/mp3.h>
#include <ttLibC/frame/audio/aac.h>
#include <ttLibC/frame/audio/speex.h>
#include <ttLibC/frame/audio/opus.h>

#include <ttLibC/frame/video/bgr.h>
#include <ttLibC/frame/video/yuv420.h>
#include <ttLibC/frame/video/h264.h>
#include <ttLibC/frame/video/jpeg.h>

#include <ttLibC/resampler/imageResampler.h>
#include <ttLibC/resampler/audioResampler.h>

#include <ttLibC/util/hexUtil.h>


#if defined(__ENABLE_X264__) && defined(__ENABLE_OPENCV__) && defined(__ENABLE_OPENH264__)
typedef struct x264TestData{
	ttLibC_Openh264Decoder *decoder;
	ttLibC_CvWindow *dec_win;
	ttLibC_Bgr *dbgr;
} x264TestData;

bool x264Test_decodeCallback(void *ptr, ttLibC_Yuv420 *yuv420) {
	x264TestData *testData = (x264TestData *)ptr;
	ttLibC_Bgr *b = ttLibC_ImageResampler_makeBgrFromYuv420(testData->dbgr, BgrType_bgr, yuv420);
	if(b == NULL) {
		return false;
	}
	testData->dbgr = b;
	ttLibC_CvWindow_showBgr(testData->dec_win, testData->dbgr);
	return true;
}

bool x264Test_encodeCallback(void *ptr, ttLibC_H264 *h264) {
	x264TestData *testData = (x264TestData *)ptr;
	if(h264->type == H264Type_unknown) {
		return true;
	}
	ttLibC_Openh264Decoder_decode(testData->decoder, h264, x264Test_decodeCallback, ptr);
	return true;
}
#endif

static void x264Test() {
	LOG_PRINT("x264Test");
#if defined(__ENABLE_X264__) && defined(__ENABLE_OPENCV__) && defined(__ENABLE_OPENH264__)
	// x264 does not have decoder.
	// encode with x264 and decode with openh264.
	uint32_t width = 320, height = 240;
	ttLibC_CvCapture *capture = ttLibC_CvCapture_make(0, width, height);
	ttLibC_CvWindow *window = ttLibC_CvWindow_make("original");
	ttLibC_CvWindow *dec_win = ttLibC_CvWindow_make("decode");
	ttLibC_X264Encoder *encoder = ttLibC_X264Encoder_make(width, height);
	ttLibC_Openh264Decoder *decoder = ttLibC_Openh264Decoder_make();
	ttLibC_Bgr *bgr = NULL, *dbgr = NULL, *b;
	ttLibC_Yuv420 *yuv = NULL, *y;
	while(true) {
		b = ttLibC_CvCapture_queryFrame(capture, bgr);
		if(b == NULL) {
			break;
		}
		bgr = b;
		ttLibC_CvWindow_showBgr(window, bgr);
		y = ttLibC_ImageResampler_makeYuv420FromBgr(yuv, Yuv420Type_planar, bgr);
		if(y == NULL) {
			break;
		}
		yuv = y;
		x264TestData testData;
		testData.dbgr = dbgr;
		testData.dec_win = dec_win;
		testData.decoder = decoder;
		ttLibC_X264Encoder_encode(encoder, yuv, x264Test_encodeCallback, &testData);
		dbgr = testData.dbgr;
		uint8_t key = ttLibC_CvWindow_waitForKeyInput(10);
		if(key == Keychar_Esc) {
			break;
		}
	}
	ttLibC_Bgr_close(&bgr);
	ttLibC_Bgr_close(&dbgr);
	ttLibC_Yuv420_close(&yuv);
	ttLibC_Openh264Decoder_close(&decoder);
	ttLibC_X264Encoder_close(&encoder);
	ttLibC_CvWindow_close(&dec_win);
	ttLibC_CvWindow_close(&window);
	ttLibC_CvCapture_close(&capture);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

#if defined(__ENABLE_JPEG__) && defined(__ENABLE_OPENCV__)
typedef struct {
	ttLibC_JpegDecoder *decoder;
	ttLibC_CvWindow *dec_win;
	ttLibC_Bgr *dbgr;
} jpegTestData;

static bool jpegDecoderTestCallback(void *ptr, ttLibC_Yuv420 *yuv) {
	jpegTestData *testData = (jpegTestData *)ptr;
	ttLibC_Bgr *b = ttLibC_ImageResampler_makeBgrFromYuv420(testData->dbgr, BgrType_bgr, yuv);
	if(b == NULL) {
		return false;
	}
	testData->dbgr = b;
	ttLibC_CvWindow_showBgr(testData->dec_win, testData->dbgr);
	return true;
}

static bool jpegEncoderTestCallback(void *ptr, ttLibC_Jpeg *jpeg) {
	jpegTestData *testData = (jpegTestData *)ptr;
	ttLibC_JpegDecoder_decode(testData->decoder, jpeg, jpegDecoderTestCallback, ptr);
	return true;
}
#endif

static void jpegTest() {
	LOG_PRINT("jpegTest");
#if defined(__ENABLE_JPEG__) && defined(__ENABLE_OPENCV__)
	uint32_t width = 320, height = 180;
	ttLibC_CvCapture   *capture = ttLibC_CvCapture_make(0, width, height);
	ttLibC_CvWindow    *window  = ttLibC_CvWindow_make("original");
	ttLibC_JpegEncoder *encoder = ttLibC_JpegEncoder_make(width, height, 80);
	ttLibC_Bgr    *bgr = NULL, *b;
	ttLibC_Yuv420 *yuv = NULL, *y;
	jpegTestData testData;
	testData.dbgr = NULL;
	testData.dec_win = ttLibC_CvWindow_make("decoded");
	testData.decoder = ttLibC_JpegDecoder_make();
	while(true) {
		b = ttLibC_CvCapture_queryFrame(capture, bgr);
		if(b == NULL) {
			break;
		}
		bgr = b;
		ttLibC_CvWindow_showBgr(window, bgr);
		y = ttLibC_ImageResampler_makeYuv420FromBgr(yuv, Yuv420Type_planar, bgr);
		if(y == NULL) {
			break;
		}
		yuv = y;
		ttLibC_JpegEncoder_encode(encoder, yuv, jpegEncoderTestCallback, &testData);
		// now try to make jpeg.
		uint8_t key = ttLibC_CvWindow_waitForKeyInput(10);
		if(key == Keychar_Esc) {
			break;
		}
	}
	ttLibC_Bgr_close(&testData.dbgr);
	ttLibC_Bgr_close(&bgr);
	ttLibC_Yuv420_close(&yuv);
	ttLibC_JpegDecoder_close(&testData.decoder);
	ttLibC_JpegEncoder_close(&encoder);
	ttLibC_CvWindow_close(&window);
	ttLibC_CvWindow_close(&testData.dec_win);
	ttLibC_CvCapture_close(&capture);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

#if defined(__ENABLE_OPUS__) && defined(__ENABLE_OPENAL__)
typedef struct {
	ttLibC_AlDevice *device;
	ttLibC_OpusDecoder *decoder;
} opusTest_t;

bool opusDecoderCallback(void *ptr, ttLibC_PcmS16 *pcms16) {
	opusTest_t *testData = (opusTest_t *)ptr;
	ttLibC_AlDevice_queue(testData->device, pcms16);
	return true;
}

bool opusEncoderCallback(void *ptr, ttLibC_Opus *opus) {
//	LOG_PRINT("encoded.:%d", opus->inherit_super.inherit_super.pts);
	opusTest_t *testData = (opusTest_t *)ptr;
	ttLibC_OpusDecoder_decode(testData->decoder, opus, opusDecoderCallback, ptr);
	return true;
}
#endif

static void opusTest() {
	LOG_PRINT("opusTest");
#if defined(__ENABLE_OPUS__) && defined(__ENABLE_OPENAL__)
	uint32_t sample_rate = 48000;
	uint32_t channel_num = 2;
	opusTest_t testData;
	ttLibC_BeepGenerator *generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian, 440, sample_rate, channel_num);
	testData.device = ttLibC_AlDevice_make(256);
	testData.decoder = ttLibC_OpusDecoder_make(sample_rate, channel_num);
	ttLibC_OpusEncoder *encoder = ttLibC_OpusEncoder_make(sample_rate, channel_num, 480);
	ttLibC_PcmS16 *pcm = NULL, *p;
	for(int i = 0;i < 10;++ i) {
		p = ttLibC_BeepGenerator_makeBeepBySampleNum(generator, pcm, 4900);
		if(p == NULL) {
			break;
		}
		pcm = p;
		ttLibC_OpusEncoder_encode(encoder, pcm, opusEncoderCallback, &testData);
	}
	ttLibC_AlDevice_proceed(testData.device, -1);
	ttLibC_OpusDecoder_close(&testData.decoder);
	ttLibC_OpusEncoder_close(&encoder);
	ttLibC_AlDevice_close(&testData.device);
	ttLibC_PcmS16_close(&pcm);
	ttLibC_BeepGenerator_close(&generator);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

#if defined(__ENABLE_SPEEX__) && defined(__ENABLE_OPENAL__)
typedef struct {
	ttLibC_AlDevice *device;
	ttLibC_SpeexDecoder *decoder;
} speexTest_t;

bool speexDecoderCallback(void *ptr, ttLibC_PcmS16 *pcms16) {
//	LOG_PRINT("decoded.");
	speexTest_t *testData = (speexTest_t *)ptr;
	// play with openal.
	ttLibC_AlDevice_queue(testData->device, pcms16);
	return true;
}

bool speexEncoderCallback(void *ptr, ttLibC_Speex *speex) {
	speexTest_t *testData = (speexTest_t *)ptr;
	ttLibC_SpeexDecoder_decode(testData->decoder, speex, speexDecoderCallback, ptr);
	return true;
}
#endif

static void speexTest() {
	LOG_PRINT("speexTest");
#if defined(__ENABLE_SPEEX__) && defined(__ENABLE_OPENAL__)
	uint32_t sample_rate = 32000;
	uint32_t channel_num = 1;
	speexTest_t testData;
	ttLibC_BeepGenerator *generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian, 440, sample_rate, channel_num);
	testData.device = ttLibC_AlDevice_make(256);
	testData.decoder = ttLibC_SpeexDecoder_make(sample_rate, channel_num);
	ttLibC_SpeexEncoder *encoder = ttLibC_SpeexEncoder_make(sample_rate, channel_num, 10);
	ttLibC_PcmS16 *pcm = NULL, *p;
	for(int i = 0;i < 10;++ i) {
		p = ttLibC_BeepGenerator_makeBeepByMiliSec(generator, pcm, 510);
		if(p == NULL) {
			break;
		}
		pcm = p;
		ttLibC_SpeexEncoder_encode(encoder, pcm, speexEncoderCallback, &testData);
//		ttLibC_AlDevice_queue(testData.device, pcm);
//		break;
	}
	ttLibC_AlDevice_proceed(testData.device, -1);
	ttLibC_SpeexDecoder_close(&testData.decoder);
	ttLibC_SpeexEncoder_close(&encoder);
	ttLibC_AlDevice_close(&testData.device);
	ttLibC_PcmS16_close(&pcm);
	ttLibC_BeepGenerator_close(&generator);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

static void speexdspResamplerTest() {
	LOG_PRINT("speexdspResamplerTest");
#if defined(__ENABLE_SPEEXDSP__) && defined(__ENABLE_OPENAL__)
	uint32_t channel = 2;
	uint32_t in_sample_rate = 44100;
	uint32_t out_sample_rate = 48000;
	ttLibC_BeepGenerator *generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian, 440, in_sample_rate, channel);
	ttLibC_AlDevice *device = ttLibC_AlDevice_make(10);
	ttLibC_SpeexdspResampler *resampler = ttLibC_SpeexdspResampler_make(channel, in_sample_rate, out_sample_rate, 5);

	ttLibC_PcmS16 *pcm = NULL, *resampled_pcm = NULL, *p;
	for(int i = 0;i < 5;i ++) {
		p = ttLibC_BeepGenerator_makeBeepByMiliSec(generator, pcm, 500);
		if(p == NULL) {
			break;
		}
		pcm = p;
		p = ttLibC_SpeexdspResampler_resample(resampler, resampled_pcm, pcm);
		if(p == NULL) {
			break;
		}
		resampled_pcm = p;
		ttLibC_AlDevice_queue(device, resampled_pcm);
	}
	ttLibC_AlDevice_proceed(device, -1);
	ttLibC_PcmS16_close(&pcm);
	ttLibC_PcmS16_close(&resampled_pcm);
	ttLibC_SpeexdspResampler_close(&resampler);
	ttLibC_AlDevice_close(&device);
	ttLibC_BeepGenerator_close(&generator);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

#if defined(__ENABLE_OPENH264__) && defined(__ENABLE_OPENCV__)

typedef struct openh264TestData{
	ttLibC_Openh264Decoder *decoder;
	ttLibC_CvWindow *dec_win;
	ttLibC_Bgr *dbgr;
} openh264TestData;

bool openh264DecoderTestCallback(void *ptr, ttLibC_Yuv420 *yuv420) {
	openh264TestData *testData = (openh264TestData *)ptr;
	ttLibC_Bgr *b = ttLibC_ImageResampler_makeBgrFromYuv420(testData->dbgr, BgrType_bgr, yuv420);
	if(b == NULL) {
		return false;
	}
	testData->dbgr = b;
	ttLibC_CvWindow_showBgr(testData->dec_win, testData->dbgr);
	return true;
}

bool openh264EncoderTestCallback(void *ptr, ttLibC_H264 *h264) {
	openh264TestData *testData = (openh264TestData *)ptr;
	if(h264->type == H264Type_unknown) {
		return true;
	}
	ttLibC_Openh264Decoder_decode(testData->decoder, h264, openh264DecoderTestCallback, testData);
	return true;
}
#endif

static void openh264Test() {
	LOG_PRINT("openh264Test");
#if defined(__ENABLE_OPENH264__) && defined(__ENABLE_OPENCV__)
	uint32_t width = 320, height = 240;
	ttLibC_CvCapture       *capture = ttLibC_CvCapture_make(0, width, height);
	ttLibC_CvWindow        *window  = ttLibC_CvWindow_make("original");
	ttLibC_CvWindow        *dec_win = ttLibC_CvWindow_make("decode");
	ttLibC_Openh264Encoder *encoder = ttLibC_Openh264Encoder_make(width, height);
	ttLibC_Openh264Decoder *decoder = ttLibC_Openh264Decoder_make();
	ttLibC_Bgr    *bgr = NULL, *dbgr = NULL, *b;
	ttLibC_Yuv420 *yuv = NULL, *y;
	int count = 0;
	while(true) {
		count ++;
		b = ttLibC_CvCapture_queryFrame(capture, bgr);
		if(b == NULL) {
			break;
		}
		bgr = b;
		ttLibC_CvWindow_showBgr(window, bgr);
		y = ttLibC_ImageResampler_makeYuv420FromBgr(yuv, Yuv420Type_planar, bgr);
		if(y == NULL) {
			break;
		}
		yuv = y;
		openh264TestData testData;
		testData.decoder = decoder;
		testData.dec_win = dec_win;

		testData.dbgr = dbgr;
		if(count == 20) {
			ttLibC_Openh264Encoder_forceNextKeyFrame(encoder);
		}
		ttLibC_Openh264Encoder_encode(encoder, yuv, openh264EncoderTestCallback, &testData);
		dbgr = testData.dbgr;
		uint8_t key = ttLibC_CvWindow_waitForKeyInput(10);
		if(key == Keychar_Esc) {
			break;
		}
	}
	ttLibC_Bgr_close(&bgr);
	ttLibC_Bgr_close(&dbgr);
	ttLibC_Yuv420_close(&yuv);
	ttLibC_Openh264Decoder_close(&decoder);
	ttLibC_Openh264Encoder_close(&encoder);
	ttLibC_CvWindow_close(&dec_win);
	ttLibC_CvWindow_close(&window);
	ttLibC_CvCapture_close(&capture);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

bool faacEncoderTestCallback(void *ptr, ttLibC_Aac *aac) {
	FILE* fp = (FILE *)ptr;
	LOG_PRINT("encoded. pts:%llu size:%lu", aac->inherit_super.inherit_super.pts, aac->inherit_super.inherit_super.buffer_size);
	if(fp) {
		fwrite(aac->inherit_super.inherit_super.data, 1, aac->inherit_super.inherit_super.buffer_size, fp);
	}
	return true;
}

static void faacEncoderTest() {
	LOG_PRINT("faacEncoderTest");
#ifdef __ENABLE_FAAC_ENCODE__
	uint32_t sample_rate = 44100;
	uint32_t channel_num = 2;
	ttLibC_FaacEncoder *encoder = ttLibC_FaacEncoder_make(FaacEncoderType_Main, sample_rate, channel_num, 96000);
	ttLibC_BeepGenerator *generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian, 440, sample_rate, channel_num);
	generator->amplitude = 32000;
	ttLibC_PcmS16 *pcm = NULL, *p;
	FILE *fp = fopen("output.aac", "wb");

	for(int i = 0;i < 5; ++ i) {
		// make 1sec beep.
		p = ttLibC_BeepGenerator_makeBeepByMiliSec(generator, pcm, 1000);
		if(p == NULL) {
			break;
		}
		pcm = p;
		// encode data.
		ttLibC_FaacEncoder_encode(encoder, pcm, faacEncoderTestCallback, fp);
	}
	if(fp) {
		fclose(fp);
	}
	ttLibC_PcmS16_close(&pcm);
	ttLibC_BeepGenerator_close(&generator);
	ttLibC_FaacEncoder_close(&encoder);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

#if defined(__ENABLE_MP3LAME_ENCODE__) && defined(__ENABLE_MP3LAME_DECODE__) && (__ENABLE_OPENAL__)
typedef struct {
	ttLibC_Mp3lameDecoder *decoder;
	ttLibC_AlDevice *device;
} mp3lameTest_TestData_t;

bool mp3lameDecoderTestCallback(void *ptr, ttLibC_PcmS16 *pcm) {
	mp3lameTest_TestData_t *testData = (mp3lameTest_TestData_t *)ptr;
	ttLibC_AlDevice_queue(testData->device, pcm);
	return true;
}

bool mp3lameEncoderTestCallback(void *ptr, ttLibC_Mp3 *mp3) {
	LOG_PRINT("encoded. pts:%llu size:%lu", mp3->inherit_super.inherit_super.pts, mp3->inherit_super.inherit_super.buffer_size);
	LOG_PRINT("sample_num:%d", mp3->inherit_super.sample_num);
	mp3lameTest_TestData_t *testData = (mp3lameTest_TestData_t *)ptr;
	ttLibC_Mp3lameDecoder_decode(testData->decoder, mp3, mp3lameDecoderTestCallback, ptr);
	return true;
}
#endif

static void mp3lameTest() {
	LOG_PRINT("mp3lameTest");
#if defined(__ENABLE_MP3LAME_ENCODE__) && defined(__ENABLE_MP3LAME_DECODE__) && (__ENABLE_OPENAL__)
	ttLibC_Mp3lameEncoder *encoder = ttLibC_Mp3lameEncoder_make(22050, 1, 10);
	ttLibC_Mp3lameDecoder *decoder = ttLibC_Mp3lameDecoder_make();
	ttLibC_BeepGenerator *generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian_planar, 440, 22050, 1);
	ttLibC_AlDevice *device = ttLibC_AlDevice_make(255);
	ttLibC_PcmS16 *pcm = NULL, *p;

	for(int i = 0;i < 3; ++ i) {
		// make 1sec beep.
		p = ttLibC_BeepGenerator_makeBeepByMiliSec(generator, pcm, 1000);
		if(p == NULL) {
			break;
		}
		pcm = p;
		// encode data.
		mp3lameTest_TestData_t testData;
		testData.decoder = decoder;
		testData.device  = device;
		ttLibC_Mp3lameEncoder_encode(encoder, pcm, mp3lameEncoderTestCallback, &testData);
	}
	ttLibC_AlDevice_proceed(device, -1);
	ttLibC_AlDevice_close(&device);
	ttLibC_PcmS16_close(&pcm);
	ttLibC_BeepGenerator_close(&generator);
	ttLibC_Mp3lameDecoder_close(&decoder);
	ttLibC_Mp3lameEncoder_close(&encoder);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

/**
 * how to use imageResampler for bgr data.
 */
static void imageResamplerTest() {
	LOG_PRINT("imageResamplerTest");
#ifdef __ENABLE_OPENCV__
	ttLibC_CvCapture *capture = ttLibC_CvCapture_make(0, 320, 240);
	ttLibC_CvWindow *window = ttLibC_CvWindow_make("opencvTest");
	ttLibC_CvWindow *resampled_window = ttLibC_CvWindow_make("resampled");
	ttLibC_Bgr *bgr = NULL;
	ttLibC_Yuv420 *yuv420 = NULL;
	ttLibC_Bgr *resampled_bgr = NULL;
	while(true) {
		ttLibC_Bgr *b = ttLibC_CvCapture_queryFrame(capture, bgr);
		if(b == NULL) {
			break;
		}
		bgr = b;
		ttLibC_Yuv420 *yuv = ttLibC_ImageResampler_makeYuv420FromBgr(yuv420, Yuv420Type_planar, bgr);
		if(yuv == NULL) {
			break;
		}
		yuv420 = yuv;
		b = ttLibC_ImageResampler_makeBgrFromYuv420(resampled_bgr, BgrType_abgr, yuv420);
		if(b == NULL) {
			break;
		}
		resampled_bgr = b;
		ttLibC_CvWindow_showBgr(window, bgr);
		ttLibC_CvWindow_showBgr(resampled_window, resampled_bgr);
		uint8_t key = ttLibC_CvWindow_waitForKeyInput(10);
		if(key == Keychar_Esc) {
			break;
		}
	}
	ttLibC_Bgr_close(&resampled_bgr);
	ttLibC_Yuv420_close(&yuv420);
	ttLibC_Bgr_close(&bgr);
	ttLibC_CvWindow_close(&resampled_window);
	ttLibC_CvWindow_close(&window);
	ttLibC_CvCapture_close(&capture);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

/**
 * define all test for encoder and decoder package.
 * @param s cute::suite obj
 * @return cute::suite obj
 */
cute::suite encoderDecoderTests(cute::suite s) {
	s.clear();
	s.push_back(CUTE(x264Test));
	s.push_back(CUTE(jpegTest));
	s.push_back(CUTE(opusTest));
	s.push_back(CUTE(speexTest));
	s.push_back(CUTE(speexdspResamplerTest));
	s.push_back(CUTE(openh264Test));
	s.push_back(CUTE(faacEncoderTest));
	s.push_back(CUTE(mp3lameTest));
	s.push_back(CUTE(imageResamplerTest));
	return s;
}
