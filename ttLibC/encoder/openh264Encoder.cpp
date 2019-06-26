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
#include "../_log.h"
#include "../allocator.h"
#include "../util/dynamicBufferUtil.h"

#include <wels/codec_api.h>
#include <wels/codec_ver.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define Param_set(a, b) {if(a != b) {a = b;}}

/*
 * h264 encoder detail definition.
 */
typedef struct ttLibC_Encoder_Openh264Encoder_ {
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
	/** for control idr */
	int32_t idr_interval_count;
	/** reduce_mode */
	bool is_reduce_mode;
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
	int iTraceLevel = WELS_LOG_QUIET;
	encoder->encoder->SetOption(ENCODER_OPTION_TRACE_LEVEL,  &iTraceLevel);
	SEncParamExt paramExt;
	memset(&paramExt, 0, sizeof(SEncParamExt));
	encoder->encoder->GetDefaultParams(&paramExt);
	Param_set(paramExt.iUsageType,     pParamExt->iUsageType);
	Param_set(paramExt.iPicWidth,      pParamExt->iPicWidth);
	Param_set(paramExt.iPicHeight,     pParamExt->iPicHeight);
	Param_set(paramExt.iTargetBitrate, pParamExt->iTargetBitrate);
	Param_set(paramExt.iRCMode,        pParamExt->iRCMode);
	Param_set(paramExt.fMaxFrameRate,  pParamExt->fMaxFrameRate);

	Param_set(paramExt.iTemporalLayerNum, pParamExt->iTemporalLayerNum);
	Param_set(paramExt.iSpatialLayerNum,  pParamExt->iSpatialLayerNum);
	for(int i = 0;i < MAX_SPATIAL_LAYER_NUM;++ i) {
		Param_set(paramExt.sSpatialLayers[i].iVideoWidth,        pParamExt->sSpatialLayers[i].iVideoWidth);
		Param_set(paramExt.sSpatialLayers[i].iVideoHeight,       pParamExt->sSpatialLayers[i].iVideoHeight);
		Param_set(paramExt.sSpatialLayers[i].fFrameRate,         pParamExt->sSpatialLayers[i].fFrameRate);
		Param_set(paramExt.sSpatialLayers[i].iSpatialBitrate,    pParamExt->sSpatialLayers[i].iSpatialBitrate);
		Param_set(paramExt.sSpatialLayers[i].iMaxSpatialBitrate, pParamExt->sSpatialLayers[i].iMaxSpatialBitrate);
		Param_set(paramExt.sSpatialLayers[i].uiProfileIdc,       pParamExt->sSpatialLayers[i].uiProfileIdc);
		Param_set(paramExt.sSpatialLayers[i].uiLevelIdc,         pParamExt->sSpatialLayers[i].uiLevelIdc);
		Param_set(paramExt.sSpatialLayers[i].iDLayerQp,          pParamExt->sSpatialLayers[i].iDLayerQp);
#if (OPENH264_MAJOR >= 1) && (OPENH264_MINOR >= 6)
		Param_set(paramExt.sSpatialLayers[i].bVideoSignalTypePresent,   pParamExt->sSpatialLayers[i].bVideoSignalTypePresent);
		Param_set(paramExt.sSpatialLayers[i].uiVideoFormat,             pParamExt->sSpatialLayers[i].uiVideoFormat);
		Param_set(paramExt.sSpatialLayers[i].bFullRange,                pParamExt->sSpatialLayers[i].bFullRange);
		Param_set(paramExt.sSpatialLayers[i].bColorDescriptionPresent,  pParamExt->sSpatialLayers[i].bColorDescriptionPresent);
		Param_set(paramExt.sSpatialLayers[i].uiColorPrimaries,          pParamExt->sSpatialLayers[i].uiColorPrimaries);
		Param_set(paramExt.sSpatialLayers[i].uiTransferCharacteristics, pParamExt->sSpatialLayers[i].uiTransferCharacteristics);
		Param_set(paramExt.sSpatialLayers[i].uiColorMatrix,             pParamExt->sSpatialLayers[i].uiColorMatrix);
		Param_set(paramExt.sSpatialLayers[i].sSliceArgument.uiSliceMode,           pParamExt->sSpatialLayers[i].sSliceArgument.uiSliceMode);
		Param_set(paramExt.sSpatialLayers[i].sSliceArgument.uiSliceNum,            pParamExt->sSpatialLayers[i].sSliceArgument.uiSliceNum);
		Param_set(paramExt.sSpatialLayers[i].sSliceArgument.uiSliceSizeConstraint, pParamExt->sSpatialLayers[i].sSliceArgument.uiSliceSizeConstraint);
		for(int j = 0, max = MAX_SLICES_NUM_TMP;j < max;++ j) {
			Param_set(paramExt.sSpatialLayers[i].sSliceArgument.uiSliceMbNum[j],   pParamExt->sSpatialLayers[i].sSliceArgument.uiSliceMbNum[j]);
		}
#else
		Param_set(paramExt.sSpatialLayers[i].sSliceCfg.uiSliceMode,                          pParamExt->sSpatialLayers[i].sSliceCfg.uiSliceMode);
		Param_set(paramExt.sSpatialLayers[i].sSliceCfg.sSliceArgument.uiSliceNum,            pParamExt->sSpatialLayers[i].sSliceCfg.sSliceArgument.uiSliceNum);
		Param_set(paramExt.sSpatialLayers[i].sSliceCfg.sSliceArgument.uiSliceSizeConstraint, pParamExt->sSpatialLayers[i].sSliceCfg.sSliceArgument.uiSliceSizeConstraint);
		for(int j = 0, max = MAX_SLICES_NUM_TMP;j < max;++ j) {
			Param_set(paramExt.sSpatialLayers[i].sSliceCfg.sSliceArgument.uiSliceMbNum[j],   pParamExt->sSpatialLayers[i].sSliceCfg.sSliceArgument.uiSliceMbNum[j]);
		}
#endif
	}
	Param_set(paramExt.iComplexityMode, pParamExt->iComplexityMode);
	Param_set(paramExt.uiIntraPeriod,   pParamExt->uiIntraPeriod); // Gop
	Param_set(paramExt.iNumRefFrame,    pParamExt->iNumRefFrame);

	Param_set(paramExt.eSpsPpsIdStrategy,    pParamExt->eSpsPpsIdStrategy);
	Param_set(paramExt.bPrefixNalAddingCtrl, pParamExt->bPrefixNalAddingCtrl);

	Param_set(paramExt.bEnableSSEI,            pParamExt->bEnableSSEI);
	Param_set(paramExt.bSimulcastAVC,          pParamExt->bSimulcastAVC);
	Param_set(paramExt.iPaddingFlag,           pParamExt->iPaddingFlag);
	Param_set(paramExt.iEntropyCodingModeFlag, pParamExt->iEntropyCodingModeFlag);

	Param_set(paramExt.bEnableFrameSkip, pParamExt->bEnableFrameSkip);
	Param_set(paramExt.iMaxBitrate,      pParamExt->iMaxBitrate);
	Param_set(paramExt.iMinQp,           pParamExt->iMinQp);
	Param_set(paramExt.iMaxQp,           pParamExt->iMaxQp);
	Param_set(paramExt.uiMaxNalSize,     pParamExt->uiMaxNalSize);

	Param_set(paramExt.bEnableLongTermReference, pParamExt->bEnableLongTermReference);
	Param_set(paramExt.iLTRRefNum,               pParamExt->iLTRRefNum);
	Param_set(paramExt.iLtrMarkPeriod,           pParamExt->iLtrMarkPeriod);
	Param_set(paramExt.iMultipleThreadIdc,       pParamExt->iMultipleThreadIdc);
#if (OPENH264_MAJOR >= 1) && (OPENH264_MINOR >= 6)
	Param_set(paramExt.bUseLoadBalancing,        pParamExt->bUseLoadBalancing);
#endif
	Param_set(paramExt.iLoopFilterDisableIdc,      pParamExt->iLoopFilterDisableIdc);
	Param_set(paramExt.iLoopFilterAlphaC0Offset,   pParamExt->iLoopFilterAlphaC0Offset);
	Param_set(paramExt.iLoopFilterBetaOffset,      pParamExt->iLoopFilterBetaOffset);
	Param_set(paramExt.bEnableDenoise,             pParamExt->bEnableDenoise);
	Param_set(paramExt.bEnableAdaptiveQuant,       pParamExt->bEnableAdaptiveQuant);
	Param_set(paramExt.bEnableBackgroundDetection, pParamExt->bEnableBackgroundDetection);
	Param_set(paramExt.bEnableFrameCroppingFlag,   pParamExt->bEnableFrameCroppingFlag);
	Param_set(paramExt.bEnableSceneChangeDetect,   pParamExt->bEnableSceneChangeDetect);
	Param_set(paramExt.bIsLosslessLink,            pParamExt->bIsLosslessLink);

	res = encoder->encoder->InitializeExt(&paramExt);
	if(res != 0) {
		ERR_PRINT("failed to initialize encoder params.");
		WelsDestroySVCEncoder(encoder->encoder);
		ttLibC_free(encoder);
		return NULL;
	}
	// append additional data.
	int videoFormat = videoFormatI420;
	encoder->encoder->SetOption(ENCODER_OPTION_DATAFORMAT,   &videoFormat);
//	iTraceLevel = WELS_LOG_QUIET;
//	encoder->encoder->SetOption(ENCODER_OPTION_TRACE_LEVEL,  &iTraceLevel);
	ttLibC_Openh264Encoder_setIDRInterval((ttLibC_Openh264Encoder *)encoder, paramExt.uiIntraPeriod);

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
	encoder->is_reduce_mode = false;
	return (ttLibC_Openh264Encoder *)encoder;
}

