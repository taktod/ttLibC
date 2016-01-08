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
#include "../allocator.h"

#include <wels/codec_api.h>
#include <stdio.h>
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
 * make openh264 encoder
 * @param paramExt SEncParamExt struct obj.
 * @return openh264 encoder
 */
static ttLibC_Openh264Encoder *Openh264Encoder_make(SEncParamExt *pParamExt) {
	ttLibC_Openh264Encoder_ *encoder = (ttLibC_Openh264Encoder_ *)ttLibC_malloc(sizeof(ttLibC_Openh264Encoder_));
	if(encoder == NULL) {
		ERR_PRINT("failed to alloc encoder object.");
		return NULL;
	}
	uint32_t res = WelsCreateSVCEncoder(&encoder->encoder);
	if(res != 0 || encoder->encoder == NULL) {
		ERR_PRINT("failed to create svcEncoder.");
		ttLibC_free(encoder);
		return NULL;
	}
	SEncParamExt paramExt;
	memset(&paramExt, 0, sizeof(SEncParamExt));
	encoder->encoder->GetDefaultParams(&paramExt);
	paramExt.iUsageType     = pParamExt->iUsageType;
	paramExt.fMaxFrameRate  = pParamExt->fMaxFrameRate;
	paramExt.iPicWidth      = pParamExt->iPicWidth;
	paramExt.iPicHeight     = pParamExt->iPicHeight;
	paramExt.iTargetBitrate = pParamExt->iTargetBitrate; // in bit / sec
	paramExt.iMaxBitrate    = pParamExt->iMaxBitrate;
	paramExt.iMinQp         = pParamExt->iMinQp;
	paramExt.iMaxQp         = pParamExt->iMaxQp;

	paramExt.iRCMode                    = pParamExt->iRCMode;
	paramExt.iTemporalLayerNum          = pParamExt->iTemporalLayerNum;
	paramExt.iSpatialLayerNum           = pParamExt->iSpatialLayerNum;
	paramExt.bEnableDenoise             = pParamExt->bEnableDenoise;
	paramExt.bEnableBackgroundDetection = pParamExt->bEnableBackgroundDetection;
	paramExt.bEnableAdaptiveQuant       = pParamExt->bEnableAdaptiveQuant;
	paramExt.bEnableFrameSkip           = pParamExt->bEnableFrameSkip;
	paramExt.bEnableLongTermReference   = pParamExt->bEnableLongTermReference;
	paramExt.iLtrMarkPeriod             = pParamExt->iLtrMarkPeriod;
	paramExt.uiIntraPeriod              = pParamExt->uiIntraPeriod; // GOP
	paramExt.eSpsPpsIdStrategy          = pParamExt->eSpsPpsIdStrategy;

	paramExt.bPrefixNalAddingCtrl   = pParamExt->bPrefixNalAddingCtrl;
	paramExt.iLoopFilterDisableIdc  = pParamExt->iLoopFilterDisableIdc;
	paramExt.iEntropyCodingModeFlag = pParamExt->iEntropyCodingModeFlag;
	paramExt.iMultipleThreadIdc     = pParamExt->iMultipleThreadIdc;
	for(int i = 0;i < MAX_SPATIAL_LAYER_NUM;++ i) {
		if(i == 0 || pParamExt->sSpatialLayers[i].iVideoWidth != 0)
			paramExt.sSpatialLayers[i].iVideoWidth                         = pParamExt->sSpatialLayers[0].iVideoWidth;
		if(i == 0 || pParamExt->sSpatialLayers[i].iVideoHeight != 0)
			paramExt.sSpatialLayers[i].iVideoHeight                        = pParamExt->sSpatialLayers[0].iVideoHeight;
		if(i == 0 || pParamExt->sSpatialLayers[i].fFrameRate != 0)
			paramExt.sSpatialLayers[i].fFrameRate                          = pParamExt->sSpatialLayers[0].fFrameRate;
		if(i == 0 || pParamExt->sSpatialLayers[i].iSpatialBitrate != 0)
			paramExt.sSpatialLayers[i].iSpatialBitrate                     = pParamExt->sSpatialLayers[0].iSpatialBitrate;
		if(i == 0 || pParamExt->sSpatialLayers[i].iMaxSpatialBitrate != 0)
			paramExt.sSpatialLayers[i].iMaxSpatialBitrate                  = pParamExt->sSpatialLayers[0].iMaxSpatialBitrate;
		if(i == 0 || pParamExt->sSpatialLayers[i].sSliceCfg.uiSliceMode != 0)
			paramExt.sSpatialLayers[i].sSliceCfg.uiSliceMode               = pParamExt->sSpatialLayers[0].sSliceCfg.uiSliceMode;
		if(i == 0 || pParamExt->sSpatialLayers[i].sSliceCfg.sSliceArgument.uiSliceNum != 0)
			paramExt.sSpatialLayers[i].sSliceCfg.sSliceArgument.uiSliceNum = pParamExt->sSpatialLayers[0].sSliceCfg.sSliceArgument.uiSliceNum;
		if(pParamExt->sSpatialLayers[i].uiProfileIdc != 0)
			paramExt.sSpatialLayers[i].uiProfileIdc                        = pParamExt->sSpatialLayers[i].uiProfileIdc;
		if(pParamExt->sSpatialLayers[i].uiLevelIdc != 0)
			paramExt.sSpatialLayers[i].uiLevelIdc                          = pParamExt->sSpatialLayers[i].uiLevelIdc;
		if(pParamExt->sSpatialLayers[i].sSliceCfg.sSliceArgument.uiSliceSizeConstraint != 0) {
			paramExt.sSpatialLayers[i].sSliceCfg.sSliceArgument.uiSliceSizeConstraint =
					pParamExt->sSpatialLayers[i].sSliceCfg.sSliceArgument.uiSliceSizeConstraint;
		}
		for(int j = 0;j < MAX_SLICES_NUM_TMP;++ j) {
			if(pParamExt->sSpatialLayers[i].sSliceCfg.sSliceArgument.uiSliceMbNum[j] != 0) {
				paramExt.sSpatialLayers[i].sSliceCfg.sSliceArgument.uiSliceMbNum[j] =
						pParamExt->sSpatialLayers[i].sSliceCfg.sSliceArgument.uiSliceMbNum[j];
			}
		}
	}

	if(pParamExt->iComplexityMode != 0)          paramExt.iComplexityMode          = pParamExt->iComplexityMode;
	if(pParamExt->iNumRefFrame != 0)             paramExt.iNumRefFrame             = pParamExt->iNumRefFrame;
	if(pParamExt->bEnableSSEI != 0)              paramExt.bEnableSSEI              = pParamExt->bEnableSSEI;
	if(pParamExt->bSimulcastAVC != 0)            paramExt.bSimulcastAVC            = pParamExt->bSimulcastAVC;
	if(pParamExt->iPaddingFlag != 0)             paramExt.iPaddingFlag             = pParamExt->iPaddingFlag;
	if(pParamExt->uiMaxNalSize != 0)             paramExt.uiMaxNalSize             = pParamExt->uiMaxNalSize;
	if(pParamExt->iLTRRefNum != 0)               paramExt.iLTRRefNum               = pParamExt->iLTRRefNum;
	if(pParamExt->iLoopFilterAlphaC0Offset != 0) paramExt.iLoopFilterAlphaC0Offset = pParamExt->iLoopFilterAlphaC0Offset;
	if(pParamExt->iLoopFilterBetaOffset != 0)    paramExt.iLoopFilterBetaOffset    = pParamExt->iLoopFilterBetaOffset;
	if(pParamExt->bEnableFrameCroppingFlag != 0) paramExt.bEnableFrameCroppingFlag = pParamExt->bEnableFrameCroppingFlag;
	if(pParamExt->bEnableSceneChangeDetect != 0) paramExt.bEnableSceneChangeDetect = pParamExt->bEnableSceneChangeDetect;
	if(pParamExt->bIsLosslessLink != 0)          paramExt.bIsLosslessLink          = pParamExt->bIsLosslessLink;

	res = encoder->encoder->InitializeExt(&paramExt);
	if(res != 0) {
		ERR_PRINT("failed to initialize encoder params.");
		WelsDestroySVCEncoder(encoder->encoder);
		ttLibC_free(encoder);
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
	// can check default configData (sps and pps).(now skip it.)
/*	res = encoder->encoder->EncodeParameterSets(&encoder->info);
	if(res != 0) {
		ERR_PRINT("failed to set encode parameter set");
		encoder->encoder->Uninitialize();
		WelsDestroySVCEncoder(encoder->encoder);
		ttLibC_free(encoder);
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
	encoder->inherit_super.width  = pParamExt->iPicWidth;
	encoder->inherit_super.height = pParamExt->iPicHeight;
	encoder->configData = NULL;
	encoder->h264       = NULL;
	return (ttLibC_Openh264Encoder *)encoder;
}

/*
 * handle encoded data after encode.
 * @param encoder  encoder object
 * @param callback callback func
 * @param ptr      user def data pointer
 */
static bool checkEncodedData(
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
			if(!ttLibC_H264_getNalInfo(&nalInfo, buf, (size_t)layerInfo.pNalLengthInByte[j])) {
				// failed to get data.
				ERR_PRINT("h264 data is corrupted.");
				return false;
			}
			target_size += layerInfo.pNalLengthInByte[j];
			switch(nalInfo.nal_unit_type) {
			case H264NalType_error:
				ERR_PRINT("unknown nal type is found.");
				return false;
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
					return false;
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
					return false;
				}
				break;
			case H264NalType_sequenceParameterSet:
			case H264NalType_pictureParameterSet:
				if(target_type != H264Type_configData) {
					if(target_type != H264Type_unknown) {
						// not tested yet.(no way to here with openh264?)
						ERR_PRINT("need to make h264 object.");
						return false;
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
					return false;
				}
				break;
			}
			// go next position.
			buf += layerInfo.pNalLengthInByte[j];
		}
		ttLibC_H264 *h264 = NULL;
		if(target_type == H264Type_configData) {
			// put the memory hold. in order to refer from enoder.
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
				if(!callback(ptr, encoder->configData)) {
					return false;
				}
			}
			else {
				ERR_PRINT("failed to make h264 object(type configData)");
				return false;
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
				if(!callback(ptr, encoder->h264)) {
					return false;
				}
			}
			else {
				ERR_PRINT("failed to make h264 object(type h264)");
				return false;
			}
		}
	}
	return true;
}

