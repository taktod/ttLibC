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
	bool is_first_frame;
	ttLibC_H265 *h265;
	uint64_t pts;
	uint32_t timebase;
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
	encoder->is_first_frame = true;
	encoder->h265 = NULL;
	encoder->pts = 0;
	encoder->timebase = 1000;
	// encoder is ready.
	return (ttLibC_X265Encoder *)encoder;
}

static bool X265Encoder_makeH265Frame(
		ttLibC_H265_Type target_type,
		ttLibC_X265Encoder_ *encoder,
		ttLibC_DynamicBuffer *buffer,
		ttLibC_X265EncodeFunc callback,
		void *ptr) {
	ttLibC_H265 *h265 = NULL;
	h265 = ttLibC_H265_make(
			encoder->h265,
			target_type,
			encoder->param->sourceWidth,
			encoder->param->sourceHeight,
			ttLibC_DynamicBuffer_refData(buffer),
			ttLibC_DynamicBuffer_refSize(buffer),
			true,
			encoder->pts,
			encoder->timebase);
	if(h265 == NULL) {
		ERR_PRINT("failed to make h265 frame.");
		return false;
	}
	encoder->h265 = h265;
	if(callback != NULL) {
		return callback(ptr, h265);
	}
	return true;
}

static bool X265Encoder_checkEncodedData(
		ttLibC_X265Encoder_ *encoder,
		x265_nal *nal,
		int32_t nal_count,
		int32_t output_flag,
		ttLibC_X265EncodeFunc callback,
		void *ptr) {
	if(output_flag == 0) {
		return true;
	}
	ttLibC_DynamicBuffer *target_buffer = ttLibC_DynamicBuffer_make();
	ttLibC_H265_Type target_type =H265Type_unknown;
	ttLibC_H265_NalInfo nal_info;
	for(int32_t i = 0;i < nal_count;++ i, ++ nal) {
		uint8_t *buf = nal->payload;
		size_t buf_size = nal->sizeBytes;
		if(!ttLibC_H265_getNalInfo(&nal_info, buf, (size_t)nal->sizeBytes)) {
			ERR_PRINT("h265 data is corrupted.");
			ttLibC_DynamicBuffer_close(&target_buffer);
			return false;
		}
		switch(nal_info.nal_unit_type) {
		case H265NalType_error:
			ERR_PRINT("unknown nal type is found.");
			ttLibC_DynamicBuffer_close(&target_buffer);
			return false;
		default:
			LOG_PRINT("nal type is not implemented now.:%x", nal_info.nal_unit_type);
			break;
		case H265NalType_vpsNut:
		case H265NalType_spsNut:
		case H265NalType_ppsNut:
			if(target_type != H265Type_configData) {
				if(target_type != H265Type_unknown) {
					bool result = X265Encoder_makeH265Frame(
							target_type,
							encoder,
							target_buffer,
							callback,
							ptr);
					if(!result) {
						ttLibC_DynamicBuffer_close(&target_buffer);
						return false;
					}
				}
			}
			target_type = H265Type_configData;
			ttLibC_DynamicBuffer_append(target_buffer, buf, buf_size);
			break;
		case H265NalType_prefixSeiNut:
			break;
		case H265NalType_idrWRadl:
			if(target_type != H265Type_sliceIDR) {
				if(target_type != H265Type_unknown) {
					// save prev information.
					bool result = X265Encoder_makeH265Frame(
							target_type,
							encoder,
							target_buffer,
							callback,
							ptr);
					if(!result) {
						ttLibC_DynamicBuffer_close(&target_buffer);
						return false;
					}
				}
			}
			target_type = H265Type_sliceIDR;
			ttLibC_DynamicBuffer_append(target_buffer, buf, buf_size);
			break;
		case H265NalType_trailN:
		case H265NalType_trailR:
			if(target_type != H265Type_slice) {
				if(target_type != H265Type_unknown) {
					// save prev information.
					bool result = X265Encoder_makeH265Frame(
							target_type,
							encoder,
							target_buffer,
							callback,
							ptr);
					if(!result) {
						ttLibC_DynamicBuffer_close(&target_buffer);
						return false;
					}
				}
			}
			target_type = H265Type_slice;
			ttLibC_DynamicBuffer_append(target_buffer, buf, buf_size);
			break;
		}
	}
	bool result = X265Encoder_makeH265Frame(
			target_type,
			encoder,
			target_buffer,
			callback,
			ptr);
	ttLibC_DynamicBuffer_close(&target_buffer);
	return result;
}

bool ttLibC_X265Encoder_encode(
		ttLibC_X265Encoder *encoder,
		ttLibC_Yuv420 *yuv420,
		ttLibC_X265EncodeFunc callback,
		void *ptr) {
	// do encode.
	ttLibC_X265Encoder_ *encoder_ = (ttLibC_X265Encoder_ *)encoder;
	if(encoder_ == NULL) {
		return false;
	}
	if(yuv420 == NULL) {
		return true;
	}
	switch(yuv420->type) {
	case Yuv420Type_planar:
	case Yvu420Type_planar:
		break;
	case Yuv420Type_semiPlanar:
	case Yvu420Type_semiPlanar:
		ERR_PRINT("only support planar.");
		return false;
	}
	encoder_->timebase = yuv420->inherit_super.inherit_super.timebase;
	if(encoder_->is_first_frame) {
		x265_nal *nal;
		uint32_t i_nal;
		int output_flag = encoder_->api->encoder_headers(encoder_->encoder, &nal, &i_nal);
		if(!X265Encoder_checkEncodedData(encoder_, nal, i_nal, output_flag, callback, ptr)) {
			return false;
		}
		encoder_->is_first_frame = false;
	}
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
	int32_t output_flag = encoder_->api->encoder_encode(encoder_->encoder, &nal, &i_nal, &pic, &pic_out);
	if(output_flag < 0) {
		ERR_PRINT("failed to encode data.");
		return false;
	}
	if(output_flag != 0) {
		encoder_->pts = pic_out.pts;
	}
	return X265Encoder_checkEncodedData(encoder_, nal, i_nal, output_flag, callback, ptr);
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
	ttLibC_H265_close(&target->h265);
	ttLibC_free(target);
	*encoder = NULL;
}

#endif
