/**
 * @file   avcodecTest.cpp
 * @brief  avcodec convert test code.
 *
 * this code is under GPLv3 license.
 *
 * @author taktod
 * @date   2015/08/11
 */
#include <cute.h>

#include <ttLibC/log.h>

#ifdef __ENABLE_AVCODEC__
extern "C" {
#	include <libavcodec/avcodec.h>
}
#	include <ttLibC/encoder/avcodecEncoder.h>
#	include <ttLibC/decoder/avcodecDecoder.h>
#endif

#ifdef __ENABLE_OPENCV__
#	include <ttLibC/util/opencvUtil.h>
#endif

#ifdef __ENABLE_OPENAL__
#	include <ttLibC/util/openalUtil.h>
#endif

#include <ttLibC/util/beepUtil.h>
#include <ttLibC/resampler/imageResampler.h>
#include <ttLibC/resampler/audioResampler.h>

#include <ttLibC/frame/audio/pcms16.h>
#include <ttLibC/frame/audio/pcmf32.h>
#include <ttLibC/frame/audio/audio.h>
#include <ttLibC/frame/frame.h>

#if defined(__ENABLE_AVCODEC__) && defined(__ENABLE_OPENAL__)
typedef struct {
	ttLibC_AlDevice *device;
	ttLibC_PcmS16 *pcms16;
	ttLibC_AvcodecDecoder *decoder;
} avcodecAudioTest_t;

void avcodecAudioDecodeCallback(void *ptr, ttLibC_Frame *frame) {
	avcodecAudioTest_t *testData = (avcodecAudioTest_t *)ptr;
	// convert from frame into pcms16 interleave data.
	ttLibC_PcmS16 *pcms16 = (ttLibC_PcmS16 *)ttLibC_AudioResampler_convertFormat(
			(ttLibC_Audio *)testData->pcms16,
			frameType_pcmS16,
			(uint32_t)PcmS16Type_littleEndian,
			2,
			(ttLibC_Audio *)frame);
	if(pcms16 == NULL) {
		return;
	}
	// queue sound.
	ttLibC_AlDevice_queue(testData->device, pcms16);
}

void avcodecAudioEncodeCallback(void *ptr, ttLibC_Frame *frame) {
	avcodecAudioTest_t *testData = (avcodecAudioTest_t *)ptr;
	if(frame == NULL) {
		return;
	}
	LOG_PRINT("encoded type:%u pts:%llu", frame->type, frame->pts);
	if(testData == NULL) {
		return;
	}
	ttLibC_AvcodecDecoder_decode(testData->decoder, frame, avcodecAudioDecodeCallback, ptr);
}
#endif

static void aacTest() {
	LOG_PRINT("aacTest");
#if defined(__ENABLE_AVCODEC__) && defined(__ENABLE_OPENAL__)
	avcodecAudioTest_t testData;
	uint32_t sample_rate = 44100, channel_num = 2;
	ttLibC_BeepGenerator *generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian, 440, sample_rate, channel_num);
	generator->amplitude = 30000;
	ttLibC_AvcodecEncoder *encoder = ttLibC_AvcodecAudioEncoder_make(frameType_aac, sample_rate, channel_num);
	testData.device = ttLibC_AlDevice_make(256);
	testData.pcms16 = NULL;
	testData.decoder = ttLibC_AvcodecAudioDecoder_make(frameType_aac, sample_rate, channel_num);
	ttLibC_PcmS16 *pcms16 = NULL, *p;

	for(int i = 0;i < 10;++ i) {
		p = ttLibC_BeepGenerator_makeBeepByMiliSec(generator, pcms16, 200);
		if(p == NULL) {
			break;
		}
		pcms16 = p;
		ttLibC_AvcodecEncoder_encode(encoder, (ttLibC_Frame *)pcms16, avcodecAudioEncodeCallback, &testData);
	}
	// start to play
	ttLibC_AlDevice_proceed(testData.device, -1);

	ttLibC_PcmS16_close(&pcms16);
	ttLibC_AvcodecDecoder_close(&testData.decoder);
	ttLibC_PcmS16_close(&testData.pcms16);
	ttLibC_AlDevice_close(&testData.device);
	ttLibC_AvcodecEncoder_close(&encoder);
	ttLibC_BeepGenerator_close(&generator);