/*
 * encode frame.
 * @param encoder  openh264 encoder object
 * @param yuv420   source yuv420 data.
 * @param callback callback func for h264 creation.
 * @param ptr      pointer for user def value, which will call in callback.
 */
static bool Openh264Encoder_encode(
		ttLibC_Openh264Encoder *encoder,
		ttLibC_Yuv420 *yuv,
		ttLibC_Openh264EncodeFunc callback,
		void *ptr) {
	if(encoder == NULL) {
		return false;
	}
	if(yuv == NULL) {
		return true;
	}
	switch(yuv->type) {
	case Yuv420Type_planar:
		break;
	case Yuv420Type_semiPlanar:
	case Yvu420Type_planar:
	case Yvu420Type_semiPlanar:
		ERR_PRINT("support only yuv420 planar.");
		return false;
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
		return false;
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
//		LOG_PRINT("skip timestamp: in %llu, out %llu", yuv->inherit_super.inherit_super.pts, encoder_->info.uiTimeStamp);
		break;
	case videoFrameTypeIDR:
		LOG_PRINT("idr timestamp: in %llu, out %llu", yuv->inherit_super.inherit_super.pts, encoder_->info.uiTimeStamp);
		break;
	case videoFrameTypeP:
		LOG_PRINT("p timestamp: in %llu, out %llu", yuv->inherit_super.inherit_super.pts, encoder_->info.uiTimeStamp);
		break;
	}*/
	return checkEncodedData(encoder_, callback, ptr);
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
	ttLibC_free(target);
	*encoder = NULL;
}

