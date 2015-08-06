/*
 * @file   vp8.c
 * @brief  vp8 image frame information
 *
 * this code is under 3-Cause BSD license
 *
 * @author taktod
 * @date   2015/07/31
 */

#include "vp8.h"
#include "../../log.h"

typedef ttLibC_Frame_Video_Vp8 ttLibC_Vp8_;

/*
 * make vp8 frame
 * @param prev_frame    reuse frame
 * @param video_type    video type of vp8
 * @param width         width
 * @param height        height
 * @param data          vp8 data
 * @param data_size     vp8 data size
 * @param non_copy_mode true:hold the data pointer. false:data will copy.
 * @param pts           pts for vp8 data.
 * @param timebase      timebase number for pts.
 */
ttLibC_Vp8 *ttLibC_Vp8_make(
		ttLibC_Vp8 *prev_frame,
		ttLibC_Video_Type video_type,
		uint32_t width,
		uint32_t height,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	return ttLibC_Video_make(prev_frame, sizeof(ttLibC_Vp8_), frameType_vp8, video_type, width, height, data, data_size, non_copy_mode, pts, timebase);
}

/*
 * check if the vp8 binary is key frame.
 * @param data      vp8 data
 * @param data_size vp8 data size
 * @return true: key frame false:inter frame
 */
bool ttLibC_Vp8_isKey(void *data, size_t data_size) {
	if(data == NULL) {
		return false;
	}
	uint8_t *dat = (uint8_t *)data;
	if(data_size <= 0x0A) {
		ERR_PRINT("data size is too small for analyze.");
		return false;
	}
	return (dat[3] == 0x9D && dat[4] == 0x01 && dat[5] == 0x2A);
}

/*
 * analyze the width information from vp8 binary.
 * @param prev_frame ref for prev analyzed vp8 frame.
 * @param data       vp8 data
 * @param data_size  vp8 data size
 * @return 0:error or width size.
 */
uint32_t ttLibC_Vp8_getWidth(ttLibC_Vp8 *prev_frame, uint8_t *data, size_t data_size) {
	if(data == NULL) {
		return false;
	}
	uint8_t *dat = (uint8_t *)data;
	if(data_size <= 0x0A) {
		ERR_PRINT("data size is too small for analyze.");
		return 0;
	}
	if(atsfLib_Vp8_isKey(data, data_size)) {
		return ((dat[7] & 0x3F) << 8) | dat[6];
	}
	else {
		if(prev_frame == NULL) {
			ERR_PRINT("prev frame is missing.");
			return 0;
		}
		return prev_frame->inherit_super.width;
	}
}

/*
 * analyze the height information from vp8 binary.
 * @param prev_frame ref for prev analyzed vp8 frame.
 * @param data       vp8 data
 * @param data_size  vp8 data size
 * @return 0:error or height size.
 */
uint32_t ttLibC_Vp8_getHeight(ttLibC_Vp8 *prev_frame, uint8_t *data, size_t data_size) {
	if(data == NULL) {
		return false;
	}
	uint8_t *dat = (uint8_t *)data;
	if(data_size <= 0x0A) {
		ERR_PRINT("data size is too small for analyze.");
		return 0;
	}
	if(atsfLib_Vp8_isKey(data, data_size)) {
		return ((dat[9] & 0x3F) << 8) | dat[8];
	}
	else {
		if(prev_frame == NULL) {
			ERR_PRINT("prev frame is missing.");
			return 0;
		}
		return prev_frame->inherit_super.height;
 	}
}

/*
 * make frame object from vp8 binary data.
 * @param prev_frame ref for prev analyzed vp8 frame.
 * @param data          vp8 data
 * @param data_size     vp8 data size
 * @param non_copy_mode true:hold pointer. false:copy data.
 * @param pts           pts for vp8 frame.
 * @param timebase      timebase for pts.
 * @return vp8 frame
 */
ttLibC_Vp8 *ttLibC_Vp8_getFrame(ttLibC_Vp8 *prev_frame, uint8_t *data, size_t data_size, bool non_copy_mode, uint64_t pts, uint32_t timebase) {
	if(data_size <= 0x0A) {
		ERR_PRINT("data size is too small for analyze.");
		return NULL;
	}
	bool isKey = ttLibC_Vp8_isKey(data, data_size);
	uint32_t width  = ttLibC_Vp8_getWidth(prev_frame, data, data_size);
	uint32_t height = ttLibC_Vp8_getHeight(prev_frame, data, data_size);
	return ttLibC_Vp8_make(prev_frame, isKey ? videoType_key : videoType_inter, width, height, data, data_size, non_copy_mode, pts, timebase);
}

/*
 * close frame
 * @param frame
 */
void ttLibC_Vp8_close(ttLibC_Vp8 **frame) {
	ttLibC_Video_close_((ttLibC_Video **)frame);
}
