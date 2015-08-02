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
#endif

#ifdef __ENABLE_AVCODEC__
	extern "C" {
#		include <libavcodec/avcodec.h>
	}
#endif

static void avcodecTest() {
	LOG_PRINT("avcodecTest");
#if defined(__ENABLE_AVCODEC__) && defined(__ENABLE_OPENCV__)
	uint32_t width=320, height=240;
	ttLibC_CvCapture *capture = ttLibC_CvCapture_make(0, width, height);
	ttLibC_CvWindow *window   = ttLibC_CvWindow_make("original");
	ttLibC_CvWindow *dec_win  = ttLibC_CvWindow_make("decoded");
	ttLibC_Bgr    *bgr = NULL, *b, *dbgr = NULL;
	ttLibC_Yuv420 *yuv = NULL, *y, *dyuv = NULL;

	AVCodec *codec;
	AVCodecContext *enc = NULL;
	AVFrame *picture = NULL;

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
	uint8_t *outbuf = (uint8_t *)malloc(outbuf_size);

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
		// now convert this yuv with avcodec.
		picture->data[0] = yuv->y_data;
		picture->data[1] = yuv->u_data;
		picture->data[2] = yuv->v_data;

		packet.data = NULL;
		packet.size = 0;
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
		free(outbuf);
	}
	ttLibC_Yuv420_close(&yuv);
	ttLibC_Yuv420_close(&dyuv);
	ttLibC_Bgr_close(&bgr);
	ttLibC_Bgr_close(&dbgr);
	ttLibC_CvWindow_close(&window);
	ttLibC_CvWindow_close(&dec_win);
	ttLibC_CvCapture_close(&capture);
#endif
}

static void h264SequenceParameterSetAnalyzeTest() {
	LOG_PRINT("h264SequenceParameterSetAnalyzeTest");
	uint8_t buf[256];
	uint32_t size = ttLibC_HexUtil_makeBuffer("00000001674D401E924201405FF2E02200000300C800002ED51E2C5C90", buf, 256);
	uint32_t width = ttLibC_H264_getWidth(NULL, buf, size);
	uint32_t height = ttLibC_H264_getHeight(NULL, buf, size);
	LOG_PRINT("%d x %d", width, height);
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
	paramExt.sSpatialLayers[0].sSliceCfg.uiSliceMode = SM_AUTO_SLICE;
	paramExt.sSpatialLayers[0].sSliceCfg.sSliceArgument.uiSliceNum = 0;

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
	decParam.eOutputColorFormat = videoFormatI420;
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
	ttLibC_CvWindow *decode_window = ttLibC_CvWindow_make("decode");
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
}

/**
 * define all test for video package.
 * @param s cute::suite obj
 * @return cute::suite obj
 */
cute::suite videoTests(cute::suite s) {
	s.clear();
	s.push_back(CUTE(avcodecTest));
	s.push_back(CUTE(h264SequenceParameterSetAnalyzeTest));
	s.push_back(CUTE(openh264Test));
	return s;
}