extern "C" {

/*
 * call make for c code
 */
ttLibC_Openh264Encoder *ttLibC_Openh264Encoder_make(
		uint32_t width,
		uint32_t height) {
	SEncParamExt paramExt;
	ttLibC_Openh264Encoder_getDefaultSEncParamExt(&paramExt, width, height);
	return Openh264Encoder_make(&paramExt);
}

/*
 * call make for c code
 */
ttLibC_Openh264Encoder *ttLibC_Openh264Encoder_make_ex(
		uint32_t width,
		uint32_t height,
		uint32_t max_quantizer,
		uint32_t min_quantizer,
		uint32_t bitrate) {
	SEncParamExt paramExt;
	ttLibC_Openh264Encoder_getDefaultSEncParamExt(&paramExt, width, height);
	paramExt.iMaxQp         = max_quantizer;
	paramExt.iMinQp         = min_quantizer;
	paramExt.iMaxBitrate    = bitrate;
	paramExt.iTargetBitrate = bitrate;
	paramExt.sSpatialLayers[0].iSpatialBitrate = bitrate;
	paramExt.sSpatialLayers[0].iMaxSpatialBitrate = bitrate;
	return Openh264Encoder_make(&paramExt);
}

void ttLibC_Openh264Encoder_getDefaultSEncParamExt(
		void *paramExt,
		uint32_t width,
		uint32_t height) {
	SEncParamExt *pParamExt = (SEncParamExt *)paramExt;
	memset(paramExt, 0, sizeof(SEncParamExt));
	pParamExt->iUsageType     = CAMERA_VIDEO_REAL_TIME;
	pParamExt->fMaxFrameRate  = 15;
	pParamExt->iPicWidth      = width;
	pParamExt->iPicHeight     = height;
	pParamExt->iTargetBitrate = 650000;
	pParamExt->iMaxBitrate    = 650000;
	pParamExt->iMinQp         = 4;
	pParamExt->iMaxQp         = 20;

	pParamExt->iRCMode                    = RC_BITRATE_MODE;
	pParamExt->iTemporalLayerNum          = 1;
	pParamExt->iSpatialLayerNum           = 1;
	pParamExt->bEnableDenoise             = true;
	pParamExt->bEnableBackgroundDetection = false;
	pParamExt->bEnableAdaptiveQuant       = true;
	pParamExt->bEnableFrameSkip           = true;
	pParamExt->bEnableLongTermReference   = false;
	pParamExt->iLtrMarkPeriod             = 30;
	pParamExt->uiIntraPeriod              = 15;
	pParamExt->eSpsPpsIdStrategy          = CONSTANT_ID;

	pParamExt->bPrefixNalAddingCtrl   = false;
	pParamExt->iLoopFilterDisableIdc  = 1;
	pParamExt->iEntropyCodingModeFlag = 0;
	pParamExt->iMultipleThreadIdc     = 0;

	pParamExt->sSpatialLayers[0].iVideoWidth                         = pParamExt->iPicWidth;
	pParamExt->sSpatialLayers[0].iVideoHeight                        = pParamExt->iPicHeight;
	pParamExt->sSpatialLayers[0].fFrameRate                          = pParamExt->fMaxFrameRate;
	pParamExt->sSpatialLayers[0].iSpatialBitrate                     = pParamExt->iTargetBitrate;
	pParamExt->sSpatialLayers[0].iMaxSpatialBitrate                  = pParamExt->iMaxBitrate;
	pParamExt->sSpatialLayers[0].sSliceCfg.uiSliceMode               = SM_AUTO_SLICE;
	pParamExt->sSpatialLayers[0].sSliceCfg.sSliceArgument.uiSliceNum = 0;
}

ttLibC_Openh264Encoder *ttLibC_Openh264Encoder_makeWithSEncParamExt(void *paramExt) {
	return Openh264Encoder_make((SEncParamExt *)paramExt);
}

/*
 * call encode for c code
 */
bool ttLibC_Openh264Encoder_encode(
		ttLibC_Openh264Encoder* encoder,
		ttLibC_Yuv420 *yuv420,
		ttLibC_Openh264EncodeFunc callback,
		void *ptr) {
	return Openh264Encoder_encode(
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
