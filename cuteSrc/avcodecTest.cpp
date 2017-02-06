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
#include <ttLibC/allocator.h>

#ifdef __ENABLE_AVCODEC__
extern "C" {
#	include <libavcodec/avcodec.h>
#	include <libavutil/opt.h>
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

#include <ttLibC/container/flv.h>

#if defined(__ENABLE_AVCODEC__)
typedef struct flvDecodeTest_t {
	ttLibC_AvcodecDecoder *videoDecoder;
	ttLibC_AvcodecDecoder *audioDecoder;
} flvVp6DecodeTest_t;

static bool flvDecoderTest_vp6DecodeCallback(void *ptr, ttLibC_Frame *frame) {
	ttLibC_Video *video = (ttLibC_Video *)frame;
	LOG_PRINT("decode:type:%d %d x %d", frame->type, video->width, video->height);
	return true;
}

static bool flvDecoderTest_audioDecodeCallback(void *ptr, ttLibC_Frame *frame) {
	LOG_PRINT("decode:type:%d id:%d", frame->type, frame->id);
	return true;
}

static bool flvDecodeTest_flvFrameCallback(void *ptr, ttLibC_Frame *frame) {
	flvDecodeTest_t *testData = (flvDecodeTest_t *)ptr;
	switch(frame->type) {
	case frameType_vp6:
		{
			LOG_PRINT("vp6");
			// just check to decode.
			ttLibC_Frame *cloned_frame = ttLibC_Frame_clone(NULL, frame);
			bool result = ttLibC_AvcodecDecoder_decode(testData->videoDecoder, cloned_frame, flvDecoderTest_vp6DecodeCallback, ptr);
			ttLibC_Frame_close(&cloned_frame);
			return result;
		}
	case frameType_mp3:
		LOG_PRINT("mp3");
		break;
	case frameType_speex:
		LOG_PRINT("speex");
		break;
	case frameType_nellymoser:
		{
			ttLibC_Audio *audio = (ttLibC_Audio *)frame;
			LOG_PRINT("nellymoser %d %d", audio->inherit_super.pts, audio->sample_rate);
			ttLibC_Frame *cloned_frame = ttLibC_Frame_clone(NULL, frame);
			bool result = ttLibC_AvcodecDecoder_decode(testData->audioDecoder, frame, flvDecoderTest_audioDecodeCallback, ptr);
			ttLibC_Frame_close(&cloned_frame);
			return result;
		}
		break;
	default:
		break;
	}
	return true;
}

static bool flvDecodeTest_flvReadCallback(void *ptr, ttLibC_Flv *flv) {
	return ttLibC_Flv_getFrame(flv, flvDecodeTest_flvFrameCallback, ptr);
}
#endif

static void flvDecodeTest() {
	LOG_PRINT("flvVp6DecodeTest");
#if defined(__ENABLE_AVCODEC__)
//	FILE *fp = fopen("smile.vp6.mp3.flv", "rb");
//	FILE *fp = fopen("smile.vp6.speex.flv", "rb");
	FILE *fp = fopen("smile.vp6.nelly8.flv", "rb");
//	FILE *fp = fopen("smile.vp6.nelly16.flv", "rb");
//	FILE *fp = fopen("smile.vp6.nelly44.flv", "rb");
//	FILE *fp = fopen("smile.vp6.nelly22.flv", "rb");
	if(fp != NULL) {
		ttLibC_FlvReader *reader = ttLibC_FlvReader_make();
		flvVp6DecodeTest_t testData;
		testData.videoDecoder = ttLibC_AvcodecVideoDecoder_make(frameType_vp6, 512, 384);
		testData.audioDecoder = ttLibC_AvcodecAudioDecoder_make(frameType_nellymoser, 8000, 1);
		uint8_t buffer[65536];
		while(!feof(fp)) {
			size_t read_size = fread(buffer, 1, 65536, fp);
			if(!ttLibC_FlvReader_read(reader, buffer, read_size, flvDecodeTest_flvReadCallback, &testData)) {
				LOG_PRINT("failed to get flv tag.");
				break;
			}
		}
		ttLibC_AvcodecDecoder_close(&testData.videoDecoder);
		ttLibC_AvcodecDecoder_close(&testData.audioDecoder);
		ttLibC_FlvReader_close(&reader);
		fclose(fp);
	}
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

#if defined(__ENABLE_AVCODEC__) && defined(__ENABLE_OPENAL__)
typedef struct {
	ttLibC_AlDevice *device;
	ttLibC_PcmS16 *pcms16;
	ttLibC_AvcodecDecoder *decoder;
} avcodecAudioTest_t;

bool avcodecAudioDecodeCallback(void *ptr, ttLibC_Frame *frame) {
	avcodecAudioTest_t *testData = (avcodecAudioTest_t *)ptr;
	// convert from frame into pcms16 interleave data.
	ttLibC_PcmS16 *pcms16 = (ttLibC_PcmS16 *)ttLibC_AudioResampler_convertFormat(
			(ttLibC_Audio *)testData->pcms16,
			frameType_pcmS16,
			(uint32_t)PcmS16Type_littleEndian,
			2,
			(ttLibC_Audio *)frame);
	if(pcms16 == NULL) {
		return true;
	}
	testData->pcms16 = pcms16;
	// queue sound.
	ttLibC_AlDevice_queue(testData->device, pcms16);
	return true;
}

bool avcodecAudioEncodeCallback(void *ptr, ttLibC_Frame *frame) {
	avcodecAudioTest_t *testData = (avcodecAudioTest_t *)ptr;
	if(testData == NULL) {
		return false;
	}
	if(frame == NULL) {
		return true;
	}
	LOG_PRINT("encoded type:%u pts:%llu", frame->type, frame->pts);
	ttLibC_AvcodecDecoder_decode(testData->decoder, frame, avcodecAudioDecodeCallback, ptr);
	return true;
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
	ASSERT(ttLibC_Allocator_dump() == 0);
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
	ASSERT(ttLibC_Allocator_dump() == 0);
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
	ASSERT(ttLibC_Allocator_dump() == 0);
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
	ASSERT(ttLibC_Allocator_dump() == 0);
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
	ASSERT(ttLibC_Allocator_dump() == 0);
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
	ASSERT(ttLibC_Allocator_dump() == 0);
}

static void opusTest() {
	LOG_PRINT("opusTest");
#if defined(__ENABLE_AVCODEC__) && defined(__ENABLE_OPENAL__)
	avcodecAudioTest_t testData;
	uint32_t sample_rate = 48000, channel_num = 1;
	ttLibC_BeepGenerator *generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian, 440, sample_rate, channel_num);
	generator->amplitude = 30000;

	AVCodecContext *enc = (AVCodecContext *)ttLibC_AvcodecEncoder_getAVCodecContext(frameType_opus);
	enc->bit_rate = 96000;
	enc->sample_rate = sample_rate;
	enc->time_base = (AVRational){1, (int)sample_rate};
	enc->channels = channel_num;
	enc->channel_layout = av_get_default_channel_layout(channel_num);
	enc->sample_fmt = AV_SAMPLE_FMT_S16;
	enc->strict_std_compliance = -2;
	ttLibC_AvcodecEncoder *encoder = ttLibC_AvcodecEncoder_makeWithAVCodecContext(enc);

	testData.device = ttLibC_AlDevice_make(256);
	testData.pcms16 = NULL;
	testData.decoder = ttLibC_AvcodecAudioDecoder_make_ex(
			frameType_opus,
			sample_rate,
			channel_num,
			ttLibC_AvcodecEncoder_getExtraData(encoder),
			ttLibC_AvcodecEncoder_getExtraDataSize(encoder));
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
	ASSERT(ttLibC_Allocator_dump() == 0);
}

static void speexTest() {
	LOG_PRINT("speexTest");
#if defined(__ENABLE_AVCODEC__) && defined(__ENABLE_OPENAL__)
	avcodecAudioTest_t testData;
	uint32_t sample_rate = 32000, channel_num = 1;
	ttLibC_BeepGenerator *generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian, 440, sample_rate, channel_num);
	generator->amplitude = 30000;

	AVCodecContext *enc = (AVCodecContext *)ttLibC_AvcodecEncoder_getAVCodecContext(frameType_speex);
	enc->bit_rate = 96000;
	enc->sample_rate = sample_rate;
	enc->time_base = (AVRational){1, (int)sample_rate};
	enc->channels = channel_num;
	enc->channel_layout = av_get_default_channel_layout(channel_num);
	enc->sample_fmt = AV_SAMPLE_FMT_S16;
	enc->strict_std_compliance = -2;
	ttLibC_AvcodecEncoder *encoder = ttLibC_AvcodecEncoder_makeWithAVCodecContext(enc);

	testData.device = ttLibC_AlDevice_make(256);
	testData.pcms16 = NULL;
	testData.decoder = ttLibC_AvcodecAudioDecoder_make_ex(
			frameType_speex,
			sample_rate,
			channel_num,
			ttLibC_AvcodecEncoder_getExtraData(encoder),
			ttLibC_AvcodecEncoder_getExtraDataSize(encoder));
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
	ASSERT(ttLibC_Allocator_dump() == 0);
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
	enc->time_base = (AVRational){1, (int)sample_rate};
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
	ASSERT(ttLibC_Allocator_dump() == 0);
}

