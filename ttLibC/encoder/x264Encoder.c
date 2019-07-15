/**
 * @file   x264Encoder.c
 * @brief  encode h264 with x264
 *
 * this code is under GPLv3 license
 *
 * @author taktod
 * @date   2016/02/18
 */

#ifdef __ENABLE_X264__

#include "x264Encoder.h"
#include "../ttLibC_predef.h"
#include "../_log.h"
#include "../allocator.h"
#include "../util/hexUtil.h"
#include "../util/dynamicBufferUtil.h"
#include <x264.h>
#include <string.h>

typedef struct ttLibC_Encoder_X264Encoder_ {
	ttLibC_X264Encoder inherit_super;
	bool is_pic_alloc;
	x264_picture_t pic;
	x264_t *enc;
	ttLibC_H264 *configData;
	ttLibC_H264 *h264;
	uint32_t luma_size;
	uint32_t chroma_size;
	uint64_t pts;
	uint32_t timebase;
	uint32_t id;
} ttLibC_Encoder_X264Encoder_;

typedef ttLibC_Encoder_X264Encoder_ ttLibC_X264Encoder_;

ttLibC_X264Encoder TT_VISIBILITY_DEFAULT *ttLibC_X264Encoder_make(
		uint32_t width,
		uint32_t height) {
	x264_param_t param;
	ttLibC_X264Encoder_getDefaultX264ParamT(&param, width, height);
	if(x264_param_apply_profile(&param, "baseline") < 0) {
		ERR_PRINT("failed to apply baseline.");
		return NULL;
	}
	return ttLibC_X264Encoder_makeWithX264ParamT(&param);
}

ttLibC_X264Encoder TT_VISIBILITY_DEFAULT *ttLibC_X264Encoder_make_ex(
		uint32_t width,
		uint32_t height,
		uint32_t max_quantizer,
		uint32_t min_quantizer,
		uint32_t bitrate) {
	x264_param_t param;
	ttLibC_X264Encoder_getDefaultX264ParamT(&param, width, height);
	param.rc.i_qp_max = max_quantizer;
	param.rc.i_qp_min = min_quantizer;
	param.rc.i_bitrate = bitrate / 1000;
	if(x264_param_apply_profile(&param, "baseline") < 0) {
		ERR_PRINT("failed to apply baseline.");
		return NULL;
	}
	return ttLibC_X264Encoder_makeWithX264ParamT(&param);
}

bool TT_VISIBILITY_DEFAULT ttLibC_X264Encoder_getDefaultX264ParamT(
		void *param_t,
		uint32_t width,
		uint32_t height) {
	return ttLibC_X264Encoder_getDefaultX264ParamTWithPresetTune(
			param_t,
			width,
			height,
			NULL,
			NULL);
}

bool TT_VISIBILITY_DEFAULT ttLibC_X264Encoder_getDefaultX264ParamTWithPresetTune(
		void *param_t,
		uint32_t width,
		uint32_t height,
		const char *preset,
		const char *tune) {
	x264_param_t *param = (x264_param_t *)param_t;
	char *_preset = (char *)preset;
	char *_tune = (char *)tune;
	if(preset == NULL || strlen(preset) == 0) {
		_preset = "ultrafast";
	}
	if(tune == NULL || strlen(tune) == 0) {
		_tune = "zerolatency";
//		_tune = "ssim";
	}
	if(x264_param_default_preset(param, _preset, _tune) < 0) {
		ERR_PRINT("failed to get default preset.");
		return false;
	}
	param->i_csp = X264_CSP_I420;
	param->i_width = width;
	param->i_height = height;
	param->b_vfr_input = 1;
	param->b_repeat_headers = 1;
	param->b_annexb = 1;

	param->i_log_level = X264_LOG_NONE;
	param->i_timebase_den = 1000;
	param->i_timebase_num = 1;
	param->i_keyint_max = 15;
	param->i_keyint_min = 15;
	return true;
}