/*
 * make h264 frame from analyzed buffer.
 * @param target_type
 * @param encoder
 * @param buffer
 * @param callback
 * @param ptr
 * @return true:success false:some error.(close buffer inside this function for error.)
 */
static bool Openh264Encoder_makeH264Frame(
		ttLibC_H264_Type target_type,
		ttLibC_Openh264Encoder_ *encoder,
		ttLibC_DynamicBuffer *buffer,
		ttLibC_Openh264EncodeFunc callback,
		void *ptr) {
	ttLibC_H264 *h264 = NULL;
	if(target_type == H264Type_configData) {
		h264 = ttLibC_H264_make(
				encoder->configData,
				target_type,
				encoder->inherit_super.width,
				encoder->inherit_super.height,
				ttLibC_DynamicBuffer_refData(buffer),
				ttLibC_DynamicBuffer_refSize(buffer),
				false,
				encoder->info.uiTimeStamp,
				1000);
		if(h264 == NULL) {
			ERR_PRINT("failed to make configData fro h264Frame");
			ttLibC_DynamicBuffer_close(&buffer);
			return false;
		}
		encoder->configData = h264;
	}
	else {
		h264 = ttLibC_H264_make(
				encoder->h264,
				target_type,
				encoder->inherit_super.width,
				encoder->inherit_super.height,
				ttLibC_DynamicBuffer_refData(buffer),
				ttLibC_DynamicBuffer_refSize(buffer),
				true,
				encoder->info.uiTimeStamp,
				1000);
		if(h264 == NULL) {
			ERR_PRINT("failed to make h264 frame.");
			ttLibC_DynamicBuffer_close(&buffer);
			return false;
		}
		encoder->h264 = h264;
	}
	if(!callback(ptr, h264)) {
		ttLibC_DynamicBuffer_close(&buffer);
		return false;
	}
	return true;
}

