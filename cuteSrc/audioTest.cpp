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

#ifdef __ENABLE_FAAD_DECODE__
#	include <faad.h>
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

#ifdef __ENABLE_OPUS__
#	include <opus/opus.h>
#endif

#include <ttLibC/resampler/audioResampler.h>

#include <ttLibC/frame/audio/mp3.h>
#include <ttLibC/frame/audio/speex.h>
#include <ttLibC/allocator.h>

#ifdef __ENABLE_VORBIS_ENCODE__
#	include <vorbis/vorbisenc.h>
#endif

#ifdef __ENABLE_VORBIS_DECODE__
#	include <vorbis/codec.h>
#endif

#ifdef __ENABLE_APPLE__
#	include <ttLibC/util/audioUnitUtil.h>
#endif

#ifdef __ENABLE_FDKAAC__
#	include <fdk-aac/aacenc_lib.h>
#	include <fdk-aac/aacdecoder_lib.h>
#endif

#include <stdio.h>
#include <unistd.h>

static void fdkaacTest() {
	LOG_PRINT("fdkaacTest");
#if defined(__ENABLE_FDKAAC__) && defined(__ENABLE_OPENAL__)
	uint32_t sample_rate = 44100, channel_num = 2, bitrate = 96000;
	ttLibC_BeepGenerator *generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian, 440, sample_rate, channel_num);
	ttLibC_AlDevice *device = ttLibC_AlDevice_make(256);
	ttLibC_PcmS16 *pcm = NULL, *p, *dpcm = NULL;
	HANDLE_AACENCODER encHandle;
	AACENC_InfoStruct info;
	AACENC_ERROR err;
	HANDLE_AACDECODER decHandle;
	AAC_DECODER_ERROR derr = AAC_DEC_OK;
	// encoder
	if((err = aacEncOpen(&encHandle, 0, channel_num)) != AACENC_OK) {
		ERR_PRINT("failed to open HANDLE_AACENCODER");
	}
	uint32_t value = AOT_AAC_LC;
	if(err == AACENC_OK && (err = aacEncoder_SetParam(encHandle, AACENC_AOT, value)) != AACENC_OK) {
		ERR_PRINT("failed to set profile.");
	}
	if(err == AACENC_OK && (err = aacEncoder_SetParam(encHandle, AACENC_SAMPLERATE, sample_rate)) != AACENC_OK) {
		ERR_PRINT("failed to set sample_rate.");
	}
	if(err == AACENC_OK) {
		switch(channel_num) {
		case 1:
			value = MODE_1;
			break;
		case 2:
			value = MODE_2;
			break;
		default:
			ERR_PRINT("unknown channel_num:%d", channel_num);
			err = AACENC_INIT_ERROR;
			break;
		}
	}
	if(err == AACENC_OK && (err = aacEncoder_SetParam(encHandle, AACENC_CHANNELMODE, value)) != AACENC_OK) {
		ERR_PRINT("failed to set channel_num.");
	}
	if(err == AACENC_OK && (err = aacEncoder_SetParam(encHandle, AACENC_CHANNELORDER, 1)) != AACENC_OK) {
		ERR_PRINT("failed to set channelorder.");
	}
	if(err == AACENC_OK && (err = aacEncoder_SetParam(encHandle, AACENC_BITRATE, bitrate)) != AACENC_OK) {
		ERR_PRINT("failed to set bitrate.");
	}
	if(err == AACENC_OK && (err = aacEncoder_SetParam(encHandle, AACENC_TRANSMUX, 2)) != AACENC_OK) {
		ERR_PRINT("failed to set bitrate.");
	}
	if(err == AACENC_OK && (err = aacEncEncode(encHandle, NULL, NULL, NULL, NULL)) != AACENC_OK) {
		ERR_PRINT("failed to initialize encoder.");
	}
	if(err == AACENC_OK && (err = aacEncInfo(encHandle, &info)) != AACENC_OK) {
		ERR_PRINT("failed to get info.");
	}
	if(err != AACENC_OK) {
		if(encHandle) {
			aacEncClose(&encHandle);
			encHandle = NULL;
		}
	}

	// decoder
	decHandle = aacDecoder_Open(TT_MP4_ADTS, 1);
	if(decHandle == NULL) {
		ERR_PRINT("failed to open decoder handle.");
		derr = AAC_DEC_INVALID_HANDLE;
	}
	if(derr != AAC_DEC_OK) {
		if(decHandle != NULL) {
			aacDecoder_Close(decHandle);
			decHandle = NULL;
		}
	}
	for(int i = 0;i < 100;i ++) {
		if(encHandle == NULL || decHandle == NULL) {
			break;
		}
		p = ttLibC_BeepGenerator_makeBeepBySampleNum(generator, pcm, info.frameLength);
		if(p == NULL) {
			break;
		}
		pcm = p;
		AACENC_BufDesc in_buf = {}, out_buf = {};
		AACENC_InArgs  in_args = {};
		AACENC_OutArgs out_args = {};
		int in_identifier = IN_AUDIO_DATA;
		int in_elem_size = sizeof(int16_t);
		int out_identifier = OUT_BITSTREAM_DATA;
		uint8_t out[20480];
		void *out_ptr = out;
		int out_size = 20480;
		int out_elem_size = 1;

		in_args.numInSamples = info.frameLength * channel_num;
		in_buf.numBufs = 1;
		in_buf.bufferIdentifiers = &in_identifier;
		in_buf.bufs = &pcm->inherit_super.inherit_super.data;
		in_buf.bufSizes = (int32_t *)&pcm->inherit_super.inherit_super.buffer_size;
		in_buf.bufElSizes = &in_elem_size;

		out_buf.numBufs = 1;
		out_buf.bufferIdentifiers = &out_identifier;
		out_buf.bufs = &out_ptr;
		out_buf.bufSizes = &out_size;
		out_buf.bufElSizes = &out_elem_size;

		if((err = aacEncEncode(encHandle, &in_buf, &out_buf, &in_args, &out_args)) != AACENC_OK) {
			ERR_PRINT("failed to encode.");
			break;
		}
		// done.
//		LOG_DUMP(out, out_args.numOutBytes, true);
		uint32_t size = out_args.numOutBytes;
		uint32_t valid = out_args.numOutBytes;
		uint8_t *in_packet_ptr = (uint8_t *)out;
		derr = aacDecoder_Fill(decHandle, &in_packet_ptr, &size, &valid);
		if(derr != AAC_DEC_OK) {
			ERR_PRINT("failed to fill aac data.");
			break;
		}
		int16_t decodeBuffer[20480];
		int16_t *decode_buf = decodeBuffer;
		derr = aacDecoder_DecodeFrame(decHandle, decode_buf, sizeof(decodeBuffer), 0);
		if(derr == AAC_DEC_NOT_ENOUGH_BITS) {
			ERR_PRINT("need more bits");
			continue;
		}
		if(derr != AAC_DEC_OK) {
			ERR_PRINT("failed to decode frame.");
			break;
		}
		p = ttLibC_PcmS16_make(
				dpcm,
				PcmS16Type_littleEndian,
				sample_rate,
				1024,
				channel_num,
				decode_buf,
				1024 * channel_num * 2,
				decode_buf,
				1024 * channel_num * 2,
				NULL,
				0,
				false,
				pcm->inherit_super.inherit_super.pts,
				pcm->inherit_super.inherit_super.timebase);
		if(p == NULL) {
			ERR_PRINT("failed to make decoded pcm object.");
			break;
		}
		dpcm = p;
		// try to decode.
		ttLibC_AlDevice_queue(device, dpcm);
	}
	ttLibC_AlDevice_proceed(device, -1);
	if(encHandle) {
		aacEncClose(&encHandle);
		encHandle = NULL;
	}
	if(decHandle) {
		aacDecoder_Close(decHandle);
		decHandle = NULL;
	}
	ttLibC_AlDevice_close(&device);
	ttLibC_PcmS16_close(&dpcm);
	ttLibC_PcmS16_close(&pcm);
	ttLibC_BeepGenerator_close(&generator);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

static void vorbisTest() {
	LOG_PRINT("vorbisTest");
#if defined(__ENABLE_VORBIS_ENCODE__) && defined(__ENABLE_VORBIS_DECODE__)
	ttLibC_BeepGenerator *generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian, 440, 44100, 1);
	ttLibC_PcmS16 *pcm = NULL, *p;
	generator->amplitude = 30000;

	// decoder test
	vorbis_info dec_vi;
	vorbis_comment dec_vc;
	vorbis_dsp_state dec_vd;
	vorbis_block dec_vb;
	char *buffer;
	int bytes;

	// encoder test
	ogg_packet op;
	vorbis_info vi;
	vorbis_comment vc;
	vorbis_dsp_state vd;
	vorbis_block vb;

	vorbis_info_init(&vi);
	int ret = vorbis_encode_init_vbr(&vi, 1, 44100, 0.4);
	if(ret != 0) {
		LOG_PRINT("failed to init vorbis.");
	}
	vorbis_comment_init(&vc);
	vorbis_comment_add_tag(&vc, "ENCODER", "ttLibC_test");

	vorbis_analysis_init(&vd, &vi);
	vorbis_block_init(&vd, &vb);

	vorbis_info_init(&dec_vi);
	vorbis_comment_init(&dec_vc);
	ogg_packet header, header_comm, header_code;
	vorbis_analysis_headerout(&vd, &vc, &header, &header_comm, &header_code);

	LOG_DUMP(header.packet, header.bytes, true);
	if(vorbis_synthesis_headerin(&dec_vi, &dec_vc, &header) < 0) {
		LOG_PRINT("failed to read vorbis header.");
	}
	LOG_DUMP(header_comm.packet, header_comm.bytes, true);
	if(vorbis_synthesis_headerin(&dec_vi, &dec_vc, &header_comm) < 0) {
		LOG_PRINT("failed to read vorbis header.");
	}
	LOG_DUMP(header_code.packet, header_code.bytes, true);
	if(vorbis_synthesis_headerin(&dec_vi, &dec_vc, &header_code) < 0) {
		LOG_PRINT("failed to read vorbis header.");
	}
	if(vorbis_synthesis_init(&dec_vd, &dec_vi) == 0) {
		LOG_PRINT("decoder init OK");
		vorbis_block_init(&dec_vd, &dec_vb);
	}

	for(int i = 0;i < 5;++ i) {
		p = ttLibC_BeepGenerator_makeBeepByMiliSec(generator, pcm, 500);
		if(p == NULL) {
			break;
		}
		pcm = p;
		float **buffer = vorbis_analysis_buffer(&vd, p->inherit_super.sample_num);
		int16_t *data = (int16_t *)pcm->inherit_super.inherit_super.data;
		int j = 0;
		for(j = 0;j < pcm->inherit_super.sample_num;++ j) {
			buffer[0][j] = (*data) / 32768.f;
			++ data;
		}
		vorbis_analysis_wrote(&vd, j);
		while(vorbis_analysis_blockout(&vd, &vb) == 1) {
			vorbis_analysis(&vb, NULL);
			vorbis_bitrate_addblock(&vb);

			while(vorbis_bitrate_flushpacket(&vd, &op)) {
				LOG_DUMP(op.packet, op.bytes, true);
				if(vorbis_synthesis(&dec_vb, &op) == 0) {
					LOG_PRINT("decode try, ok");
					vorbis_synthesis_blockin(&dec_vd, &dec_vb);
				}
				float **pcm;
				int samples;
				while((samples = vorbis_synthesis_pcmout(&dec_vd, &pcm)) > 0) {
					LOG_PRINT("decoded, samples:%d", samples);
					vorbis_synthesis_read(&dec_vd, samples);
				}
			}
		}
	}
	vorbis_block_clear(&dec_vb);
	vorbis_dsp_clear(&dec_vd);
	vorbis_comment_clear(&dec_vc);
	vorbis_info_clear(&dec_vi);

	vorbis_block_clear(&vb);
	vorbis_dsp_clear(&vd);
	vorbis_comment_clear(&vc);
	vorbis_info_clear(&vi);
	ttLibC_BeepGenerator_close(&generator);
	ttLibC_PcmS16_close(&pcm);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

