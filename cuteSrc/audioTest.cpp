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

#ifdef __ENABLE_MP3LAME_ENCODE__
#	include <ttLibC/encoder/mp3lameEncoder.h>
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

#ifdef __ENABLE_SPEEX__
#	include <speex/speex.h>
#	include <speex/speex_header.h>
#	include <speex/speex_stereo.h>
#endif

#include <ttLibC/frame/audio/mp3.h>

#if defined(__ENABLE_MP3LAME__) && defined(__ENABLE_MP3LAME_ENCODE__) && defined(__ENABLE_OPENAL__)
static hip_t hip_gflags;
static ttLibC_PcmS16 *decoded_pcm;
static ttLibC_AlDevice *device;

void mp3EncodeProc(void *ptr, ttLibC_Mp3 *mp3) {
	LOG_PRINT("success to make mp3");
	short left[65536], right[65536];
	short data[65536];
	short *buf = data;
	// do decode.
//	hip_decode(hip_gflags, mp3->inherit_super.inherit_super.data, mp3->inherit_super.inherit_super.buffer_size);
	int num = hip_decode(hip_gflags, (unsigned char *)mp3->inherit_super.inherit_super.data, (size_t)mp3->inherit_super.inherit_super.buffer_size, (short *)left, (short *)right);
	LOG_PRINT("num:%d", num);
	for(int i = 0;i < num;++ i) {
		(*buf) = left[i];
		buf ++;
		(*buf) = right[i];
		buf ++;
	}
	ttLibC_PcmS16 *p = ttLibC_PcmS16_make(
			decoded_pcm, PcmS16Type_littleEndian,
			44100, 1152, 2, data, num * 4, true, mp3->inherit_super.inherit_super.pts, mp3->inherit_super.inherit_super.timebase);
	if(p == NULL) {
		ERR_PRINT("failed to make pcm data.");
		return;
	}
	decoded_pcm = p;
	ttLibC_AlDevice_queue(device, decoded_pcm);
	ttLibC_AlDevice_proceed(device, 10);
}
#endif

static void mp3DecodeTest() {
	LOG_PRINT("mp3decodeTest");
#if defined(__ENABLE_MP3LAME__) && defined(__ENABLE_MP3LAME_ENCODE__) && defined(__ENABLE_OPENAL__)
	hip_gflags = NULL;
	decoded_pcm = NULL;
	uint32_t sample_rate = 44100;
	uint32_t channel_num = 2;
	ttLibC_BeepGenerator *generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian, 440, sample_rate, channel_num);
	device = ttLibC_AlDevice_make(100);
	ttLibC_Mp3lameEncoder *encoder = ttLibC_Mp3lameEncoder_make(sample_rate, channel_num, 2);
	hip_gflags = hip_decode_init();
	if(!hip_gflags) {
		ERR_PRINT("failed to initialize hip.");
	}
	ttLibC_PcmS16 *pcm = NULL, *p;
	for(int i = 0;i < 10;i ++) {
		p = ttLibC_BeepGenerator_makeBeepByMiliSec(generator, pcm, 500);
		if(p == NULL) {
			break;
		}
		pcm = p;
		ttLibC_Mp3lameEncoder_encode(encoder, pcm, mp3EncodeProc, NULL);
//		ttLibC_AlDevice_queue(device, pcm);
//		ttLibC_AlDevice_proceed(device, 10);
	}
	ttLibC_AlDevice_proceed(device, -1);
	ttLibC_AlDevice_close(&device);
	ttLibC_PcmS16_close(&pcm);
	ttLibC_PcmS16_close(&decoded_pcm);
	if(hip_gflags) {
		hip_decode_exit(hip_gflags);
	}
	ttLibC_BeepGenerator_close(&generator);
#endif
}

static void speexTest() {
	LOG_PRINT("speexTest");
#if defined(__ENABLE_SPEEX__) && defined(__ENABLE_OPENAL__)
//	speex_lib_get_mode();
	uint32_t sample_rate = 8000;
	uint32_t channel_num = 1;
	ttLibC_BeepGenerator *generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian, 440, sample_rate, channel_num);
	ttLibC_PcmS16 *pcm = NULL, *p;

	// encoder
		SpeexBits bits;
		SpeexHeader header;
	uint8_t *header_data = NULL;
	int header_size;
		void *enc_state;

	// decoder
		SpeexBits bits_dec;
