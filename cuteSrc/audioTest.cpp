/**
 * @file   audioTest.cpp
 * @brief  audio test code
 *
 * this code is under GPLv3 license.
 *
 * @author taktod
 * @date   2015/07/18
 */

#include <cute.h>
#include <ttLibC/log.h>

#include <ttLibC/util/beepUtil.h>
#include <ttLibC/util/hexUtil.h>

#ifdef __ENABLE_MP3LAME__
#	include <lame/lame.h>
#endif

#ifdef __ENABLE_FAAC__
#	include <faac.h>
#	include <faaccfg.h>
#endif

#ifdef __ENABLE_SPEEXDSP__
#	include <speex/speex_resampler.h>
#endif

#ifdef __ENABLE_OPENAL__
#	include <ttLibC/util/openalUtil.h>
#endif

#include <ttLibC/frame/audio/mp3.h>

static void speexdspResampleTest() {
	LOG_PRINT("speexdspResampleTest");
#if defined(__ENABLE_SPEEXDSP__) && defined(__ENABLE_OPENAL__)

	SpeexResamplerState *resampler = NULL;
	uint32_t channel = 2;
	uint32_t input_sample_rate = 44100;
	uint32_t output_sample_rate = 48000;
	int error_num;
	resampler = speex_resampler_init(channel, input_sample_rate, output_sample_rate, 5, &error_num);
	if(error_num != 0) {
		ERR_PRINT("failed to initialize speex resampler.");
		return;
	}
	int input_latency = speex_resampler_get_input_latency(resampler);
	LOG_PRINT("input latency:%d", input_latency);
	int output_latency = speex_resampler_get_output_latency(resampler);
	LOG_PRINT("output latency:%d", output_latency);

	ttLibC_BeepGenerator *generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian, 440, input_sample_rate, channel);
	ttLibC_AlDevice *device = ttLibC_AlDevice_make(10);
	ttLibC_PcmS16 *pcm = NULL, *p;
	int16_t resampled_data[100000];
	uint32_t out_size;
	uint32_t in_size;
	for(int i = 0;i < 5;i ++) {
		p = ttLibC_BeepGenerator_makeBeepByMiliSec(generator, pcm, 500);
		if(p == NULL) {
			break;
		}
		pcm = p;
		in_size = pcm->inherit_super.sample_num;
		out_size = 100000;
		int res = speex_resampler_process_interleaved_int(resampler, (const spx_int16_t *)pcm->inherit_super.inherit_super.data, &in_size, resampled_data, &out_size);
		if(res != 0) {
			LOG_PRINT("error happens:%s", speex_resampler_strerror(res));
		}
		else {
			LOG_PRINT("in_size:%d, out_size:%d", in_size, out_size);
		}
		ttLibC_PcmS16 *resampled_pcm = ttLibC_PcmS16_make(NULL, PcmS16Type_littleEndian, output_sample_rate, out_size, channel, resampled_data, out_size * sizeof(int16_t) * 2, true, 0, 1000);
		LOG_PRINT("sample_num:%d rate:%d", resampled_pcm->inherit_super.sample_num, resampled_pcm->inherit_super.sample_rate);
		LOG_PRINT("sample_num:%d rate:%d", pcm->inherit_super.sample_num, pcm->inherit_super.sample_rate);
		ttLibC_AlDevice_queue(device, resampled_pcm);
//		ttLibC_AlDevice_proceed(device, 1);
		ttLibC_PcmS16_close(&resampled_pcm);
	}
	ttLibC_AlDevice_proceed(device, -1);
	ttLibC_AlDevice_close(&device);
	ttLibC_BeepGenerator_close(&generator);
	speex_resampler_destroy(resampler);
#endif
}