#if defined(__ENABLE_AVCODEC__) && defined(__ENABLE_OPENCV__)
typedef struct {
	ttLibC_CvWindow *window;
	ttLibC_Bgr *bgr;
	ttLibC_AvcodecDecoder *decoder;
} avcodecVideoTest_t;

bool avcodecVideoDecoderCallback(void *ptr, ttLibC_Frame *frame) {
	avcodecVideoTest_t *testData = (avcodecVideoTest_t *)ptr;
	switch(frame->type) {
	case frameType_yuv420:
		{
			ttLibC_Bgr *b = ttLibC_ImageResampler_makeBgrFromYuv420(testData->bgr, BgrType_bgr, (ttLibC_Yuv420 *)frame);
			if(b == NULL) {
				return false;
			}
			testData->bgr = b;
			ttLibC_CvWindow_showBgr(testData->window, testData->bgr);
		}
		break;
	default:
		break;
	}
	return true;
}

bool avcodecVideoEncoderCallback(void *ptr, ttLibC_Frame *frame) {
	avcodecVideoTest_t *testData = (avcodecVideoTest_t *)ptr;
	ttLibC_Video *video = (ttLibC_Video *)frame;
	LOG_PRINT("keyFrame:%u size:%lu", video->type, video->inherit_super.buffer_size);
	ttLibC_AvcodecDecoder_decode(testData->decoder, frame, avcodecVideoDecoderCallback, ptr);
	return true;
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
	ttLibC_AvcodecEncoder *encoder = ttLibC_AvcodecVideoEncoder_make_ex(frameType_flv1, width, height, 0, 650000, 100);
	testData.window  = ttLibC_CvWindow_make("target");
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
	ASSERT(ttLibC_Allocator_dump() == 0);
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
	ttLibC_AvcodecEncoder *encoder = ttLibC_AvcodecVideoEncoder_make_ex(frameType_vp8, width, height, 0, 650000, 100);
	testData.window  = ttLibC_CvWindow_make("target");
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
	ASSERT(ttLibC_Allocator_dump() == 0);
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
	ttLibC_AvcodecEncoder *encoder = ttLibC_AvcodecVideoEncoder_make_ex(frameType_wmv1, width, height, 0, 650000, 100);
	testData.window  = ttLibC_CvWindow_make("target");
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
	ASSERT(ttLibC_Allocator_dump() == 0);
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
	testData.window  = ttLibC_CvWindow_make("target");
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
	ASSERT(ttLibC_Allocator_dump() == 0);
}

