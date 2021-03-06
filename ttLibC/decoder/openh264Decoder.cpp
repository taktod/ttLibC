/*
 * @file   openh264Decoder.cpp
 * @brief  decode h264 with openh264.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/26
 */

#ifdef __ENABLE_OPENH264__

#include "openh264Decoder.h"
#include "../ttLibC_predef.h"
#include "../_log.h"
#include "../allocator.h"

#include <wels/codec_api.h>
#include <wels/codec_ver.h>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define Param_set(a, b) {if(a != b) {a = b;}}

/*
 * h264 decoder detail definition.
 */
typedef struct {
	/** inherit data from ttLibC_Openh264Decoder */
	ttLibC_Openh264Decoder inherit_super;
	/** svcdecoder object(openh264) */
	ISVCDecoder *decoder;
	/** buffer information(openh264) */
	SBufferInfo bufInfo;
	/** yuv420 frame. */
	ttLibC_Yuv420 *yuv420;
} ttLibC_Decoder_Openh264Decoder_;

typedef ttLibC_Decoder_Openh264Decoder_ ttLibC_Openh264Decoder_;

static ttLibC_Openh264Decoder *Openh264Decoder_make(SDecodingParam *param) {
	ttLibC_Openh264Decoder_ *decoder = (ttLibC_Openh264Decoder_ *)ttLibC_malloc(sizeof(ttLibC_Openh264Decoder_));
	if(decoder == NULL) {
		ERR_PRINT("failed to alloc decoder object.");
		return NULL;
	}
	uint32_t res = WelsCreateDecoder(&decoder->decoder);
	if(res != 0 || decoder == NULL) {
		ERR_PRINT("failed to make decoder.");
		ttLibC_free(decoder);
		return NULL;
	}
	SDecodingParam decParam;
	memset(&decParam, 0, sizeof(SDecodingParam));
	Param_set(decParam.pFileNameRestructed, param->pFileNameRestructed);
#if (OPENH264_MAJOR <= 1) && (OPENH264_MINOR <= 5)
	Param_set(decParam.eOutputColorFormat,  param->eOutputColorFormat);
#endif
	Param_set(decParam.uiCpuLoad,           param->uiCpuLoad);
	Param_set(decParam.uiTargetDqLayer,     param->uiTargetDqLayer);
	Param_set(decParam.eEcActiveIdc,        param->eEcActiveIdc);
	Param_set(decParam.bParseOnly,          param->bParseOnly);
	Param_set(decParam.sVideoProperty.size,         param->sVideoProperty.size);
	Param_set(decParam.sVideoProperty.eVideoBsType, param->sVideoProperty.eVideoBsType);
	res = decoder->decoder->Initialize(&decParam);
	if(res != 0) {
		ERR_PRINT("failed to initialize decoder.");
		WelsDestroyDecoder(decoder->decoder);
		ttLibC_free(decoder);
		return NULL;
	}
	decoder->yuv420 = NULL;
	decoder->inherit_super.width  = 0;
	decoder->inherit_super.height = 0;
	return (ttLibC_Openh264Decoder *)decoder;
}

/*
 * decode frame.
 * @param decoder  openh264 decoder object.
 * @param h264     source h264 data.
 * @param callback callback func for h264 decode.
 * @param ptr      pointer for user def value, which will call in callback.
 * @return true / false
 */
