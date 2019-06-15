/*
 * @file   png.c
 * @brief  png image frame information
 * 
 * this code is under 3-Cause BSD license.
 * 
 * @author taktod
 * @date   2018/07/24
 */

#include "png.h"
#include "../../ttLibC_predef.h"
#include "../../_log.h"

typedef ttLibC_Frame_Video_Png ttLibC_Png_;

ttLibC_Png TT_VISIBILITY_DEFAULT *ttLibC_Png_make(
    ttLibC_Png *prev_frame,
    uint32_t width,
    uint32_t height,
    void *data,
    size_t data_size,
    bool non_copy_mode,
    uint64_t pts,
    uint32_t timebase) {
  return (ttLibC_Png *)ttLibC_Video_make(
			(ttLibC_Video *)prev_frame,
			sizeof(ttLibC_Png_),
			frameType_png,
			videoType_key,
			width,
			height,
			data,
			data_size,
			non_copy_mode,
			pts,
			timebase);
}

ttLibC_Png TT_VISIBILITY_DEFAULT *ttLibC_Png_clone(
    ttLibC_Png *prev_frame,
    ttLibC_Png *src_frame) {
  if(src_frame == NULL) {
    return NULL;
  }
  if(src_frame->inherit_super.inherit_super.type != frameType_png) {
    ERR_PRINT("try to clone non png frame.");
    return NULL;
  }
  if(prev_frame != NULL && prev_frame->inherit_super.inherit_super.type != frameType_png) {
    ERR_PRINT("try to use non png frame for reuse.");
  }
  ttLibC_Png *png = ttLibC_Png_make(
      prev_frame,
      src_frame->inherit_super.width,
      src_frame->inherit_super.height,
      src_frame->inherit_super.inherit_super.data,
      src_frame->inherit_super.inherit_super.buffer_size,
      false,
      src_frame->inherit_super.inherit_super.pts,
      src_frame->inherit_super.inherit_super.timebase);
  if(png != NULL) {
    png->inherit_super.inherit_super.id = src_frame->inherit_super.inherit_super.id;
  }
  return png;
}

ttLibC_Png TT_VISIBILITY_DEFAULT *ttLibC_Png_getFrame(
    ttLibC_Png *prev_frame,
    uint8_t *data,
    size_t data_size,
    bool non_copy_mode,
    uint64_t pts,
    uint32_t timebase) {
  uint8_t *buf = data;
  size_t buf_size = data_size;
  // header 89 50 4E 47 0D 0A 1A 0A
  if(buf_size < 8
  || *buf != 0x89
  || *(buf + 1) != 0x50
  || *(buf + 2) != 0x4E
  || *(buf + 3) != 0x47
  || *(buf + 4) != 0x0D
  || *(buf + 5) != 0x0A
  || *(buf + 6) != 0x1A
  || *(buf + 7) != 0x0A) {
    ERR_PRINT("binary is not png");
    return NULL;
  }
//  bool is_find_size = false;
  uint32_t width = 0;
  uint32_t height = 0;
  // あとは、縦横のサイズを取得したい。
  buf += 8;
  buf_size -= 8;
  do {
    // need chunksize and type (4byte each)
    if(buf_size <= 8) {
      ERR_PRINT("failed to get size information.");
      return NULL;
    }
    uint32_t chunk_size = (*buf << 24) | (*(buf + 1) << 16) | (*(buf + 2) << 8) | *(buf + 3);
    buf += 4;
    buf_size -= 4;
    uint32_t tag = (*buf << 24) | (*(buf + 1) << 16) | (*(buf + 2) << 8) | *(buf + 3);
    buf += 4;
    buf_size -= 4;
    if(buf_size < chunk_size) {
      ERR_PRINT("png data is missing. broken?");
      return NULL;
    }
    // I want IHDR
    if(chunk_size == 0x0D) {
      if(tag == 0x49484452) { // IHDR
        width = (*buf << 24) | (*(buf + 1) << 16) | (*(buf + 2) << 8) | *(buf + 3);
        height = (*(buf + 4) << 24) | (*(buf + 5) << 16) | (*(buf + 6) << 8) | *(buf + 7);
        break;
      }
    }
    buf += chunk_size;
    buf_size -= chunk_size;
  } while(buf_size > 0);
  if(width == 0 || height == 0) {
    ERR_PRINT("failed to get width height information.");
    return NULL;
  }

  return ttLibC_Png_make(
    prev_frame,
    width,
    height,
    data,
    data_size,
    non_copy_mode,
    pts,
    timebase);
}

void TT_VISIBILITY_DEFAULT ttLibC_Png_close(ttLibC_Png **frame) {
  ttLibC_Video_close_((ttLibC_Video **)frame);
} 