#endif
}

static void adpcmImaWavTest() {
	LOG_PRINT("adpcmImaWavTest");
#if defined(__ENABLE_AVCODEC__) && defined(__ENABLE_OPENAL__)
	avcodecAudioTest_t testData;
	uint32_t sample_rate = 44100, channel_num = 2;
	ttLibC_BeepGenerator *generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian_planar, 440, sample_rate, channel_num);
	generator->amplitude = 30000;
	ttLibC_AvcodecEncoder *encoder = ttLibC_AvcodecAudioEncoder_make(frameType_adpcm_ima_wav, sample_rate, channel_num);
	testData.device = ttLibC_AlDevice_make(256);
	testData.pcms16 = NULL;
	testData.decoder = ttLibC_AvcodecAudioDecoder_make(frameType_adpcm_ima_wav, sample_rate, channel_num);
	ttLibC_PcmS16 *pcms16 = NULL, *p;

	for(int i = 0;i < 10;++ i) {
		p = ttLibC_BeepGenerator_makeBeepByMiliSec(generator, pcms16, 200);
		if(p == NULL) {
			break;
		}
		pcms16 = p;
		ttLibC_AvcodecEncoder_encode(encoder, (ttLibC_Frame *)pcms16, avcodecAudioEncodeCallback, &testData);
	}
	// start to play
	ttLibC_AlDevice_proceed(testData.device, -1);

	ttLibC_PcmS16_close(&pcms16);
	ttLibC_AvcodecDecoder_close(&testData.decoder);
	ttLibC_PcmS16_close(&testData.pcms16);
	ttLibC_AlDevice_close(&testData.device);
	ttLibC_AvcodecEncoder_close(&encoder);
	ttLibC_BeepGenerator_close(&generator);
#endif
}

static void mp3Test() {
	LOG_PRINT("mp3Test");
	// TODO have some noise on the begining.
#if defined(__ENABLE_AVCODEC__) && defined(__ENABLE_OPENAL__)
	avcodecAudioTest_t testData;
	uint32_t sample_rate = 44100, channel_num = 2;
	ttLibC_BeepGenerator *generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian, 440, sample_rate, channel_num);
	generator->amplitude = 30000;
	ttLibC_AvcodecEncoder *encoder = ttLibC_AvcodecAudioEncoder_make(frameType_mp3, sample_rate, channel_num);
	testData.device = ttLibC_AlDevice_make(256);
	testData.pcms16 = NULL;
	testData.decoder = ttLibC_AvcodecAudioDecoder_make(frameType_mp3, sample_rate, channel_num);
	ttLibC_PcmS16 *pcms16 = NULL, *p;
	ttLibC_PcmF32 *pcmf32 = NULL, *pf;

	for(int i = 0;i < 10;++ i) {
		p = ttLibC_BeepGenerator_makeBeepByMiliSec(generator, pcms16, 200);
		if(p == NULL) {
			break;
		}
		pcms16 = p;
		pf = ttLibC_AudioResampler_makePcmF32FromPcmS16(pcmf32, PcmF32Type_planar, pcms16);
		if(pf == NULL) {
			break;
		}
		pcmf32 = pf;
		ttLibC_AvcodecEncoder_encode(encoder, (ttLibC_Frame *)pcmf32, avcodecAudioEncodeCallback, &testData);
	}
	// start to play
	ttLibC_AlDevice_proceed(testData.device, -1);

	ttLibC_PcmF32_close(&pcmf32);
	ttLibC_PcmS16_close(&pcms16);
	ttLibC_AvcodecDecoder_close(&testData.decoder);
	ttLibC_PcmS16_close(&testData.pcms16);
	ttLibC_AlDevice_close(&testData.device);
	ttLibC_AvcodecEncoder_close(&encoder);
	ttLibC_BeepGenerator_close(&generator);
