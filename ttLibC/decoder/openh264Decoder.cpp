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
#include "../log.h"
#include "../allocator.h"

#include <wels/codec_api.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

/*
 * make openh264 decoder (maybe add more params later.)
 */
static ttLibC_Openh264Decoder *Openh264Decoder_make() {
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
	decParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_SVC;
	decParam.bParseOnly = false;
	decParam.eOutputColorFormat = videoFormatI420;
	decParam.uiTargetDqLayer = UCHAR_MAX;
	decParam.eEcActiveIdc = ERROR_CON_SLICE_COPY;
	res = decoder->decoder->Initialize(&decParam);
	if(res != 0) {
		ERR_PRINT("failed to initialize decoder.");
		WelsDestroyDecoder(decoder->decoder);
		ttLibC_free(decoder);
		return NULL;
	}
	decoder->yuv420 = NULL;
	return (ttLibC_Openh264Decoder *)decoder;
}

/*
 * decode frame.
 * @param decoder  openh264 decoder object.
 * @param h264     source h264 data.
 * @param callback callback func for h264 decode.
 * @param ptr      pointer for user def value, which will call in callback.
 */
static void Openh264Decoder_decode(
		ttLibC_Openh264Decoder *decoder,
		ttLibC_H264 *h264,
		ttLibC_Openh264DecodeFunc callback,
		void *ptr) {
	ttLibC_Openh264Decoder_ *decoder_ = (ttLibC_Openh264Decoder_ *)decoder;
	if(decoder == NULL) {
		return;
	}
	if(h264 == NULL) {
		return;
	}
	uint8_t *decodeBuf[3] = {0};
	uint32_t res = decoder_->decoder->DecodeFrame2((const unsigned char *)h264->inherit_super.inherit_super.data, h264->inherit_super.inherit_super.buffer_size, decodeBuf, &decoder_->bufInfo);
	if(res != 0) {
		ERR_PRINT("failed to decode data.:%x", res);
		return;
	}
	if(decoder_->bufInfo.UsrData.sSystemBuffer.iWidth == 0
	|| decoder_->bufInfo.UsrData.sSystemBuffer.iHeight == 0) {
		// dimention is null. happen on first buffer. or supply configData.
		return;
	}
	decoder_->inherit_super.width  = decoder_->bufInfo.UsrData.sSystemBuffer.iWidth;
	decoder_->inherit_super.height = decoder_->bufInfo.UsrData.sSystemBuffer.iHeight;
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
			h264->inherit_super.inherit_super.pts, h264->inherit_super.inherit_super.timebase);
	if(yuv == NULL) {
		ERR_PRINT("failed to make yuv420 frame.");
		return;
	}
	decoder_->yuv420 = yuv;
	callback(ptr, yuv);
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
ttLibC_Openh264Decoder *ttLibC_Openh264Decoder_make() {
	return Openh264Decoder_make();
}

/*
 * call decoder for c code.
 */
void ttLibC_Openh264Decoder_decode(
		ttLibC_Openh264Decoder *decoder,
		ttLibC_H264 *h264,
		ttLibC_Openh264DecodeFunc callback,
		void *ptr) {
	Openh264Decoder_decode(decoder, h264, callback, ptr);
}

/*
 * call close for c code
 */
void ttLibC_Openh264Decoder_close(ttLibC_Openh264Decoder **decoder) {
	Openh264Decoder_close(decoder);
}

} /* extern "C" */

#endif