ttLibC_X264Encoder TT_VISIBILITY_DEFAULT *ttLibC_X264Encoder_makeWithX264ParamT(void *param_t) {
	// make memory object.
	x264_param_t *param = (x264_param_t *)param_t;
	ttLibC_X264Encoder_ *encoder = (ttLibC_X264Encoder_ *)ttLibC_malloc(sizeof(ttLibC_X264Encoder_));
	if(encoder == NULL) {
		ERR_PRINT("failed to alloc encoder object.");
		return NULL;
	}
	encoder->is_pic_alloc = false;
	encoder->enc = NULL;
	encoder->configData = NULL;
	encoder->h264 = NULL;

	encoder->luma_size = param->i_width * param->i_height;
	encoder->chroma_size = (encoder->luma_size >> 2);
	encoder->inherit_super.width = param->i_width;
	encoder->inherit_super.height = param->i_height;
	// calc for pts.
	encoder->pts = 0;
	encoder->timebase = (uint32_t)(param->i_timebase_den / param->i_timebase_num);
	// make picture.
/*	if(x264_picture_alloc(&encoder->pic, param->i_csp, param->i_width, param->i_height) < 0) {
		ERR_PRINT("failed to make picture.");
		ttLibC_free(encoder);
		return NULL;
	}
	encoder->is_pic_alloc = true;*/
	x264_picture_init(&encoder->pic);
	// open encoder.
	encoder->enc = x264_encoder_open(param);
	if(!encoder->enc) {
		ERR_PRINT("failed to open x264Encoder");
		x264_picture_clean(&encoder->pic);
		encoder->is_pic_alloc = false;
		ttLibC_free(encoder);
		return NULL;
	}
	encoder->pic.img.i_csp = param->i_csp;
	encoder->pic.img.i_plane = 3;
	return (ttLibC_X264Encoder *)encoder;
}

/*
 * force next encode picture will be target frame type.
 */
bool TT_VISIBILITY_DEFAULT ttLibC_X264Encoder_forceNextFrameType(
		ttLibC_X264Encoder *encoder,
		ttLibC_X264Encoder_FrameType frame_type) {
	ttLibC_X264Encoder_ *encoder_ = (ttLibC_X264Encoder_ *)encoder;
	if(encoder_ == NULL) {
		return false;
	}
	switch(frame_type) {
	default:
	case X264FrameType_Auto:
		encoder_->pic.i_type = X264_TYPE_AUTO;
		break;
	case X264FrameType_IDR:
		encoder_->pic.i_type = X264_TYPE_IDR;
		break;
	case X264FrameType_I:
		encoder_->pic.i_type = X264_TYPE_I;
		break;
	case X264FrameType_P:
		encoder_->pic.i_type = X264_TYPE_P;
		break;
	case X264FrameType_Bref:
		encoder_->pic.i_type = X264_TYPE_BREF;
		break;
	case X264FrameType_B:
		encoder_->pic.i_type = X264_TYPE_B;
		break;
	case X264FrameType_KeyFrame:
		encoder_->pic.i_type = X264_TYPE_KEYFRAME;
		break;
	}
	return true;
}

static bool X264Encoder_makeH264Frame(
		ttLibC_H264_Type target_type,
		ttLibC_X264Encoder_ *encoder,
		ttLibC_DynamicBuffer *buffer,
		ttLibC_X264EncodeFunc callback,
		void *ptr) {
	// in the case of error, close dynamicBuffer here.
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
				0,
				encoder->timebase);
		if(h264 == NULL) {
			ERR_PRINT("failed to make configData for h264Frame.");
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
				encoder->pts,
				encoder->timebase);
		if(h264 == NULL) {
			ERR_PRINT("failed to make h264 frame");
			ttLibC_DynamicBuffer_close(&buffer);
			return false;
		}
		encoder->h264 = h264;
	}
	h264->inherit_super.inherit_super.id = encoder->id;
	if(!callback(ptr, h264)) {
		ttLibC_DynamicBuffer_close(&buffer);
		return false;
	}
	return true;
}