/*
 * handle encoded data after encode.
 * @param encoder  encoder object
 * @param callback callback func
 * @param ptr      user def data pointer
 */
static bool Openh264Encoder_checkEncodedData(
		ttLibC_Openh264Encoder_ *encoder,
		ttLibC_Openh264EncodeFunc callback,
		void *ptr) {
	for(int i = 0;i < encoder->info.iLayerNum;++ i) {
		const SLayerBSInfo& layerInfo = encoder->info.sLayerInfo[i];
		// check nal and decide the size of data.
		// for slice(with first mb in slice & 0x10 != 0x00 tells new frame.)
		uint8_t *buf = layerInfo.pBsBuf;
		ttLibC_DynamicBuffer *target_buffer = ttLibC_DynamicBuffer_make();
		ttLibC_H264_NalInfo nalInfo;

		ttLibC_H264_Type target_type = H264Type_unknown;
		for(int j = 0;j < layerInfo.iNalCount;++ j) {
			if(!ttLibC_H264_getNalInfo(&nalInfo, buf, (size_t)layerInfo.pNalLengthInByte[j])) {
				// failed to get data.
				ERR_PRINT("h264 data is corrupted.");
				return false;
			}
			switch(nalInfo.nal_unit_type) {
			case H264NalType_error:
				ERR_PRINT("unknown nal type is found.");
				ttLibC_DynamicBuffer_close(&target_buffer);
				return false;
			default:
//				LOG_PRINT("nal type is not implemented now.");
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
					ttLibC_DynamicBuffer_close(&target_buffer);
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
					ttLibC_DynamicBuffer_close(&target_buffer);
					return false;
				}
				break;
			case H264NalType_sequenceParameterSet:
			case H264NalType_pictureParameterSet:
				if(target_type != H264Type_configData) {
					if(target_type != H264Type_unknown) {
						// not tested yet.(no way to here with openh264?)
						ERR_PRINT("need to make h264 object.");
						ttLibC_DynamicBuffer_close(&target_buffer);
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
					ttLibC_DynamicBuffer_close(&target_buffer);
					return false;
				}
				break;
			}
			if(encoder->is_reduce_mode) {
				if(nalInfo.data_pos == 4) {
					// copy_data on buffer.
					ttLibC_DynamicBuffer_append(target_buffer, buf + 1, layerInfo.pNalLengthInByte[j] - 1);
				}
				else {
					// copy_data on buffer.
					ttLibC_DynamicBuffer_append(target_buffer, buf, layerInfo.pNalLengthInByte[j]);
				}
			}
			else {
				ttLibC_DynamicBuffer_append(target_buffer, buf, layerInfo.pNalLengthInByte[j]);
			}
			// go next position.
			buf += layerInfo.pNalLengthInByte[j];
		}
		if(!Openh264Encoder_makeH264Frame(
				target_type,
				encoder,
				target_buffer,
				callback,
				ptr)) {
			return false;
		}
		ttLibC_DynamicBuffer_close(&target_buffer);
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
	case Yvu420Type_planar:
		break;
	case Yuv420Type_semiPlanar:
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
	encoder_->picture.iStride[2]   = yuv->v_stride;
	encoder_->picture.pData[0]     = yuv->y_data;
	encoder_->picture.pData[1]     = yuv->u_data;
	encoder_->picture.pData[2]     = yuv->v_data;
	encoder_->picture.uiTimeStamp  = yuv->inherit_super.inherit_super.pts * 1000 / yuv->inherit_super.inherit_super.timebase;
	uint32_t res = encoder_->encoder->EncodeFrame(&encoder_->picture, &encoder_->info);
	if(res != cmResultSuccess) {
		ERR_PRINT("failed to encode picture.");
		return false;
	}
	if(encoder_->inherit_super.idr_interval <= 0) {
		++ encoder_->idr_interval_count;
		int iIDRPeriod = encoder_->idr_interval_count;
		encoder_->encoder->SetOption(ENCODER_OPTION_IDR_INTERVAL, &iIDRPeriod);
	}
	else {
		if(encoder_->inherit_super.idr_interval != encoder_->idr_interval_count) {
			int iIDRPeriod = encoder_->inherit_super.idr_interval;
			encoder_->encoder->SetOption(ENCODER_OPTION_IDR_INTERVAL, &iIDRPeriod);
		}
	}
	return Openh264Encoder_checkEncodedData(encoder_, callback, ptr);
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
ttLibC_Openh264Encoder TT_ATTRIBUTE_API *ttLibC_Openh264Encoder_make(
		uint32_t width,
		uint32_t height) {
	SEncParamExt paramExt;
	ttLibC_Openh264Encoder_getDefaultSEncParamExt(&paramExt, width, height);
	return Openh264Encoder_make(&paramExt);
}

/*
 * call make for c code
 */
ttLibC_Openh264Encoder TT_ATTRIBUTE_API *ttLibC_Openh264Encoder_make_ex(
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

void TT_ATTRIBUTE_API ttLibC_Openh264Encoder_getDefaultSEncParamExt(
		void *paramExt,
		uint32_t width,
		uint32_t height) {
	SEncParamExt *pParamExt = (SEncParamExt *)paramExt;
	memset(paramExt, 0, sizeof(SEncParamExt));
	pParamExt->iUsageType     = CAMERA_VIDEO_REAL_TIME;
	pParamExt->iPicWidth      = width;
	pParamExt->iPicHeight     = height;
	pParamExt->iTargetBitrate = 650000;
	pParamExt->iRCMode        = RC_BITRATE_MODE;
	pParamExt->fMaxFrameRate  = 15;

	pParamExt->iTemporalLayerNum = 1;
	pParamExt->iSpatialLayerNum  = 1;
	for(int i = 0;i < MAX_SPATIAL_LAYER_NUM;++ i) {
		pParamExt->sSpatialLayers[i].iVideoWidth        = pParamExt->iPicWidth;
		pParamExt->sSpatialLayers[i].iVideoHeight       = pParamExt->iPicHeight;
		pParamExt->sSpatialLayers[i].fFrameRate         = pParamExt->fMaxFrameRate;
		pParamExt->sSpatialLayers[i].iSpatialBitrate    = pParamExt->iTargetBitrate;
		pParamExt->sSpatialLayers[i].iMaxSpatialBitrate = pParamExt->iMaxBitrate;
		pParamExt->sSpatialLayers[i].uiProfileIdc       = PRO_BASELINE;
		pParamExt->sSpatialLayers[i].uiLevelIdc         = LEVEL_UNKNOWN;
		pParamExt->sSpatialLayers[i].iDLayerQp          = 0;
#if (OPENH264_MAJOR >= 1) && (OPENH264_MINOR >= 6)
		pParamExt->sSpatialLayers[i].bVideoSignalTypePresent   = false;
		pParamExt->sSpatialLayers[i].uiVideoFormat             = VF_COMPONENT;
		pParamExt->sSpatialLayers[i].bFullRange                = false;
		pParamExt->sSpatialLayers[i].bColorDescriptionPresent  = false;
		pParamExt->sSpatialLayers[i].uiColorPrimaries          = CP_UNDEF;
		pParamExt->sSpatialLayers[i].uiTransferCharacteristics = TRC_UNDEF;
		pParamExt->sSpatialLayers[i].uiColorMatrix             = CM_UNDEF;
		pParamExt->sSpatialLayers[i].sSliceArgument.uiSliceMode           = SM_SINGLE_SLICE;
		pParamExt->sSpatialLayers[i].sSliceArgument.uiSliceNum            = 1;
		pParamExt->sSpatialLayers[i].sSliceArgument.uiSliceSizeConstraint = 1500;
		for(int j = 0, max = MAX_SLICES_NUM_TMP;j < max;++ j) {
			pParamExt->sSpatialLayers[i].sSliceArgument.uiSliceMbNum[j]   = 0;
		}
#else
		pParamExt->sSpatialLayers[i].sSliceCfg.uiSliceMode                          = SM_SINGLE_SLICE;
		pParamExt->sSpatialLayers[i].sSliceCfg.sSliceArgument.uiSliceNum            = 1;
		pParamExt->sSpatialLayers[i].sSliceCfg.sSliceArgument.uiSliceSizeConstraint = 1500;
		for(int j = 0, max = MAX_SLICES_NUM_TMP;j < max;++ j) {
			pParamExt->sSpatialLayers[i].sSliceCfg.sSliceArgument.uiSliceMbNum[j]   = 0;
		}
#endif
	}

	pParamExt->iComplexityMode = MEDIUM_COMPLEXITY;
	pParamExt->uiIntraPeriod   = 0;
	pParamExt->iNumRefFrame    = 1;

	pParamExt->eSpsPpsIdStrategy    = CONSTANT_ID;
	pParamExt->bPrefixNalAddingCtrl = false;

	pParamExt->bEnableSSEI            = false;
	pParamExt->bSimulcastAVC          = false;
	pParamExt->iPaddingFlag           = 0;
	pParamExt->iEntropyCodingModeFlag = 0;

	pParamExt->bEnableFrameSkip = true;
	pParamExt->iMaxBitrate      = 0;
	pParamExt->iMinQp           = 4;
	pParamExt->iMaxQp           = 20;
	pParamExt->uiMaxNalSize     = 0;

	pParamExt->bEnableLongTermReference = false;
	pParamExt->iLTRRefNum               = 0;
	pParamExt->iLtrMarkPeriod           = 30;
	pParamExt->iMultipleThreadIdc       = 1;
#if (OPENH264_MAJOR >= 1) && (OPENH264_MINOR >= 6)
	pParamExt->bUseLoadBalancing        = 0;
#endif

	pParamExt->iLoopFilterDisableIdc      = 0;
	pParamExt->iLoopFilterAlphaC0Offset   = 0;
	pParamExt->iLoopFilterBetaOffset      = 0;
	pParamExt->bEnableDenoise             = false;
	pParamExt->bEnableAdaptiveQuant       = true;
	pParamExt->bEnableBackgroundDetection = true;
	pParamExt->bEnableFrameCroppingFlag   = true;
	pParamExt->bEnableSceneChangeDetect   = false;
	pParamExt->bIsLosslessLink            = false;
}

ttLibC_Openh264Encoder TT_ATTRIBUTE_API *ttLibC_Openh264Encoder_makeWithSEncParamExt(void *paramExt) {
	return Openh264Encoder_make((SEncParamExt *)paramExt);
}

/*
 * call encode for c code
 */
bool TT_ATTRIBUTE_API ttLibC_Openh264Encoder_encode(
		ttLibC_Openh264Encoder* encoder,
		ttLibC_Yuv420 *yuv420,
		ttLibC_Openh264EncodeFunc callback,
		void *ptr) {
	return Openh264Encoder_encode(
			encoder, yuv420, callback, ptr);
}

/*
 * ref liopenh264 native encoder object.
 * @param encoder openh264 encoder object.
 * @return ISVCEncoder pointer.
 */
void TT_ATTRIBUTE_API *ttLibC_Openh264Encoder_refNativeEncoder(ttLibC_Openh264Encoder *encoder) {
	ttLibC_Openh264Encoder_ *encoder_ = (ttLibC_Openh264Encoder_ *)encoder;
	return encoder_->encoder;
}

/*
 * set idr interval
 * @param encoder
 * @param interval
 * @return true / false
 */
bool TT_ATTRIBUTE_API ttLibC_Openh264Encoder_setIDRInterval(
		ttLibC_Openh264Encoder *encoder,
		int32_t interval) {
	ttLibC_Openh264Encoder_ *encoder_ = (ttLibC_Openh264Encoder_ *)encoder;
	int iIDRPeriod = interval;
	if(interval == -1) {
		iIDRPeriod = 1;
		encoder_->idr_interval_count = 1;
	}
	else if(interval == 0 || interval < 1){
		return false;
	}
	else {
		iIDRPeriod = interval;
		encoder_->idr_interval_count = interval;
	}
	encoder_->inherit_super.idr_interval = interval;
	encoder_->encoder->SetOption(ENCODER_OPTION_IDR_INTERVAL, &iIDRPeriod);
	return true;
}

/*
 * force next encode picture will be key frame(sliceIDR).
 * @param
 */
bool TT_ATTRIBUTE_API ttLibC_Openh264Encoder_forceNextKeyFrame(ttLibC_Openh264Encoder *encoder) {
	ttLibC_Openh264Encoder_ *encoder_ = (ttLibC_Openh264Encoder_ *)encoder;
	int iIDRPeriod  = 1;
	encoder_->idr_interval_count = 1;
	encoder_->encoder->SetOption(ENCODER_OPTION_IDR_INTERVAL, &iIDRPeriod);
	return true;
}

bool TT_ATTRIBUTE_API ttLibC_Openh264Encoder_setRCMode(
		ttLibC_Openh264Encoder *encoder,
		ttLibC_Openh264Encoder_RCType rcType) {
	ttLibC_Openh264Encoder_ *encoder_ = (ttLibC_Openh264Encoder_ *)encoder;
	RC_MODES rc_mode = RC_OFF_MODE;
	switch(rcType) {
	case Openh264EncoderRCType_BitrateMode:
		rc_mode = RC_BITRATE_MODE;
		break;
#if (OPENH264_MAJOR >= 1) && (OPENH264_MINOR >= 5)
	case Openh264EncoderRCType_BitrateModePostSkip:
		rc_mode = RC_BITRATE_MODE_POST_SKIP;
		break;
#endif
	case Openh264EncoderRCType_BufferbasedMode:
		rc_mode = RC_BUFFERBASED_MODE;
		break;
	default:
	case Openh264EncoderRCType_OffMode:
		break;
	case Openh264EncoderRCType_QualityMode:
		rc_mode = RC_QUALITY_MODE;
		break;
	case Openh264EncoderRCType_TimestampMode:
		rc_mode = RC_TIMESTAMP_MODE;
		break;
	}
	encoder_->encoder->SetOption(ENCODER_OPTION_RC_MODE, &rc_mode);
	return true;
}

bool TT_ATTRIBUTE_API ttLibC_Openh264Encoder_setReduceMode(
		ttLibC_Openh264Encoder *encoder,
		bool reduce_mode_flag) {
	ttLibC_Openh264Encoder_ *encoder_ = (ttLibC_Openh264Encoder_ *)encoder;
	if(encoder_ == NULL) {
		return false;
	}
	encoder_->is_reduce_mode = reduce_mode_flag;
	return true;
}


/*
 * call close for c code
 */
void TT_ATTRIBUTE_API ttLibC_Openh264Encoder_close(ttLibC_Openh264Encoder **encoder) {
	Openh264Encoder_close(encoder);
}

/**
 * parse param with c string.
 */
bool TT_ATTRIBUTE_API ttLibC_Openh264Encoder_paramParse(void *paramExt, const char *name, const char *value) {
	char *end = NULL;
	SEncParamExt *pExt = (SEncParamExt *)paramExt;
#define OPT(str)	else if(!strcmp(name, str))
#define ENUM(str)	else if(!strcmp(value, str))
#define SetI(target)	do {int val = strtol(value, &end, 0); \
							if(end == value || *end != '\0') return false;\
							pExt->target = val;\
						} while(0);
#define SetD(target)	do {double val = strtod(value, &end); \
							if(end == value || *end != '\0') return false;\
							pExt->target = val;\
						} while(0);
#define SetB(target)	do {bool val; \
							if(!strcmp(value, "true")) val = true; \
							else if(!strcmp(value, "false")) val = false; \
							else { \
								int v = strtol(value, &end, 0); \
								if(end == value || *end != '\0') return false;\
								val = (v != 0); \
							} \
							pExt->target = val;\
						} while(0);
	if(0) {}
	OPT("iUsageType")
	{
		EUsageType type;
		if(0) {}
		ENUM("CAMERA_VIDEO_REAL_TIME")
			type = CAMERA_VIDEO_REAL_TIME;
		ENUM("SCREEN_CONTENT_REAL_TIME")
			type = SCREEN_CONTENT_REAL_TIME;
		ENUM("CAMERA_VIDEO_NON_REAL_TIME")
			type = CAMERA_VIDEO_NON_REAL_TIME;
		else return false;
		pExt->iUsageType = type;
	}
	OPT("iTargetBitrate")
	{
		SetI(iTargetBitrate);
	}
	OPT("iRCMode")
	{
		RC_MODES mode;
		if(0) {}
		ENUM("RC_QUALITY_MODE")
			mode = RC_QUALITY_MODE;
		ENUM("RC_BITRATE_MODE")
			mode = RC_BITRATE_MODE;
		ENUM("RC_BUFFERBASED_MODE")
			mode = RC_BUFFERBASED_MODE;
		ENUM("RC_TIMESTAMP_MODE")
			mode = RC_TIMESTAMP_MODE;
#if OPENH264_MAJOR >= 1 && OPENH264_MINOR >= 5
		ENUM("RC_BITRATE_MODE_POST_SKIP")
			mode = RC_BITRATE_MODE_POST_SKIP;
#endif
		ENUM("RC_OFF_MODE")
			mode = RC_OFF_MODE;
		else return false;
		pExt->iRCMode = mode;
	}
	OPT("fMaxFrameRate")
	{
		SetD(fMaxFrameRate);
	}
	OPT("iTemporalLayerNum")
	{
		SetI(iTemporalLayerNum);
	}
	OPT("iSpatialLayerNum")
	{
		SetI(iSpatialLayerNum);
	}
	OPT("iComplexityMode")
	{
		ECOMPLEXITY_MODE mode;
		if(0) {}
		ENUM("LOW_COMPLEXITY")
			mode = LOW_COMPLEXITY;
		ENUM("MEDIUM_COMPLEXITY")
			mode = MEDIUM_COMPLEXITY;
		ENUM("HIGH_COMPLEXITY")
			mode = HIGH_COMPLEXITY;
		else return false;
		pExt->iComplexityMode = mode;
	}
	OPT("uiIntraPeriod")
	{
		SetI(uiIntraPeriod);
	}
	OPT("iNumRefFrame")
	{
		SetI(iNumRefFrame);
	}
	OPT("eSpsPpsIdStrategy")
	{
		EParameterSetStrategy strategy;
		if(0) {}
		ENUM("CONSTANT_ID")
			strategy = CONSTANT_ID;
		ENUM("INCREASING_ID")
			strategy = INCREASING_ID;
		ENUM("SPS_LISTING")
			strategy = SPS_LISTING;
		ENUM("SPS_LISTING_AND_PPS_INCREASING")
			strategy = SPS_LISTING_AND_PPS_INCREASING;
		ENUM("SPS_PPS_LISTING")
			strategy = SPS_PPS_LISTING;
		else return false;
		pExt->eSpsPpsIdStrategy = strategy;
	}
	OPT("bPrefixNalAddingCtrl")
	{
		SetB(bPrefixNalAddingCtrl);
	}
	OPT("bEnableSSEI")
	{
		SetB(bEnableSSEI);
	}
	OPT("bSimulcastAVC")
	{
		SetB(bSimulcastAVC);
	}
	OPT("iPaddingFlag")
	{
		SetI(iPaddingFlag);
	}
	OPT("iEntropyCodingModeFlag")
	{
		SetI(iEntropyCodingModeFlag);
	}
	OPT("bEnableFrameSkip")
	{
		SetB(bEnableFrameSkip);
	}
	OPT("iMaxBitrate")
	{
		SetI(iMaxBitrate);
	}
	OPT("iMaxQp")
	{
		SetI(iMaxQp);
	}
	OPT("iMinQp")
	{
		SetI(iMinQp);
	}
	OPT("uiMaxNalSize")
	{
		SetI(uiMaxNalSize);
	}
	OPT("bEnableLongTermReference")
	{
		SetB(bEnableLongTermReference);
	}
	OPT("iLTRRefNum")
	{
		SetI(iLTRRefNum);
	}
	OPT("iLtrMarkPeriod")
	{
		SetI(iLtrMarkPeriod);
	}
	OPT("iMultipleThreadIdc")
	{
		SetI(iMultipleThreadIdc);
	}
#if OPENH264_MAJOR >= 1 && OPENH264_MINOR >= 6
	OPT("bUseLoadBalancing")
	{
		SetB(bUseLoadBalancing);
	}
#endif
	OPT("iLoopFilterDisableIdc")
	{
		SetI(iLoopFilterDisableIdc);
	}
	OPT("iLoopFilterAlphaC0Offset")
	{
		SetI(iLoopFilterAlphaC0Offset);
	}
	OPT("iLoopFilterBetaOffset")
	{
		SetI(iLoopFilterBetaOffset);
	}
	OPT("bEnableDenoise")
	{
		SetB(bEnableDenoise);
	}
	OPT("bEnableBackgroundDetection")
	{
		SetB(bEnableBackgroundDetection);
	}
	OPT("bEnableAdaptiveQuant")
	{
		SetB(bEnableAdaptiveQuant);
	}
	OPT("bEnableFrameCroppingFlag")
	{
		SetB(bEnableFrameCroppingFlag);
	}
	OPT("bEnableSceneChangeDetect")
	{
		SetB(bEnableSceneChangeDetect);
	}
	OPT("bIsLosslessLink")
	{
		SetB(bIsLosslessLink);
	}
	else {
		return false;
	}
#undef OPT
#undef ENUM
#undef SetI
#undef SetD
#undef SetB
	return true;
}
bool TT_ATTRIBUTE_API ttLibC_Openh264Encoder_spatialParamParse(void *paramExt, uint32_t id, const char *name, const char *value) {
	if(id >= MAX_SPATIAL_LAYER_NUM) {
		return false;
	}
	char *end = NULL;
	SEncParamExt *pExt = (SEncParamExt *)paramExt;
#define OPT(str)	else if(!strcmp(name, str))
#define ENUM(str)	else if(!strcmp(value, str))
#define SetI(target)	do {int val = strtol(value, &end, 0); \
							if(end == value || *end != '\0') return false;\
							pExt->sSpatialLayers[id].target = val;\
						} while(0);
