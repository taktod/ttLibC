/*
 * @file   jpegEncoder.c
 * @brief  encode yuv420 with libjpeg.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/11/05
 */

#ifdef __ENABLE_JPEG__

#include "jpegEncoder.h"
#include "../ttLibC_predef.h"
#include "../_log.h"
#include "../allocator.h"
#include "../util/dynamicBufferUtil.h"
#include <jpeglib.h>

typedef struct {
	struct jpeg_compress_struct cinfo;
	ttLibC_DynamicBuffer       *buffer;
	uint8_t                    *data;
	size_t                      data_size;
} JpegEncoder_jpeg_compress_struct;

/*
 * detail definition of jpeg encoder.
 */
typedef struct {
	/** inherit data from ttLibC_JpegEncoder */
	ttLibC_JpegEncoder          inherit_super;
	JpegEncoder_jpeg_compress_struct cinfo;
	struct jpeg_error_mgr       jerr;
	struct jpeg_destination_mgr dmgr;
	ttLibC_Yuv420 *fullrange_yuv;
	ttLibC_Jpeg   *jpeg;
} ttLibC_Encoder_JpegEncoder_;

typedef ttLibC_Encoder_JpegEncoder_ ttLibC_JpegEncoder_;

// setup buffer.
static void ttLibC_JpegEncoder_init_buffer(j_compress_ptr cinfo) {
	(void)cinfo;
}

// in the case of buffer is full.
static boolean ttLibC_JpegEncoder_empty_buffer(j_compress_ptr cinfo) {
	JpegEncoder_jpeg_compress_struct *cinfo_ex = (JpegEncoder_jpeg_compress_struct *)cinfo;

	ttLibC_DynamicBuffer_append(cinfo_ex->buffer, cinfo_ex->data, cinfo_ex->data_size);
	cinfo->dest->next_output_byte = cinfo_ex->data;
	cinfo->dest->free_in_buffer = cinfo_ex->data_size;
	return true;
}

// in the case of no more use.
static void ttLibC_JpegEncoder_term_buffer(j_compress_ptr cinfo) {
	(void)cinfo;
}

/*
 * make jpeg encoder
 * @param width   target width
 * @param height  target height
 * @param quality target quality 0 - 100 100 is best quality.
 * @return jpegEncoder object.
 */
ttLibC_JpegEncoder TT_VISIBILITY_DEFAULT *ttLibC_JpegEncoder_make(
		uint32_t width,
		uint32_t height,
		uint32_t quality) {
	ttLibC_JpegEncoder_ *encoder = (ttLibC_JpegEncoder_ *)ttLibC_malloc(sizeof(ttLibC_JpegEncoder_));
	if(encoder == NULL) {
		ERR_PRINT("failed to alloc encoder object.");
		return NULL;
	}
	encoder->cinfo.data_size = 65536;
	encoder->cinfo.data = ttLibC_malloc(encoder->cinfo.data_size);
	if(encoder->cinfo.data == NULL) {
		ttLibC_free(encoder);
		ERR_PRINT("failed to alloc memory.");
		return NULL;
	}
	encoder->cinfo.buffer = ttLibC_DynamicBuffer_make();
	if(encoder->cinfo.buffer == NULL) {
		ttLibC_free(encoder->cinfo.data);
		ttLibC_free(encoder);
		ERR_PRINT("failed to alloc dynamicBuffer.");
		return NULL;
	}

	encoder->cinfo.cinfo.err = jpeg_std_error(&encoder->jerr);
	jpeg_create_compress(&encoder->cinfo.cinfo);
	encoder->cinfo.cinfo.image_width = width;
	encoder->cinfo.cinfo.image_height = height;
	encoder->cinfo.cinfo.input_components = 3;
	jpeg_set_defaults(&encoder->cinfo.cinfo);
	encoder->cinfo.cinfo.dct_method = JDCT_FLOAT;
	jpeg_set_colorspace(&encoder->cinfo.cinfo, JCS_YCbCr);
	encoder->cinfo.cinfo.raw_data_in = true;
	encoder->cinfo.cinfo.comp_info[0].h_samp_factor = 2;
	encoder->cinfo.cinfo.comp_info[0].v_samp_factor = 2;
	encoder->cinfo.cinfo.comp_info[1].h_samp_factor = 1;
	encoder->cinfo.cinfo.comp_info[1].v_samp_factor = 1;
	encoder->cinfo.cinfo.comp_info[2].h_samp_factor = 1;
	encoder->cinfo.cinfo.comp_info[2].v_samp_factor = 1;
	encoder->cinfo.cinfo.optimize_coding = true;
	jpeg_set_quality(&encoder->cinfo.cinfo, quality, true);
#if JPEG_LIB_VERSION >= 70
	encoder->cinfo.cinfo.do_fancy_downsampling = false;
#endif
	encoder->dmgr.init_destination    = ttLibC_JpegEncoder_init_buffer;
	encoder->dmgr.empty_output_buffer = ttLibC_JpegEncoder_empty_buffer;
	encoder->dmgr.term_destination    = ttLibC_JpegEncoder_term_buffer;

	encoder->cinfo.cinfo.dest = &encoder->dmgr;

	encoder->jpeg = NULL;
	encoder->fullrange_yuv = NULL;
	encoder->inherit_super.width = width;
	encoder->inherit_super.height = height;
	encoder->inherit_super.quality = quality;
	return (ttLibC_JpegEncoder *)encoder;
}