//		SpeexStereoState stereo;
		void *dec_state;

	LOG_PRINT("encoder_init");
	int spx_mode = 0;
	const SpeexMode *mode;
	switch(sample_rate) {
	case 8000:
		spx_mode = 0;
		break;
	case 16000:
		spx_mode = 1;
		break;
	case 32000:
		spx_mode = 2;
		break;
	default:
		LOG_PRINT("support only 8k 16k 32kHz.");
		return;
	}
	mode = speex_lib_get_mode(spx_mode);
	// encode
	enc_state = speex_encoder_init(mode);
	if(!enc_state) {
		LOG_PRINT("failed to initialize libspeex.");
		return;
	}
	speex_init_header(&header, 8000, 1, mode);
	LOG_PRINT("frame_size:%d", header.frame_size);
	// init bit writer.
	LOG_PRINT("bits_init");
	speex_bits_init(&bits);

	header_data = (uint8_t *)speex_header_to_packet(&header, &header_size);
	ttLibC_HexUtil_dump(header_data, header_size, true);
	speex_header_free(header_data);

	// decode
	speex_bits_init(&bits_dec);
	dec_state = speex_decoder_init(mode);

	uint8_t encoded_buf[65536];
	int16_t decoded_buf[65536];
//	int16_t *decoded_buf;
	for(int i = 0;i < 10;i ++) {
		p = ttLibC_BeepGenerator_makeBeepByMiliSec(generator, pcm, 1000);
		if(p == NULL) {
			break;
		}
		pcm = p;
		// output or input is 20 mili sec fixed.
		speex_encode_int(enc_state, (int16_t *)pcm->inherit_super.inherit_super.data, &bits);
		int write_size = speex_bits_write(&bits, (char *)encoded_buf, 65536);
		LOG_PRINT("write_size:%d", write_size);
		ttLibC_HexUtil_dump(encoded_buf, write_size, true);
		speex_bits_read_from(&bits_dec, (char *)encoded_buf, write_size);

		LOG_PRINT("read ok");
		// 20mili sec for decode.
		// 8kHz :160 samples
		// 16kHz:320 samples
		// 32kHz:640 samples
		int decode_size = speex_decode_int(dec_state, &bits_dec, decoded_buf);

//		LOG_PRINT("ptr:%d", decoded_buf);
		LOG_PRINT("decode_size:%d", decode_size);
		ttLibC_HexUtil_dump(decoded_buf, 10000, true);

		write_size = speex_bits_write(&bits, (char *)encoded_buf, 65536);
		LOG_PRINT("write_size:%d", write_size);
		ttLibC_HexUtil_dump(encoded_buf, write_size, true);
		speex_bits_read_from(&bits_dec, (char *)encoded_buf, write_size);

		LOG_PRINT("read ok");
		decode_size = speex_decode_int(dec_state, &bits_dec, decoded_buf);

//		LOG_PRINT("ptr:%d", decoded_buf);
		LOG_PRINT("decode_size:%d", decode_size);
		ttLibC_HexUtil_dump(decoded_buf, 10000, true);
		break;
	}

	LOG_PRINT("bits destroy");
	speex_bits_destroy(&bits_dec);
	speex_decoder_destroy(dec_state);
	// destroy bit write.
	speex_bits_destroy(&bits);
	LOG_PRINT("encoder destroy");
	// destroy libspeex.
	speex_encoder_destroy(enc_state);
	ttLibC_BeepGenerator_close(&generator);
	LOG_PRINT("all end.");
#endif
}

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
	s.push_back(CUTE(mp3DecodeTest));
	s.push_back(CUTE(speexTest));
	s.push_back(CUTE(speexdspResampleTest));
	s.push_back(CUTE(faacTest));
	s.push_back(CUTE(mp3lameTest));
	return s;
}
