/**
 * @file   swscaleResampler.c
 * @brief  
 * @author taktod
 * @date   2017/04/28
 */

#ifdef __ENABLE_SWSCALE__

#include "swscaleResampler.h"
#include <libswscale/swscale.h>
#include "../allocator.h"
#include "../_log.h"

typedef struct {
	ttLibC_SwscaleResampler inherit_super;
	struct SwsContext      *convertCtx;
	ttLibC_Frame_Type       input_type;
	uint32_t                input_sub_type;
	uint32_t								input_width;
	uint32_t								input_height;
	ttLibC_Frame_Type       output_type;
	uint32_t                output_sub_type;
	uint32_t 								output_width;
	uint32_t								output_height;
} ttLibC_Resampler_SwscaleResampler_;

typedef ttLibC_Resampler_SwscaleResampler_ ttLibC_SwscaleResampler_;

static enum AVPixelFormat SwscaleResampler_getPixFormat(
		ttLibC_Frame_Type type,
		uint32_t sub_type) {
	switch(type) {
	case frameType_bgr:
		{
			ttLibC_Bgr_Type bgrType = (ttLibC_Bgr_Type)sub_type;
			switch(bgrType) {
			case BgrType_abgr:
				return AV_PIX_FMT_ABGR;
			case BgrType_bgr:
				return AV_PIX_FMT_BGR24;
			case BgrType_bgra:
				return AV_PIX_FMT_BGRA;
			case BgrType_argb:
				return AV_PIX_FMT_ARGB;
			case BgrType_rgb:
				return AV_PIX_FMT_RGB24;
			case BgrType_rgba:
				return AV_PIX_FMT_RGBA;
			default:
				return AV_PIX_FMT_NONE;
			}
		}
		break;
	case frameType_yuv420:
		{
			ttLibC_Yuv420_Type yuvType = (ttLibC_Yuv420_Type)sub_type;
			switch(yuvType) {
			case Yuv420Type_planar:
				return AV_PIX_FMT_YUV420P;
			case Yuv420Type_semiPlanar:
				return AV_PIX_FMT_NV12;
			case Yvu420Type_planar:
				return AV_PIX_FMT_YUV420P;
			case Yvu420Type_semiPlanar:
				return AV_PIX_FMT_NV21;
			default:
				return AV_PIX_FMT_NONE;
			}
		}
		break;
	default:
		return AV_PIX_FMT_NONE;
	}
}

static bool SwscaleResampler_setupDataStride(
		uint8_t     **data,
		uint32_t     *strides,
		ttLibC_Video *video) {
	switch(video->inherit_super.type) {
	case frameType_bgr:
		{
			ttLibC_Bgr *bgr = (ttLibC_Bgr *)video;
			data[0]    = bgr->data;
			data[1]    = NULL;
			data[2]    = NULL;
			data[3]    = NULL;
			strides[0] = bgr->width_stride;
			strides[1] = 0;
			strides[2] = 0;
			strides[3] = 0;
		}
		break;
	case frameType_yuv420:
		{
			ttLibC_Yuv420 *yuv = (ttLibC_Yuv420 *)video;
			switch(yuv->type) {
			case Yuv420Type_planar:
			case Yvu420Type_planar:
				{
					data[0] = yuv->y_data;
					data[1] = yuv->u_data;
					data[2] = yuv->v_data;
					data[3] = NULL;
					strides[0] = yuv->y_stride;
					strides[1] = yuv->u_stride;
					strides[2] = yuv->v_stride;
					strides[3] = 0;
				}
				break;
			case Yuv420Type_semiPlanar:
				{
					data[0] = yuv->y_data;
					data[1] = yuv->u_data;
					data[2] = NULL;
					data[3] = NULL;
					strides[0] = yuv->y_stride;
					strides[1] = yuv->u_stride;
					strides[2] = 0;
					strides[3] = 0;
				}
				break;
			case Yvu420Type_semiPlanar:
				{
					data[0] = yuv->y_data;
					data[1] = yuv->v_data;
					data[2] = NULL;
					data[3] = NULL;
					strides[0] = yuv->y_stride;
					strides[1] = yuv->v_stride;
					strides[2] = 0;
					strides[3] = 0;
				}
				break;
			default:
				return false;
			}
		}
		break;
	default:
		return false;
	}
	return true;
}