static bool X264Encoder_checkEncodedData(
		ttLibC_X264Encoder_ *encoder,
		x264_nal_t *nal,
		int32_t nal_count,
		int32_t frame_size,
 		ttLibC_X264EncodeFunc callback,
		void *ptr) {
	if(frame_size == 0) {
		return true;
	}
	ttLibC_DynamicBuffer *target_buffer = ttLibC_DynamicBuffer_make();
	ttLibC_H264_Type target_type = H264Type_unknown;
	ttLibC_H264_NalInfo nal_info;
	for(int32_t i = 0;i < nal_count;++ i, ++ nal) {
		uint8_t *buf = nal->p_payload;
		size_t buf_size = nal->i_payload;
		frame_size -= buf_size;
		// analyze nal information.
		if(!ttLibC_H264_getNalInfo(&nal_info, buf, (size_t)nal->i_payload)) {
			ERR_PRINT("h264 data is corrupted.");
			ttLibC_DynamicBuffer_close(&target_buffer);
			return false;
		}
		switch(nal_info.nal_unit_type) {
		case H264NalType_error:
			ERR_PRINT("unknown nal type is found.");
			ttLibC_DynamicBuffer_close(&target_buffer);
			return false;
		default:
			LOG_PRINT("nal type is not implemented now.:%x", nal_info.nal_unit_type);
			break;
		case H264NalType_supplementalEnhancementInformation:
			break;
		case H264NalType_slice:
			if(target_type != H264Type_slice) {
				if(target_type != H264Type_unknown) {
					// save prev information.
					if(!X264Encoder_makeH264Frame(
							target_type,
							encoder,
							target_buffer,
							callback,
							ptr)) {
						return false;
					}
					ttLibC_DynamicBuffer_empty(target_buffer);
				}
			}
			target_type = H264Type_slice;
			ttLibC_DynamicBuffer_append(target_buffer, buf, buf_size);
			break;
		case H264NalType_sliceIDR:
			if(target_type != H264Type_sliceIDR) {
				if(target_type != H264Type_unknown) {
					// save prev information.
					if(!X264Encoder_makeH264Frame(
							target_type,
							encoder,
							target_buffer,
							callback,
							ptr)) {
						return false;
					}
					ttLibC_DynamicBuffer_empty(target_buffer);
				}
			}
			target_type = H264Type_sliceIDR;
			ttLibC_DynamicBuffer_append(target_buffer, buf, buf_size);
			break;
		case H264NalType_sequenceParameterSet:
		case H264NalType_pictureParameterSet:
			if(target_type != H264Type_configData) {
				if(target_type != H264Type_unknown) {
					// save prev information.
					if(!X264Encoder_makeH264Frame(
							target_type,
							encoder,
							target_buffer,
							callback,
							ptr)) {
						return false;
					}
					ttLibC_DynamicBuffer_empty(target_buffer);
				}
			}
			target_type = H264Type_configData;
			ttLibC_DynamicBuffer_append(target_buffer, buf, buf_size);
 			break;
		}
	}
	if(frame_size != 0) {
		ERR_PRINT("nalSize or frame_size is corrupted.");
		ttLibC_DynamicBuffer_close(&target_buffer);
		return false;
	}
	if(!X264Encoder_makeH264Frame(
			target_type,
			encoder,
			target_buffer,
			callback,
			ptr)) {
		return false;
	}
	ttLibC_DynamicBuffer_close(&target_buffer);
	return true;
}

