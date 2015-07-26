/*
 * @file   openh264Encoder.cpp
 * @brief  encode h264 with openh264.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/24
 */

#ifdef __ENABLE_OPENH264__

#include "openh264Encoder.h"
#include "../log.h"

#include <wels/codec_api.h>
#include <stdlib.h>
#include <string.h>

/*
 * h264 encoder detail definition.
 */
typedef struct {
	/** inherit data from ttLibC_Openh264Encoder */
	ttLibC_Openh264Encoder inherit_super;
	/** svcencoder object (openh264) */
	ISVCEncoder *encoder;
	/** encode information (openh264) */
	SFrameBSInfo info;
	/** input picture information (openh264) */
	SSourcePicture picture;
	/** configData (allocate data memory.) */
	ttLibC_H264 *configData;
	/** h264 frame (only ref) */
	ttLibC_H264 *h264;
} ttLibC_Encoder_Openh264Encoder_;

typedef ttLibC_Encoder_Openh264Encoder_ ttLibC_Openh264Encoder_;

/*
 * make openh264 encoder(baseline only.) (maybe add more params later)
 * @param width  target width
 * @param height target height
 */
static ttLibC_Openh264Encoder *Openh264Encoder_make(
		uint32_t width,
		uint32_t height) {
	ttLibC_Openh264Encoder_ *encoder = (ttLibC_Openh264Encoder_ *)malloc(sizeof(ttLibC_Openh264Encoder_));
	if(encoder == NULL) {
		ERR_PRINT("failed to alloc encoder object.");
		return NULL;
	}
	uint32_t res = WelsCreateSVCEncoder(&encoder->encoder);
	if(res != 0 || encoder->encoder == NULL) {
		ERR_PRINT("failed to create svcEncoder.");
		free(encoder);
		return NULL;
	}
	SEncParamExt paramExt;
	memset(&paramExt, 0, sizeof(SEncParamExt));
	encoder->encoder->GetDefaultParams(&paramExt);
	paramExt.iUsageType     = CAMERA_VIDEO_REAL_TIME;
	paramExt.fMaxFrameRate  = 5; // fps
	paramExt.iPicWidth      = width;
	paramExt.iPicHeight     = height;
	paramExt.iTargetBitrate = 500000; // in bit / sec
	paramExt.iMaxBitrate    = 500000;
	paramExt.iMinQp         = 4;
	paramExt.iMaxQp         = 20;

	paramExt.iRCMode                    = RC_QUALITY_MODE;
	paramExt.iTemporalLayerNum          = 1;
	paramExt.iSpatialLayerNum           = 1;
	paramExt.bEnableDenoise             = 1;
	paramExt.bEnableBackgroundDetection = 0;
	paramExt.bEnableAdaptiveQuant       = 1;
	paramExt.bEnableFrameSkip           = 1;
	paramExt.bEnableLongTermReference   = 0;
	paramExt.iLtrMarkPeriod             = 30;
	paramExt.uiIntraPeriod              = 15; // GOP
	paramExt.eSpsPpsIdStrategy          = CONSTANT_ID;

	paramExt.bPrefixNalAddingCtrl   = 0;
	paramExt.iLoopFilterDisableIdc  = 1;
	paramExt.iEntropyCodingModeFlag = 0;
	paramExt.iMultipleThreadIdc     = 0;
	paramExt.sSpatialLayers[0].iVideoWidth                         = paramExt.iPicWidth;
	paramExt.sSpatialLayers[0].iVideoHeight                        = paramExt.iPicHeight;
	paramExt.sSpatialLayers[0].fFrameRate                          = paramExt.fMaxFrameRate;
	paramExt.sSpatialLayers[0].iSpatialBitrate                     = paramExt.iTargetBitrate;
	paramExt.sSpatialLayers[0].iMaxSpatialBitrate                  = paramExt.iMaxBitrate;
	paramExt.sSpatialLayers[0].sSliceCfg.uiSliceMode               = SM_AUTO_SLICE;
	paramExt.sSpatialLayers[0].sSliceCfg.sSliceArgument.uiSliceNum = 0;
	res = encoder->encoder->InitializeExt(&paramExt);
	if(res != 0) {
		ERR_PRINT("failed to initialize encoder params.");
		WelsDestroySVCEncoder(encoder->encoder);
		free(encoder);
		return NULL;
	}
	// append additional data.
	int iTraceLevel = WELS_LOG_QUIET;
	encoder->encoder->SetOption(ENCODER_OPTION_TRACE_LEVEL,  &iTraceLevel);
	int videoFormat = videoFormatI420;
	encoder->encoder->SetOption(ENCODER_OPTION_DATAFORMAT,   &videoFormat);
	int iIDRPeriod  = 15;
	encoder->encoder->SetOption(ENCODER_OPTION_IDR_INTERVAL, &iIDRPeriod);
	bool bval       = false;
	encoder->encoder->SetOption(ENCODER_OPTION_ENABLE_SPS_PPS_ID_ADDITION, &bval);

	memset(&encoder->info,    0, sizeof(SFrameBSInfo));
	memset(&encoder->picture, 0, sizeof(SSourcePicture));
	// can check default config Data.(now skip it.)
/*	res = encoder->encoder->EncodeParameterSets(&encoder->info);
	if(res != 0) {
		ERR_PRINT("failed to set encode parameter set");
		encoder->encoder->Uninitialize();
		WelsDestroySVCEncoder(encoder->encoder);
		free(encoder);
		return NULL;
	}
	// now we are ready for sps and pps.
	const SLayerBSInfo& layerInfo = encoder->info.sLayerInfo[0];
	// sps
	LOG_PRINT("nal count:%d", layerInfo.iNalCount);
	uint8_t *data = layerInfo.pBsBuf;
	for(uint32_t i = 0;i < layerInfo.iNalCount;++ i) {
		ttLibC_HexUtil_dump(data, layerInfo.pNalLengthInByte[i], true);
		data += layerInfo.pNalLengthInByte[i];
	} // */
	encoder->inherit_super.width = width;
	encoder->inherit_super.height = height;
	encoder->configData = NULL;
	encoder->h264 = NULL;
	return (ttLibC_Openh264Encoder *)encoder;
}

