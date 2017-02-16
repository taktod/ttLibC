/**
 * @file   videoTest.cpp
 * @brief  video test code.
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

#include <ttLibC/util/hexUtil.h>
#include <ttLibC/frame/video/bgr.h>
#include <ttLibC/frame/video/yuv420.h>
#include <ttLibC/frame/video/h264.h>
#include <ttLibC/resampler/imageResampler.h>

#ifdef __ENABLE_OPENCV__
#	include <ttLibC/util/opencvUtil.h>
#endif

#ifdef __ENABLE_OPENH264__
#	include <wels/codec_api.h>
#	include <wels/codec_ver.h>
#endif

#ifdef __ENABLE_AVCODEC__
	extern "C" {
#		include <libavcodec/avcodec.h>
	}
#endif

#ifdef __ENABLE_THEORA__
#	include <theora/theoraenc.h>
#	include <theora/theoradec.h>
#endif


#ifdef __ENABLE_DAALA_ENCODE__
#	include <daala/daalaenc.h>
#endif

#ifdef __ENABLE_DAALA_DECODE__
#	include <daala/daaladec.h>
#endif

static void daalaTest() {
	LOG_PRINT("daalaTest");
#if defined(__ENABLE_DAALA_ENCODE__) && defined(__ENABLE_DAALA_DECODE__) && defined(__ENABLE_OPENCV__)
	uint32_t width = 320, height = 240;
	ttLibC_CvCapture *capture = ttLibC_CvCapture_make(0, width, height);
	ttLibC_CvWindow *window   = ttLibC_CvWindow_make("original");
	ttLibC_CvWindow *dec_win  = ttLibC_CvWindow_make("target");

	ttLibC_Bgr    *bgr = NULL, *dbgr = NULL, *b;
	ttLibC_Yuv420 *yuv = NULL, *dyuv = NULL, *y;

	// decode
	daala_dec_ctx *dec_ctx = NULL;
	daala_setup_info *dec_ds = NULL;
	daala_info dec_di;
	daala_comment dec_dc;
	daala_image dec_img;

	// encode
	daala_enc_ctx *ctx = NULL;
	daala_info di;
	daala_comment dc;
	daala_packet dp;
	daala_image img;
	daala_info_init(&di);
	di.pic_width = width;
	di.pic_height = height;
	di.frame_duration = 1;
	di.bitdepth_mode = OD_BITDEPTH_MODE_8;
	di.timebase_numerator = 15;
	di.timebase_denominator = 1;
	di.pixel_aspect_numerator = 1;
	di.pixel_aspect_denominator = 1;
	di.full_precision_references = 0;
	di.nplanes = 3;
	di.plane_info[0].xdec = 0;
	di.plane_info[0].ydec = 0;
	di.plane_info[1].xdec = 1;
	di.plane_info[1].ydec = 1;
	di.plane_info[2].xdec = 1;
	di.plane_info[2].ydec = 1;
	int result = 0;
	ctx = daala_encode_create(&di);
	if(ctx != NULL) {
		daala_comment_init(&dc);
		daala_info_init(&dec_di);
		daala_comment_init(&dec_dc);
		// for header information.
		while(true) {
			result = daala_encode_flush_header(ctx, &dc, &dp);
			if(result > 0) {
				LOG_PRINT("success to make header.");
				int res = daala_decode_header_in(&dec_di, &dec_dc, &dec_ds, &dp);
			}
			if(result == 0) {
				LOG_PRINT("header is finished.");
				break;
			}
		}
		// header is applyed for dec_di, make decoder now.
		dec_ctx = daala_decode_create(&dec_di, dec_ds);
		if(dec_ds != NULL) {
			daala_setup_free(dec_ds);
		}
		daala_comment_clear(&dc);
		while(true) {
			// capture from opencv
			b = ttLibC_CvCapture_queryFrame(capture, bgr);
			if(b == NULL) {
				break;
			}
			bgr = b;
			// show the image.
			ttLibC_CvWindow_showBgr(window, bgr);
			// make yuv
			y = ttLibC_ImageResampler_makeYuv420FromBgr(yuv, Yuv420Type_planar, bgr);
			if(y == NULL) {
				break;
			}
			yuv = y;
			// put into daala image buffer.
			img.width = width;
			img.height = height;
			img.nplanes = 3;
			img.planes[0].bitdepth = 8;
			img.planes[0].data = yuv->y_data;
			img.planes[0].xdec = 0;
			img.planes[0].ydec = 0;
			img.planes[0].xstride = 1;
			img.planes[0].ystride = yuv->y_stride;
			img.planes[1].bitdepth = 8;
			img.planes[1].data = yuv->u_data;
			img.planes[1].xdec = 1;
 			img.planes[1].ydec = 1;
			img.planes[1].xstride = 1;
			img.planes[1].ystride = yuv->u_stride;
			img.planes[2].bitdepth = 8;
			img.planes[2].data = yuv->v_data;
			img.planes[2].xdec = 1;
 			img.planes[2].ydec = 1;
			img.planes[2].xstride = 1;
			img.planes[2].ystride = yuv->v_stride;
			result = daala_encode_img_in(ctx, &img, yuv->inherit_super.inherit_super.pts);
			if(result == 0) {
				LOG_PRINT("success apply yuv image.");
				while((result = daala_encode_packet_out(ctx, 0, &dp)) != 0) {
					LOG_PRINT("result:%d", result);
					// daala binary is ready, now try to decode.
					result = daala_decode_packet_in(dec_ctx, &dp);
					if(result == 0) {
						// pick up image.
						if(daala_decode_img_out(dec_ctx, &dec_img) == 1) {
							// get image.
							y = ttLibC_Yuv420_make(dyuv, Yuv420Type_planar, width, height, NULL, 0,
									dec_img.planes[0].data, width,
									dec_img.planes[1].data, width >> 1,
									dec_img.planes[2].data, width >> 1,
									true, 0, 1000);
							if(y == NULL) {
								break;
							}
							dyuv = y;
							// back to bgr
							b = ttLibC_ImageResampler_makeBgrFromYuv420(dbgr, BgrType_bgr, dyuv);
							if(b == NULL) {
								break;
							}
							dbgr = b;
							// show image with opencv window.
							ttLibC_CvWindow_showBgr(dec_win, dbgr);
						}
					}
				}
			}
			else {
				LOG_PRINT("failed to apply yuv image:%d", result);
			}
			uint8_t key_char = ttLibC_CvWindow_waitForKeyInput(33);
			if(key_char == Keychar_Esc) {
				break;
			}
		}
		daala_info_clear(&dec_di);
		daala_comment_clear(&dec_dc);
	}
	if(ctx != NULL) {
		daala_encode_free(ctx);
	}
	if(dec_ctx != NULL) {
		daala_decode_free(dec_ctx);
	}
	daala_info_clear(&di);
	ttLibC_CvCapture_close(&capture);
	ttLibC_CvWindow_close(&window);
	ttLibC_CvWindow_close(&dec_win);
	ttLibC_Bgr_close(&dbgr);
	ttLibC_Yuv420_close(&dyuv);
	ttLibC_Bgr_close(&bgr);
	ttLibC_Yuv420_close(&yuv);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

static void theoraTest() {
	LOG_PRINT("theoraTest");
#if defined(__ENABLE_THEORA__) && defined(__ENABLE_OPENCV__)
	uint32_t width = 320, height = 240;
	ttLibC_CvCapture *capture = ttLibC_CvCapture_make(0, width, height);
	ttLibC_CvWindow *window   = ttLibC_CvWindow_make("original");
	ttLibC_CvWindow *dec_win  = ttLibC_CvWindow_make("target");

	ttLibC_Bgr    *bgr = NULL, *dbgr = NULL, *b;
	ttLibC_Yuv420 *yuv = NULL, *dyuv = NULL, *y;

	// for decode
	th_dec_ctx *dec_ctx = NULL;
	th_setup_info *dec_ts = NULL;
	th_info dec_ti;
	th_comment dec_tc;
	ogg_int64_t videobuf_granulepos = 0;

	// for encode
	ogg_packet o_packet;
	th_enc_ctx *ctx = NULL;
	th_info ti;
	th_comment tc;
	th_ycbcr_buffer t_imageBuffer;
	th_info_init(&ti);
	ti.frame_width = width; // should be 16x?
	ti.frame_height = height;
	ti.pic_width = width;
	ti.pic_height = height;
	ti.pic_x = 0;
	ti.pic_y = 0;
	ti.fps_numerator = 15;
	ti.fps_denominator = 1;

	ti.aspect_numerator = 1;
	ti.aspect_denominator = 1;

	ti.pixel_fmt = TH_PF_420;

	ti.target_bitrate = 320000;
	ti.quality = 0;
	ctx = th_encode_alloc(&ti);
	if(ctx != NULL) {
		// set gop?
/*		int gop_size = 15;
		if(th_encode_ctl(ctx, TH_ENCCTL_SET_KEYFRAME_FREQUENCY_FORCE, &gop_size, sizeof(gop_size))) {
			ERR_PRINT("failed to set gop.");
		}*/
		th_comment_init(&tc);
		// dec_ti need to zero clear, or failed to decode.
		th_info_init(&dec_ti);
		th_comment_init(&dec_tc);
