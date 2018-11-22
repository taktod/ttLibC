/*
 * @file   jpegDecoder.c
 * @brief  decode jpeg with libjpeg
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/11/05
 */

#ifdef __ENABLE_JPEG__

#include "jpegDecoder.h"
#include "../ttLibC_predef.h"
#include "../_log.h"
#include "../allocator.h"
#include "../util/hexUtil.h"

#include <jpeglib.h>
#include <setjmp.h>

/*
 * detail definition of jpeg decoder.
 */
typedef struct {
	/** inherit data from ttLibC_JpegDecoder */
	ttLibC_JpegDecoder inherit_super;
	struct jpeg_decompress_struct dinfo;
	struct jpeg_error_mgr jerr;
	ttLibC_Yuv420 *yuv420;
	void *dummy_buffer;
	size_t dummy_buffer_size;
} ttLibC_Decoder_JpegDecoder_;

typedef ttLibC_Decoder_JpegDecoder_ ttLibC_JpegDecoder_;

/*
 * make jpeg decoder
 * @return jpegDecoder object.
 */
ttLibC_JpegDecoder TT_VISIBILITY_DEFAULT *ttLibC_JpegDecoder_make() {
	ttLibC_JpegDecoder_ *decoder = ttLibC_malloc(sizeof(ttLibC_JpegDecoder_));
	if(decoder == NULL) {
		ERR_PRINT("failed to allocate decoder object.");
		return NULL;
	}
	decoder->dinfo.err = jpeg_std_error(&decoder->jerr);
	jpeg_create_decompress(&decoder->dinfo);

	decoder->yuv420 = NULL;
	decoder->dummy_buffer = NULL;
	decoder->dummy_buffer_size = 0;
	return (ttLibC_JpegDecoder *)decoder;
}

/*
 * decode frame
 * @param decoder  jpeg decoder object.
 * @param jpeg     source jpeg data
 * @param callback callback func for jpeg decode
 * @param ptr      pointer for use def value, which will call in callback.
 * @return true / false
 */
