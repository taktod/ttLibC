/**
 * @file   pngDecoder.c
 * @brief  decode png with libpng.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2018/11/25
 */

#ifdef __ENABLE_LIBPNG__
#include "pngDecoder.h"
#include "../ttLibC_predef.h"
#include "../allocator.h"
#include "../_log.h"
#include <setjmp.h>
#include <png.h>
#include <string.h>

typedef struct ttLibC_Decoder_PngDecoder_ {
  ttLibC_PngDecoder inherit_super;
  uint8_t    *buffer;
  size_t      buffer_size;
  uint32_t    offset;
  bool        failed_flag;
  ttLibC_Bgr *bgr;
} ttLibC_Decoder_PngDecoder_;

typedef ttLibC_Decoder_PngDecoder_ ttLibC_PngDecoder_;

static void PngDecoder_memoryReadFunc(
    png_structp png_ptr,
    png_bytep buf,
    png_size_t size) {
  ttLibC_PngDecoder_ *decoder = (ttLibC_PngDecoder_ *)png_get_io_ptr(png_ptr);
  if(decoder->buffer_size - decoder->offset > size) {
    memcpy(buf, decoder->buffer + decoder->offset, size);
    decoder->offset += size;
  }
  else {
    decoder->failed_flag = true;
    ERR_PRINT("failed to read data.");
  }
}

ttLibC_PngDecoder *ttLibC_PngDecoder_make() {
  ttLibC_PngDecoder_ *decoder = ttLibC_malloc(sizeof(ttLibC_PngDecoder_));
  if(decoder == NULL) {
    ERR_PRINT("failed to allocate memory for decoder.");
    return NULL;
  }

  decoder->offset = 0;
  decoder->buffer = NULL;
  decoder->buffer_size = 0;
  decoder->failed_flag = false;
  decoder->bgr = NULL;
  return (ttLibC_PngDecoder *)decoder;
}

bool ttLibC_PngDecoder_decode(
		ttLibC_PngDecoder *decoder,
		ttLibC_Png *png,
		ttLibC_PngDecodeFunc callback,
		void *ptr) {
  ttLibC_PngDecoder_ *decoder_ = (ttLibC_PngDecoder_ *)decoder;
  if(decoder == NULL) {
    return false;
  }
  if(png == NULL) {
    return true;
  }
  // now we can work.
  decoder_->buffer      = png->inherit_super.inherit_super.data;
  decoder_->buffer_size = png->inherit_super.inherit_super.buffer_size;
  decoder_->offset      = 0;
  decoder_->failed_flag = false;

  png_structp png_ptr;
  png_infop   info_ptr;
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if(png_ptr == NULL) {
    ERR_PRINT("failed to make new png structp");
    return false;
  }
  info_ptr = png_create_info_struct(png_ptr);
  if(info_ptr == NULL) {
    ERR_PRINT("failed to make new png infop");
		png_destroy_read_struct(&png_ptr, NULL, NULL);
    return false;
  }
  png_set_read_fn(png_ptr, (png_voidp)decoder_, (png_rw_ptr)PngDecoder_memoryReadFunc);

  png_uint_32 width, height;
  int bit_depth, color_type, interlace_type;
  int compression_type, filter_type;

  png_read_info(png_ptr, info_ptr);
  png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_type);

  if(bit_depth != 8) {
    ERR_PRINT("bitdepth is not valid for ttLibC:%d", bit_depth);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return false;
  }
  ttLibC_Bgr_Type bgrType;
  switch(color_type) {
  case PNG_COLOR_TYPE_RGB:
    bgrType = BgrType_rgb;
    break;
  case PNG_COLOR_TYPE_RGBA:
  case PNG_COLOR_TYPE_PALETTE:
    bgrType = BgrType_rgba;
    break;
  default:
    {
      ERR_PRINT("color type is not valid for ttLibC:%d", color_type);
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
      return false;
    }
  }

	uint32_t row_size = png_get_rowbytes(png_ptr, info_ptr);
  // now decode is complete... try to get data.
  uint32_t width_stride = row_size;
  if(color_type == PNG_COLOR_TYPE_PALETTE) {
    width_stride = row_size * 4;
  }
  ttLibC_Bgr *bgr = ttLibC_Bgr_makeEmptyFrame2(
      decoder_->bgr,
      bgrType,
      width,
      height);
  if(bgr == NULL) {
    ERR_PRINT("failed to make new bgr frame.");
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return false;
  }
  decoder_->bgr = bgr;
  if(color_type == PNG_COLOR_TYPE_PALETTE) {
    // check transalpha
    png_bytep trans_alpha = NULL;
    int num_trans = 0; 
    png_color_16p trans_color = NULL;

    png_get_tRNS(png_ptr, info_ptr, &trans_alpha, &num_trans, &trans_color);

    // get color palette;
    int color_num;
    png_colorp palette_ptr;
    uint8_t *row_data = ttLibC_malloc(row_size);
    png_get_PLTE(png_ptr, info_ptr, &palette_ptr, &color_num);
    uint8_t *dst = bgr->data;
    for(uint32_t i = 0;i < height;++ i) {
      uint8_t *d = dst;
      png_read_row(png_ptr, row_data, NULL);
      for(uint32_t j = 0;j < row_size;++ j) {
        png_colorp colorp = &palette_ptr[row_data[j]];
        *d = colorp->red;
        *(d+1) = colorp->green;
        *(d+2) = colorp->blue;
        if(num_trans == 0) {
          *(d+3) = 255;
        }
        else {
          *(d+3) = trans_alpha[row_data[j]];
        }
        d += bgr->unit_size;
      }
      dst += bgr->width_stride;
    }
    ttLibC_free(row_data);
  }
  else {
    uint8_t *dst = bgr->data;
    for(uint32_t i = 0;i < height;++ i) {
      png_read_row(png_ptr, dst, NULL);
      dst += bgr->width_stride;
    }
  }
  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
  if(callback != NULL) {
    if(!callback(ptr, decoder_->bgr)) {
      return false;
    }
  }
  return true;
}

void ttLibC_PngDecoder_close(ttLibC_PngDecoder **decoder) {
  ttLibC_PngDecoder_ *target = (ttLibC_PngDecoder_ *)*decoder;
  if(target == NULL) {
    return;
  }
  ttLibC_Bgr_close(&target->bgr);
  ttLibC_free(target);
  *decoder = NULL;
}

#endif
