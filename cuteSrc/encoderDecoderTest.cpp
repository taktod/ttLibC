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

#ifdef __ENABLE_OPENCV__
#	include <ttLibC/util/opencvUtil.h>
#endif

#ifdef __ENABLE_OPENAL__
#	include <ttLibC/util/openalUtil.h>
#endif

#ifdef __ENABLE_MP3LAME_ENCODE__
#	include <ttLibC/encoder/mp3lameEncoder.h>
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

#include <ttLibC/util/beepUtil.h>
#include <ttLibC/frame/audio/pcms16.h>
#include <ttLibC/frame/audio/mp3.h>
#include <ttLibC/frame/audio/aac.h>

#include <ttLibC/frame/video/bgr.h>
#include <ttLibC/frame/video/yuv420.h>
#include <ttLibC/frame/video/h264.h>

#include <ttLibC/resampler/imageResampler.h>

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
}

typedef struct {
	ttLibC_Openh264Decoder *decoder;
	ttLibC_CvWindow *dec_win;
	ttLibC_Bgr *dbgr;
} openh264TestData;

void openh264DecoderTestCallback(void *ptr, ttLibC_Yuv420 *yuv420) {
	openh264TestData *testData = (openh264TestData *)ptr;
	ttLibC_Bgr *b = ttLibC_ImageResampler_makeBgrFromYuv420(testData->dbgr, BgrType_bgr, yuv420);
	if(b == NULL) {
		return;
	}
	testData->dbgr = b;
	ttLibC_CvWindow_showBgr(testData->dec_win, testData->dbgr);
}

void openh264EncoderTestCallback(void *ptr, ttLibC_H264 *h264) {
	openh264TestData *testData = (openh264TestData *)ptr;
/*	switch(h264->type) {
	case H264Type_configData:
		LOG_PRINT("encoded. config:   pts:%llu size:%lu", h264->inherit_super.inherit_super.pts, h264->inherit_super.inherit_super.buffer_size);
		break;
	case H264Type_slice:
		LOG_PRINT("encoded. slice:    pts:%llu size:%lu", h264->inherit_super.inherit_super.pts, h264->inherit_super.inherit_super.buffer_size);
		break;
	case H264Type_sliceIDR:
		LOG_PRINT("encoded. sliceIDR: pts:%llu size:%lu", h264->inherit_super.inherit_super.pts, h264->inherit_super.inherit_super.buffer_size);
		break;
	default:
		LOG_PRINT("encoded. unknown:  pts:%llu size:%lu", h264->inherit_super.inherit_super.pts, h264->inherit_super.inherit_super.buffer_size);
		break;
	}*/
	ttLibC_Openh264Decoder_decode(testData->decoder, h264, openh264DecoderTestCallback, testData);
}

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
		openh264TestData testData;
		testData.decoder = decoder;
		testData.dec_win = dec_win;

		testData.dbgr = dbgr;
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
}

void faacEncoderTestCallback(void *ptr, ttLibC_Aac *aac) {
	LOG_PRINT("encoded. pts:%llu size:%lu", aac->inherit_super.inherit_super.pts, aac->inherit_super.inherit_super.buffer_size);
}

static void faacEncoderTest() {
	LOG_PRINT("faacEncoderTest");
#ifdef __ENABLE_FAAC_ENCODE__
	ttLibC_FaacEncoder *encoder = ttLibC_FaacEncoder_make(22050, 2, 96000);
	ttLibC_BeepGenerator *generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian, 440, 22050, 2);
	ttLibC_PcmS16 *pcm = NULL, *p;

	for(int i = 0;i < 3; ++ i) {
		// make 1sec beep.
		p = ttLibC_BeepGenerator_makeBeepByMiliSec(generator, pcm, 1000);
		if(p == NULL) {
			break;
		}
		pcm = p;
		// encode data.
		ttLibC_FaacEncoder_encode(encoder, pcm, faacEncoderTestCallback, NULL);
	}
	ttLibC_PcmS16_close(&pcm);
	ttLibC_BeepGenerator_close(&generator);
	ttLibC_FaacEncoder_close(&encoder);
 #endif
}

void mp3lameEncoderTestCallback(void *ptr, ttLibC_Mp3 *mp3) {
	LOG_PRINT("encoded. pts:%llu size:%lu", mp3->inherit_super.inherit_super.pts, mp3->inherit_super.inherit_super.buffer_size);
}

static void mp3lameEncoderTest() {
	LOG_PRINT("mp3lameEncoderTest");
#ifdef __ENABLE_MP3LAME_ENCODE__
	ttLibC_Mp3lameEncoder *encoder = ttLibC_Mp3lameEncoder_make(22050, 1, 10);
	ttLibC_BeepGenerator *generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian_planar, 440, 22050, 1);
	ttLibC_PcmS16 *pcm = NULL, *p;

	for(int i = 0;i < 3; ++ i) {
		// make 1sec beep.
		p = ttLibC_BeepGenerator_makeBeepByMiliSec(generator, pcm, 1000);
		if(p == NULL) {
			break;
		}
		pcm = p;
		// encode data.
		ttLibC_Mp3lameEncoder_encode(encoder, pcm, mp3lameEncoderTestCallback, NULL);
	}
	ttLibC_PcmS16_close(&pcm);
	ttLibC_BeepGenerator_close(&generator);
	ttLibC_Mp3lameEncoder_close(&encoder);
#endif
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
}

/**
 * define all test for encoder and decoder package.
 * @param s cute::suite obj
 * @return cute::suite obj
 */
cute::suite encoderDecoderTests(cute::suite s) {
	s.clear();
	s.push_back(CUTE(speexdspResamplerTest));
	s.push_back(CUTE(openh264Test));
	s.push_back(CUTE(faacEncoderTest));
	s.push_back(CUTE(mp3lameEncoderTest));
	s.push_back(CUTE(imageResamplerTest));
	return s;
}