static void faacTest() {
	LOG_PRINT("faacTest");
#ifdef __ENABLE_FAAC_ENCODE__
	int channel = 2;
	ttLibC_BeepGenerator *generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian, 440, 44100, channel);
	ttLibC_PcmS16 *pcm = NULL, *p;
	FILE *fp = fopen("output.aac", "wb");
	faacEncHandle faacHandle;
	faacEncConfiguration *faacConfig;
	unsigned long int samples_input, max_bytes_output;
	faacHandle = faacEncOpen(44100, channel, &samples_input, &max_bytes_output);
	if(!faacHandle) {
		ERR_PRINT("failed to open faac.");
		return;
	}
	LOG_PRINT("samples_input:%lu, max_bytes_output:%lu", samples_input, max_bytes_output);
	faacConfig = faacEncGetCurrentConfiguration(faacHandle);
	if(faacConfig->version != FAAC_CFG_VERSION) {
		ERR_PRINT("varsion is mismatch for faac");
	}
	else {
		faacConfig->aacObjectType = MAIN;
		faacConfig->mpegVersion = MPEG4;
		faacConfig->useTns = 0;
		faacConfig->allowMidside = 1;
		faacConfig->bitRate = 96000;
		faacConfig->outputFormat = 1; // 0:raw 1:adts raw for data with global header.
		faacConfig->inputFormat = FAAC_INPUT_16BIT;
		if(!faacEncSetConfiguration(faacHandle, faacConfig)) {
			ERR_PRINT("failed to set configuration.");
		}
		else {
			uint8_t buf[65536];
			for(int i = 0;i < 200;i ++) {
				p = ttLibC_BeepGenerator_makeBeepBySampleNum(generator, pcm, samples_input / channel);
				if(p == NULL) {
					break;
				}
				pcm = p;
				size_t res = faacEncEncode(faacHandle, (int32_t *)pcm->inherit_super.inherit_super.data, samples_input, buf, 65536);
				if(res > 0) {
					if(fp) {
						fwrite(buf, 1, res, fp);
					}
				}
				ttLibC_HexUtil_dump(buf, res, true);
			}
		}
	}
	if(fp) {
		fclose(fp);
	}
	if(faacHandle) {
		faacEncClose(faacHandle);
	}
	ttLibC_PcmS16_close(&pcm);
	ttLibC_BeepGenerator_close(&generator);
#endif
}

static void mp3lameTest() {
	LOG_PRINT("mp3lameTest");
#ifdef __ENABLE_MP3LAME_ENCODE__
	LOG_PRINT("version:%s", get_lame_version());

	lame_global_flags *gflags = NULL;
	gflags = lame_init();

	if(gflags == NULL) {
		ERR_PRINT("failed to initialize lame.");
		return;
	}

	lame_set_num_channels(gflags, 2);
	lame_set_mode(gflags, STEREO);
	lame_set_in_samplerate(gflags, 44100);
	lame_set_out_samplerate(gflags, 44100);

	lame_set_quality(gflags, 5);
	lame_set_VBR(gflags, vbr_default);
	lame_set_VBR_quality(gflags, 5);
	lame_set_bWriteVbrTag(gflags, 0);

	lame_get_quality(gflags);
	lame_get_mode(gflags);
	if(lame_init_params(gflags) < 0) {
		ERR_PRINT("failed to setup lame.");
		return;
	}
	FILE *fp = fopen("output2.mp3", "wb");
	ttLibC_BeepGenerator *generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian_planar, 440, 44100, 2);
	ttLibC_PcmS16 *pcm = NULL, *p;
	for(int i = 0;i < 10;i ++) {
		p = ttLibC_BeepGenerator_makeBeepByMiliSec(generator, pcm, 1000);
		if(p == NULL) {
			break;
		}
		pcm = p;
		uint8_t buf[65536];
		short *right_buf = (short *)pcm->inherit_super.inherit_super.data;
		uint32_t size = lame_encode_buffer(gflags, (const short *)pcm->inherit_super.inherit_super.data, (const short *)(right_buf + pcm->inherit_super.sample_num), pcm->inherit_super.sample_num, buf, sizeof(buf));
		fwrite(buf, size, 1, fp);
//		ttLibC_HexUtil_dump(buf, size, true);
	}
	ttLibC_BeepGenerator_close(&generator);
	fclose(fp);
	lame_close(gflags);
#endif
}

/**
 * define all test for audio package.
 * @param s cute::suite obj
 * @return cute::suite obj
 */
cute::suite audioTests(cute::suite s) {
	s.clear();
	s.push_back(CUTE(speexdspResampleTest));
	s.push_back(CUTE(faacTest));
	s.push_back(CUTE(mp3lameTest));
	return s;
}