//		memset(&dec_ti, 0, sizeof(dec_ti));
		while(th_encode_flushheader(ctx, &tc, &o_packet)) {
//			LOG_DUMP(o_packet.packet, o_packet.bytes, true);
			int res = th_decode_headerin(&dec_ti, &dec_tc, &dec_ts, &o_packet);
			if(res == TH_EBADHEADER) {
				LOG_PRINT("need more binary, or broken data.");
			}
			else if(res > 0) {
				LOG_PRINT("header reading is complete.");
			}
		}
		// setup decoder.
		dec_ctx = th_decode_alloc(&dec_ti, dec_ts);
		switch(dec_ti.pixel_fmt) {
		case TH_PF_420:
			LOG_PRINT("yuv420");
			break;
		default:
			LOG_PRINT("unexpected frame.:%d", dec_ti.pixel_fmt);
			break;
		}
		// what is pp level?
		int pp_level_max;
		int pp_level;
		int pp_inc;
		th_decode_ctl(dec_ctx, TH_DECCTL_GET_PPLEVEL_MAX, &pp_level_max, sizeof(pp_level_max));
		pp_level = pp_level_max;
		th_decode_ctl(dec_ctx, TH_DECCTL_SET_PPLEVEL, &pp_level, sizeof(pp_level));
		pp_inc = 0;
		if(dec_ts != NULL) {
			th_setup_free(dec_ts);
		}
		// now ready for encoder and decoder.
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
			t_imageBuffer[0].width  = yuv->inherit_super.width;
			t_imageBuffer[0].height = yuv->inherit_super.height;
			t_imageBuffer[0].stride = yuv->y_stride;
			t_imageBuffer[0].data   = yuv->y_data;
			t_imageBuffer[1].width  = yuv->inherit_super.width >> 1;
			t_imageBuffer[1].height = yuv->inherit_super.height >> 1;
			t_imageBuffer[1].stride = yuv->u_stride;
			t_imageBuffer[1].data   = yuv->u_data;
			t_imageBuffer[2].width  = yuv->inherit_super.width >> 1;
			t_imageBuffer[2].height = yuv->inherit_super.height >> 1;
			t_imageBuffer[2].stride = yuv->v_stride;
			t_imageBuffer[2].data   = yuv->v_data;
			// register yuv image.
			int result = th_encode_ycbcr_in(ctx, t_imageBuffer);
			if(result != 0) {
				ERR_PRINT("failed to register yuv image.");
				break;
			}
			// pick up ogg packet binary.
			result = th_encode_packetout(ctx, 0, &o_packet);
			if(result != 1) {
				ERR_PRINT("failed to get encode data.");
				break;
			}
			else {
				if(o_packet.granulepos >= 0) {
					th_decode_ctl(dec_ctx, TH_DECCTL_SET_GRANPOS, &o_packet.granulepos, sizeof(o_packet.granulepos));
				}
				if((result = th_decode_packetin(dec_ctx, &o_packet, &videobuf_granulepos)) == 0) {
					th_granule_time(dec_ctx, videobuf_granulepos);
					th_ycbcr_buffer dec_yuv;
					if(th_decode_ycbcr_out(dec_ctx, dec_yuv) == 0) {
						// now convert yuv -> bgr and show image on window.
						y = ttLibC_Yuv420_make(dyuv, Yuv420Type_planar, width, height, NULL, 0, dec_yuv[0].data, dec_yuv[0].stride, dec_yuv[1].data, dec_yuv[1].stride, dec_yuv[2].data, dec_yuv[2].stride, true, 0, 1000);
						if(y == NULL) {
							break;
						}
						dyuv = y;
						b = ttLibC_ImageResampler_makeBgrFromYuv420(dbgr, BgrType_bgr, dyuv);
						if(b == NULL) {
							break;
						}
						dbgr = b;
						ttLibC_CvWindow_showBgr(dec_win, dbgr);
					}
				}
				else {
					ERR_PRINT("failed to decode.:%d", result);
					break;
				}
			}
			uint8_t key_char = ttLibC_CvWindow_waitForKeyInput(33);
			if(key_char == Keychar_Esc) {
				// quit with esc key.
				break;
			}
		}
		th_info_clear(&dec_ti);
		th_comment_clear(&dec_tc);
	}
	th_info_clear(&ti);

	th_comment_clear(&tc);
	if(ctx != NULL) {
		th_encode_free(ctx);
		ctx = NULL;
	}
	if(dec_ctx != NULL) {
		th_decode_free(dec_ctx);
	}
	ttLibC_CvCapture_close(&capture);
	ttLibC_CvWindow_close(&window);
	ttLibC_CvWindow_close(&dec_win);
	ttLibC_Bgr_close(&dbgr);
	ttLibC_Yuv420_close(&dyuv);
	ttLibC_Bgr_close(&bgr);
	ttLibC_Yuv420_close(&yuv);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