bool TT_VISIBILITY_DEFAULT ttLibC_X264Encoder_encode(
		ttLibC_X264Encoder *encoder,
		ttLibC_Yuv420 *yuv420,
		ttLibC_X264EncodeFunc callback,
		void *ptr) {
	ttLibC_X264Encoder_ *encoder_ = (ttLibC_X264Encoder_ *)encoder;
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
	encoder_->id = yuv420->inherit_super.inherit_super.id;
	// copy yuv data to pic.
	// check: can I use ref copy?
//	memcpy(encoder_->pic.img.plane[0], yuv420->y_data, encoder_->luma_size);
//	memcpy(encoder_->pic.img.plane[1], yuv420->u_data, encoder_->chroma_size);
//	memcpy(encoder_->pic.img.plane[2], yuv420->v_data, encoder_->chroma_size);
	encoder_->pic.img.plane[0] = yuv420->y_data;
	encoder_->pic.img.plane[1] = yuv420->u_data;
	encoder_->pic.img.plane[2] = yuv420->v_data;
	encoder_->pic.img.i_stride[0] = yuv420->y_stride;
	encoder_->pic.img.i_stride[1] = yuv420->u_stride;
	encoder_->pic.img.i_stride[2] = yuv420->v_stride;
	// update pic pts.
	encoder_->pic.i_pts = (int64_t)(yuv420->inherit_super.inherit_super.pts * encoder_->timebase / yuv420->inherit_super.inherit_super.timebase);

	x264_nal_t *nal;
	int32_t i_nal;
	x264_picture_t pic;
	int32_t frame_size = x264_encoder_encode(encoder_->enc, &nal, &i_nal, &encoder_->pic, &pic);
	if(frame_size < 0) {
		ERR_PRINT("failed to encode data.");
		return false;
	}
	encoder_->pic.i_type = X264_TYPE_AUTO;
	if(frame_size != 0) {
		// TODO shoud I support dts?
		encoder_->pts = pic.i_pts;
	}
	/*
	switch(pic.i_type) {
	case X264_TYPE_IDR:
		LOG_PRINT("idr frame");
		break;
	case X264_TYPE_I:
		LOG_PRINT("i frame");
		break;
	case X264_TYPE_P:
		LOG_PRINT("p frame");
		break;
	case X264_TYPE_BREF:
		LOG_PRINT("bref frame");
		break;
	case X264_TYPE_B:
		LOG_PRINT("b frame");
		break;
	case X264_TYPE_KEYFRAME:
		LOG_PRINT("key frame");
		break;
	case X264_TYPE_AUTO:
	default:
		LOG_PRINT("unknown");
		break;
	}
	*/
	return X264Encoder_checkEncodedData(encoder_, nal, i_nal, frame_size, callback, ptr);
}

/*
 * parse params
 * @param param_t structure pointer for x264_param_t on x264.h
 * @param key     key
 * @param value   value
 * @return int
 */
int TT_VISIBILITY_DEFAULT ttLibC_X264Encoder_paramParse(
		void *param_t,
		const char *key,
		const char *value) {
	return x264_param_parse((x264_param_t *)param_t, key, value);
}

/**
 * apply profile
 * @param param_t structure pointer for x264_param_t on x264.h
 * @param profile profile
 * @return int
 */
int TT_VISIBILITY_DEFAULT ttLibC_X264Encoder_paramApplyProfile(
	void * param_t,
	const char *profile) {
	return x264_param_apply_profile((x264_param_t *)param_t, profile);
}

void TT_VISIBILITY_DEFAULT ttLibC_X264Encoder_close(ttLibC_X264Encoder **encoder) {
	ttLibC_X264Encoder_ *target = (ttLibC_X264Encoder_ *)*encoder;
	if(target == NULL) {
		return;
	}
	if(target->enc) {
		x264_encoder_close(target->enc);
		target->enc = NULL;
	}
/*	if(target->is_pic_alloc) {
		x264_picture_clean(&target->pic);
		target->is_pic_alloc = false;
	}*/
	ttLibC_H264_close(&target->h264);
	ttLibC_H264_close(&target->configData);
	ttLibC_free(target);
	*encoder = NULL;
}

#endif