#define SetD(target)	do {double val = strtod(value, &end); \
							if(end == value || *end != '\0') return false;\
							pExt->sSpatialLayers[id].target = val;\
						} while(0);
#define SetB(target)	do {bool val; \
							if(!strcmp(value, "true")) val = true; \
							else if(!strcmp(value, "false")) val = false; \
							else { \
								int v = strtol(value, &end, 0); \
								if(end == value || *end != '\0') return false;\
								val = (v != 0); \
							} \
							pExt->sSpatialLayers[id].target = val;\
						} while(0);
	if(0) {}
	OPT("iVideoWidth")
	{
		SetI(iVideoWidth);
	}
	OPT("iVideoHeight")
	{
		SetI(iVideoHeight);
	}
	OPT("fFrameRate")
	{
		SetD(fFrameRate);
	}
	OPT("iSpatialBitrate")
	{
		SetI(iSpatialBitrate);
	}
	OPT("iMaxSpatialBitrate")
	{
		SetI(iMaxSpatialBitrate);
	}
	OPT("uiProfileIdc")
	{
		EProfileIdc idc;
		if(0) {}
		ENUM("PRO_UNKNOWN")
			idc = PRO_UNKNOWN;
		ENUM("PRO_BASELINE")
			idc = PRO_BASELINE;
		ENUM("PRO_MAIN")
			idc = PRO_MAIN;
		ENUM("PRO_EXTENDED")
			idc = PRO_EXTENDED;
		ENUM("PRO_HIGH")
			idc = PRO_HIGH;
		ENUM("PRO_HIGH10")
			idc = PRO_HIGH10;
		ENUM("PRO_HIGH422")
			idc = PRO_HIGH422;
		ENUM("PRO_HIGH444")
			idc = PRO_HIGH444;
		ENUM("PRO_CAVLC444")
			idc = PRO_CAVLC444;
		ENUM("PRO_SCALABLE_BASELINE")
			idc = PRO_SCALABLE_BASELINE;
		ENUM("PRO_SCALABLE_HIGH")
			idc = PRO_SCALABLE_HIGH;
		else return false;
		pExt->sSpatialLayers[id].uiProfileIdc = idc;
	}
	OPT("uiLevelIdc")
	{
		ELevelIdc idc;
		if(0) {}
		ENUM("LEVEL_UNKNOWN")
			idc = LEVEL_UNKNOWN;
		ENUM("LEVEL_1_0")
			idc = LEVEL_1_0;
		ENUM("LEVEL_1_B")
			idc = LEVEL_1_B;
		ENUM("LEVEL_1_1")
			idc = LEVEL_1_1;
		ENUM("LEVEL_1_2")
			idc = LEVEL_1_2;
		ENUM("LEVEL_1_3")
			idc = LEVEL_1_3;
		ENUM("LEVEL_2_0")
			idc = LEVEL_2_0;
		ENUM("LEVEL_2_1")
			idc = LEVEL_2_1;
		ENUM("LEVEL_2_2")
			idc = LEVEL_2_2;
		ENUM("LEVEL_3_0")
			idc = LEVEL_3_0;
		ENUM("LEVEL_3_1")
			idc = LEVEL_3_1;
		ENUM("LEVEL_3_2")
			idc = LEVEL_3_2;
		ENUM("LEVEL_4_0")
			idc = LEVEL_4_0;
		ENUM("LEVEL_4_1")
			idc = LEVEL_4_1;
		ENUM("LEVEL_4_2")
			idc = LEVEL_4_2;
		ENUM("LEVEL_5_0")
			idc = LEVEL_5_0;
		ENUM("LEVEL_5_1")
			idc = LEVEL_5_1;
		ENUM("LEVEL_5_2")
			idc = LEVEL_5_2;
		else return false;
		pExt->sSpatialLayers[id].uiLevelIdc = idc;
	}
	OPT("iDLayerQp")
	{
		SetI(iDLayerQp);
	}