/*
 * handle encoded data after encode.
 * @param encoder  encoder object
 * @param callback callback func
 * @param ptr      user def data pointer
 */
static void checkEncodedData(
		ttLibC_Openh264Encoder_ *encoder,
		ttLibC_Openh264EncodeFunc callback,
		void *ptr) {
	for(int i = 0;i < encoder->info.iLayerNum;++ i) {
		const SLayerBSInfo& layerInfo = encoder->info.sLayerInfo[i];
		// check nal and decide the size of data.
		// for slice(with first mb in slice & 0x10 != 0x00 tells new frame.)
		uint8_t *buf = layerInfo.pBsBuf;
		uint8_t *target_buf = buf;
		size_t target_size = 0;
		ttLibC_H264_NalInfo nalInfo;

		ttLibC_H264_Type target_type = H264Type_unknown;
		for(int j = 0;j < layerInfo.iNalCount;++ j) {
			if(!ttLibC_H264_getNalInfo(&nalInfo, buf, layerInfo.pNalLengthInByte[j])) {
				// failed to get data.
				ERR_PRINT("h264 data is corrupted.");
				return;
			}
			target_size += layerInfo.pNalLengthInByte[j];
			switch(nalInfo.nal_unit_type) {
			case H264NalType_error:
				ERR_PRINT("unknown nal type is found.");
				break;
			default:
				LOG_PRINT("nal type for not implemented now.");
				break;
			case H264NalType_slice:
				if(target_type != H264Type_slice
				|| (*(buf + nalInfo.data_pos + 1) & 0x80) != 0x00) {
					if(target_type != H264Type_unknown) {
						// not tested yet.(no way to here with openh264?)
						ERR_PRINT("need to make h264 object.");
					}
					// for first data. initialize target type.
					target_type = H264Type_unknown;
				}
				switch(target_type) {
				case H264Type_unknown:
					target_type = H264Type_slice;
					break;
				case H264Type_slice:
					break;
				default:
					ERR_PRINT("never come here.");
					break;
				}
				break;
			case H264NalType_sliceIDR:
				if(target_type != H264Type_sliceIDR
				|| (*(buf + nalInfo.data_pos + 1) & 0x80) != 0x00) {
					if(target_type != H264Type_unknown) {
						// not tested yet.(no way to here with openh264?)
						ERR_PRINT("need to make h264 object.");
					}
					// for first data. initialize target type.
					target_type = H264Type_unknown;
				}
				switch(target_type) {
				case H264Type_unknown:
					target_type = H264Type_sliceIDR;
					break;
				case H264Type_sliceIDR:
					break;
				default:
					ERR_PRINT("never come here.");
					break;
				}
				break;
			case H264NalType_sequenceParameterSet:
			case H264NalType_pictureParameterSet:
				if(target_type != H264Type_configData) {
					if(target_type != H264Type_unknown) {
						// not tested yet.(no way to here with openh264?)
						ERR_PRINT("need to make h264 object.");
					}
					// for first data. initialize target type.
					target_type = H264Type_unknown;
				}
				switch(target_type) {
				case H264Type_unknown:
					target_type = H264Type_configData;
					break;
				case H264Type_configData:
					break;
				default:
					ERR_PRINT("never come here.");
					break;
				}
				break;
			}
			// go next position.
			buf += layerInfo.pNalLengthInByte[j];
		}
		ttLibC_H264 *h264 = NULL;
		if(target_type == H264Type_configData) {
			h264 = ttLibC_H264_make(
					encoder->configData,
					target_type,
					encoder->inherit_super.width,
					encoder->inherit_super.height,
					target_buf,
					target_size,
					false,
					encoder->info.uiTimeStamp,
					1000);
			if(h264 != NULL) {
				encoder->configData = h264;
				callback(ptr, encoder->configData);
			}
			else {
				ERR_PRINT("failed to make h264 object(type configData)");
			}
		}
		else {
			h264 = ttLibC_H264_make(
					encoder->h264,
					target_type,
					encoder->inherit_super.width,
					encoder->inherit_super.height,
					target_buf,
					target_size,
					true,
					encoder->info.uiTimeStamp,
					1000);
			if(h264 != NULL) {
				encoder->h264 = h264;
				callback(ptr, encoder->h264);
			}
			else {
				ERR_PRINT("failed to make h264 object(type h264)");
			}
		}
	}
}

