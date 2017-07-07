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
#include <jpeglib.h>

/*
 * detail definition of jpeg encoder.
 */
typedef struct {
	/** inherit data from ttLibC_JpegEncoder */
	ttLibC_JpegEncoder          inherit_super;
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr       jerr;
	struct jpeg_destination_mgr dmgr;
	ttLibC_Jpeg *jpeg;
} ttLibC_Encoder_JpegEncoder_;

typedef ttLibC_Encoder_JpegEncoder_ ttLibC_JpegEncoder_;

// setup buffer.
static void ttLibC_JpegEncoder_init_buffer(j_compress_ptr cinfo) {
	(void)cinfo;
}

// in the case of buffer is full.
static boolean ttLibC_JpegEncoder_empty_buffer(j_compress_ptr cinfo) {
	(void)cinfo;
	ERR_PRINT("need more buffer to make jpeg.");
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
	encoder->cinfo.err = jpeg_std_error(&encoder->jerr);
	jpeg_create_compress(&encoder->cinfo);
	encoder->cinfo.image_width = width;
	encoder->cinfo.image_height = height;
	encoder->cinfo.input_components = 3;
	jpeg_set_defaults(&encoder->cinfo);
	encoder->cinfo.dct_method = JDCT_FLOAT;
	jpeg_set_colorspace(&encoder->cinfo, JCS_YCbCr);
	encoder->cinfo.raw_data_in = true;
	encoder->cinfo.comp_info[0].h_samp_factor = 2;
	encoder->cinfo.comp_info[0].v_samp_factor = 2;
	encoder->cinfo.comp_info[1].h_samp_factor = 1;
	encoder->cinfo.comp_info[1].v_samp_factor = 1;
	encoder->cinfo.comp_info[2].h_samp_factor = 1;
	encoder->cinfo.comp_info[2].v_samp_factor = 1;
	encoder->cinfo.optimize_coding = true;
	jpeg_set_quality(&encoder->cinfo, quality, true);
#if JPEG_LIB_VERSION >= 70
	encoder->cinfo.do_fancy_downsampling = false;
#endif
	encoder->dmgr.init_destination    = ttLibC_JpegEncoder_init_buffer;
	encoder->dmgr.empty_output_buffer = ttLibC_JpegEncoder_empty_buffer;
	encoder->dmgr.term_destination    = ttLibC_JpegEncoder_term_buffer;

	encoder->cinfo.dest = &encoder->dmgr;

	encoder->jpeg = NULL;
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
	// jpeg size expected less than yuv420.
	size_t wh = yuv->inherit_super.width * yuv->inherit_super.height;
	uint8_t *data = NULL;
	size_t data_size = wh + (wh >> 1);
	bool alloc_flag = false;
	// check prev data.
	ttLibC_Jpeg *jpeg = encoder_->jpeg;

	if(jpeg != NULL) {
		if(!jpeg->inherit_super.inherit_super.is_non_copy) {
			// buffer can reuse.
			if(jpeg->inherit_super.inherit_super.data_size >= data_size) {
				// size is enough.
				data = jpeg->inherit_super.inherit_super.data;
				data_size = jpeg->inherit_super.inherit_super.data_size;
			}
			else {
				// size is too small for reuse.
				ttLibC_free(jpeg->inherit_super.inherit_super.data);
			}
		}
		if(data == NULL) {
			jpeg->inherit_super.inherit_super.data = NULL;
			jpeg->inherit_super.inherit_super.data_size = 0;
		}
		jpeg->inherit_super.inherit_super.is_non_copy = true;
	}
	if(data == NULL) {
		data = ttLibC_malloc(data_size);
		if(data == NULL) {
			ERR_PRINT("failed to allocate data buffer.");
			return false;
		}
		alloc_flag = true;
	}
	// do convert.
	encoder_->dmgr.next_output_byte = data;
	encoder_->dmgr.free_in_buffer = data_size;
	JSAMPROW y[16], cb[16], cr[16];
	JSAMPARRAY planes[3];
	planes[0] = y;
	planes[1] = cb;
	planes[2] = cr;
	jpeg_start_compress(&encoder_->cinfo, true);
	for(uint32_t j = 0;j < yuv->inherit_super.height;j += 16) {
		int max = yuv->inherit_super.height - j;
		if(max >= 16) {
			max = 16;
		}
		for(int i = 0;i < 16;i ++) {
			if(i >= max) {
				y[i] = yuv->y_data;
				if((i & 0x01) == 0) {
					cb[(i >> 1)] = yuv->u_data;
					cr[(i >> 1)] = yuv->v_data;
				}
			}
			else {
				y[i] = yuv->y_data + yuv->y_stride * (i + j);
				if((i & 0x01) == 0) {
					cb[(i >> 1)] = yuv->u_data + ((yuv->u_stride * (i + j)) >> 1);
					cr[(i >> 1)] = yuv->v_data + ((yuv->v_stride * (i + j)) >> 1);
				}
			}
		}
		jpeg_write_raw_data(&encoder_->cinfo, planes, 16);
	}
	jpeg_finish_compress(&encoder_->cinfo);
	jpeg = ttLibC_Jpeg_make(
			jpeg,
			encoder_->inherit_super.width,
			encoder_->inherit_super.height,
			data,
			data_size,
			true,
			yuv->inherit_super.inherit_super.pts,
			yuv->inherit_super.inherit_super.timebase);
	if(jpeg == NULL) {
		LOG_PRINT("jpeg output is null.");
		if(alloc_flag) {
			ttLibC_free(data);
		}
		return false;
	}
	// done.
	jpeg->inherit_super.inherit_super.buffer_size = encoder_->cinfo.dest->next_output_byte - data;
	jpeg->inherit_super.inherit_super.is_non_copy = false;
	encoder_->jpeg = jpeg;
	if(!callback(ptr, jpeg)) {
		return false;
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
	jpeg_set_quality(&encoder_->cinfo, quality, true);
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
	jpeg_destroy_compress(&target->cinfo);
	ttLibC_Jpeg_close(&target->jpeg);
	ttLibC_free(target);
	*encoder = NULL;
}

#endif