static bool Openh264Decoder_decode(
		ttLibC_Openh264Decoder *decoder,
		ttLibC_H264 *h264,
		ttLibC_Openh264DecodeFunc callback,
		void *ptr) {
	ttLibC_Openh264Decoder_ *decoder_ = (ttLibC_Openh264Decoder_ *)decoder;
	if(decoder == NULL) {
		return false;
	}
	if(h264 == NULL) {
		return true;
	}
	uint8_t *decodeBuf[3] = {0};
	decoder_->bufInfo.uiInBsTimeStamp = h264->inherit_super.inherit_super.pts;
	uint32_t res = decoder_->decoder->DecodeFrame2((const unsigned char *)h264->inherit_super.inherit_super.data, h264->inherit_super.inherit_super.buffer_size, decodeBuf, &decoder_->bufInfo);
	if(res != 0) {
		ERR_PRINT("failed to decode data.:%x", res);
		return false;
	}
	if(h264->inherit_super.width == 0
	|| h264->inherit_super.height == 0) {
		// dimention for h264 is null. happen on first buffer. or supply configData.
		return true;
	}
	if(decoder_->bufInfo.UsrData.sSystemBuffer.iWidth == 0
	|| decoder_->bufInfo.UsrData.sSystemBuffer.iHeight == 0) {
		// dimention is null. happen on first buffer. or supply configData.
		return true;
	}
//	decoder_->inherit_super.width  = decoder_->bufInfo.UsrData.sSystemBuffer.iWidth;
//	decoder_->inherit_super.height = decoder_->bufInfo.UsrData.sSystemBuffer.iHeight;
	decoder_->inherit_super.width  = h264->inherit_super.width;
	decoder_->inherit_super.height = h264->inherit_super.height;
	ttLibC_Yuv420 *yuv = ttLibC_Yuv420_make(
			decoder_->yuv420,
			Yuv420Type_planar,
			decoder_->inherit_super.width,
			decoder_->inherit_super.height,
			NULL, 0,
			decodeBuf[0], decoder_->bufInfo.UsrData.sSystemBuffer.iStride[0],
			decodeBuf[1], decoder_->bufInfo.UsrData.sSystemBuffer.iStride[1],
			decodeBuf[2], decoder_->bufInfo.UsrData.sSystemBuffer.iStride[1],
			true,
			decoder_->bufInfo.uiOutYuvTimeStamp,
			h264->inherit_super.inherit_super.timebase);
	if(yuv == NULL) {
		ERR_PRINT("failed to make yuv420 frame.");
		return false;
	}
	decoder_->yuv420 = yuv;
	decoder_->yuv420->inherit_super.inherit_super.id = h264->inherit_super.inherit_super.id;
	if(!callback(ptr, yuv)) {
		return false;
	}
	return true;
}

/*
 * close openh264 decoder
 * @param decoder
 */
static void Openh264Decoder_close(ttLibC_Openh264Decoder **decoder) {
	ttLibC_Openh264Decoder_ *target = (ttLibC_Openh264Decoder_ *)*decoder;
	if(target == NULL) {
		return;
	}
	if(target->decoder != NULL) {
		target->decoder->Uninitialize();
		WelsDestroyDecoder(target->decoder);
	}
	ttLibC_Yuv420_close(&target->yuv420);
	ttLibC_free(target);
	*decoder = NULL;
}

extern "C" {

/**
 * call make for c code.
 */
ttLibC_Openh264Decoder TT_VISIBILITY_DEFAULT *ttLibC_Openh264Decoder_make() {
	SDecodingParam pParam;
	ttLibC_Openh264Decoder_getDefaultSDecodingParam(&pParam);
	return Openh264Decoder_make(&pParam);
}

/*
 * setup SDecodingParam with ttLibC default.
 * @param param structore pointer for SDecodingParam on wels/codec_api.h
 */
void TT_VISIBILITY_DEFAULT ttLibC_Openh264Decoder_getDefaultSDecodingParam(void *param) {
	SDecodingParam *pParam = (SDecodingParam *)param;
	memset(pParam, 0, sizeof(SDecodingParam));
//	pParam->pFileNameRestructed;
#if (OPENH264_MAJOR <= 1) && (OPENH264_MINOR <= 5)
	pParam->eOutputColorFormat = videoFormatI420;
#endif
	pParam->uiCpuLoad       = 0;
	pParam->uiTargetDqLayer = UCHAR_MAX;
	pParam->eEcActiveIdc    = ERROR_CON_SLICE_COPY;
	pParam->bParseOnly      = false;
	pParam->sVideoProperty.size         = sizeof(SVideoProperty);
	pParam->sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_SVC;
}

/*
 * make openh264 decoder with SDecodingParam
 * @param param structore pointer for SDecodingParam on wels/codec_api.h
 * @return ttLibC_Openh264Decoder object.
 */
ttLibC_Openh264Decoder TT_VISIBILITY_DEFAULT *ttLibC_Openh264Decoder_makeWithSDecodingParam(void *param) {
	return Openh264Decoder_make((SDecodingParam *)param);
}

/*
 * call decoder for c code.
 */
bool TT_VISIBILITY_DEFAULT ttLibC_Openh264Decoder_decode(
		ttLibC_Openh264Decoder *decoder,
		ttLibC_H264 *h264,
		ttLibC_Openh264DecodeFunc callback,
		void *ptr) {
	return Openh264Decoder_decode(decoder, h264, callback, ptr);
}

/*
 * call close for c code
 */
void TT_VISIBILITY_DEFAULT ttLibC_Openh264Decoder_close(ttLibC_Openh264Decoder **decoder) {
	Openh264Decoder_close(decoder);
}

} /* extern "C" */

#endif