static void avcodecTest() {
	LOG_PRINT("avcodecTest");
#if defined(__ENABLE_AVCODEC__) && defined(__ENABLE_OPENCV__)
	uint32_t width=320, height=240;
	ttLibC_CvCapture *capture = ttLibC_CvCapture_make(0, width, height);
	ttLibC_CvWindow *window   = ttLibC_CvWindow_make("original");
	ttLibC_CvWindow *dec_win  = ttLibC_CvWindow_make("target");
	ttLibC_Bgr    *bgr = NULL, *b, *dbgr = NULL;
	ttLibC_Yuv420 *yuv = NULL, *y, *dyuv = NULL;

	AVCodec *codec;
	AVCodecContext *enc = NULL;
	AVFrame *picture = NULL;

	avcodec_register_all();
	codec = avcodec_find_encoder(AV_CODEC_ID_FLV1);
	if(!codec) {
		ERR_PRINT("failed to get codec information.");
	}
	else {
		enc = avcodec_alloc_context3(codec);
		if(enc == NULL) {
			ERR_PRINT("failed to get encode context.");
		}
		else {
			enc->bit_rate = 150000;
			enc->width = width;
			enc->height = height;
			enc->time_base = (AVRational){1,12};
			enc->gop_size = 10;
			enc->max_b_frames = 0;
			enc->pix_fmt = AV_PIX_FMT_YUV420P;
			if(avcodec_open2(enc, codec, NULL) < 0) {
				LOG_PRINT("failed to open codec.");
				av_free(enc);
				enc = NULL;
			}
			else {
				picture = av_frame_alloc();
				if(picture == NULL) {
					ERR_PRINT("failed to make picture.");
					avcodec_close(enc);
					av_free(enc);
					enc = NULL;
				}
				else {
					picture->linesize[0] = enc->width;
					picture->linesize[1] = (enc->width >> 1);
					picture->linesize[2] = (enc->width >> 1);
				}
			}
		}
	}

	AVCodec *decodec;
	AVCodecContext *dec = NULL;
	AVFrame *decpicture = NULL;

	decodec = avcodec_find_decoder(AV_CODEC_ID_FLV1);
	if(!decodec) {
		ERR_PRINT("failed to get codec information.");
	}
	else {
		dec = avcodec_alloc_context3(decodec);
		if(dec == NULL) {
			ERR_PRINT("failed to get decode context");
		}
		else {
			if(avcodec_open2(dec, decodec, NULL) < 0) {
				ERR_PRINT("failed to open codec.");
				av_free(dec);
				dec = NULL;
			}
			else {
				decpicture = av_frame_alloc();
				if(decpicture == NULL) {
					ERR_PRINT("failed to make picture");
					av_free(dec);
					dec = NULL;
				}
			}
		}
	}

	AVPacket packet;
	av_init_packet(&packet);

	int got_output;
	int out_size;
	int outbuf_size = 100000;
	uint8_t *outbuf = (uint8_t *)ttLibC_malloc(outbuf_size);

	while(true) {
		if(!codec || !decodec) {
			break;
		}
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
		// now convert this yuv with avcodec.
		picture->data[0] = yuv->y_data;
		picture->data[1] = yuv->u_data;
		picture->data[2] = yuv->v_data;

		packet.data = NULL;
		packet.size = 0;
		picture->format = AV_PIX_FMT_YUV420P;
		picture->width = width;
		picture->height = height;
		out_size = avcodec_encode_video2(enc, &packet, picture, &got_output);
		if(out_size != 0) {
			LOG_PRINT("failed to encode.");
		}
		else {
			// success..
//			ttLibC_HexUtil_dump(packet.data, packet.size, true);
			// now for decode.
			int got_picture;
			out_size = avcodec_decode_video2(dec, decpicture, &got_picture, &packet);
			if(out_size < 0) {
				LOG_PRINT("failed to decode");
			}
			else {
				if(got_picture) {
					if(dec->pix_fmt == AV_PIX_FMT_YUV420P) {
						y = ttLibC_Yuv420_make(dyuv, Yuv420Type_planar, decpicture->width, decpicture->height, NULL, 0,
								decpicture->data[0], decpicture->linesize[0],
								decpicture->data[1], decpicture->linesize[1],
								decpicture->data[2], decpicture->linesize[2],
								true, 0, 1000);
						if(y == NULL) {
							break;
						}
						dyuv = y;
						b = ttLibC_ImageResampler_makeBgrFromYuv420(dbgr, BgrType_bgr, dyuv);
						if(b == NULL) {
							break;
						}
						dbgr = b;
						ttLibC_CvWindow_showBgr(dec_win, dbgr);
					}
				}
			}
		}
		uint8_t key = ttLibC_CvWindow_waitForKeyInput(10);
		if(key == Keychar_Esc) {
			break;
		}
	}
	if(dec != NULL) {
		avcodec_close(dec);
		av_free(dec);
		av_free(decpicture);
	}
	if(enc != NULL) {
		avcodec_close(enc);
		av_free(enc);
		av_free(picture);
	}
	if(outbuf) {
		ttLibC_free(outbuf);
	}
	ttLibC_Yuv420_close(&yuv);
	ttLibC_Yuv420_close(&dyuv);
	ttLibC_Bgr_close(&bgr);
	ttLibC_Bgr_close(&dbgr);
	ttLibC_CvWindow_close(&window);
	ttLibC_CvWindow_close(&dec_win);
	ttLibC_CvCapture_close(&capture);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

static void h264SequenceParameterSetAnalyzeTest() {
	LOG_PRINT("h264SequenceParameterSetAnalyzeTest");
	uint8_t buf[256];
	uint32_t size = ttLibC_HexUtil_makeBuffer("00000001674D401E924201405FF2E02200000300C800002ED51E2C5C90", buf, 256);
	uint32_t width = ttLibC_H264_getWidth(NULL, buf, size);
	uint32_t height = ttLibC_H264_getHeight(NULL, buf, size);
	LOG_PRINT("%d x %d", width, height);
	ASSERT(ttLibC_Allocator_dump() == 0);
}

static void openh264Test() {
	LOG_PRINT("openh264Test");
#if defined(__ENABLE_OPENH264__) && defined(__ENABLE_OPENCV__)
	uint32_t width = 320;
	uint32_t height = 240;
	ISVCEncoder *encoder = NULL;
	ISVCDecoder *decoder = NULL;
	// setup encoder.
	int res = WelsCreateSVCEncoder(&encoder);
	if(res != 0 || encoder == NULL) {
		ERR_PRINT("failed to create svcEncoder.");
		return;
	}
	SEncParamExt paramExt;
	memset(&paramExt, 0, sizeof(SEncParamExt));
	encoder->GetDefaultParams(&paramExt);
	paramExt.iUsageType = CAMERA_VIDEO_REAL_TIME;
	paramExt.fMaxFrameRate = 20; // fps
	paramExt.iPicWidth = width;
	paramExt.iPicHeight = height;
	paramExt.iTargetBitrate = 500000; // in bit / sec
	paramExt.iMaxBitrate = 500000;
	paramExt.iMinQp = 4;
	paramExt.iMaxQp = 20;

	paramExt.iRCMode = RC_QUALITY_MODE;
	paramExt.iTemporalLayerNum = 1;
	paramExt.iSpatialLayerNum = 1;
	paramExt.bEnableDenoise = 1;
	paramExt.bEnableBackgroundDetection = 0;
	paramExt.bEnableAdaptiveQuant = 1;
	paramExt.bEnableFrameSkip = 1;
	paramExt.bEnableLongTermReference = 0;
	paramExt.iLtrMarkPeriod = 30;
	paramExt.uiIntraPeriod = 15; // GOP
	paramExt.eSpsPpsIdStrategy = CONSTANT_ID;

	paramExt.bPrefixNalAddingCtrl = 0;
	paramExt.iLoopFilterDisableIdc = 1;
	paramExt.iEntropyCodingModeFlag = 0;
	paramExt.iMultipleThreadIdc = 0;
	paramExt.sSpatialLayers[0].iVideoWidth = paramExt.iPicWidth;
	paramExt.sSpatialLayers[0].iVideoHeight = paramExt.iPicHeight;
	paramExt.sSpatialLayers[0].fFrameRate = paramExt.fMaxFrameRate;
	paramExt.sSpatialLayers[0].iSpatialBitrate = paramExt.iTargetBitrate;
	paramExt.sSpatialLayers[0].iMaxSpatialBitrate = paramExt.iMaxBitrate;
#if (OPENH264_MAJOR <= 1) && (OPENH264_MINOR <= 5)
	paramExt.sSpatialLayers[0].sSliceCfg.uiSliceMode = SM_AUTO_SLICE;
	paramExt.sSpatialLayers[0].sSliceCfg.sSliceArgument.uiSliceNum = 0;
#endif
	res = encoder->InitializeExt(&paramExt);
	if(res != 0) {
		ERR_PRINT("failed to initialize encoder params.");
		WelsDestroySVCEncoder(encoder);
		return;
	}
	// append additional data.
	int iTraceLevel = WELS_LOG_QUIET;
	encoder->SetOption(ENCODER_OPTION_TRACE_LEVEL, &iTraceLevel);
	int videoFormat = videoFormatI420;
	encoder->SetOption(ENCODER_OPTION_DATAFORMAT, &videoFormat);
	int iIDRPeriod = 15;
	encoder->SetOption(ENCODER_OPTION_IDR_INTERVAL, &iIDRPeriod);
	bool bval = false;
	encoder->SetOption(ENCODER_OPTION_ENABLE_SPS_PPS_ID_ADDITION, &bval);

	SFrameBSInfo info;
/*	memset(&info, 0, sizeof(SFrameBSInfo));
	res = encoder->EncodeParameterSets(&info);
	if(res != 0) {
		ERR_PRINT("failed to set encode parameter set");
		encoder->Uninitialize();
		WelsDestroySVCEncoder(encoder);
		return;
	}
	// now we are ready for sps and pps.
	const SLayerBSInfo& layerInfo = info.sLayerInfo[0];
	// sps
	LOG_PRINT("nal count:%d", layerInfo.iNalCount);
	uint8_t *data = layerInfo.pBsBuf;
	for(uint32_t i = 0;i < layerInfo.iNalCount;++ i) {
		ttLibC_HexUtil_dump(data, layerInfo.pNalLengthInByte[i], true);
		data += layerInfo.pNalLengthInByte[i];
	}*/

	// setup decoder...
	SBufferInfo bufInfo;
	memset(&bufInfo, 0, sizeof(SBufferInfo));
	res = WelsCreateDecoder(&decoder);
	if(res != 0 || decoder == NULL) {
		ERR_PRINT("failed to make decoder.");
		if(encoder != NULL) {
			encoder->Uninitialize();
			WelsDestroySVCEncoder(encoder);
		}
		return;
	}
	SDecodingParam decParam = {0};
	decParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_SVC;
	decParam.bParseOnly = false;
#if (OPENH264_MAJOR <= 1) && (OPENH264_MINOR <= 5)
	decParam.eOutputColorFormat = videoFormatI420;
#endif
	decParam.uiTargetDqLayer = UCHAR_MAX;
	decParam.eEcActiveIdc = ERROR_CON_SLICE_COPY;
	res = decoder->Initialize(&decParam);
	if(res != 0) {
		ERR_PRINT("failed to initialize decoder.");
		if(encoder != NULL) {
			encoder->Uninitialize();
			WelsDestroySVCEncoder(encoder);
		}
		if(decoder != NULL) {
			WelsDestroyDecoder(decoder);
		}
		return;
	}

	ttLibC_CvCapture *capture = ttLibC_CvCapture_make(0, width, height);
	ttLibC_CvWindow *window = ttLibC_CvWindow_make("original");
	ttLibC_CvWindow *decode_window = ttLibC_CvWindow_make("target");
	// try to encode.
	ttLibC_Bgr *bgr = NULL, *b;
	ttLibC_Yuv420 *yuv = NULL, *y;
	ttLibC_Yuv420 *dyuv = NULL;
	ttLibC_Bgr *dbgr = NULL;
	uint8_t *decodeBuf[3] = {0};
	uint8_t key;
	do {
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
		// encode this yuv into h264.
		yuv = y;
		memset(&info, 0, sizeof(SFrameBSInfo));
		SSourcePicture pic;
		memset(&pic, 0, sizeof(SSourcePicture));
		pic.iPicWidth = yuv->inherit_super.width;
		pic.iPicHeight = yuv->inherit_super.height;
		pic.iColorFormat = videoFormatI420;
		pic.iStride[0] = yuv->y_stride;
		pic.iStride[1] = yuv->u_stride; // u & v stride is shared.
		pic.pData[0] = yuv->y_data;
		pic.pData[1] = yuv->u_data;
		pic.pData[2] = yuv->v_data;
		// TODO timebase is unknown... 1000? (mili sec?)
		pic.uiTimeStamp = yuv->inherit_super.inherit_super.pts * 1000 / yuv->inherit_super.inherit_super.timebase;
		res = encoder->EncodeFrame(&pic, &info);
		if(res == cmResultSuccess) {
			switch(info.eFrameType) {
			case videoFrameTypeI:
				LOG_PRINT("type I"); // possible to call?
				break;
			case videoFrameTypeIPMixed:
				LOG_PRINT("type IPMixed"); // posible to call?
				break;
			case videoFrameTypeInvalid:
				LOG_PRINT("type invalid");
				break;
			case videoFrameTypeSkip:
				// possible to call.
				LOG_PRINT("type skip");
				break;
			case videoFrameTypeIDR:
//				LOG_PRINT("key frame(idr).");
			case videoFrameTypeP:
				int len = 0;
//				printf("iLayerNum:%d ", info.iLayerNum);
				uint8_t *data = info.sLayerInfo[0].pBsBuf;
				for(int i = 0;i < info.iLayerNum;++ i) {
					const SLayerBSInfo& layerInfo = info.sLayerInfo[i];
//					printf("%d: nalCount:%d \n", i + 1, layerInfo.iNalCount);
					for(int j = 0;j < layerInfo.iNalCount;++ j) {
						len += layerInfo.pNalLengthInByte[j];
//						ttLibC_HexUtil_dump(data, layerInfo.pNalLengthInByte[j], true);
//						data += layerInfo.pNalLengthInByte[j];
					}
				}
				// TODO only accept nal style data? no need to have access limit delimiter? ... check them.
//				printf("\n");
				// do decode.
				res = decoder->DecodeFrame2(data, len, decodeBuf, &bufInfo);
				if(res == 0) {
					if(bufInfo.UsrData.sSystemBuffer.iWidth == 0 || bufInfo.UsrData.sSystemBuffer.iHeight == 0) {
						// for the first frame, we could have size 0 image, which will crush opencv work.
						// skip it.
						break;
					}
					// make ttLibC_Yuv420 frame from output of openh264.
					y = ttLibC_Yuv420_make(
							dyuv, Yuv420Type_planar,
							bufInfo.UsrData.sSystemBuffer.iWidth,
							bufInfo.UsrData.sSystemBuffer.iHeight,
							NULL, 0,
							decodeBuf[0], bufInfo.UsrData.sSystemBuffer.iStride[0],
							decodeBuf[1], bufInfo.UsrData.sSystemBuffer.iStride[1],
							decodeBuf[2], bufInfo.UsrData.sSystemBuffer.iStride[1], // stride is provided only 2, u & v are shared.
							true,
							0, 1000);
					if(y == NULL) {
						ERR_PRINT("failed to make yuv420 frame.");
						break; // TODO break is not good, cause keep in the loop.
					}
					dyuv = y;
					b = ttLibC_ImageResampler_makeBgrFromYuv420(dbgr, BgrType_bgr, dyuv);
					if(b == NULL) {
						ERR_PRINT("failed to make bgr frame, from yuv420.");
						break; // TODO break is not good, cause keep in the loop.
					}
					dbgr = b;
					ttLibC_CvWindow_showBgr(decode_window, dbgr);
				}
				break;
			}
		}
		key = ttLibC_CvWindow_waitForKeyInput(10);
	} while(key != Keychar_Esc);

	ttLibC_Bgr_close(&dbgr);
	ttLibC_Yuv420_close(&dyuv);
	ttLibC_Yuv420_close(&yuv);
	ttLibC_Bgr_close(&bgr);
	ttLibC_CvCapture_close(&capture);
	ttLibC_CvWindow_close(&window);
	ttLibC_CvWindow_close(&decode_window);
	if(decoder != NULL) {
		decoder->Uninitialize();
		WelsDestroyDecoder(decoder);
	}
	if(encoder != NULL) {
		encoder->Uninitialize();
		WelsDestroySVCEncoder(encoder);
	}
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

static void yuvCloneTest() {
	LOG_PRINT("yuv data clone test");
#ifdef __ENABLE_OPENCV__
	uint32_t width = 320, height = 240;
	ttLibC_CvCapture *capture = ttLibC_CvCapture_make(0, width, height);
	ttLibC_CvWindow *window = ttLibC_CvWindow_make("test");
	ttLibC_Bgr *bgr = NULL, *b, *rbgr = NULL;
	ttLibC_Yuv420 *yuv = NULL, *y, *ryuv = NULL;
	while(true) {
		b = ttLibC_CvCapture_queryFrame(capture, bgr);
		if(b == NULL) {
			break;
		}
		bgr = b;
		y = ttLibC_ImageResampler_makeYuv420FromBgr(yuv, Yvu420Type_semiPlanar, bgr);
		if(y == NULL) {
			break;
		}
		yuv = y;
		y = ttLibC_Yuv420_clone(ryuv, yuv);
		if(y == NULL) {
			break;
		}
		ryuv =y;
		b = ttLibC_ImageResampler_makeBgrFromYuv420(rbgr, BgrType_bgr, ryuv);
		if(b == NULL) {
			break;
		}
		rbgr = b;
		ttLibC_CvWindow_showBgr(window, rbgr);
		uint8_t key_char = ttLibC_CvWindow_waitForKeyInput(10);
		if(key_char == Keychar_Esc) {
			break;
		}
	}
	ttLibC_Yuv420_close(&yuv);
	ttLibC_Yuv420_close(&ryuv);
	ttLibC_Bgr_close(&bgr);
	ttLibC_Bgr_close(&rbgr);
	ttLibC_CvWindow_close(&window);
	ttLibC_CvCapture_close(&capture);
#endif
	ASSERT(ttLibC_Allocator_dump() == 0);
}

/**
 * define all test for video package.
 * @param s cute::suite obj
 * @return cute::suite obj
 */
cute::suite videoTests(cute::suite s) {
	s.clear();
	s.push_back(CUTE(daalaTest));
	s.push_back(CUTE(theoraTest));
	s.push_back(CUTE(avcodecTest));
	s.push_back(CUTE(h264SequenceParameterSetAnalyzeTest));
	s.push_back(CUTE(openh264Test));
	s.push_back(CUTE(yuvCloneTest));
	return s;
}