/*
 * encode frame.
 * @param encoder  openh264 encoder object
 * @param yuv420   source yuv420 data.
 * @param callback callback func for h264 creation.
 * @param ptr      pointer for user def value, which will call in callback.
 */
static void Openh264Encoder_encode(
		ttLibC_Openh264Encoder *encoder,
		ttLibC_Yuv420 *yuv,
		ttLibC_Openh264EncodeFunc callback,
		void *ptr) {
	if(encoder == NULL) {
		return;
	}
	if(yuv == NULL) {
		return;
	}
	switch(yuv->type) {
	case Yuv420Type_planar:
		break;
	case Yuv420Type_semiPlanar:
	case Yvu420Type_planar:
	case Yvu420Type_semiPlanar:
		ERR_PRINT("support only yuv420 planar.");
		return;
	}
	ttLibC_Openh264Encoder_ *encoder_ = (ttLibC_Openh264Encoder_ *)encoder;
	encoder_->picture.iPicWidth    = yuv->inherit_super.width;
	encoder_->picture.iPicHeight   = yuv->inherit_super.height;
	encoder_->picture.iColorFormat = videoFormatI420;
	encoder_->picture.iStride[0]   = yuv->y_stride;
	encoder_->picture.iStride[1]   = yuv->u_stride;
	encoder_->picture.pData[0]     = yuv->y_data;
	encoder_->picture.pData[1]     = yuv->u_data;
	encoder_->picture.pData[2]     = yuv->v_data;
	encoder_->picture.uiTimeStamp  = yuv->inherit_super.inherit_super.pts * 1000 / yuv->inherit_super.inherit_super.timebase;
	uint32_t res = encoder_->encoder->EncodeFrame(&encoder_->picture, &encoder_->info);
	if(res != cmResultSuccess) {
		ERR_PRINT("failed to encode picture.");
		return;
	}
/*
	switch(encoder_->info.eFrameType) {
	case videoFrameTypeI:
		LOG_PRINT("i timestamp: in %llu, out %llu", yuv->inherit_super.inherit_super.pts, encoder_->info.uiTimeStamp);
		break;
	case videoFrameTypeIPMixed:
		LOG_PRINT("ipmixed timestamp: in %llu, out %llu", yuv->inherit_super.inherit_super.pts, encoder_->info.uiTimeStamp);
		break;
	case videoFrameTypeInvalid:
		LOG_PRINT("invalid timestamp: in %llu, out %llu", yuv->inherit_super.inherit_super.pts, encoder_->info.uiTimeStamp);
		break;
	case videoFrameTypeSkip:
		LOG_PRINT("skip timestamp: in %llu, out %llu", yuv->inherit_super.inherit_super.pts, encoder_->info.uiTimeStamp);
		break;
	case videoFrameTypeIDR:
		LOG_PRINT("idr timestamp: in %llu, out %llu", yuv->inherit_super.inherit_super.pts, encoder_->info.uiTimeStamp);
		break;
	case videoFrameTypeP:
		LOG_PRINT("p timestamp: in %llu, out %llu", yuv->inherit_super.inherit_super.pts, encoder_->info.uiTimeStamp);
		break;
	}*/
	checkEncodedData(encoder_, callback, ptr);
}

/*
 * close openh264 encoder
 * @param encoder
 */
static void Openh264Encoder_close(ttLibC_Openh264Encoder **encoder) {
	ttLibC_Openh264Encoder_ *target = (ttLibC_Openh264Encoder_ *)*encoder;
	if(target == NULL) {
		return;
	}
	if(target->encoder != NULL) {
		target->encoder->Uninitialize();
		WelsDestroySVCEncoder(target->encoder);
	}
	ttLibC_H264_close(&target->h264);
	ttLibC_H264_close(&target->configData);
	free(target);
	*encoder = NULL;
}

extern "C" {

/*
 * call make for c code
 */
ttLibC_Openh264Encoder *ttLibC_Openh264Encoder_make(
		uint32_t width,
		uint32_t height) {
	return Openh264Encoder_make(
			width,
			height);
}

/*
 * call encode for c code
 */
void ttLibC_Openh264Encoder_encode(
		ttLibC_Openh264Encoder* encoder,
		ttLibC_Yuv420 *yuv420,
		ttLibC_Openh264EncodeFunc callback,
		void *ptr) {
	Openh264Encoder_encode(
			encoder, yuv420, callback, ptr);
}

/*
 * call close for c code
 */
void ttLibC_Openh264Encoder_close(ttLibC_Openh264Encoder **encoder) {
	Openh264Encoder_close(encoder);
}

} /* extern "C" */

#endif