#endif
}

static void nellymoserTest() {
	LOG_PRINT("nellymoserTest");
#if defined(__ENABLE_AVCODEC__) && defined(__ENABLE_OPENAL__)
	avcodecAudioTest_t testData;
	uint32_t sample_rate = 44100, channel_num = 1;
	ttLibC_BeepGenerator *generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian, 440, sample_rate, channel_num);
	generator->amplitude = 30000;
	ttLibC_AvcodecEncoder *encoder = ttLibC_AvcodecAudioEncoder_make(frameType_nellymoser, sample_rate, channel_num);
	testData.device = ttLibC_AlDevice_make(256);
	testData.pcms16 = NULL;
	testData.decoder = ttLibC_AvcodecAudioDecoder_make(frameType_nellymoser, sample_rate, channel_num);
	ttLibC_PcmS16 *pcms16 = NULL, *p;
	ttLibC_PcmF32 *pcmf32 = NULL, *pf;

	for(int i = 0;i < 10;++ i) {
		p = ttLibC_BeepGenerator_makeBeepByMiliSec(generator, pcms16, 100);
		if(p == NULL) {
			break;
		}
		pcms16 = p;
		pf = ttLibC_AudioResampler_makePcmF32FromPcmS16(pcmf32, PcmF32Type_interleave, pcms16);
		if(pf == NULL) {
			break;
		}
		pcmf32 = pf;
		ttLibC_AvcodecEncoder_encode(encoder, (ttLibC_Frame *)pcmf32, avcodecAudioEncodeCallback, &testData);
	}
	// start to play
	ttLibC_AlDevice_proceed(testData.device, -1);

	ttLibC_PcmF32_close(&pcmf32);
	ttLibC_PcmS16_close(&pcms16);
	ttLibC_AvcodecDecoder_close(&testData.decoder);
	ttLibC_PcmS16_close(&testData.pcms16);
	ttLibC_AlDevice_close(&testData.device);
	ttLibC_AvcodecEncoder_close(&encoder);
	ttLibC_BeepGenerator_close(&generator);
#endif
}

static void pcmAlawTest() {
	LOG_PRINT("pcmAlawTest");
#if defined(__ENABLE_AVCODEC__) && defined(__ENABLE_OPENAL__)
	avcodecAudioTest_t testData;
	uint32_t sample_rate = 44100, channel_num = 2;
	ttLibC_BeepGenerator *generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian, 440, sample_rate, channel_num);
	generator->amplitude = 30000;
	ttLibC_AvcodecEncoder *encoder = ttLibC_AvcodecAudioEncoder_make(frameType_pcm_alaw, sample_rate, channel_num);
	testData.device = ttLibC_AlDevice_make(256);
	testData.pcms16 = NULL;
	testData.decoder = ttLibC_AvcodecAudioDecoder_make(frameType_pcm_alaw, sample_rate, channel_num);
	ttLibC_PcmS16 *pcms16 = NULL, *p;

	for(int i = 0;i < 10;++ i) {
		p = ttLibC_BeepGenerator_makeBeepByMiliSec(generator, pcms16, 100);
		if(p == NULL) {
			break;
		}
		pcms16 = p;
		ttLibC_AvcodecEncoder_encode(encoder, (ttLibC_Frame *)pcms16, avcodecAudioEncodeCallback, &testData);
	}
	// start to play
	ttLibC_AlDevice_proceed(testData.device, -1);

	ttLibC_PcmS16_close(&pcms16);
	ttLibC_AvcodecDecoder_close(&testData.decoder);
	ttLibC_PcmS16_close(&testData.pcms16);
	ttLibC_AlDevice_close(&testData.device);
	ttLibC_AvcodecEncoder_close(&encoder);
	ttLibC_BeepGenerator_close(&generator);
