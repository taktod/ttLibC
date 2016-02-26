/**
 * @file   x265Encoder.c
 * @brief  encode h265 with x265
 *
 * this code is under GPLv3 license.
 *
 * @author taktod
 * @date   2016/02/26
 */

#ifdef __ENABLE_X265__

#include "x265Encoder.h"
#include "../log.h"
#include "../allocator.h"
#include "../util/dynamicBufferUtil.h"
#include <x265.h>
#include <string.h>

typedef struct ttLibC_Encoder_X265Encoder_ {
	ttLibC_X265Encoder inherit_super;
	const x265_api *api;
	x265_param *param;
	x265_encoder *encoder;
	bool is_pic_alloc;
} ttLibC_Encoder_X265Encoder_;

typedef ttLibC_Encoder_X265Encoder_ ttLibC_X265Encoder_;

ttLibC_X265Encoder *ttLibC_X265Encoder_make(
		uint32_t width,
		uint32_t height) {
	ttLibC_X265Encoder_ *encoder = ttLibC_malloc(sizeof(ttLibC_X265Encoder_));
	if(encoder == NULL) {
		return NULL;
	}
	// use default
	encoder->api = x265_api_get(0);

	encoder->param = encoder->api->param_alloc();
	encoder->encoder = NULL;
	char *preset = "ultrafast";
	char *tune = "grain";
	// get preset.
	if(encoder->api->param_default_preset(encoder->param, preset, tune) < 0) {
		ERR_PRINT("failed to understand preset or tune.");
		ttLibC_X265Encoder_close((ttLibC_X265Encoder **)&encoder);
		return NULL;
	}
	encoder->param->bAnnexB = 1;
	encoder->param->internalCsp = X265_CSP_I420;
	encoder->param->sourceWidth = width;
	encoder->param->sourceHeight = height;
	encoder->param->fpsNum = 15;
	encoder->param->fpsDenom = 1;
	encoder->encoder = encoder->api->encoder_open(encoder->param);
	if(!encoder->encoder) {
		LOG_PRINT("failed to open encoder.");
		ttLibC_X265Encoder_close((ttLibC_X265Encoder **)&encoder);
		return NULL;
	}
	// encoder is ready.
	return (ttLibC_X265Encoder *)encoder;
}

bool ttLibC_X265Encoder_encode(
		ttLibC_X265Encoder *encoder,
		ttLibC_Yuv420 *yuv420,
		ttLibC_X265EncodeFunc callback,
		void *ptr) {
	// do encode.
	ttLibC_X265Encoder_ *encoder_ = (ttLibC_X265Encoder_ *)encoder;
	// setup picture.
	x265_picture pic;
	encoder_->api->picture_init(encoder_->param, &pic);
	pic.stride[0] = yuv420->inherit_super.width;
	pic.stride[1] = yuv420->inherit_super.width / 2;
	pic.stride[2] = yuv420->inherit_super.width / 2;
	pic.planes[0] = yuv420->y_data;
	pic.planes[1] = yuv420->u_data;
	pic.planes[2] = yuv420->v_data;
	pic.pts = (int64_t)(yuv420->inherit_super.inherit_super.pts);

	// do encode.
	x265_nal *nal;
	uint32_t i_nal;
	x265_picture pic_out;
	int32_t frame_size = encoder_->api->encoder_encode(encoder_->encoder, &nal, &i_nal, &pic, &pic_out);
	// TODO make h265 data from nal.
	LOG_PRINT("output frame_size:%d", frame_size);
	return true;
}

void ttLibC_X265Encoder_close(ttLibC_X265Encoder **encoder) {
	ttLibC_X265Encoder_ *target = (ttLibC_X265Encoder_ *)*encoder;
	if(target == NULL) {
		return;
	}
	if(target->encoder != NULL) {
		target->api->encoder_close(target->encoder);
		target->encoder = NULL;
	}
	if(target->param != NULL) {
		target->api->param_free(target->param);
		target->api->cleanup();
		target->param = NULL;
	}
	ttLibC_free(target);
	*encoder = NULL;
}

#endif
