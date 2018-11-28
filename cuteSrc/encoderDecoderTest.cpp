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

#ifdef __ENABLE_AVCODEC__
#	include <ttLibC/decoder/avcodecDecoder.h>
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

#ifdef __ENABLE_X265__
#	include <ttLibC/encoder/x265Encoder.h>
#endif

#ifdef __ENABLE_APPLE__
#	include <ttLibC/util/audioUnitUtil.h>
#	include <ttLibC/encoder/audioConverterEncoder.h>
#	include <ttLibC/decoder/audioConverterDecoder.h>
#	include <ttLibC/encoder/vtCompressSessionEncoder.h>
#	include <ttLibC/decoder/vtDecompressSessionDecoder.h>
#endif

#ifdef __ENABLE_THEORA__
#	include <ttLibC/encoder/theoraEncoder.h>
#	include <ttLibC/decoder/theoraDecoder.h>
#endif

#ifdef __ENABLE_VORBIS_ENCODE__
#	include <ttLibC/encoder/vorbisEncoder.h>
#endif

#ifdef __ENABLE_VORBIS_DECODE__
#	include <ttLibC/decoder/vorbisDecoder.h>
#endif

#ifdef __ENABLE_LIBYUV__
#	include <ttLibC/resampler/libyuvResampler.h>
#endif

#ifdef __ENABLE_SWSCALE__
#	include <ttLibC/resampler/swscaleResampler.h>
#endif

#ifdef __ENABLE_SWRESAMPLE__
#	include <ttLibC/resampler/swresampleResampler.h>
#endif

#ifdef __ENABLE_LIBPNG__
#	include <ttLibC/decoder/pngDecoder.h>
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
#include <ttLibC/frame/video/h265.h>
#include <ttLibC/frame/video/jpeg.h>
#include <ttLibC/frame/video/png.h>

#include <ttLibC/resampler/imageResampler.h>
#include <ttLibC/resampler/imageResizer.h>
#include <ttLibC/resampler/audioResampler.h>

#include <ttLibC/util/hexUtil.h>

#include <unistd.h>

#if defined(__ENABLE_LIBPNG__) && defined(__ENABLE_JPEG__) && defined(__ENABLE_SWSCALE__)

static bool pngDecodeBinaryTest2_encodeCallback(void *ptr, ttLibC_Jpeg *jpeg) {
	FILE *fp = fopen("./cuteSrc/output_png2.jpeg", "wb");
	if(fp) {
		fwrite(jpeg->inherit_super.inherit_super.data, 1, jpeg->inherit_super.inherit_super.buffer_size, fp);
		fclose(fp);
	}
	return true;
}

static bool pngDecodeBinaryTest2_scaleCallback(void *ptr, ttLibC_Frame *frame) {
	ttLibC_Yuv420 *yuv = (ttLibC_Yuv420 *)frame;
	LOG_PRINT("%d %d %d", yuv->y_stride, yuv->u_stride, yuv->v_stride);
	ttLibC_JpegEncoder *encoder = ttLibC_JpegEncoder_make(200, 200, 90);
	ttLibC_JpegEncoder_encode(encoder, yuv, pngDecodeBinaryTest2_encodeCallback, NULL);
	ttLibC_JpegEncoder_close(&encoder);
	return true;
}

static bool pngDecodeBinaryTest2_decodeCallback(void *ptr, ttLibC_Bgr *frame) {
	ttLibC_Bgr *bgr = frame;
	ttLibC_SwscaleResampler *resampler = ttLibC_SwscaleResampler_make(bgr->inherit_super.inherit_super.type, bgr->type, bgr->inherit_super.width, bgr->inherit_super.height,
		frameType_yuv420, Yuv420Type_planar, bgr->inherit_super.width, bgr->inherit_super.height,
		SwscaleResampler_Bilinear);
	ttLibC_SwscaleResampler_resample(resampler, (ttLibC_Frame *)frame, pngDecodeBinaryTest2_scaleCallback, NULL);
	ttLibC_SwscaleResampler_close(&resampler);
	return true;
}
#endif

static void pngDecodeBinaryTest2() {
	LOG_PRINT("pngDecodeBinaryTest2");
#if defined(__ENABLE_LIBPNG__) && defined(__ENABLE_JPEG__) && defined(__ENABLE_SWSCALE__)
	FILE *fp = fopen("./cuteSrc/target_p.png", "rb");
	if(fp) {
		fpos_t fsize;
		fseek(fp,0,SEEK_END);
		fgetpos(fp,&fsize);
		uint8_t *buf = new uint8_t[fsize];
		fseek(fp,0,SEEK_SET);
		fread(buf, 1, fsize, fp);
		ttLibC_Png *png = ttLibC_Png_getFrame(NULL, buf, fsize, false, 0, 1000);
		ttLibC_PngDecoder *decoder = ttLibC_PngDecoder_make();
		ttLibC_PngDecoder_decode(decoder, png, pngDecodeBinaryTest2_decodeCallback, NULL);
		ttLibC_PngDecoder_close(&decoder);
		ttLibC_Png_close(&png);
		delete[] buf;
		fclose(fp);
	}
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}


#if defined(__ENABLE_AVCODEC__) && defined(__ENABLE_JPEG__) && defined(__ENABLE_SWSCALE__)

static bool pngDecodeBinaryTest_encodeCallback(void *ptr, ttLibC_Jpeg *jpeg) {
	FILE *fp = fopen("./cuteSrc/output_png.jpeg", "wb");
	if(fp) {
		fwrite(jpeg->inherit_super.inherit_super.data, 1, jpeg->inherit_super.inherit_super.buffer_size, fp);
		fclose(fp);
	}
	return true;
}

static bool pngDecodeBinaryTest_scaleCallback(void *ptr, ttLibC_Frame *frame) {
	ttLibC_Yuv420 *yuv = (ttLibC_Yuv420 *)frame;
	LOG_PRINT("%d %d %d", yuv->y_stride, yuv->u_stride, yuv->v_stride);
	ttLibC_JpegEncoder *encoder = ttLibC_JpegEncoder_make(200, 200, 90);
	ttLibC_JpegEncoder_encode(encoder, yuv, pngDecodeBinaryTest_encodeCallback, NULL);
	ttLibC_JpegEncoder_close(&encoder);
	return true;
}

static bool pngDecodeBinaryTest_decodeCallback(void *ptr, ttLibC_Frame *frame) {
	ttLibC_Bgr *bgr = (ttLibC_Bgr *)frame;
	ttLibC_SwscaleResampler *resampler = ttLibC_SwscaleResampler_make(bgr->inherit_super.inherit_super.type, bgr->type, bgr->inherit_super.width, bgr->inherit_super.height,
		frameType_yuv420, Yuv420Type_planar, bgr->inherit_super.width, bgr->inherit_super.height,
		SwscaleResampler_Bilinear);
	ttLibC_SwscaleResampler_resample(resampler, frame, pngDecodeBinaryTest_scaleCallback, NULL);
	ttLibC_SwscaleResampler_close(&resampler);
	return true;
}
#endif