#endif
}

static void pcmMulawTest() {
	LOG_PRINT("pcmMulawTest");
#if defined(__ENABLE_AVCODEC__) && defined(__ENABLE_OPENAL__)
	avcodecAudioTest_t testData;
	uint32_t sample_rate = 44100, channel_num = 2;
	ttLibC_BeepGenerator *generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian, 440, sample_rate, channel_num);
	generator->amplitude = 30000;
	ttLibC_AvcodecEncoder *encoder = ttLibC_AvcodecAudioEncoder_make(frameType_pcm_mulaw, sample_rate, channel_num);
	testData.device = ttLibC_AlDevice_make(256);
	testData.pcms16 = NULL;
	testData.decoder = ttLibC_AvcodecAudioDecoder_make(frameType_pcm_mulaw, sample_rate, channel_num);
	ttLibC_PcmS16 *pcms16 = NULL, *p;

	for(int i = 0;i < 10;++ i) {
		p = ttLibC_BeepGenerator_makeBeepByMiliSec(generator, pcms16, 100);
		if(p == NULL) {
			break;
		}
		pcms16 = p;
		ttLibC_AvcodecEncoder_encode(encoder, (ttLibC_Frame *)pcms16, avcodecAudioEncodeCallback, &testData);
	}
	// start to play
	ttLibC_AlDevice_proceed(testData.device, -1);

	ttLibC_PcmS16_close(&pcms16);
	ttLibC_AvcodecDecoder_close(&testData.decoder);
	ttLibC_PcmS16_close(&testData.pcms16);
	ttLibC_AlDevice_close(&testData.device);
	ttLibC_AvcodecEncoder_close(&encoder);
	ttLibC_BeepGenerator_close(&generator);
#endif
}

static void vorbisTest() {
	LOG_PRINT("vorbisTest");
#if defined(__ENABLE_AVCODEC__) && defined(__ENABLE_OPENAL__)
	avcodecAudioTest_t testData;
	uint32_t sample_rate = 44100, channel_num = 2;
	ttLibC_BeepGenerator *generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian, 440, sample_rate, channel_num);
	generator->amplitude = 30000;

	AVCodecContext *enc = (AVCodecContext *)ttLibC_AvcodecEncoder_getAVCodecContext(frameType_vorbis);
	enc->bit_rate = 96000;
	enc->sample_rate = sample_rate;
	enc->time_base = (AVRational){1, sample_rate};
	enc->channels = channel_num;
	enc->channel_layout = av_get_default_channel_layout(channel_num);
	enc->sample_fmt = AV_SAMPLE_FMT_FLTP;
	enc->strict_std_compliance = -2;
	ttLibC_AvcodecEncoder *encoder = ttLibC_AvcodecEncoder_makeWithAVCodecContext(enc);

	testData.device = ttLibC_AlDevice_make(256);
	testData.pcms16 = NULL;
	testData.decoder = ttLibC_AvcodecAudioDecoder_make_ex(
			frameType_vorbis,
			sample_rate,
			channel_num,
			ttLibC_AvcodecEncoder_getExtraData(encoder),
			ttLibC_AvcodecEncoder_getExtraDataSize(encoder));
	ttLibC_PcmS16 *pcms16 = NULL, *p;
	ttLibC_PcmF32 *pcmf32 = NULL, *pf;

	for(int i = 0;i < 10;++ i) {
		p = ttLibC_BeepGenerator_makeBeepByMiliSec(generator, pcms16, 100);
		if(p == NULL) {
			break;
		}
		pcms16 = p;
		pf = ttLibC_AudioResampler_makePcmF32FromPcmS16(pcmf32, PcmF32Type_planar, pcms16);
		if(pf == NULL) {
			break;
		}
		pcmf32 = pf;
		ttLibC_AvcodecEncoder_encode(encoder, (ttLibC_Frame *)pcmf32, avcodecAudioEncodeCallback, &testData);
	}
	// start to play
	ttLibC_AlDevice_proceed(testData.device, -1);

	ttLibC_PcmF32_close(&pcmf32);
	ttLibC_PcmS16_close(&pcms16);
	ttLibC_AvcodecDecoder_close(&testData.decoder);
	ttLibC_PcmS16_close(&testData.pcms16);
	ttLibC_AlDevice_close(&testData.device);
	ttLibC_AvcodecEncoder_close(&encoder);
	ttLibC_BeepGenerator_close(&generator);