#if OPENH264_MAJOR <= 1 && OPENH264_MINOR <= 5
	OPT("sSliceCfg.uiSliceMode")
	{
		SliceModeEnum mode;
		if(0) {}
		ENUM("SM_SINGLE_SLICE")
			mode = SM_SINGLE_SLICE;
		ENUM("SM_FIXEDSLCNUM_SLICE")
			mode = SM_FIXEDSLCNUM_SLICE;
		ENUM("SM_RASTER_SLICE")
			mode = SM_RASTER_SLICE;
		ENUM("SM_ROWMB_SLICE")
			mode = SM_ROWMB_SLICE;
		ENUM("SM_DYN_SLICE")
			mode = SM_DYN_SLICE;
		ENUM("SM_AUTO_SLICE")
			mode = SM_AUTO_SLICE;
		ENUM("SM_RESERVED")
			mode = SM_RESERVED;
		else return false;
		layer.sSliceCfg.uiSliceMode = mode;
	}
/*	OPT("sSliceCfg.sSliceArgument.uiSliceMbNum")
	{
		// no idea to make...
	}*/
	OPT("sSliceCfg.sSliceArgument.uiSliceNum")
	{
		SetI(sSliceCfg.sSliceArgument.uiSliceNum);
	}
	OPT("sSliceCfg.sSliceArgument.uiSliceSizeConstraint")
	{
		SetI(sSliceCfg.sSliceArgument.uiSliceSizeConstraint);
	}
