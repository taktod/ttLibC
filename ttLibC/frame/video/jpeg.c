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
#include "../../log.h"

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
ttLibC_Jpeg *ttLibC_Jpeg_make(
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
 * make frame object from jpeg binary data.
 */
ttLibC_Jpeg *ttLibC_Jpeg_getFrame(
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
		uint32_t tag = (*buf << 8) | *(buf + 1);
		buf += 2;
		buf_size -= 2;
		uint32_t size = (*buf << 8) | *(buf + 1);
		switch(tag) {
		case 0xFFC0:
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
	if(width == 0 || height == 0) {
		return NULL;
	}

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
void ttLibC_Jpeg_close(ttLibC_Jpeg **frame) {
	ttLibC_Video_close_((ttLibC_Video **)frame);
}