#endif
}

#if defined(__ENABLE_AVCODEC__) && defined(__ENABLE_OPENCV__)
typedef struct {
	ttLibC_CvWindow *window;
	ttLibC_Bgr *bgr;
	ttLibC_AvcodecDecoder *decoder;
} avcodecVideoTest_t;

void avcodecVideoDecoderCallback(void *ptr, ttLibC_Frame *frame) {
	avcodecVideoTest_t *testData = (avcodecVideoTest_t *)ptr;
	switch(frame->type) {
	case frameType_yuv420:
		{
			ttLibC_Bgr *b = ttLibC_ImageResampler_makeBgrFromYuv420(testData->bgr, BgrType_bgr, (ttLibC_Yuv420 *)frame);
			if(b == NULL) {
				return;
			}
			testData->bgr = b;
			ttLibC_CvWindow_showBgr(testData->window, testData->bgr);
		}
		break;
	default:
		return;
	}
}

void avcodecVideoEncoderCallback(void *ptr, ttLibC_Frame *frame) {
	avcodecVideoTest_t *testData = (avcodecVideoTest_t *)ptr;
	ttLibC_Video *video = (ttLibC_Video *)frame;
	LOG_PRINT("keyFrame:%u size:%lu", video->type, video->inherit_super.buffer_size);
	ttLibC_AvcodecDecoder_decode(testData->decoder, frame, avcodecVideoDecoderCallback, ptr);
}
#endif


static void flv1Test() {
	LOG_PRINT("flv1Test");
#if defined(__ENABLE_AVCODEC__) && defined(__ENABLE_OPENCV__)
	uint32_t width = 320, height = 240;
	ttLibC_CvCapture *capture = ttLibC_CvCapture_make(0, width, height);
	ttLibC_CvWindow  *window  = ttLibC_CvWindow_make("original");
	ttLibC_Bgr    *bgr = NULL, *b;
	ttLibC_Yuv420 *yuv = NULL, *y;
	avcodecVideoTest_t testData;
	ttLibC_AvcodecEncoder *encoder = ttLibC_AvcodecVideoEncoder_make_ex(frameType_flv1, width, height, 0, 650000, 15, 100);
	testData.window  = ttLibC_CvWindow_make("flv1 decode");
	testData.bgr     = NULL;
	testData.decoder = ttLibC_AvcodecVideoDecoder_make(frameType_flv1, width, height);
	while(true) {
		// capture
		b = ttLibC_CvCapture_queryFrame(capture, bgr);
		if(b == NULL) {
			break;
		}
		bgr = b;
		// convert to yuv420p
		y = ttLibC_ImageResampler_makeYuv420FromBgr(yuv, Yuv420Type_planar, bgr);
		if(y == NULL) {
			break;
		}
		yuv = y;
		// encode
		ttLibC_AvcodecEncoder_encode(encoder, (ttLibC_Frame *)yuv, avcodecVideoEncoderCallback, &testData);
		// show original frame.
		ttLibC_CvWindow_showBgr(window, bgr);
		// if press esc key, exit.
		uint8_t keychar = ttLibC_CvWindow_waitForKeyInput(10);
		if(keychar == Keychar_Esc) {
			break;
		}
	}
	ttLibC_AvcodecEncoder_close(&encoder);
	ttLibC_Yuv420_close(&yuv);
	ttLibC_Bgr_close(&bgr);
	ttLibC_AvcodecDecoder_close(&testData.decoder);
	ttLibC_Bgr_close(&testData.bgr);
	ttLibC_CvWindow_close(&testData.window);
	ttLibC_CvWindow_close(&window);
	ttLibC_CvCapture_close(&capture);
#endif
}