#elif OPENH264_MAJOR >= 1 && OPENH264_MINOR >= 6
	OPT("sSliceArgument.uiSliceMode")
	{
		SliceModeEnum mode;
		if(0) {}
		ENUM("SM_SINGLE_SLICE")
			mode = SM_SINGLE_SLICE;
		ENUM("SM_FIXEDSLCNUM_SLICE")
			mode = SM_FIXEDSLCNUM_SLICE;
		ENUM("SM_RASTER_SLICE")
			mode = SM_RASTER_SLICE;
		ENUM("SM_SIZELIMITED_SLICE")
			mode = SM_SIZELIMITED_SLICE;
		ENUM("SM_RESERVED")
			mode = SM_RESERVED;
		else return false;
		pExt->sSpatialLayers[id].sSliceArgument.uiSliceMode = mode;
	}
	OPT("sSliceArgument.uiSliceNum")
	{
		SetI(sSliceArgument.uiSliceNum);
	}
	OPT("sSliceArgument.uiSliceSizeConstraint")
	{
		SetI(sSliceArgument.uiSliceSizeConstraint);
	}
/*	OPT("sSliceArgument.uiSliceMbNum")
	{
		// no idea to make...
	}*/
	OPT("bVideoSignalTypePresent")
	{
		SetB(bVideoSignalTypePresent);
	}
	OPT("uiVideoFormat")
	{
		EVideoFormatSPS format;
		if(0) {}
		ENUM("VF_COMPONENT")
			format = VF_COMPONENT;
		ENUM("VF_PAL")
			format = VF_PAL;
		ENUM("VF_NTSC")
			format = VF_NTSC;
		ENUM("VF_SECAM")
			format = VF_SECAM;
		ENUM("VF_MAC")
			format = VF_MAC;
		ENUM("VF_UNDEF")
			format = VF_UNDEF;
		ENUM("VF_NUM_ENUM")
			format = VF_NUM_ENUM;
		else return false;
		pExt->sSpatialLayers[id].uiVideoFormat = format;
	}
	OPT("bFullRange")
	{
		SetB(bFullRange);
	}
	OPT("bColorDescriptionPresent")
	{
		SetB(bColorDescriptionPresent);
	}
	OPT("uiColorPrimaries")
	{
		EColorPrimaries primaries;
		if(0) {}
		ENUM("CP_RESERVED0")
			primaries = CP_RESERVED0;
		ENUM("CP_BT709")
			primaries = CP_BT709;
		ENUM("CP_UNDEF")
			primaries = CP_UNDEF;
		ENUM("CP_RESERVED3")
			primaries = CP_RESERVED3;
		ENUM("CP_BT470M")
			primaries = CP_BT470M;
		ENUM("CP_BT470BG")
			primaries = CP_BT470BG;
		ENUM("CP_SMPTE170M")
			primaries = CP_SMPTE170M;
		ENUM("CP_SMPTE240M")
			primaries = CP_SMPTE240M;
		ENUM("CP_FILM")
			primaries = CP_FILM;
		ENUM("CP_BT2020")
			primaries = CP_BT2020;
		ENUM("CP_NUM_ENUM")
			primaries = CP_NUM_ENUM;
		else return false;
		pExt->sSpatialLayers[id].uiColorPrimaries = primaries;
	}
	OPT("uiTransferCharacteristics")
	{
		ETransferCharacteristics characteristics;
		if(0) {}
		ENUM("TRC_RESERVED0")
			characteristics = TRC_RESERVED0;
		ENUM("TRC_BT709")
			characteristics = TRC_BT709;
		ENUM("TRC_UNDEF")
			characteristics = TRC_UNDEF;
		ENUM("TRC_RESERVED3")
			characteristics = TRC_RESERVED3;
		ENUM("TRC_BT470M")
			characteristics = TRC_BT470M;
		ENUM("TRC_BT470BG")
			characteristics = TRC_BT470BG;
		ENUM("TRC_SMPTE170M")
			characteristics = TRC_SMPTE170M;
		ENUM("TRC_SMPTE240M")
			characteristics = TRC_SMPTE240M;
		ENUM("TRC_LINEAR")
			characteristics = TRC_LINEAR;
		ENUM("TRC_LOG100")
			characteristics = TRC_LOG100;
		ENUM("TRC_LOG316")
			characteristics = TRC_LOG316;
		ENUM("TRC_IEC61966_2_4")
			characteristics = TRC_IEC61966_2_4;
		ENUM("TRC_BT1361E")
			characteristics = TRC_BT1361E;
		ENUM("TRC_IEC61966_2_1")
			characteristics = TRC_IEC61966_2_1;
		ENUM("TRC_BT2020_10")
			characteristics = TRC_BT2020_10;
		ENUM("TRC_BT2020_12")
			characteristics = TRC_BT2020_12;
		ENUM("TRC_NUM_ENUM")
			characteristics = TRC_NUM_ENUM;
		else return false;
		pExt->sSpatialLayers[id].uiTransferCharacteristics = characteristics;
	}
	OPT("uiColorMatrix")
	{
		EColorMatrix matrix;
		if(0) {}
		ENUM("CM_GBR")
			matrix = CM_GBR;
		ENUM("CM_BT709")
			matrix = CM_BT709;
		ENUM("CM_UNDEF")
			matrix = CM_UNDEF;
		ENUM("CM_RESERVED3")
			matrix = CM_RESERVED3;
		ENUM("CM_FCC")
			matrix = CM_FCC;
		ENUM("CM_BT470BG")
			matrix = CM_BT470BG;
		ENUM("CM_SMPTE170M")
			matrix = CM_SMPTE170M;
		ENUM("CM_SMPTE240M")
			matrix = CM_SMPTE240M;
		ENUM("CM_YCGCO")
			matrix = CM_YCGCO;
		ENUM("CM_BT2020NC")
			matrix = CM_BT2020NC;
		ENUM("CM_BT2020C")
			matrix = CM_BT2020C;
		ENUM("CM_NUM_ENUM")
			matrix = CM_NUM_ENUM;
		else return false;
		pExt->sSpatialLayers[id].uiColorMatrix = matrix;
	}
#endif
	else {
		return false;
	}
#undef OPT
#undef ENUM
#undef SetI
#undef SetD
#undef SetB
	return true;
}


} /* extern "C" */

#endif
