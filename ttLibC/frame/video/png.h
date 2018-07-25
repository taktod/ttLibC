/**
 * @file   png.h
 * @brief  png image frame information
 * 
 * this code is under 3-Cause BSD license
 * 
 * @author taktod
 * @date   2018/07/24
 */

#ifndef TTLIBC_FRAME_VIDEO_PNG_H_
#define TTLIBC_FRAME_VIDEO_PNG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "video.h"

typedef struct ttLibC_Frame_Video_Png {
  ttLibC_Video inherit_super;
} ttLibC_Frame_Video_Png;

typedef ttLibC_Frame_Video_Png ttLibC_Png;

/**
 * make png frame
 * @param prev_frame    reuse frame
 * @param width         width
 * @param height        height
 * @param data          png data
 * @param data_size     png data size
 * @param non_copy_mode true:hold the data pointer. false:data will copy.
 * @param pts           pts for png data.
 * @param timebase      timebase number for pts
 */
ttLibC_Png *ttLibC_Png_make(
    ttLibC_Png *prev_frame,
    uint32_t width,
    uint32_t height,
    void *data,
    size_t data_size,
    bool non_copy_mode,
    uint64_t pts,
    uint32_t timebase);

/**
 * make clone frame.
 * always make copy buffer on it.
 * @param prev_frame reuse frame object.
 * @param src_frame  source of clone.
 */
ttLibC_Png *ttLibC_Png_clone(
    ttLibC_Png *prev_frame,
    ttLibC_Png *src_frame);

/**
 * make frame object from png binary data.
 */
ttLibC_Png *ttLibC_Png_getFrame(
    ttLibC_Png *prev_frame,
    uint8_t *data,
    size_t data_size,
    bool non_copy_mode,
    uint64_t pts,
    uint32_t timebase);

/**
 * close frame
 * @param frame
 */
void ttLibC_Png_close(ttLibC_Png **frame);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif