/*
 * @file   jpeg.c
 * @brief  jpeg image frame information
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/11/05
 */

#include "jpeg.h"
#include "../../ttLibC_predef.h"
#include "../../_log.h"

typedef ttLibC_Frame_Video_Jpeg ttLibC_Jpeg_;

/*
 * make jpeg frame
 * @param prev_frame    reuse frame
 * @param width         width
 * @param height        height
 * @param data          jpeg data
 * @param data_size     jpeg data size
 * @param non_copy_mode true:hold the data pointer. false:data will copy.
 * @param pts           pts for jpeg data.
 * @param timebase      timebase number for pts
 */
ttLibC_Jpeg TT_VISIBILITY_DEFAULT *ttLibC_Jpeg_make(
		ttLibC_Jpeg *prev_frame,
		uint32_t width,
		uint32_t height,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	return (ttLibC_Jpeg *)ttLibC_Video_make(
			(ttLibC_Video *)prev_frame,
			sizeof(ttLibC_Jpeg_),
			frameType_jpeg,
			videoType_key,
			width,
			height,
			data,
			data_size,
			non_copy_mode,
			pts,
			timebase);
}

/**
 * make clone frame.
 * always make copy buffer on it.
 * @param prev_frame reuse frame object.
 * @param src_frame  source of clone.
 */
ttLibC_Jpeg TT_VISIBILITY_DEFAULT *ttLibC_Jpeg_clone(
		ttLibC_Jpeg *prev_frame,
		ttLibC_Jpeg *src_frame) {
	if(src_frame == NULL) {
		return NULL;
	}
	if(src_frame->inherit_super.inherit_super.type != frameType_jpeg) {
		ERR_PRINT("try to clone non jpeg frame.");
		return NULL;
	}
	if(prev_frame != NULL && prev_frame->inherit_super.inherit_super.type != frameType_jpeg) {
		ERR_PRINT("try to use non jpeg frame for reuse.");
		return NULL;
	}
	ttLibC_Jpeg *jpeg = ttLibC_Jpeg_make(
			prev_frame,
			src_frame->inherit_super.width,
			src_frame->inherit_super.height,
			src_frame->inherit_super.inherit_super.data,
			src_frame->inherit_super.inherit_super.buffer_size,
			false,
			src_frame->inherit_super.inherit_super.pts,
			src_frame->inherit_super.inherit_super.timebase);
	if(jpeg != NULL) {
		jpeg->inherit_super.inherit_super.id = src_frame->inherit_super.inherit_super.id;
	}
	return jpeg;
}

/**
 * make frame object from jpeg binary data.
 */
ttLibC_Jpeg TT_VISIBILITY_DEFAULT *ttLibC_Jpeg_getFrame(
		ttLibC_Jpeg *prev_frame,
		uint8_t *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	uint8_t *buf = data;
	size_t buf_size = data_size;
	if(*buf != 0xFF || *(buf + 1) != 0xD8) {
		ERR_PRINT("binary is not jpeg.");
		return NULL;
	}
	bool is_find_size = false;
	uint32_t width = 0;
	uint32_t height = 0;
	buf += 2;
	buf_size -= 2;
	do {
		// check tag.
		if(*buf != 0xFF) {
			break;
		}
		uint32_t tag = (*buf << 8) | *(buf + 1);
		buf += 2;
		buf_size -= 2;
		uint32_t size = (*buf << 8) | *(buf + 1);
		switch(tag) {
		case 0xFFC0:
		case 0xFFC2:
			is_find_size = true;
			height = (*(buf + 3) << 8) | *(buf + 4);
			width = (*(buf + 5) << 8) | *(buf + 6);
			buf += size;
			buf_size -= size;
			break;
		default:
			// skip this tag.
			buf += size;
			buf_size -= size;
			break;
		}
		if(is_find_size) {
			break;
		}
	} while(buf_size > 0);

	return ttLibC_Jpeg_make(
			prev_frame,
			width,
			height,
			data,
			data_size,
			non_copy_mode,
			pts,
			timebase);
}


/*
 * close frame
 * @param frame
 */
void TT_VISIBILITY_DEFAULT ttLibC_Jpeg_close(ttLibC_Jpeg **frame) {
	ttLibC_Video_close_((ttLibC_Video **)frame);
}