static void faadTest() {
	LOG_PRINT("faadTest");
#if defined(__ENABLE_FAAD_DECODE__)
	NeAACDecHandle hDecoder;
	NeAACDecFrameInfo frameInfo;
	NeAACDecConfigurationPtr config;

	hDecoder = NeAACDecOpen();
	config = NeAACDecGetCurrentConfiguration(hDecoder);
	config->defObjectType = MAIN;
	config->outputFormat = FAAD_FMT_16BIT;
	config->downMatrix = 0;
	config->useOldADTSFormat = 1;
	NeAACDecSetConfiguration(hDecoder, config);
	uint8_t buf[256];
	size_t size = ttLibC_HexUtil_makeBuffer("1210", buf, 256);
	unsigned long sample_rate;
	uint8_t channel_num;
	NeAACDecInit2(hDecoder, buf, size, &sample_rate, &channel_num);
	LOG_PRINT("sample_rate:%lu channel_num:%d", sample_rate, channel_num);
	size = ttLibC_HexUtil_makeBuffer("210049900219002380", buf, 256);
	int16_t *data = NULL;
	data = (int16_t *)NeAACDecDecode(hDecoder, &frameInfo, buf, size);
	LOG_PRINT("returned samples:%lu", frameInfo.samples);
	data = (int16_t *)NeAACDecDecode(hDecoder, &frameInfo, buf, size);
	LOG_PRINT("returned samples:%lu", frameInfo.samples);
	data = (int16_t *)NeAACDecDecode(hDecoder, &frameInfo, buf, size);
	LOG_PRINT("returned samples:%lu", frameInfo.samples);
	data = (int16_t *)NeAACDecDecode(hDecoder, &frameInfo, buf, size);
	LOG_PRINT("returned samples:%lu", frameInfo.samples);
	data = (int16_t *)NeAACDecDecode(hDecoder, &frameInfo, buf, size);
	LOG_PRINT("returned samples:%lu", frameInfo.samples);
	data = (int16_t *)NeAACDecDecode(hDecoder, &frameInfo, buf, size);
	LOG_PRINT("returned samples:%lu", frameInfo.samples);
	data = (int16_t *)NeAACDecDecode(hDecoder, &frameInfo, buf, size);
	LOG_PRINT("returned samples:%lu", frameInfo.samples);
	NeAACDecClose(hDecoder);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

static void audioResamplerTest() {
	LOG_PRINT("audioResamplerTest");
	int16_t data[10] = {32767, 30000, 10000, 13000, 0, 0, -10000, -13000, -32767, -30000};
	ttLibC_PcmS16 *pcms16 = ttLibC_PcmS16_make(
			NULL,
			PcmS16Type_littleEndian,
			5,
			5,
			2,
			&data,
			10,
			&data,
			10,
			NULL,
			0,
			true,
			0,
			1000);
/*	ttLibC_PcmF32 *pcmf32 = (ttLibC_PcmF32 *)ttLibC_AudioResampler_convertFormat(NULL, frameType_pcmF32, PcmF32Type_interleave, 1, (ttLibC_Audio *)pcms16);
	float *f = (float *)pcmf32->inherit_super.inherit_super.data;
	for(int i = 0;i < 5;i ++) {
		LOG_PRINT("val:%f", f[i]);
	}
	ttLibC_PcmF32_close(&pcmf32);*/

	ttLibC_PcmS16 *pcms16_out = (ttLibC_PcmS16 *)ttLibC_AudioResampler_convertFormat(NULL, frameType_pcmS16, PcmS16Type_littleEndian, 1, (ttLibC_Audio *)pcms16);
	int16_t *dat = (int16_t *)pcms16_out->inherit_super.inherit_super.data;
	for(int i = 0;i < 5;i ++) {
		LOG_PRINT("val:%d", dat[i]);
	}
	ttLibC_PcmS16_close(&pcms16_out);
	ttLibC_PcmS16_close(&pcms16);
	ASSERT(ttLibC_Allocator_dump() == 0);
}

static void opusTest() {
	LOG_PRINT("opusTest");
#if defined(__ENABLE_OPUS__) && defined(__ENABLE_OPENAL__)
	uint32_t sample_rate = 48000, channel_num = 2;
	ttLibC_BeepGenerator *generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian, 440, sample_rate, channel_num);
	ttLibC_AlDevice *device = ttLibC_AlDevice_make(256);
	ttLibC_PcmS16 *pcm = NULL, *p, *dpcm = NULL;
	OpusEncoder *encoder = NULL;
	OpusDecoder *decoder = NULL;
	int error;
	encoder = opus_encoder_create(sample_rate, channel_num, OPUS_APPLICATION_RESTRICTED_LOWDELAY, &error);
	if(error != OPUS_OK) {
		ERR_PRINT("error happen during opus encoder create:%s", opus_strerror(error));
		encoder = NULL;
	}
	else if(encoder == NULL) {
		ERR_PRINT("failed to make opusEncoder");
	}
	decoder = opus_decoder_create(sample_rate, channel_num, &error);
	if(error != OPUS_OK) {
		ERR_PRINT("error happen during opus decoder create:%s", opus_strerror(error));
		decoder = NULL;
	}
	else if(decoder == NULL){
		ERR_PRINT("failed to make opusDecoder");
	}
	for(int i = 0;i < 100;i ++) {
		if(encoder == NULL || decoder == NULL) {
			break;
		}
		p = ttLibC_BeepGenerator_makeBeepBySampleNum(generator, pcm, 480);
		if(p == NULL) {
			break;
		}
		pcm = p;
		// generator opus, decode, and play.
//		ttLibC_AlDevice_queue(device, pcm);
		uint8_t opus[48000];
		int size = opus_encode(encoder, (int16_t *)pcm->inherit_super.inherit_super.data, 480, opus, 48000);
		if(size == 0) {
			ERR_PRINT("failed to make opus.");
			break;
		}
		int16_t data[4800];
		size = opus_decode(decoder, opus, size, data, 4800, 0);
		LOG_PRINT("decode size:%d", size);
		if(size == 0) {
			break;
		}
		p = ttLibC_PcmS16_make(
				dpcm,
				PcmS16Type_littleEndian,
				sample_rate,
				480,
				channel_num,
				data,
				size * channel_num * sizeof(int16_t),
				data,
				size * channel_num * sizeof(int16_t),
				NULL,
				0,
				true,
				0,
				1000);
		if(p == NULL) {
			break;
		}
		dpcm = p;
		ttLibC_AlDevice_queue(device, dpcm);
	}
	if(encoder != NULL) {
		opus_encoder_destroy(encoder);
	}
	if(decoder != NULL) {
		opus_decoder_destroy(decoder);
	}
	ttLibC_PcmS16_close(&dpcm);
	ttLibC_PcmS16_close(&pcm);
	ttLibC_AlDevice_proceed(device, -1);
	ttLibC_AlDevice_close(&device);
	ttLibC_BeepGenerator_close(&generator);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

#if defined(__ENABLE_MP3LAME__) && defined(__ENABLE_MP3LAME_ENCODE__) && defined(__ENABLE_OPENAL__)
static hip_t hip_gflags;
static ttLibC_PcmS16 *decoded_pcm;
static ttLibC_AlDevice *device;

bool mp3EncodeProc(void *ptr, ttLibC_Mp3 *mp3) {
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
			decoded_pcm,
			PcmS16Type_littleEndian,
			44100,
			1152,
			2,
			data,
			num * 4,
			data,
			1152 * 2 * 2,
			NULL,
			0,
			true,
			mp3->inherit_super.inherit_super.pts,
			mp3->inherit_super.inherit_super.timebase);
	if(p == NULL) {
		ERR_PRINT("failed to make pcm data.");
		return false;
	}
	decoded_pcm = p;
	ttLibC_AlDevice_queue(device, decoded_pcm);
	ttLibC_AlDevice_proceed(device, 10);
	return true;
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
	ttLibC_Mp3lameEncoder_close(&encoder);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

static void speexFrameTest() {
	LOG_PRINT("speexFrameTest");
	char buffer[512];
	ttLibC_Speex *spx = NULL;
	size_t buffer_size = 0;
	// header
/*	buffer_size = ttLibC_HexUtil_makeBuffer("53 70 65 65 78 20 20 20 31 2E 32 72 63 31 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 00 00 00 50 00 00 00 00 7D 00 00 02 00 00 00 04 00 00 00 01 00 00 00 A0 73 00 00 80 02 00 00 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00", buffer, 512);
	spx = ttLibC_Speex_getFrame(spx, buffer, buffer_size, true, 0, 32000);
	if(spx != NULL) {
		LOG_PRINT("type:%d", spx->type);
	}*/
	// comment
	buffer_size = ttLibC_HexUtil_makeBuffer("0D 00 00 00 4C 61 76 66 35 37 2E 34 31 2E 31 30 30 0A 00 00 00 1E 00 00 00 65 6E 63 6F 64 65 72 3D 4C 61 76 63 35 37 2E 34 38 2E 31 30 31 20 6C 69 62 73 70 65 65 78 0B 00 00 00 73 74 61 72 74 74 69 6D 65 3D 30 11 00 00 00 74 6F 74 61 6C 64 75 72 61 74 69 6F 6E 3D 31 33 32 11 00 00 00 74 6F 74 61 6C 64 61 74 61 72 61 74 65 3D 39 31 34 13 00 00 00 62 79 74 65 6C 65 6E 67 74 68 3D 31 35 30 39 36 31 31 37 12 00 00 00 63 61 6E 73 65 65 6B 6F 6E 74 69 6D 65 3D 74 72 75 65 26 00 00 00 73 6F 75 72 63 65 64 61 74 61 3D 42 44 41 42 30 46 32 30 37 48 48 31 33 31 36 33 31 32 33 32 37 39 38 36 32 36 31 05 00 00 00 70 75 72 6C 3D 05 00 00 00 70 6D 73 67 3D 47 00 00 00 68 74 74 70 68 6F 73 74 68 65 61 64 65 72 3D 6F 2D 6F 2E 70 72 65 66 65 72 72 65 64 2E 73 6F 66 74 62 61 6E 6B 62 62 2D 68 6E 64 31 2E 76 32 30 2E 6C 73 63 61 63 68 65 33 2E 63 2E 79 6F 75 74 75 62 65 2E 63 6F 6D", buffer, 512);
	spx = ttLibC_Speex_getFrame(spx, buffer, buffer_size, true, 0, 32000);
	if(spx != NULL) {
		LOG_PRINT("type:%d", spx->type);
	}
	// frame n w sw
	buffer_size = ttLibC_HexUtil_makeBuffer("36 9D 1B 9A 20 00 01 68 E8 E8 E8 E8 E8 E8 E8 84 00 B4 74 74 74 74 74 74 74 42 00 5A 3A 3A 3A 3A 3A 3A 3A 21 00 2D 1D 1D 1D 1D 1D 1D 1D 1B 3B 60 AB AB AB AB AB 0A BA BA BA BA B0 AB AB AB AB AB 0A BA BA BA BA B9 3B 60 00 00", buffer, 512);
	spx = ttLibC_Speex_getFrame(spx, buffer, buffer_size, true, 0, 32000);
	if(spx != NULL) {
		LOG_PRINT("type:%d rate:%d num:%d", spx->type, spx->inherit_super.sample_rate, spx->inherit_super.sample_num);
	}
	// frame n w
	buffer_size = ttLibC_HexUtil_makeBuffer("36 9D 1B 9A 20 00 01 68 E8 E8 E8 E8 E8 E8 E8 84 00 B4 74 74 74 74 74 74 74 42 00 5A 3A 3A 3A 3A 3A 3A 3A 21 00 2D 1D 1D 1D 1D 1D 1D 1D 1B 3B 60 AB AB AB AB AB 0A BA BA BA BA B0 AB AB AB AB AB 0A BA BA BA BA B0", buffer, 512);
	spx = ttLibC_Speex_getFrame(spx, buffer, buffer_size, true, 0, 32000);
	if(spx != NULL) {
		LOG_PRINT("type:%d rate:%d num:%d", spx->type, spx->inherit_super.sample_rate, spx->inherit_super.sample_num);
	}
	ttLibC_Speex_close(&spx);
	ASSERT(ttLibC_Allocator_dump() == 0);
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
	ttLibC_PcmS16_close(&pcm);
	ttLibC_BeepGenerator_close(&generator);
	LOG_PRINT("all end.");
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
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
		ttLibC_PcmS16 *resampled_pcm = ttLibC_PcmS16_make(
				NULL,
				PcmS16Type_littleEndian,
				output_sample_rate,
				out_size,
				channel,
				resampled_data,
				out_size * sizeof(int16_t) * 2,
				resampled_data,
				out_size * sizeof(int16_t) * 2,
				NULL,
				0,
				true,
				0,
				1000);
		LOG_PRINT("sample_num:%d rate:%d", resampled_pcm->inherit_super.sample_num, resampled_pcm->inherit_super.sample_rate);
		LOG_PRINT("sample_num:%d rate:%d", pcm->inherit_super.sample_num, pcm->inherit_super.sample_rate);
		ttLibC_AlDevice_queue(device, resampled_pcm);
//		ttLibC_AlDevice_proceed(device, 1);
		ttLibC_PcmS16_close(&resampled_pcm);
	}
	ttLibC_AlDevice_proceed(device, -1);
	ttLibC_AlDevice_close(&device);
	ttLibC_PcmS16_close(&pcm);
	ttLibC_BeepGenerator_close(&generator);
	speex_resampler_destroy(resampler);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
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
	ASSERT(ttLibC_Allocator_dump() == 0);
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
	ttLibC_PcmS16_close(&pcm);
	ttLibC_BeepGenerator_close(&generator);
	fclose(fp);
	lame_close(gflags);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

static void pcmF32CloneTest() {
	LOG_PRINT("pcmF32CloneTest");
#ifdef __ENABLE_APPLE__
	ttLibC_AuPlayer *player = ttLibC_AuPlayer_make(48000, 2, AuPlayerType_DefaultOutput);
	ttLibC_BeepGenerator *generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian_planar, 440, 48000, 2);
	ttLibC_PcmS16 *pcm = NULL, *p;
	ttLibC_PcmS16 *ppcm = NULL;
	ttLibC_PcmF32 *fpcm = NULL, *cfpcm = NULL, *f;
	for(int i = 0;i < 100;i ++) {
		p = ttLibC_BeepGenerator_makeBeepBySampleNum(generator, pcm, 480);
		if(p == NULL) {
			break;
		}
		pcm = p;
		f = ttLibC_AudioResampler_makePcmF32FromPcmS16(fpcm, PcmF32Type_planar, pcm);
		if(f == NULL) {
			break;
		}
		fpcm = f;
		f = ttLibC_PcmF32_clone(cfpcm, fpcm);
		if(f == NULL) {
			break;
		}
		cfpcm = f;
		p = (ttLibC_PcmS16 *)ttLibC_AudioResampler_convertFormat((ttLibC_Audio *)ppcm, frameType_pcmS16, PcmS16Type_littleEndian, 2, (ttLibC_Audio *)cfpcm);
		if(p == NULL) {
			break;
		}
		ppcm = p;
		while(!ttLibC_AuPlayer_queue(player, ppcm)) {
			usleep(100);
		}
	}
	sleep(1);
	ttLibC_AuPlayer_close(&player);
	ttLibC_PcmS16_close(&pcm);
	ttLibC_PcmF32_close(&fpcm);
	ttLibC_PcmF32_close(&cfpcm);
	ttLibC_PcmS16_close(&ppcm);
	ttLibC_BeepGenerator_close(&generator);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

static void pcmS16CloneTest() {
	LOG_PRINT("pcmS16CloneTest");
#ifdef __ENABLE_APPLE__
	ttLibC_AuPlayer *player = ttLibC_AuPlayer_make(48000, 2, AuPlayerType_DefaultOutput);
	ttLibC_BeepGenerator *generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian, 440, 48000, 2);
	ttLibC_PcmS16 *pcm = NULL, *cpcm = NULL, *p;
	ttLibC_PcmS16 *ppcm = NULL;
	for(int i = 0;i < 100;i ++) {
		p = ttLibC_BeepGenerator_makeBeepBySampleNum(generator, pcm, 480);
		if(p == NULL) {
			break;
		}
		pcm = p;
		p = ttLibC_PcmS16_clone(cpcm, pcm);
		if(p == NULL) {
			break;
		}
		cpcm = p;
		p = (ttLibC_PcmS16 *)ttLibC_AudioResampler_convertFormat((ttLibC_Audio *)ppcm, frameType_pcmS16, PcmS16Type_littleEndian, 2, (ttLibC_Audio *)cpcm);
		if(p == NULL) {
			break;
		}
		ppcm = p;
		while(!ttLibC_AuPlayer_queue(player, ppcm)) {
			usleep(100);
		}
	}
	sleep(1);
	ttLibC_AuPlayer_close(&player);
	ttLibC_PcmS16_close(&pcm);
	ttLibC_PcmS16_close(&cpcm);
	ttLibC_PcmS16_close(&ppcm);
	ttLibC_BeepGenerator_close(&generator);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

/**
 * define all test for audio package.
 * @param s cute::suite obj
 * @return cute::suite obj
 */
cute::suite audioTests(cute::suite s) {
	s.clear();
	s.push_back(CUTE(fdkaacTest));
	s.push_back(CUTE(vorbisTest));
	s.push_back(CUTE(faadTest));
	s.push_back(CUTE(audioResamplerTest));
	s.push_back(CUTE(opusTest));
	s.push_back(CUTE(mp3DecodeTest));
	s.push_back(CUTE(speexFrameTest));
	s.push_back(CUTE(speexTest));
	s.push_back(CUTE(speexdspResampleTest));
	s.push_back(CUTE(faacTest));
	s.push_back(CUTE(mp3lameTest));
	s.push_back(CUTE(pcmF32CloneTest));
	s.push_back(CUTE(pcmS16CloneTest));
	return s;
}