static void pngDecodeBinaryTest() {
	LOG_PRINT("pngDecodeBinaryTest");
#if defined(__ENABLE_AVCODEC__) && defined(__ENABLE_JPEG__) && defined(__ENABLE_SWSCALE__)
	FILE *fp = fopen("./cuteSrc/target_p.png", "rb");
	if(fp) {
		fpos_t fsize;
		fseek(fp,0,SEEK_END);
		fgetpos(fp,&fsize);
		uint8_t *buf = new uint8_t[fsize];
		fseek(fp,0,SEEK_SET);
		fread(buf, 1, fsize, fp);
		ttLibC_Png *png = ttLibC_Png_getFrame(NULL, buf, fsize, false, 0, 1000);
		ttLibC_AvcodecDecoder *decoder = ttLibC_AvcodecVideoDecoder_make(frameType_png, png->inherit_super.width, png->inherit_super.height);
		ttLibC_AvcodecDecoder_decode(decoder, (ttLibC_Frame *)png, pngDecodeBinaryTest_decodeCallback, NULL);
		ttLibC_AvcodecDecoder_close(&decoder);
		ttLibC_Png_close(&png);
		delete[] buf;
		fclose(fp);
	}
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

#if defined(__ENABLE_AVCODEC__) && defined(__ENABLE_JPEG__)

static bool jpegDecodeBinaryTest_encodeCallback(void *ptr, ttLibC_Jpeg *jpeg) {
	FILE *fp = fopen("./cuteSrc/output_jpeg.jpeg", "wb");
	if(fp) {
		fwrite(jpeg->inherit_super.inherit_super.data, 1, jpeg->inherit_super.inherit_super.buffer_size, fp);
		fclose(fp);
	}
	return true;
}

static bool jpegDecodeBinaryTest_decodeCallback(void *ptr, ttLibC_Frame *frame) {
	ttLibC_Yuv420 *yuv = (ttLibC_Yuv420 *)frame;
	LOG_PRINT("%d %d %d", yuv->y_stride, yuv->u_stride, yuv->v_stride);
	ttLibC_JpegEncoder *encoder = ttLibC_JpegEncoder_make(200, 200, 90);
	ttLibC_JpegEncoder_encode(encoder, yuv, jpegDecodeBinaryTest_encodeCallback, NULL);
	ttLibC_JpegEncoder_close(&encoder);
	return true;
}
#endif

static void jpegDecodeBinaryTest() {
	LOG_PRINT("jpegDecodeBinaryTest");
#if defined(__ENABLE_AVCODEC__) && defined(__ENABLE_JPEG__)
	FILE *fp = fopen("./cuteSrc/target.jpeg", "rb");
	if(fp) {
		fpos_t fsize;
		fseek(fp,0,SEEK_END);
		fgetpos(fp,&fsize);
		uint8_t *buf = new uint8_t[fsize];
		fseek(fp,0,SEEK_SET);
		fread(buf, 1, fsize, fp);
		ttLibC_Jpeg *jpeg = ttLibC_Jpeg_getFrame(NULL, buf, fsize, false, 0, 1000);
		LOG_PRINT("%d x %d", jpeg->inherit_super.width, jpeg->inherit_super.height);
		ttLibC_AvcodecDecoder *decoder = ttLibC_AvcodecVideoDecoder_make(frameType_jpeg, jpeg->inherit_super.width, jpeg->inherit_super.height);
		ttLibC_AvcodecDecoder_decode(decoder, (ttLibC_Frame *)jpeg, jpegDecodeBinaryTest_decodeCallback, NULL);
		ttLibC_AvcodecDecoder_close(&decoder);
		ttLibC_Jpeg_close(&jpeg);
		delete[] buf;
		fclose(fp);
	}
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

#if defined(__ENABLE_APPLE__) && defined(__ENABLE_OPENCV__)
typedef struct {
	ttLibC_CvWindow *target;
	ttLibC_VtDecoder *decoder;
	ttLibC_Bgr *dbgr;
} vtH265Test_t;

static bool vtH265Test_decodeCallback(void *ptr, ttLibC_Yuv420 *yuv) {
	vtH265Test_t *testData = (vtH265Test_t *)ptr;
	// result of vtDecoder is reordered frame.
	// programmer need to reorder them.
	LOG_PRINT("pts:%llu", yuv->inherit_super.inherit_super.pts);
	ttLibC_Bgr *b = ttLibC_ImageResampler_makeBgrFromYuv420(testData->dbgr, BgrType_bgr, yuv);
	if(b == NULL) {
		return false;
	}
	testData->dbgr = b;
	ttLibC_CvWindow_showBgr(testData->target, testData->dbgr);
	return true;
}

static bool vtH265Test_encodeCallback(void *ptr, ttLibC_Video *video) {
	vtH265Test_t *testData = (vtH265Test_t *)ptr;
	return ttLibC_VtDecoder_decode(testData->decoder, video, vtH265Test_decodeCallback, ptr);
}
#endif

static void vtH265Test() {
	LOG_PRINT("vtH265Test");
#if defined(__ENABLE_APPLE__) && defined(__ENABLE_OPENCV__)
	uint32_t width =320, height = 240;
	ttLibC_CvCapture *capture = ttLibC_CvCapture_make(0, width, height);
	ttLibC_CvWindow *original = ttLibC_CvWindow_make("original");
	ttLibC_CvWindow *target = ttLibC_CvWindow_make("target");
	ttLibC_VtEncoder *encoder = ttLibC_VtEncoder_make(width, height, frameType_h265);
	ttLibC_VtDecoder *decoder = ttLibC_VtDecoder_make(frameType_h265);
	ttLibC_Bgr *bgr = NULL, *dbgr = NULL, *b;
	ttLibC_Yuv420 *yuv = NULL, *y;
	vtH265Test_t testData;
	testData.dbgr = NULL;
	testData.target = target;
	testData.decoder = decoder;
	while(true) {
		b = ttLibC_CvCapture_queryFrame(capture, bgr);
		if(b == NULL) {
			break;
		}
		bgr = b;
		ttLibC_CvWindow_showBgr(original, bgr);
		y = ttLibC_ImageResampler_makeYuv420FromBgr(yuv, Yuv420Type_planar, bgr);
		if(y == NULL) {
			break;
		}
		yuv = y;
		if(!ttLibC_VtEncoder_encode(encoder, yuv, vtH265Test_encodeCallback, &testData)) {
			break;
		}
		dbgr = testData.dbgr;
		uint8_t key = ttLibC_CvWindow_waitForKeyInput(66);
		if(key == Keychar_Esc) {
			break;
		}
	}
	testData.decoder = NULL;
	ttLibC_Bgr_close(&bgr);
	ttLibC_Bgr_close(&dbgr);
	ttLibC_Yuv420_close(&yuv);
	ttLibC_VtDecoder_close(&decoder);
	ttLibC_VtEncoder_close(&encoder);
	ttLibC_CvWindow_close(&original);
	ttLibC_CvWindow_close(&target);
	ttLibC_CvCapture_close(&capture);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

#if defined(__ENABLE_APPLE__) && defined(__ENABLE_OPENCV__) && defined(__ENABLE_AVCODEC__)
typedef struct {
	ttLibC_CvWindow *target;
	ttLibC_AvcodecDecoder *decoder;
	ttLibC_Bgr *dbgr;
} vtH265AvcodecDecodeTest_t;

static bool vtH265AvcodecDecodeTest_decodeCallback(void *ptr, ttLibC_Frame *yuv) {
	vtH265AvcodecDecodeTest_t *testData = (vtH265AvcodecDecodeTest_t *)ptr;
	ttLibC_Bgr *b = ttLibC_ImageResampler_makeBgrFromYuv420(testData->dbgr, BgrType_bgr, (ttLibC_Yuv420 *)yuv);
	if(b == NULL) {
		return false;
	}
	testData->dbgr = b;
	ttLibC_CvWindow_showBgr(testData->target, testData->dbgr);
	return true;
}

static bool vtH265AvcodecDecodeTest_encodeCallback(void *ptr, ttLibC_Video *video) {
	vtH265AvcodecDecodeTest_t *testData = (vtH265AvcodecDecodeTest_t *)ptr;
	ttLibC_H265 *h265 = (ttLibC_H265 *)video;
	switch(h265->frame_type) {
	case H265FrameType_B:
		LOG_PRINT("h265:disposable:%d b frame", h265->is_disposable);
		break;
	case H265FrameType_P:
		LOG_PRINT("h265:disposable:%d p frame", h265->is_disposable);
		break;
	case H265FrameType_I:
		LOG_PRINT("h265:disposable:%d i frame", h265->is_disposable);
		break;
	case H265FrameType_unknown:
	default:
		LOG_PRINT("h265:disposable:%d", h265->is_disposable);
		break;
	}
	return ttLibC_AvcodecDecoder_decode(testData->decoder, (ttLibC_Frame *)video, vtH265AvcodecDecodeTest_decodeCallback, ptr);
}
#endif

static void vtH265AvcodecDecodeTest() {
	LOG_PRINT("vtH265AvcodecDecodeTest");
#if defined(__ENABLE_APPLE__) && defined(__ENABLE_OPENCV__) && defined(__ENABLE_AVCODEC__)
	uint32_t width =320, height = 240;
	ttLibC_CvCapture *capture = ttLibC_CvCapture_make(0, width, height);
	ttLibC_CvWindow *original = ttLibC_CvWindow_make("original");
	ttLibC_CvWindow *target = ttLibC_CvWindow_make("target");
	ttLibC_VtEncoder *encoder = ttLibC_VtEncoder_make(width, height, frameType_h265);
	ttLibC_AvcodecDecoder *decoder = ttLibC_AvcodecVideoDecoder_make(frameType_h265, width, height);
	ttLibC_Bgr *bgr = NULL, *dbgr = NULL, *b;
	ttLibC_Yuv420 *yuv = NULL, *y;
	vtH265AvcodecDecodeTest_t testData;
	testData.dbgr = NULL;
	testData.target = target;
	testData.decoder = decoder;
	while(true) {
		b = ttLibC_CvCapture_queryFrame(capture, bgr);
		if(b == NULL) {
			break;
		}
		bgr = b;
		ttLibC_CvWindow_showBgr(original, bgr);
		y = ttLibC_ImageResampler_makeYuv420FromBgr(yuv, Yuv420Type_planar, bgr);
		if(y == NULL) {
			break;
		}
		yuv = y;
		if(!ttLibC_VtEncoder_encode(encoder, yuv, vtH265AvcodecDecodeTest_encodeCallback, &testData)) {
			break;
		}
		dbgr = testData.dbgr;
		uint8_t key = ttLibC_CvWindow_waitForKeyInput(66);
		if(key == Keychar_Esc) {
			break;
		}
	}
	testData.decoder = NULL;
	ttLibC_Bgr_close(&bgr);
	ttLibC_Bgr_close(&dbgr);
	ttLibC_Yuv420_close(&yuv);
	ttLibC_AvcodecDecoder_close(&decoder);
	ttLibC_VtEncoder_close(&encoder);
	ttLibC_CvWindow_close(&original);
	ttLibC_CvWindow_close(&target);
	ttLibC_CvCapture_close(&capture);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

#if defined(__ENABLE_APPLE__) && defined(__ENABLE_OPENCV__)
typedef struct {
	ttLibC_CvWindow *target;
	ttLibC_VtDecoder *decoder;
	ttLibC_Bgr *dbgr;
} vtH264Test_t;

static bool vtH264Test_decodeCallback(void *ptr, ttLibC_Yuv420 *yuv) {
	vtH264Test_t *testData = (vtH264Test_t *)ptr;
	ttLibC_Bgr *b = ttLibC_ImageResampler_makeBgrFromYuv420(testData->dbgr, BgrType_bgr, yuv);
	if(b == NULL) {
		return false;
	}
	testData->dbgr = b;
	ttLibC_CvWindow_showBgr(testData->target, testData->dbgr);
	return true;
}

static bool vtH264Test_encodeCallback(void *ptr, ttLibC_Video *video) {
	vtH264Test_t *testData = (vtH264Test_t *)ptr;
	return ttLibC_VtDecoder_decode(testData->decoder, video, vtH264Test_decodeCallback, ptr);
}
#endif

static void vtH264Test() {
	LOG_PRINT("vtH264Test");
#if defined(__ENABLE_APPLE__) && defined(__ENABLE_OPENCV__)
	uint32_t width =320, height = 240;
	ttLibC_CvCapture *capture = ttLibC_CvCapture_make(0, width, height);
	ttLibC_CvWindow *original = ttLibC_CvWindow_make("original");
	ttLibC_CvWindow *target = ttLibC_CvWindow_make("target");
	ttLibC_VtEncoder *encoder = ttLibC_VtEncoder_make(width, height, frameType_h264);
	ttLibC_VtDecoder *decoder = ttLibC_VtDecoder_make(frameType_h264);
	ttLibC_Bgr *bgr = NULL, *dbgr = NULL, *b;
	ttLibC_Yuv420 *yuv = NULL, *y;
	vtH264Test_t testData;
	testData.dbgr = NULL;
	testData.target = target;
	testData.decoder = decoder;
	while(true) {
		b = ttLibC_CvCapture_queryFrame(capture, bgr);
		if(b == NULL) {
			break;
		}
		bgr = b;
		ttLibC_CvWindow_showBgr(original, bgr);
		y = ttLibC_ImageResampler_makeYuv420FromBgr(yuv, Yuv420Type_planar, bgr);
		if(y == NULL) {
			break;
		}
		yuv = y;
		if(!ttLibC_VtEncoder_encode(encoder, yuv, vtH264Test_encodeCallback, &testData)) {
			break;
		}
		dbgr = testData.dbgr;
		uint8_t key = ttLibC_CvWindow_waitForKeyInput(66);
		if(key == Keychar_Esc) {
			break;
		}
	}
	testData.decoder = NULL;
	ttLibC_Bgr_close(&bgr);
	ttLibC_Bgr_close(&dbgr);
	ttLibC_Yuv420_close(&yuv);
	ttLibC_VtDecoder_close(&decoder);
	ttLibC_VtEncoder_close(&encoder);
	ttLibC_CvWindow_close(&original);
	ttLibC_CvWindow_close(&target);
	ttLibC_CvCapture_close(&capture);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

#if defined(__ENABLE_APPLE__) && defined(__ENABLE_OPENCV__)
typedef struct {
	ttLibC_CvWindow *target;
	ttLibC_VtDecoder *decoder;
	ttLibC_Bgr *dbgr;
} vtJpegTest_t;

static bool vtJpegTest_decodeCallback(void *ptr, ttLibC_Yuv420 *yuv) {
	vtJpegTest_t *testData = (vtJpegTest_t *)ptr;
	ttLibC_Bgr *b = ttLibC_ImageResampler_makeBgrFromYuv420(testData->dbgr, BgrType_bgr, yuv);
	if(b == NULL) {
		return false;
	}
	testData->dbgr = b;
	ttLibC_CvWindow_showBgr(testData->target, testData->dbgr);
	return true;
}

static bool vtJpegTest_encodeCallback(void *ptr, ttLibC_Video *video) {
	vtJpegTest_t *testData = (vtJpegTest_t *)ptr;
	return ttLibC_VtDecoder_decode(testData->decoder, video, vtJpegTest_decodeCallback, ptr);
}
#endif

static void vtJpegTest() {
	LOG_PRINT("vtJpegTest");
#if defined(__ENABLE_APPLE__) && defined(__ENABLE_OPENCV__)
	uint32_t width =320, height = 240;
	ttLibC_CvCapture *capture = ttLibC_CvCapture_make(0, width, height);
	ttLibC_CvWindow *original = ttLibC_CvWindow_make("original");
	ttLibC_CvWindow *target = ttLibC_CvWindow_make("target");
	ttLibC_VtEncoder *encoder = ttLibC_VtEncoder_make(width, height, frameType_jpeg);
	ttLibC_VtDecoder *decoder = ttLibC_VtDecoder_make(frameType_jpeg);
	ttLibC_Bgr *bgr = NULL, *dbgr = NULL, *b;
	ttLibC_Yuv420 *yuv = NULL, *y;
	vtJpegTest_t testData;
	testData.dbgr = NULL;
	testData.target = target;
	testData.decoder = decoder;
	while(true) {
		b = ttLibC_CvCapture_queryFrame(capture, bgr);
		if(b == NULL) {
			break;
		}
		bgr = b;
		ttLibC_CvWindow_showBgr(original, bgr);
		y = ttLibC_ImageResampler_makeYuv420FromBgr(yuv, Yuv420Type_planar, bgr);
		if(y == NULL) {
			break;
		}
		yuv = y;
		if(!ttLibC_VtEncoder_encode(encoder, yuv, vtJpegTest_encodeCallback, &testData)) {
			break;
		}
		dbgr = testData.dbgr;
		uint8_t key = ttLibC_CvWindow_waitForKeyInput(66);
		if(key == Keychar_Esc) {
			break;
		}
	}
	testData.decoder = NULL;
	ttLibC_Bgr_close(&bgr);
	ttLibC_Bgr_close(&dbgr);
	ttLibC_Yuv420_close(&yuv);
	ttLibC_VtDecoder_close(&decoder);
	ttLibC_VtEncoder_close(&encoder);
	ttLibC_CvWindow_close(&original);
	ttLibC_CvWindow_close(&target);
	ttLibC_CvCapture_close(&capture);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

#if defined(__ENABLE_VORBIS_ENCODE__) && defined(__ENABLE_VORBIS_DECODE__) && defined(__ENABLE_OPENAL__)
typedef struct vorbisTest_t {
	ttLibC_VorbisDecoder *decoder;
	ttLibC_PcmS16 *pcm;
	ttLibC_AlPlayer *player;
} vorbisTest_t;

static bool vorbisTest_decodeCallback(void *ptr, ttLibC_PcmF32 *pcm) {
	vorbisTest_t *testData = (vorbisTest_t *)ptr;
	// make pcms16 from pcmf32
	ttLibC_PcmS16 *p = ttLibC_AudioResampler_makePcmS16FromPcmF32(
			testData->pcm,
			PcmS16Type_littleEndian,
			pcm);
	if(p == NULL) {
		return false;
	}
	testData->pcm = p;
	while(!ttLibC_AlPlayer_queue(testData->player, testData->pcm)) {
		usleep(100);
	}
	return true;
}

static bool vorbisTest_encodeCallback(void *ptr, ttLibC_Vorbis *vorbis) {
	vorbisTest_t *testData = (vorbisTest_t *)ptr;
	return ttLibC_VorbisDecoder_decode(testData->decoder, vorbis, vorbisTest_decodeCallback, ptr);
}
#endif

static void vorbisTest() {
	LOG_PRINT("vorbisTest");
#if defined(__ENABLE_VORBIS_ENCODE__) && defined(__ENABLE_VORBIS_DECODE__) && defined(__ENABLE_OPENAL__)
	uint32_t sample_rate = 44100, channel_num = 1;
	ttLibC_BeepGenerator *generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian, 440, sample_rate, channel_num);
	generator->amplitude = 30000;
	ttLibC_VorbisEncoder *encoder = ttLibC_VorbisEncoder_make(sample_rate, channel_num);
	ttLibC_VorbisDecoder *decoder = ttLibC_VorbisDecoder_make();
	ttLibC_PcmS16 *p, *pcm = NULL, *dpcm = NULL;
	ttLibC_AlPlayer *player = ttLibC_AlPlayer_make();
	vorbisTest_t testData;
	testData.decoder = decoder;
	testData.pcm = dpcm;
	testData.player = player;

	for(int i = 0;i < 5;++ i) {
		p = ttLibC_BeepGenerator_makeBeepByMiliSec(generator, pcm, 500);
		if(p == NULL) {
			break;
		}
		pcm = p;
		if(!ttLibC_VorbisEncoder_encode(encoder, (ttLibC_Audio *)pcm, vorbisTest_encodeCallback, &testData)) {
			break;
		}
		dpcm = testData.pcm;
	}

	ttLibC_PcmS16_close(&pcm);
	ttLibC_PcmS16_close(&dpcm);
	ttLibC_BeepGenerator_close(&generator);
	ttLibC_VorbisEncoder_close(&encoder);
	ttLibC_VorbisDecoder_close(&decoder);
	ttLibC_AlPlayer_close(&player);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

#if defined(__ENABLE_THEORA__) && defined(__ENABLE_OPENCV__)
typedef struct theoraTest_t {
	ttLibC_TheoraDecoder *decoder;
	ttLibC_CvWindow *target;
	ttLibC_Bgr *dbgr;
	uint32_t size;
	uint32_t count;
} theoraTest_t;

static bool theoraTest_decodeCallback(void *ptr, ttLibC_Yuv420 *yuv) {
	theoraTest_t *testData = (theoraTest_t *)ptr;
	ttLibC_Bgr *b = ttLibC_ImageResampler_makeBgrFromYuv420(testData->dbgr, BgrType_bgr, yuv);
	if(b == NULL) {
		return false;
	}
	testData->dbgr = b;
	ttLibC_CvWindow_showBgr(testData->target, testData->dbgr);
	return true;
}

static bool theoraTest_encodeCallback(void *ptr, ttLibC_Theora *theora) {
	theoraTest_t *testData = (theoraTest_t *)ptr;
	switch(theora->type) {
	case TheoraType_innerFrame:
		{
			LOG_PRINT("inner");
			testData->size += theora->inherit_super.inherit_super.buffer_size;
			testData->count ++;
		}
		break;
	case TheoraType_intraFrame:
		{
			LOG_PRINT("key");
			testData->size += theora->inherit_super.inherit_super.buffer_size;
			testData->count ++;
		}
		break;
	default:
		break;
	}
	if(testData->count > 60) {
		LOG_PRINT("sz:%d", testData->size);
		testData->size = 0;
		testData->count = 0;
	}
	return ttLibC_TheoraDecoder_decode(testData->decoder, theora, theoraTest_decodeCallback, ptr);
}

#endif
static void theoraTest() {
	LOG_PRINT("theoraTest");
#if defined(__ENABLE_THEORA__) && defined(__ENABLE_OPENCV__)
	uint32_t width = 320, height = 180;
	ttLibC_CvCapture *capture = ttLibC_CvCapture_make(0, width, height);
	ttLibC_CvWindow *window = ttLibC_CvWindow_make("original");
	ttLibC_CvWindow *dec_win = ttLibC_CvWindow_make("target");
	ttLibC_TheoraEncoder *encoder = ttLibC_TheoraEncoder_make_ex(width, height, 30, 0, 4);
	ttLibC_TheoraDecoder *decoder = ttLibC_TheoraDecoder_make();
	ttLibC_Bgr    *b, *bgr = NULL, *dbgr = NULL;
	ttLibC_Yuv420 *y, *yuv = NULL;
	theoraTest_t testData;
	testData.size = 0;
	testData.count = 0;
	testData.dbgr = NULL;
	testData.decoder = decoder;
	testData.target = dec_win;
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
		if(!ttLibC_TheoraEncoder_encode(encoder, yuv, theoraTest_encodeCallback, &testData)) {
			break;
		}
		dbgr = testData.dbgr;
		uint8_t keyChar = ttLibC_CvWindow_waitForKeyInput(33);
		if(keyChar == Keychar_Esc) {
			break;
		}
	}
	ttLibC_Bgr_close(&bgr);
	ttLibC_Bgr_close(&dbgr);
	ttLibC_Yuv420_close(&yuv);
	ttLibC_TheoraDecoder_close(&decoder);
	ttLibC_TheoraEncoder_close(&encoder);
	ttLibC_CvWindow_close(&dec_win);
	ttLibC_CvWindow_close(&window);
	ttLibC_CvCapture_close(&capture);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

#if defined(__ENABLE_APPLE__)
typedef struct acMulawTest_t{
	ttLibC_AuPlayer *player;
	ttLibC_AcDecoder *decoder;
} acMulawTest_t;

static bool acMulawTest_decodeCallback(void *ptr, ttLibC_PcmS16 *pcm) {
	acMulawTest_t *testData = (acMulawTest_t *)ptr;
	while(!ttLibC_AuPlayer_queue(testData->player, pcm)) {
		usleep(10);
	}
	return true;
}

static bool acMulawTest_encodeCallback(void *ptr, ttLibC_Audio *mulaw) {
	acMulawTest_t *testData = (acMulawTest_t *)ptr;
	return ttLibC_AcDecoder_decode(testData->decoder, mulaw, acMulawTest_decodeCallback, ptr);
}
#endif

static void acMulawTest() {
	LOG_PRINT("acMulawTest");
#if defined(__ENABLE_APPLE__)
	uint32_t sample_rate = 8000, channel_num = 1;
	ttLibC_BeepGenerator *generator = ttLibC_BeepGenerator_make(
			PcmS16Type_littleEndian,
			440,
			sample_rate,
			channel_num);
	ttLibC_AuPlayer *player = ttLibC_AuPlayer_make(sample_rate, channel_num, AuPlayerType_DefaultOutput);
	ttLibC_AcEncoder *encoder = ttLibC_AcEncoder_make(
			sample_rate,
			channel_num,
			96000,
			frameType_pcm_mulaw);
	ttLibC_AcDecoder *decoder = ttLibC_AcDecoder_make(
			sample_rate,
			channel_num,
			frameType_pcm_mulaw);
	ttLibC_PcmS16 *pcm = NULL, *p;
	acMulawTest_t testData;
	generator->amplitude = 30000;
	testData.decoder = decoder;
	testData.player = player;
	for(int i = 0;i < 100;++ i) {
		p = ttLibC_BeepGenerator_makeBeepByMiliSec(generator, pcm, 100);
		if(p == NULL) {
			break;
		}
		pcm = p;
		if(!ttLibC_AcEncoder_encode(encoder, pcm, acMulawTest_encodeCallback, &testData)) {
			break;
		}
	}
	ttLibC_AcDecoder_close(&decoder);
	ttLibC_AcEncoder_close(&encoder);
	ttLibC_PcmS16_close(&pcm);
	ttLibC_AuPlayer_close(&player);
	ttLibC_BeepGenerator_close(&generator);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

#if defined(__ENABLE_APPLE__)
typedef struct acAlawTest_t{
	ttLibC_AuPlayer *player;
	ttLibC_AcDecoder *decoder;
} acAlawTest_t;

static bool acAlawTest_decodeCallback(void *ptr, ttLibC_PcmS16 *pcm) {
	acAlawTest_t *testData = (acAlawTest_t *)ptr;
	while(!ttLibC_AuPlayer_queue(testData->player, pcm)) {
		usleep(10);
	}
	return true;
}

static bool acAlawTest_encodeCallback(void *ptr, ttLibC_Audio *alaw) {
	acAlawTest_t *testData = (acAlawTest_t *)ptr;
	return ttLibC_AcDecoder_decode(testData->decoder, alaw, acAlawTest_decodeCallback, ptr);
}
#endif

static void acAlawTest() {
	LOG_PRINT("acAlawTest");
#if defined(__ENABLE_APPLE__)
	uint32_t sample_rate = 8000, channel_num = 1;
	ttLibC_BeepGenerator *generator = ttLibC_BeepGenerator_make(
			PcmS16Type_littleEndian,
			440,
			sample_rate,
			channel_num);
	ttLibC_AuPlayer *player = ttLibC_AuPlayer_make(sample_rate, channel_num, AuPlayerType_DefaultOutput);
	ttLibC_AcEncoder *encoder = ttLibC_AcEncoder_make(
			sample_rate,
			channel_num,
			96000,
			frameType_pcm_alaw);
	ttLibC_AcDecoder *decoder = ttLibC_AcDecoder_make(
			sample_rate,
			channel_num,
			frameType_pcm_alaw);
	ttLibC_PcmS16 *pcm = NULL, *p;
	acAlawTest_t testData;
	generator->amplitude = 30000;
	testData.decoder = decoder;
	testData.player = player;
	for(int i = 0;i < 100;++ i) {
		p = ttLibC_BeepGenerator_makeBeepByMiliSec(generator, pcm, 100);
		if(p == NULL) {
			break;
		}
		pcm = p;
		if(!ttLibC_AcEncoder_encode(encoder, pcm, acAlawTest_encodeCallback, &testData)) {
			break;
		}
	}
	ttLibC_AcDecoder_close(&decoder);
	ttLibC_AcEncoder_close(&encoder);
	ttLibC_PcmS16_close(&pcm);
	ttLibC_AuPlayer_close(&player);
	ttLibC_BeepGenerator_close(&generator);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

#if defined(__ENABLE_APPLE__) && defined(__ENABLE_MP3LAME_ENCODE__)
typedef struct acMp3Test_t {
	ttLibC_AuPlayer *player;
	ttLibC_AcDecoder *decoder;
} acMp3Test_t;

static bool acMp3Test_decodeCallback(void *ptr, ttLibC_PcmS16 *pcm) {
	acMp3Test_t *testData = (acMp3Test_t *)ptr;
	while(!ttLibC_AuPlayer_queue(testData->player, pcm)) {
		usleep(10);
	}
	return true;
}

static bool acMp3Test_encodeCallback(void *ptr, ttLibC_Mp3 *mp3) {
	acMp3Test_t *testData = (acMp3Test_t *)ptr;
	return ttLibC_AcDecoder_decode(testData->decoder, (ttLibC_Audio *)mp3, acMp3Test_decodeCallback, ptr);
}
#endif

static void acMp3Test() {
	LOG_PRINT("acMp3Test");
#if defined(__ENABLE_APPLE__) && defined(__ENABLE_MP3LAME_ENCODE__)
	uint32_t sample_rate = 44100, channel_num = 2;
	ttLibC_BeepGenerator *generator = ttLibC_BeepGenerator_make(
			PcmS16Type_littleEndian,
			440,
			sample_rate,
			channel_num);
	ttLibC_AuPlayer *player = ttLibC_AuPlayer_make(sample_rate, channel_num, AuPlayerType_DefaultOutput);
	ttLibC_Mp3lameEncoder *encoder = ttLibC_Mp3lameEncoder_make(
			sample_rate,
			channel_num,
			2);
	ttLibC_AcDecoder *decoder = ttLibC_AcDecoder_make(
			sample_rate,
			channel_num,
			frameType_mp3);
	ttLibC_PcmS16 *pcm = NULL, *p;
	acMp3Test_t testData;
	generator->amplitude = 30000;
	testData.decoder = decoder;
	testData.player = player;
	for(int i = 0;i < 100;++ i) {
		p = ttLibC_BeepGenerator_makeBeepByMiliSec(generator, pcm, 100);
		if(p == NULL) {
			break;
		}
		pcm = p;
		if(!ttLibC_Mp3lameEncoder_encode(encoder, pcm, acMp3Test_encodeCallback, &testData)) {
			break;
		}
	}
	ttLibC_Mp3lameEncoder_close(&encoder);
	ttLibC_AcDecoder_close(&decoder);
	ttLibC_PcmS16_close(&pcm);
	ttLibC_AuPlayer_close(&player);
	ttLibC_BeepGenerator_close(&generator);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

#if defined(__ENABLE_APPLE__)
typedef struct acAacTest_t{
	ttLibC_AuPlayer *player;
	ttLibC_AcDecoder *decoder;
} acAacTest_t;

static bool acAacTest_decodeCallback(void *ptr, ttLibC_PcmS16 *pcm) {
	acAacTest_t *testData = (acAacTest_t *)ptr;
	while(!ttLibC_AuPlayer_queue(testData->player, pcm)) {
		usleep(10);
	}
	return true;
}

static bool acAacTest_encodeCallback(void *ptr, ttLibC_Audio *aac) {
	acAacTest_t *testData = (acAacTest_t *)ptr;
	return ttLibC_AcDecoder_decode(testData->decoder, aac, acAacTest_decodeCallback, ptr);
}
#endif

static void acAacTest() {
	LOG_PRINT("acAacTest");
#if defined(__ENABLE_APPLE__)
	uint32_t sample_rate = 44100, channel_num = 2;
	ttLibC_BeepGenerator *generator = ttLibC_BeepGenerator_make(
			PcmS16Type_littleEndian,
			440,
			sample_rate,
			channel_num);
	ttLibC_AuPlayer *player = ttLibC_AuPlayer_make(sample_rate, channel_num, AuPlayerType_DefaultOutput);
	ttLibC_AcEncoder *encoder = ttLibC_AcEncoder_make(
			sample_rate,
			channel_num,
			96000,
			frameType_aac);
	ttLibC_AcDecoder *decoder = ttLibC_AcDecoder_make(
			sample_rate,
			channel_num,
			frameType_aac);
	ttLibC_PcmS16 *pcm = NULL, *p;
	acAacTest_t testData;
	generator->amplitude = 30000;
	testData.decoder = decoder;
	testData.player = player;
	for(int i = 0;i < 100;++ i) {
		p = ttLibC_BeepGenerator_makeBeepByMiliSec(generator, pcm, 100);
		if(p == NULL) {
			break;
		}
		pcm = p;
		if(!ttLibC_AcEncoder_encode(encoder, pcm, acAacTest_encodeCallback, &testData)) {
			break;
		}
	}
	ttLibC_AcDecoder_close(&decoder);
	ttLibC_AcEncoder_close(&encoder);
	ttLibC_PcmS16_close(&pcm);
	ttLibC_AuPlayer_close(&player);
	ttLibC_BeepGenerator_close(&generator);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

#if defined(__ENABLE_X265__) && defined(__ENABLE_OPENCV__) && defined(__ENABLE_AVCODEC__)
typedef struct x265TestData {
	ttLibC_AvcodecDecoder *decoder;
	ttLibC_CvWindow *dec_win;
	ttLibC_Bgr *dbgr;
} x265TestData;

static bool x265Test_decodeCallback(void *ptr, ttLibC_Frame *yuv420) {
	x265TestData *testData = (x265TestData *)ptr;
	ttLibC_Bgr *b = ttLibC_ImageResampler_makeBgrFromYuv420(testData->dbgr, BgrType_bgr, (ttLibC_Yuv420 *)yuv420);
	if(b == NULL) {
		return false;
	}
	testData->dbgr = b;
	ttLibC_CvWindow_showBgr(testData->dec_win, testData->dbgr);
	return true;
}

static bool x265Test_encodeCallback(void *ptr, ttLibC_H265 *h265) {
	x265TestData *testData = (x265TestData *)ptr;
	if(h265->type == H265Type_unknown) {
		return true;
	}
	switch(h265->frame_type) {
	case H265FrameType_B:
		LOG_PRINT("h265:disposable:%d b frame", h265->is_disposable);
		break;
	case H265FrameType_P:
		LOG_PRINT("h265:disposable:%d p frame", h265->is_disposable);
		break;
	case H265FrameType_I:
		LOG_PRINT("h265:disposable:%d i frame", h265->is_disposable);
		break;
	case H265FrameType_unknown:
	default:
		LOG_PRINT("h265:disposable:%d", h265->is_disposable);
		break;
	}
	return ttLibC_AvcodecDecoder_decode(testData->decoder, (ttLibC_Frame *)h265, x265Test_decodeCallback, ptr);;
}
#endif

static void x265Test() {
	LOG_PRINT("x265Test");
#if defined(__ENABLE_X265__) && defined(__ENABLE_OPENCV__) && defined(__ENABLE_AVCODEC__)
	uint32_t width = 320, height = 240;
	ttLibC_CvCapture *capture = ttLibC_CvCapture_make(0, width, height);
	ttLibC_CvWindow *window = ttLibC_CvWindow_make("original");
	ttLibC_CvWindow *dec_win = ttLibC_CvWindow_make("target");
	ttLibC_X265Encoder *encoder = ttLibC_X265Encoder_make(width, height);
	ttLibC_AvcodecDecoder *decoder = ttLibC_AvcodecVideoDecoder_make(frameType_h265, width, height);
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
		x265TestData testData;
		testData.dbgr = dbgr;
		testData.dec_win = dec_win;
		testData.decoder = decoder;
		ttLibC_X265Encoder_encode(encoder, yuv, x265Test_encodeCallback, &testData);
		dbgr = testData.dbgr;
		uint8_t key_char = ttLibC_CvWindow_waitForKeyInput(10);
		if(key_char == Keychar_Esc) {
			break;
		}
	}
	ttLibC_Bgr_close(&bgr);
	ttLibC_Bgr_close(&dbgr);
	ttLibC_Yuv420_close(&yuv);
	ttLibC_AvcodecDecoder_close(&decoder);
	ttLibC_X265Encoder_close(&encoder);
	ttLibC_CvWindow_close(&dec_win);
	ttLibC_CvWindow_close(&window);
	ttLibC_CvCapture_close(&capture);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

#if defined(__ENABLE_X264__) && defined(__ENABLE_OPENCV__) && defined(__ENABLE_AVCODEC__)
typedef struct x264TestData{
	ttLibC_AvcodecDecoder *decoder;
	ttLibC_CvWindow *dec_win;
	ttLibC_Bgr *dbgr;
} x264TestData;

bool x264Test_decodeCallback(void *ptr, ttLibC_Frame *yuv420) {
	x264TestData *testData = (x264TestData *)ptr;
	ttLibC_Bgr *b = ttLibC_ImageResampler_makeBgrFromYuv420(testData->dbgr, BgrType_bgr, (ttLibC_Yuv420 *)yuv420);
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
	switch(h264->frame_type) {
	case H264FrameType_I:
		LOG_PRINT("h264:disposable:%d i frame", h264->is_disposable);
		break;
	case H264FrameType_P:
		LOG_PRINT("h264:disposable:%d p frame", h264->is_disposable);
		break;
	case H264FrameType_B:
		LOG_PRINT("h264:disposable:%d b frame", h264->is_disposable);
		break;
	case H264FrameType_SI:
		LOG_PRINT("h264:disposable:%d si frame", h264->is_disposable);
		break;
	case H264FrameType_SP:
		LOG_PRINT("h264:disposable:%d sp frame", h264->is_disposable);
		break;
	default:
	case H264FrameType_unknown:
		LOG_PRINT("h264:disposable:%d", h264->is_disposable);
		break;
	}
	ttLibC_AvcodecDecoder_decode(testData->decoder, (ttLibC_Frame *)h264, x264Test_decodeCallback, ptr);
	return true;
}
#endif

static void x264Test() {
	LOG_PRINT("x264Test");
#if defined(__ENABLE_X264__) && defined(__ENABLE_OPENCV__) && defined(__ENABLE_AVCODEC__)
	// x264 does not have decoder.
	// encode with x264 and decode with avcodec.
	uint32_t width = 320, height = 240;
	ttLibC_CvCapture *capture = ttLibC_CvCapture_make(0, width, height);
	ttLibC_CvWindow *window = ttLibC_CvWindow_make("original");
	ttLibC_CvWindow *dec_win = ttLibC_CvWindow_make("target");
	ttLibC_X264Encoder *encoder = ttLibC_X264Encoder_make(width, height);
	ttLibC_AvcodecDecoder *decoder = ttLibC_AvcodecVideoDecoder_make(frameType_h264, width, height);
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
	ttLibC_AvcodecDecoder_close(&decoder);
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
	testData.dec_win = ttLibC_CvWindow_make("target");
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

#if defined(__ENABLE_SWRESAMPLE__) && defined(__ENABLE_OPENAL__)
static bool swresampleTestCallback(void *ptr, ttLibC_Frame *pcm) {
	ttLibC_AlDevice *device = (ttLibC_AlDevice *)ptr;
	ttLibC_AlDevice_queue(device, (ttLibC_PcmS16 *)pcm);
	return true;
}
#endif

static void swresampleTest() {
	LOG_PRINT("swresampleTest");
#if defined(__ENABLE_SWRESAMPLE__) && defined(__ENABLE_OPENAL__)
	uint32_t channel = 2;
	uint32_t in_sample_rate = 44100;
	uint32_t out_sample_rate = 48000;
	ttLibC_BeepGenerator *generator = ttLibC_BeepGenerator_make(PcmS16Type_littleEndian, 440, in_sample_rate, channel);
	ttLibC_AlDevice *device = ttLibC_AlDevice_make(1000);
	ttLibC_PcmS16 *pcm = NULL, *resampled_pcm = NULL, *p;
	ttLibC_SwresampleResampler *resampler = ttLibC_SwresampleResampler_make(
			frameType_pcmS16, PcmS16Type_littleEndian, in_sample_rate, channel,
			frameType_pcmS16, PcmS16Type_littleEndian , out_sample_rate, channel);
	for(int i = 0;i < 5;i ++) {
		p = ttLibC_BeepGenerator_makeBeepByMiliSec(generator, pcm, 500);
		if(p == NULL) {
			break;
		}
		pcm = p;
		if(!ttLibC_SwresampleResampler_resample(resampler, (ttLibC_Frame *)pcm, swresampleTestCallback, device)) {
			break;
		}
	}
	ttLibC_AlDevice_proceed(device, -1);
	ttLibC_PcmS16_close(&pcm);
	ttLibC_PcmS16_close(&resampled_pcm);
	ttLibC_AlDevice_close(&device);
	ttLibC_BeepGenerator_close(&generator);
	ttLibC_SwresampleResampler_close(&resampler);
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
	ttLibC_CvWindow        *dec_win = ttLibC_CvWindow_make("target");
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

#if defined(__ENABLE_OPENCV__) && defined(__ENABLE_SWSCALE__)
static bool swscaleCallback(void *ptr, ttLibC_Frame *frame) {
	ttLibC_CvWindow *resampled_window = (ttLibC_CvWindow *)ptr;
	ttLibC_CvWindow_showBgr(resampled_window, (ttLibC_Bgr *)frame);
	return true;
}
#endif

static void swscaleTest() {
	LOG_PRINT("swscaleTest");
#if defined(__ENABLE_OPENCV__) && defined(__ENABLE_SWSCALE__)
	ttLibC_CvCapture *capture = ttLibC_CvCapture_make(0, 320, 240);
	ttLibC_CvWindow *window = ttLibC_CvWindow_make("original");
	ttLibC_CvWindow *resampled_window = ttLibC_CvWindow_make("target");
	ttLibC_SwscaleResampler *resampler = ttLibC_SwscaleResampler_make(
			frameType_bgr,
			BgrType_bgr,
			320, 240,
			frameType_bgr,
			BgrType_bgr,
			400, 300,
			SwscaleResampler_Bicubic);
	ttLibC_Bgr *bgr = NULL, *b, *rbgr = NULL;
	while(true) {
		b = ttLibC_CvCapture_queryFrame(capture, bgr);
		if(b == NULL) {
			break;
		}
		bgr = b;
		ttLibC_CvWindow_showBgr(window, bgr);
		ttLibC_SwscaleResampler_resample(resampler, (ttLibC_Frame *)bgr, swscaleCallback, resampled_window);
		uint8_t key = ttLibC_CvWindow_waitForKeyInput(10);
		if(key == Keychar_Esc) {
			break;
		}
	}
	ttLibC_Bgr_close(&rbgr);
	ttLibC_Bgr_close(&bgr);
	ttLibC_SwscaleResampler_close(&resampler);
	ttLibC_CvWindow_close(&resampled_window);
	ttLibC_CvWindow_close(&window);
	ttLibC_CvCapture_close(&capture);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

static void libyuvImageRotateTest() {
	LOG_PRINT("libyuvImageRotateTest");
#if defined(__ENABLE_OPENCV__) && defined(__ENABLE_LIBYUV__)
	ttLibC_CvCapture *capture = ttLibC_CvCapture_make(0, 320, 240);
	ttLibC_CvWindow *window = ttLibC_CvWindow_make("original");
	ttLibC_CvWindow *resampled_window = ttLibC_CvWindow_make("target");
	ttLibC_Bgr *bgr = NULL, *b, *rbgr = NULL;
	ttLibC_Yuv420 *yuv = NULL, *y, *ryuv = NULL;
	while(true) {
		b = ttLibC_CvCapture_queryFrame(capture, bgr);
		if(b == NULL) {
			break;
		}
		bgr = b;
		y = ttLibC_ImageResampler_makeYuv420FromBgr(yuv, Yuv420Type_planar, bgr);
		if(y == NULL) {
			break;
		}
		yuv = y;
		y = ttLibC_LibyuvResampler_rotate(ryuv, yuv, LibyuvRotate_270);
		if(y == NULL) {
			break;
		}
		ryuv = y;
		b = ttLibC_ImageResampler_makeBgrFromYuv420(rbgr, BgrType_abgr, ryuv);
		if(b == NULL) {
			break;
		}
		rbgr = b;
		ttLibC_CvWindow_showBgr(window, bgr);
		ttLibC_CvWindow_showBgr(resampled_window, rbgr);
		uint8_t key = ttLibC_CvWindow_waitForKeyInput(10);
		if(key == Keychar_Esc) {
			break;
		}
	}
	ttLibC_Yuv420_close(&ryuv);
	ttLibC_Yuv420_close(&yuv);
	ttLibC_Bgr_close(&rbgr);
	ttLibC_Bgr_close(&bgr);
	ttLibC_CvWindow_close(&resampled_window);
	ttLibC_CvWindow_close(&window);
	ttLibC_CvCapture_close(&capture);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

static void libyuvImageResizeTest() {
	LOG_PRINT("libyuvImageResizeTest");
#if defined(__ENABLE_OPENCV__) && defined(__ENABLE_LIBYUV__)
	ttLibC_CvCapture *capture = ttLibC_CvCapture_make(0, 320, 240);
	ttLibC_CvWindow *window = ttLibC_CvWindow_make("original");
	ttLibC_CvWindow *resampled_window = ttLibC_CvWindow_make("target");
	ttLibC_Bgr *bgr = NULL, *b, *rbgr = NULL;
	ttLibC_Yuv420 *yuv = NULL, *y, *ryuv = NULL;
	while(true) {
		b = ttLibC_CvCapture_queryFrame(capture, bgr);
		if(b == NULL) {
			break;
		}
		bgr = b;
		y = ttLibC_ImageResampler_makeYuv420FromBgr(yuv, Yuv420Type_planar, bgr);
		if(y == NULL) {
			break;
		}
		yuv = y;
		y = ttLibC_LibyuvResampler_resize(ryuv, 480, 360, yuv, LibyuvFilter_Bilinear, LibyuvFilter_Bilinear, LibyuvFilter_Bilinear);
		if(y == NULL) {
			break;
		}
		ryuv = y;
		b = ttLibC_ImageResampler_makeBgrFromYuv420(rbgr, BgrType_abgr, ryuv);
		if(b == NULL) {
			break;
		}
		rbgr = b;
		ttLibC_CvWindow_showBgr(window, bgr);
		ttLibC_CvWindow_showBgr(resampled_window, rbgr);
		uint8_t key = ttLibC_CvWindow_waitForKeyInput(10);
		if(key == Keychar_Esc) {
			break;
		}
	}
	ttLibC_Yuv420_close(&ryuv);
	ttLibC_Yuv420_close(&yuv);
	ttLibC_Bgr_close(&rbgr);
	ttLibC_Bgr_close(&bgr);
	ttLibC_CvWindow_close(&resampled_window);
	ttLibC_CvWindow_close(&window);
	ttLibC_CvCapture_close(&capture);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

static void imageResizeTest() {
	LOG_PRINT("imageResizeTest");
#ifdef __ENABLE_OPENCV__
	ttLibC_CvCapture *capture = ttLibC_CvCapture_make(0, 320, 240);
	ttLibC_CvWindow *window = ttLibC_CvWindow_make("original");
	ttLibC_CvWindow *resampled_window = ttLibC_CvWindow_make("target");
	ttLibC_Bgr *bgr = NULL, *b, *rbgr = NULL;
	ttLibC_Yuv420 *yuv = NULL, *y, *ryuv = NULL;
	while(true) {
		b = ttLibC_CvCapture_queryFrame(capture, bgr);
		if(b == NULL) {
			break;
		}
		bgr = b;
		y = ttLibC_ImageResampler_makeYuv420FromBgr(yuv, Yuv420Type_planar, bgr);
		if(y == NULL) {
			break;
		}
		yuv = y;
		y = ttLibC_ImageResizer_resizeYuv420(ryuv, Yuv420Type_planar, 480, 360, yuv, false);
		if(y == NULL) {
			break;
		}
		ryuv = y;
		// try to clip
		ryuv->inherit_super.width = 320;
		ryuv->inherit_super.height = 240;
		// move start location.
		ryuv->y_data += 80 + 120 * ryuv->y_stride;
		ryuv->u_data += 40 + 60 * ryuv->u_stride;
		ryuv->v_data += 40 + 60 * ryuv->v_stride;
		b = ttLibC_ImageResampler_makeBgrFromYuv420(rbgr, BgrType_abgr, ryuv);
		if(b == NULL) {
			break;
		}
		rbgr = b;
		ttLibC_CvWindow_showBgr(window, bgr);
		ttLibC_CvWindow_showBgr(resampled_window, rbgr);
		uint8_t key = ttLibC_CvWindow_waitForKeyInput(10);
		if(key == Keychar_Esc) {
			break;
		}
	}
	ttLibC_Yuv420_close(&ryuv);
	ttLibC_Yuv420_close(&yuv);
	ttLibC_Bgr_close(&rbgr);
	ttLibC_Bgr_close(&bgr);
	ttLibC_CvWindow_close(&resampled_window);
	ttLibC_CvWindow_close(&window);
	ttLibC_CvCapture_close(&capture);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

/**
 * how to use imageResampler for bgr data.
 */
static void imageResamplerTest() {
	LOG_PRINT("imageResamplerTest");
#ifdef __ENABLE_OPENCV__
	ttLibC_CvCapture *capture = ttLibC_CvCapture_make(0, 300, 200);
	ttLibC_CvWindow *window = ttLibC_CvWindow_make("original");
	ttLibC_CvWindow *resampled_window = ttLibC_CvWindow_make("target");
	ttLibC_Bgr *bgr = NULL;
	ttLibC_Yuv420 *yuv420 = NULL;
	ttLibC_Bgr *resampled_bgr = NULL;
	while(true) {
		ttLibC_Bgr *b = ttLibC_CvCapture_queryFrame(capture, bgr);
		if(b == NULL) {
			continue;
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
	s.push_back(CUTE(pngDecodeBinaryTest2));
	s.push_back(CUTE(pngDecodeBinaryTest));
	s.push_back(CUTE(jpegDecodeBinaryTest));
	s.push_back(CUTE(vtH265Test));
	s.push_back(CUTE(vtH265AvcodecDecodeTest));
	s.push_back(CUTE(vtH264Test));
	s.push_back(CUTE(vtJpegTest));
	s.push_back(CUTE(vorbisTest));
	s.push_back(CUTE(theoraTest));
	s.push_back(CUTE(acMulawTest));
	s.push_back(CUTE(acAlawTest));
	s.push_back(CUTE(acMp3Test));
	s.push_back(CUTE(acAacTest));
	s.push_back(CUTE(x265Test));
	s.push_back(CUTE(x264Test));
	s.push_back(CUTE(jpegTest));
	s.push_back(CUTE(opusTest));
	s.push_back(CUTE(speexTest));
	s.push_back(CUTE(swresampleTest));
	s.push_back(CUTE(speexdspResamplerTest));
	s.push_back(CUTE(openh264Test));
	s.push_back(CUTE(faacEncoderTest));
	s.push_back(CUTE(mp3lameTest));
	s.push_back(CUTE(swscaleTest));
	s.push_back(CUTE(libyuvImageRotateTest));
	s.push_back(CUTE(libyuvImageResizeTest));
	s.push_back(CUTE(imageResizeTest));
	s.push_back(CUTE(imageResamplerTest));
	return s;
}