/*
 * encode frame.
 * @param encoder  jpeg encoder object.
 * @param yuv420   source yuv420 data
 * @param callback callback func for jpeg creation.
 * @param ptr      pointer for user def value, which will call in callback.
 * @return true / false
 */
bool TT_VISIBILITY_DEFAULT ttLibC_JpegEncoder_encode(
		ttLibC_JpegEncoder *encoder,
		ttLibC_Yuv420 *yuv,
		ttLibC_JpegEncodeFunc callback,
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
		ERR_PRINT("only support planar.");
		return false;
	}

	ttLibC_JpegEncoder_ *encoder_ = (ttLibC_JpegEncoder_ *)encoder;
	uint32_t width = encoder_->inherit_super.width;
	uint32_t height = encoder_->inherit_super.height;
	ttLibC_Yuv420 *fullrange_yuv = ttLibC_Yuv420_makeEmptyFrame2(
			encoder_->fullrange_yuv,
			Yuv420Type_planar,
			width,
			height);
	if(fullrange_yuv == NULL) {
		ERR_PRINT("failed to make full range frame");
		return false;
	}
	encoder_->fullrange_yuv = fullrange_yuv;
	// 16->235 -> 0-255
	uint8_t *y_dst = fullrange_yuv->y_data;
	uint8_t *y_src = yuv->y_data;
	for(uint32_t i = 0;i < height;++ i) {
		uint8_t *yd = y_dst;
		uint8_t *ys = y_src;
		int32_t y;
		for(uint32_t j = 0;j < width;++ j) {
			y = (((((*ys) * 1197) >> 6) - 299) >> 4);
			*yd = y > 255 ? 255 : y < 0 ? 0 : y;
			yd += fullrange_yuv->y_step;
			ys += yuv->y_step;
		}
		for(uint32_t j = width;j < fullrange_yuv->y_stride;++ j) {
			*yd = y > 255 ? 255 : y < 0 ? 0 : y;
			yd += fullrange_yuv->y_step;
		}
		y_dst += fullrange_yuv->y_stride;
		y_src += yuv->y_stride;
	}
	uint32_t half_width  = ((width + 1) >> 1);
	uint32_t half_height = ((height + 1) >> 1);
	uint8_t *u_dst = fullrange_yuv->u_data;
	uint8_t *v_dst = fullrange_yuv->v_data;
	uint8_t *u_src = yuv->u_data;
	uint8_t *v_src = yuv->v_data;
	for(uint32_t i = 0;i < half_height;++ i) {
		uint8_t *ud = u_dst;
		uint8_t *us = u_src;
		uint8_t *vd = v_dst;
		uint8_t *vs = v_src;
		uint8_t u, v;
		for(uint32_t j = 0;j < half_width;++ j) {
			u = *us;
			v = *vs;
			*ud = u;
			*vd = v;
			ud += fullrange_yuv->u_step;
			us += yuv->u_step;
			vd += fullrange_yuv->v_step;
			vs += yuv->v_step;
		}
		for(uint32_t j = half_width;j < fullrange_yuv->u_stride;++ j) {
			*ud = u;
			ud += fullrange_yuv->u_step;
		}
		for(uint32_t j = half_width;j < fullrange_yuv->v_stride;++ j) {
			*vd = v;
			vd += fullrange_yuv->v_step;
		}
		u_dst += fullrange_yuv->u_stride;
		u_src += yuv->u_stride;
		v_dst += fullrange_yuv->v_stride;
		v_src += yuv->v_stride;
	}
	// do convert.
	ttLibC_DynamicBuffer_empty(encoder_->cinfo.buffer);
	encoder_->dmgr.next_output_byte = encoder_->cinfo.data;
	encoder_->dmgr.free_in_buffer = encoder_->cinfo.data_size;
	JSAMPROW y[16], cb[16], cr[16];
	JSAMPARRAY planes[3];
	planes[0] = y;
	planes[1] = cb;
	planes[2] = cr;
	jpeg_start_compress(&encoder_->cinfo.cinfo, true);
	for(uint32_t j = 0;j < height;j += 16) {
		int max = yuv->inherit_super.height - j;
		if(max >= 16) {
			max = 16;
		}
		for(int i = 0;i < 16;i ++) {
			if(i >= max) {
				y[i] = fullrange_yuv->y_data;
				if((i & 0x01) == 0) {
					cb[(i >> 1)] = fullrange_yuv->u_data;
					cr[(i >> 1)] = fullrange_yuv->v_data;
				}
			}
			else {
				y[i] = fullrange_yuv->y_data + fullrange_yuv->y_stride * (i + j);
				if((i & 0x01) == 0) {
					cb[(i >> 1)] = fullrange_yuv->u_data + ((fullrange_yuv->u_stride * (i + j)) >> 1);
					cr[(i >> 1)] = fullrange_yuv->v_data + ((fullrange_yuv->v_stride * (i + j)) >> 1);
				}
			}
		}
		jpeg_write_raw_data(&encoder_->cinfo.cinfo, planes, 16);
	}
	jpeg_finish_compress(&encoder_->cinfo.cinfo);
	ttLibC_DynamicBuffer_append(encoder_->cinfo.buffer,
		encoder_->cinfo.data,
		encoder_->cinfo.cinfo.dest->next_output_byte - encoder_->cinfo.data);

	ttLibC_Jpeg *jpeg = ttLibC_Jpeg_make(
		encoder_->jpeg,
		encoder_->inherit_super.width,
		encoder_->inherit_super.height,
		ttLibC_DynamicBuffer_refData(encoder_->cinfo.buffer),
		ttLibC_DynamicBuffer_refSize(encoder_->cinfo.buffer),
		true,
		yuv->inherit_super.inherit_super.pts,
		yuv->inherit_super.inherit_super.timebase);
	if(jpeg == NULL) {
		ERR_PRINT("failed to make jpeg data.");
		return false;
	}
	encoder_->jpeg = jpeg;
	if(callback != NULL) {
		if(!callback(ptr, jpeg)) {
			return false;
		}
	}
	return true;
}