static void vp8Test() {
	LOG_PRINT("vp8Test");
#if defined(__ENABLE_AVCODEC__) && defined(__ENABLE_OPENCV__)
	uint32_t width = 320, height = 240;
	ttLibC_CvCapture *capture = ttLibC_CvCapture_make(0, width, height);
	ttLibC_CvWindow  *window  = ttLibC_CvWindow_make("original");
	ttLibC_Bgr    *bgr = NULL, *b;
	ttLibC_Yuv420 *yuv = NULL, *y;
	avcodecVideoTest_t testData;
	ttLibC_AvcodecEncoder *encoder = ttLibC_AvcodecVideoEncoder_make_ex(frameType_vp8, width, height, 0, 650000, 15, 100);
	testData.window  = ttLibC_CvWindow_make("vp8 decode");
	testData.bgr     = NULL;
	testData.decoder = ttLibC_AvcodecVideoDecoder_make(frameType_vp8, width, height);
	while(true) {
		// capture
		b = ttLibC_CvCapture_queryFrame(capture, bgr);
		if(b == NULL) {
			break;
		}
		bgr = b;
		// convert to yuv420p
		y = ttLibC_ImageResampler_makeYuv420FromBgr(yuv, Yuv420Type_planar, bgr);
		if(y == NULL) {
			break;
		}
		yuv = y;
		// encode
		ttLibC_AvcodecEncoder_encode(encoder, (ttLibC_Frame *)yuv, avcodecVideoEncoderCallback, &testData);
		// show original frame.
		ttLibC_CvWindow_showBgr(window, bgr);
		// if press esc key, exit.
		uint8_t keychar = ttLibC_CvWindow_waitForKeyInput(10);
		if(keychar == Keychar_Esc) {
			break;
		}
	}
	ttLibC_AvcodecEncoder_close(&encoder);
	ttLibC_Yuv420_close(&yuv);
	ttLibC_Bgr_close(&bgr);
	ttLibC_AvcodecDecoder_close(&testData.decoder);
	ttLibC_Bgr_close(&testData.bgr);
	ttLibC_CvWindow_close(&testData.window);
	ttLibC_CvWindow_close(&window);
	ttLibC_CvCapture_close(&capture);
#endif
}

static void wmv1Test() {
	LOG_PRINT("wmv1Test");
#if defined(__ENABLE_AVCODEC__) && defined(__ENABLE_OPENCV__)
	uint32_t width = 320, height = 240;
	ttLibC_CvCapture *capture = ttLibC_CvCapture_make(0, width, height);
	ttLibC_CvWindow  *window  = ttLibC_CvWindow_make("original");
	ttLibC_Bgr    *bgr = NULL, *b;
	ttLibC_Yuv420 *yuv = NULL, *y;
	avcodecVideoTest_t testData;
	ttLibC_AvcodecEncoder *encoder = ttLibC_AvcodecVideoEncoder_make_ex(frameType_wmv1, width, height, 0, 650000, 15, 100);
	testData.window  = ttLibC_CvWindow_make("wmv1 decode");
	testData.bgr     = NULL;
	testData.decoder = ttLibC_AvcodecVideoDecoder_make(frameType_wmv1, width, height);
	while(true) {
		// capture
		b = ttLibC_CvCapture_queryFrame(capture, bgr);
		if(b == NULL) {
			break;
		}
		bgr = b;
		// convert to yuv420p
		y = ttLibC_ImageResampler_makeYuv420FromBgr(yuv, Yuv420Type_planar, bgr);
		if(y == NULL) {
			break;
		}
		yuv = y;
		// encode
		ttLibC_AvcodecEncoder_encode(encoder, (ttLibC_Frame *)yuv, avcodecVideoEncoderCallback, &testData);
		// show original frame.
		ttLibC_CvWindow_showBgr(window, bgr);
		// if press esc key, exit.
		uint8_t keychar = ttLibC_CvWindow_waitForKeyInput(10);
		if(keychar == Keychar_Esc) {
			break;
		}
	}
	ttLibC_AvcodecEncoder_close(&encoder);
	ttLibC_Yuv420_close(&yuv);
	ttLibC_Bgr_close(&bgr);
	ttLibC_AvcodecDecoder_close(&testData.decoder);
	ttLibC_Bgr_close(&testData.bgr);
	ttLibC_CvWindow_close(&testData.window);
	ttLibC_CvWindow_close(&window);
	ttLibC_CvCapture_close(&capture);
#endif
}