bool TT_VISIBILITY_DEFAULT ttLibC_JpegDecoder_decode(
		ttLibC_JpegDecoder *decoder,
		ttLibC_Jpeg *jpeg,
		ttLibC_JpegDecodeFunc callback,
		void *ptr) {
	if(decoder == NULL) {
		return false;
	}
	if(jpeg == NULL) {
		return true;
	}
	ttLibC_JpegDecoder_ *decoder_ = (ttLibC_JpegDecoder_ *)decoder;
	// set memory source.
	FILE *fp = NULL;
#if JPEG_LIB_VERSION >= 80 || defined(MEM_SRCDST_SUPPORTED)
	// set memory source.
	jpeg_mem_src(&decoder_->dinfo, (unsigned char *)jpeg->inherit_super.inherit_super.data, jpeg->inherit_super.inherit_super.buffer_size);
#else
	// try do with memory map, for linux only.
	fp = fmemopen((unsigned char *)jpeg->inherit_super.inherit_super.data, jpeg->inherit_super.inherit_super.buffer_size, "rb");
	jpeg_stdio_src(&decoder_->dinfo, fp);
#endif

	jpeg_read_header(&decoder_->dinfo, TRUE);

	JSAMPROW y[16], cb[8], cr[8];
	JSAMPARRAY planes[3] = {y, cb, cr};

	decoder_->dinfo.jpeg_color_space = JCS_YCbCr;
	decoder_->dinfo.raw_data_out = true;
#if JPEG_LIB_VERSION >= 70
	// fancy sampling can be the cause of error.
	decoder_->dinfo.do_fancy_upsampling = false;
#endif
	jpeg_start_decompress(&decoder_->dinfo);

	// try to make yuv buffer.
	uint8_t *data = NULL;
	decoder_->inherit_super.width = decoder_->dinfo.output_width;
	decoder_->inherit_super.height = decoder_->dinfo.output_height;
	uint32_t f_stride = ((((decoder_->inherit_super.width - 1) >> 4) + 1) << 4);
	uint32_t h_stride = (((((decoder_->inherit_super.width >> 1) - 1) >> 4) + 1) << 4);
	size_t y_size = f_stride * decoder_->inherit_super.height;
	size_t u_size = h_stride * (decoder_->inherit_super.height >> 1);
	size_t data_size = y_size + (u_size << 1);
	bool alloc_flag = false;
	// check the size is multiple of 16 or not.
	if(decoder_->inherit_super.height % 16 > 0) {
		size_t dummy_size = decoder_->inherit_super.width * 3;
		if(decoder_->dummy_buffer_size < dummy_size) {
			if(decoder_->dummy_buffer != NULL) {
				ttLibC_free(decoder_->dummy_buffer);
			}
			decoder_->dummy_buffer = ttLibC_malloc(dummy_size);
			if(decoder_->dummy_buffer == NULL) {
				ERR_PRINT("failed to make dummy buffer.");
				return false;
			}
			decoder_->dummy_buffer_size = dummy_size;
		}
	}

	// check prev yuv data for reuse.
	ttLibC_Yuv420 *yuv = decoder_->yuv420;
	if(yuv != NULL) {
		if(!yuv->inherit_super.inherit_super.is_non_copy) {
			// buffer can reuse
			if(yuv->inherit_super.inherit_super.data_size >= data_size) {
				data = yuv->inherit_super.inherit_super.data;
				data_size = yuv->inherit_super.inherit_super.data_size;
			}
			else {
				// size is not enough, free and re-alloc.
				ttLibC_free(yuv->inherit_super.inherit_super.data);
			}
		}
		if(data == NULL) {
			yuv->inherit_super.inherit_super.data = NULL;
			yuv->inherit_super.inherit_super.data_size = 0;
		}
		yuv->inherit_super.inherit_super.is_non_copy = true;
	}
	if(data == NULL) {
		data = ttLibC_malloc(data_size);
		alloc_flag = true;
	}

	uint8_t *y_data = data;
	uint8_t *u_data = data + y_size;
	uint8_t *v_data = u_data + u_size;
	for(uint32_t j = 0;j < decoder_->inherit_super.height;j += 16) {
		uint32_t max = decoder_->inherit_super.height - j;
		if(max >= 16) {
			max = 16;
		}
		for(uint32_t i = 0;i < 16;i ++) {
			if(i >= max) {
				y[i] = decoder_->dummy_buffer;
				if((i & 0x01) == 0) {
					cb[(i >> 1)] = decoder_->dummy_buffer + decoder_->inherit_super.width;
					cr[(i >> 1)] = decoder_->dummy_buffer + decoder_->inherit_super.width * 2;
				}
			}
			else {
				y[i] = y_data;
				y_data += f_stride;
				if((i & 0x01) == 0) {
					cb[(i >> 1)] = u_data;
					cr[(i >> 1)] = v_data;
					u_data += h_stride;
					v_data += h_stride;
				}
			}
		}
		jpeg_read_raw_data(&decoder_->dinfo, planes, 16);
	}
	jpeg_finish_decompress(&decoder_->dinfo);

	if(fp) {
		fclose(fp);
	}
	// 0-255 -> 16->235
	y_data = data;
	u_data = data + y_size;
	v_data = u_data + u_size;
	for(int i = 0;i < y_size;++ i) {
		(*y_data) = (uint8_t)((((*y_data) * 219 + 383) >> 8) + 16);
		++ y_data;
	}
	for(int i = 0;i < u_size;++ i) {
		(*u_data) = (uint8_t)((((*u_data) * 219 + 383) >> 8) + 16);
		(*v_data) = (uint8_t)((((*v_data) * 219 + 383) >> 8) + 16);
		++ u_data;
		++ v_data;
	}
	// setup yuv object.
	yuv = ttLibC_Yuv420_make(
			yuv,
			Yuv420Type_planar,
			decoder_->inherit_super.width,
			decoder_->inherit_super.height,
			data,
			data_size,
			data,
			decoder_->inherit_super.width,
			data + y_size,
			(decoder_->inherit_super.width >> 1),
			data + y_size + u_size,
			(decoder_->inherit_super.width >> 1),
			true,
			jpeg->inherit_super.inherit_super.pts,
			jpeg->inherit_super.inherit_super.timebase);
	if(yuv == NULL) {
		ERR_PRINT("yuv is not generated.");
		if(alloc_flag) {
			ttLibC_free(data);
		}
		return false;
	}
	yuv->inherit_super.inherit_super.is_non_copy = false;
	yuv->inherit_super.inherit_super.buffer_size = data_size;
	decoder_->yuv420 = yuv;
	if(!callback(ptr, yuv)) {
		return false;
	}
	return true;
}

/*
 * close jpeg decoder
 * @param decoder
 */
void TT_VISIBILITY_DEFAULT ttLibC_JpegDecoder_close(ttLibC_JpegDecoder **decoder) {
	ttLibC_JpegDecoder_ *target = (ttLibC_JpegDecoder_ *)*decoder;
	if(target == NULL) {
		return;
	}
	if(target->dummy_buffer != NULL) {
		ttLibC_free(target->dummy_buffer);
	}
	jpeg_destroy_decompress(&target->dinfo);
	ttLibC_Yuv420_close(&target->yuv420);
	ttLibC_free(target);
	*decoder = NULL;
}

#endif