/**
 * update jpeg quality.
 * @param encoder jpeg encoder object.
 * @param quality ftarget quality 0 - 100
 * @return true:success false:error
 */
bool TT_VISIBILITY_DEFAULT ttLibC_JpegEncoder_setQuality(
		ttLibC_JpegEncoder *encoder,
		uint32_t quality) {
	if(encoder == NULL) {
		return false;
	}
	ttLibC_JpegEncoder_ *encoder_ = (ttLibC_JpegEncoder_ *)encoder;
	jpeg_set_quality(&encoder_->cinfo.cinfo, quality, true);
	return true;
}

/*
 * close jpeg encoder.
 * @param encoder
 */
void TT_VISIBILITY_DEFAULT ttLibC_JpegEncoder_close(ttLibC_JpegEncoder **encoder) {
	ttLibC_JpegEncoder_ *target = (ttLibC_JpegEncoder_ *)*encoder;
	if(target == NULL) {
		return;
	}
	jpeg_destroy_compress(&target->cinfo.cinfo);
	if(target->cinfo.data) {
		ttLibC_free(target->cinfo.data);
	}
	ttLibC_DynamicBuffer_close(&target->cinfo.buffer);
	ttLibC_Yuv420_close(&target->fullrange_yuv);
	ttLibC_Jpeg_close(&target->jpeg);
	ttLibC_free(target);
	*encoder = NULL;
}

#endif