static void wmv2Test() {
	LOG_PRINT("wmv2Test");
#if defined(__ENABLE_AVCODEC__) && defined(__ENABLE_OPENCV__)
	// TODO failed to decode completely. do later.
/*	uint32_t width = 320, height = 240;
	ttLibC_CvCapture *capture = ttLibC_CvCapture_make(0, width, height);
	ttLibC_CvWindow  *window  = ttLibC_CvWindow_make("original");
	ttLibC_Bgr    *bgr = NULL, *b;
	ttLibC_Yuv420 *yuv = NULL, *y;
	avcodecVideoTest_t testData;
	ttLibC_AvcodecEncoder *encoder = ttLibC_AvcodecVideoEncoder_make_ex(frameType_wmv2, width, height, 0, 650000, 15, 12);
	testData.window  = ttLibC_CvWindow_make("wmv2 decode");
	testData.bgr     = NULL;
	testData.decoder = ttLibC_AvcodecVideoDecoder_make(frameType_wmv2, width, height);
	while(true) {
		// capture
		b = ttLibC_CvCapture_queryFrame(capture, bgr);
		if(b == NULL) {
			break;
		}
		bgr = b;
		// convert to yuv420p
		y = ttLibC_ImageResampler_makeYuv420FromBgr(yuv, Yuv420Type_planar, bgr);
		if(y == NULL) {
			break;
		}
		yuv = y;
		// encode
		ttLibC_AvcodecEncoder_encode(encoder, (ttLibC_Frame *)yuv, avcodecVideoEncoderCallback, &testData);
		// show original frame.
		ttLibC_CvWindow_showBgr(window, bgr);
		// if press esc key, exit.
		uint8_t keychar = ttLibC_CvWindow_waitForKeyInput(10);
		if(keychar == Keychar_Esc) {
			break;
		}
	}
	ttLibC_AvcodecEncoder_close(&encoder);
	ttLibC_Yuv420_close(&yuv);
	ttLibC_Bgr_close(&bgr);
	ttLibC_AvcodecDecoder_close(&testData.decoder);
	ttLibC_Bgr_close(&testData.bgr);
	ttLibC_CvWindow_close(&testData.window);
	ttLibC_CvWindow_close(&window);
	ttLibC_CvCapture_close(&capture);*/
#endif
}

/**
 * define all test for avcodec.
 * @param s cute::suite obj
 * @return cute::suite obj
 */
cute::suite avcodecTests(cute::suite s) {
	s.clear();
	s.push_back(CUTE(aacTest));
	s.push_back(CUTE(adpcmImaWavTest));
	s.push_back(CUTE(mp3Test));
	s.push_back(CUTE(nellymoserTest));
	s.push_back(CUTE(pcmAlawTest));
	s.push_back(CUTE(pcmMulawTest));
	s.push_back(CUTE(vorbisTest));
	s.push_back(CUTE(flv1Test));
	s.push_back(CUTE(vp8Test));
	s.push_back(CUTE(wmv1Test));
	s.push_back(CUTE(wmv2Test));
	return s;
}