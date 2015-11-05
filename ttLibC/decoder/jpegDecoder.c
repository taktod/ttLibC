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
#include "../log.h"
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
ttLibC_JpegDecoder *ttLibC_JpegDecoder_make() {
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
 */
void ttLibC_JpegDecoder_decode(
		ttLibC_JpegDecoder *decoder,
		ttLibC_Jpeg *jpeg,
		ttLibC_JpegDecodeFunc callback,
		void *ptr) {
	ttLibC_JpegDecoder_ *decoder_ = (ttLibC_JpegDecoder_ *)decoder;
	// set memory source.
	jpeg_mem_src(&decoder_->dinfo, (unsigned char *)jpeg->inherit_super.inherit_super.data, jpeg->inherit_super.inherit_super.buffer_size);

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
	size_t wh = decoder_->inherit_super.width * decoder_->inherit_super.height;
	size_t data_size = wh + (wh >> 1);
	bool is_alloc_flg = false;
	// check the size is multiple of 16 or not.
	if(decoder_->inherit_super.height % 16 > 0) {
		int add_size = ((16 - (decoder_->inherit_super.height % 16)) * decoder_->inherit_super.height) * 3 / 2;
		if(decoder_->dummy_buffer_size < add_size) {
			if(decoder_->dummy_buffer != NULL) {
				ttLibC_free(decoder_->dummy_buffer);
			}
			decoder_->dummy_buffer_size = add_size;
			decoder_->dummy_buffer = ttLibC_malloc(add_size);
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
		is_alloc_flg = true;
	}

	uint8_t *y_data = data;
	uint8_t *u_data = data + wh;
	uint8_t *v_data = u_data + (wh >> 2);
	for(int j = 0;j < jpeg->inherit_super.height;j += 16) {
		int max = decoder_->inherit_super.height - j;
		if(max >= 16) {
			max = 16;
		}
		for(int i = 0;i < 16;i ++) {
			if(i >= max) {
				y[i] = decoder_->dummy_buffer + (i - max) * decoder_->inherit_super.width;
				if((i & 0x01) == 0) {
					cb[(i >> 1)] = decoder_->dummy_buffer + (16 - max) * decoder_->inherit_super.width + (i - max) * decoder_->inherit_super.width / 4;
					cr[(i >> 1)] = decoder_->dummy_buffer + (16 - max) * decoder_->inherit_super.width * 5 / 4 + (i - max) * decoder_->inherit_super.width / 4;
				}
			}
			else {
				y[i] = y_data;
				y_data += decoder_->inherit_super.width;
				if((i & 0x01) == 0) {
					cb[(i >> 1)] = u_data;
					cr[(i >> 1)] = v_data;
					u_data += (decoder_->inherit_super.width >> 1);
					v_data += (decoder_->inherit_super.width >> 1);
				}
			}
		}
		jpeg_read_raw_data(&decoder_->dinfo, planes, 16);
	}
	jpeg_finish_decompress(&decoder_->dinfo);

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
			data + wh,
			(decoder_->inherit_super.width >> 1),
			data + wh + (wh >> 2),
			(decoder_->inherit_super.width >> 1),
			true,
			jpeg->inherit_super.inherit_super.pts,
			jpeg->inherit_super.inherit_super.timebase);
	if(yuv == NULL) {
		ERR_PRINT("yuv is not generated.");
		return;
	}
	yuv->inherit_super.inherit_super.is_non_copy = false;
	yuv->inherit_super.inherit_super.buffer_size = wh + (wh >> 1);
	decoder_->yuv420 = yuv;
	callback(ptr, yuv);
}

/*
 * close jpeg decoder
 * @param decoder
 */
void ttLibC_JpegDecoder_close(ttLibC_JpegDecoder **decoder) {
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