static void h264Test() {
	LOG_PRINT("h264Test");
#if defined(__ENABLE_AVCODEC__) && defined(__ENABLE_OPENCV__)
	// TODO h264 nal analyze is not ready. do later.
/*	uint32_t width = 320, height = 240;
	ttLibC_CvCapture *capture = ttLibC_CvCapture_make(0, width, height);
	ttLibC_CvWindow  *window  = ttLibC_CvWindow_make("original");
	ttLibC_Bgr    *bgr = NULL, *b;
	ttLibC_Yuv420 *yuv = NULL, *y;
	avcodecVideoTest_t testData;
	AVCodecContext *enc = (AVCodecContext *)ttLibC_AvcodecEncoder_getAVCodecContext(frameType_h264);
	enc->bit_rate = 650000;
	enc->width = width;
	enc->height = height;
	enc->global_quality = 10;
	enc->framerate = (AVRational){1, 15};
	enc->time_base = (AVRational){1, 1000};
	enc->gop_size = 10;
	enc->max_b_frames = 0;

	enc->level = 30;
	av_opt_set(enc, "coder", "0", 0);
	av_opt_set(enc, "qmin", "10", 0);
	av_opt_set(enc, "bf", "0", 0);
	av_opt_set(enc, "wprefp", "0", 0);
	av_opt_set(enc, "cmp", "+chroma", 0);
	av_opt_set(enc, "partitions", "-parti8x8+parti4x4+partp8x8+partp4x4-partb8x8", 0);
	av_opt_set(enc, "me_method", "hex", 0);
	av_opt_set(enc, "subq", "5", 0);
	av_opt_set(enc, "me_range", "16", 0);
	av_opt_set(enc, "keyint_min", "25", 0);
	av_opt_set(enc, "sc_threshold", "40", 0);
	av_opt_set(enc, "i_qfactor", "0.71", 0);
	av_opt_set(enc, "b_strategy", "0", 0);
	av_opt_set(enc, "qcomp", "0.6", 0);
	av_opt_set(enc, "qmax", "30", 0);
	av_opt_set(enc, "qdiff", "4", 0);
	av_opt_set(enc, "direct-pred", "0", 0);
//	av_opt_set(enc, "profile", "main", 0);
	enc->profile = FF_PROFILE_H264_BASELINE;
	enc->flags = CODEC_FLAG_LOOP_FILTER | CODEC_FLAG_CLOSED_GOP | CODEC_FLAG_LOW_DELAY;
	enc->pix_fmt = AV_PIX_FMT_YUV420P;
	ttLibC_AvcodecEncoder *encoder = ttLibC_AvcodecEncoder_makeWithAVCodecContext(enc);
	testData.window  = ttLibC_CvWindow_make("target");
	testData.bgr     = NULL;
	testData.decoder = ttLibC_AvcodecVideoDecoder_make(frameType_h264, width, height);
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
	ASSERT(ttLibC_Allocator_dump() == 0);
}

/**
 * define all test for avcodec.
 * @param s cute::suite obj
 * @return cute::suite obj
 */
cute::suite avcodecTests(cute::suite s) {
	s.clear();
	s.push_back(CUTE(flvDecodeTest));
	s.push_back(CUTE(aacTest));
	s.push_back(CUTE(adpcmImaWavTest));
	s.push_back(CUTE(mp3Test));
	s.push_back(CUTE(nellymoserTest));
	s.push_back(CUTE(pcmAlawTest));
	s.push_back(CUTE(pcmMulawTest));
	s.push_back(CUTE(opusTest));
	s.push_back(CUTE(speexTest));
	s.push_back(CUTE(vorbisTest));
	s.push_back(CUTE(flv1Test));
	s.push_back(CUTE(vp8Test));
	s.push_back(CUTE(wmv1Test));
	s.push_back(CUTE(wmv2Test));
	s.push_back(CUTE(h264Test));
	return s;
}