ttLibC_SwscaleResampler TT_ATTRIBUTE_API *ttLibC_SwscaleResampler_make(
		ttLibC_Frame_Type            input_frame_type,
		uint32_t                     input_sub_type,
		uint32_t                     input_width,
		uint32_t                     input_height,
		ttLibC_Frame_Type            output_frame_type,
		uint32_t                     output_sub_type,
		uint32_t                     output_width,
		uint32_t                     output_height,
		ttLibC_SwscaleResampler_Mode scale_mode) {
	ttLibC_SwscaleResampler_ *resampler = ttLibC_malloc(sizeof(ttLibC_SwscaleResampler_));
	if(resampler == NULL) {
		return NULL;
	}
	resampler->inherit_super.width  = output_width;
	resampler->inherit_super.height = output_height;
	resampler->input_type      = input_frame_type;
	resampler->input_sub_type  = input_sub_type;
	resampler->input_width     = input_width;
	resampler->input_height    = input_height;
	resampler->output_type     = output_frame_type;
	resampler->output_sub_type = output_sub_type;
	resampler->output_width    = output_width;
	resampler->output_height   = output_height;

	enum AVPixelFormat in_format  = SwscaleResampler_getPixFormat(input_frame_type, input_sub_type);
	enum AVPixelFormat out_format = SwscaleResampler_getPixFormat(output_frame_type, output_sub_type);

	int flags = SwscaleResampler_FastBiLinear;
	switch(scale_mode) {
	case SwscaleResampler_FastBiLinear:
		flags = SWS_FAST_BILINEAR;
		break;
	case SwscaleResampler_Bilinear:
		flags = SWS_BILINEAR;
		break;
	case SwscaleResampler_Bicubic:
		flags = SWS_BICUBIC;
		break;
	case SwscaleResampler_X:
		flags = SWS_X;
		break;
	case SwscaleResampler_Point:
		flags = SWS_POINT;
		break;
	case SwscaleResampler_Area:
		flags = SWS_AREA;
		break;
	case SwscaleResampler_Bicublin:
		flags = SWS_BICUBLIN;
		break;
	case SwscaleResampler_Gauss:
		flags = SWS_GAUSS;
		break;
	case SwscaleResampler_Sinc:
		flags = SWS_SINC;
		break;
	case SwscaleResampler_Lanczos:
		flags = SWS_LANCZOS;
		break;
	case SwscaleResampler_Spline:
		flags = SWS_SPLINE;
		break;
	default:
		break;
	};
	resampler->convertCtx = sws_getContext(
			input_width,
			input_height,
			in_format,
			output_width,
			output_height,
			out_format,
			flags,
			NULL,
			NULL,
			NULL);
	return (ttLibC_SwscaleResampler *)resampler;
}

bool TT_ATTRIBUTE_API ttLibC_SwscaleResampler_resample(
		ttLibC_SwscaleResampler *resampler,
		ttLibC_Video *dest_frame,
		ttLibC_Video *src_frame) {
	ttLibC_SwscaleResampler_ *resampler_ = (ttLibC_SwscaleResampler_ *)resampler;
	if(resampler_ == NULL) {
		return false;
	}
	if(dest_frame == NULL) {
		return false;
	}
	if(src_frame == NULL) {
		return false;
	}
	if(src_frame->inherit_super.type  != resampler_->input_type
	|| dest_frame->inherit_super.type != resampler_->output_type) {
		return false;
	}
	uint32_t src_subtype;
	uint32_t dest_subtype;
	if(src_frame->inherit_super.type == frameType_bgr) {
		src_subtype = ((ttLibC_Bgr *)src_frame)->type;
	}
	else if(src_frame->inherit_super.type == frameType_yuv420) {
		src_subtype = ((ttLibC_Yuv420 *)src_frame)->type;
	}
	else {
		return false;
	}
	if(dest_frame->inherit_super.type == frameType_bgr) {
		dest_subtype = ((ttLibC_Bgr *)dest_frame)->type;
	}
	else if(dest_frame->inherit_super.type == frameType_yuv420) {
		dest_subtype = ((ttLibC_Yuv420 *)dest_frame)->type;
	}
	else {
		return false;
	}
	uint8_t *src_data[4];
	uint32_t src_stride[4];
	uint8_t *dst_data[4];
	uint32_t dst_stride[4];
	if(!SwscaleResampler_setupDataStride(
			src_data,
			src_stride,
			src_frame)) {
		return false;
	}
	if(!SwscaleResampler_setupDataStride(
			dst_data,
			dst_stride,
			dest_frame)) {
		return false;
	}
	sws_scale(
			resampler_->convertCtx,
			(const uint8_t *const *)src_data,
			(const int *)src_stride,
			0,
			src_frame->height,
			(uint8_t *const *)dst_data,
			(const int *)dst_stride);
	return true;
}

void TT_ATTRIBUTE_API ttLibC_SwscaleResampler_close(ttLibC_SwscaleResampler **resampler) {
	ttLibC_SwscaleResampler_ *target = (ttLibC_SwscaleResampler_ *)*resampler;
	if(target == NULL) {
		return;
	}
//	sws_freeContext(target->convertCtx);
	ttLibC_free(target);
	*resampler = NULL;
}

#endif
