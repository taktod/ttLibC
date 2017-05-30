/*
 * @file   vp6.c
 * @brief  vp6 image frame information.
 *
 * this code is under 3-Cause BSD license
 *
 * @author taktod
 * @date   2015/08/06
 */

#include "vp6.h"
#include "../../_log.h"
#include "../../util/byteUtil.h"

typedef ttLibC_Frame_Video_Vp6 ttLibC_Vp6_;

/*
 * make vp6 frame
 * @param prev_frame    reuse frame
 * @param video_type    video type of vp6
 * @param width         width
 * @param height        height
 * @param data          vp6 data
 * @param data_size     vp6 data size
 * @param non_copy_mode truef:hold the data pointer. false:data will copy.
 * @param pts           pts for vp6 data.
 * @param timebase      timebase number for pts.
 */
ttLibC_Vp6 *ttLibC_Vp6_make(
		ttLibC_Vp6 *prev_frame,
		ttLibC_Video_Type video_type,
		uint32_t width,
		uint32_t height,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	return (ttLibC_Vp6 *)ttLibC_Video_make(
			(ttLibC_Video *)prev_frame,
			sizeof(ttLibC_Vp6_),
			frameType_vp6,
			video_type,
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
ttLibC_Vp6 *ttLibC_Vp6_clone(
		ttLibC_Vp6 *prev_frame,
		ttLibC_Vp6 *src_frame) {
	if(src_frame == NULL) {
		return NULL;
	}
	if(src_frame->inherit_super.inherit_super.type != frameType_vp6) {
		ERR_PRINT("try to clone non vp6 frame.");
		return NULL;
	}
	if(prev_frame != NULL && prev_frame->inherit_super.inherit_super.type != frameType_vp6) {
		ERR_PRINT("try to use non vp6 frame for reuse.");
		return NULL;
	}
	ttLibC_Vp6 *vp6 = ttLibC_Vp6_make(
			prev_frame,
			src_frame->inherit_super.type,
			src_frame->inherit_super.width,
			src_frame->inherit_super.height,
			src_frame->inherit_super.inherit_super.data,
			src_frame->inherit_super.inherit_super.buffer_size,
			false,
			src_frame->inherit_super.inherit_super.pts,
			src_frame->inherit_super.inherit_super.timebase);
	if(vp6 != NULL) {
		vp6->inherit_super.inherit_super.id = src_frame->inherit_super.inherit_super.id;
	}
	return vp6;
}

/*
 * check if the vp6 binary is key frame.
 * @param data      vp6 data
 * @param data_size vp6 data size
 * @return true: key frame false:inter frame
 */
bool ttLibC_Vp6_isKey(void *data, size_t data_size) {
	if(data_size < 1) {
		ERR_PRINT("data size is too small.");
		return false;
	}
	uint8_t *dat = data;
	return ((*dat) & 0x80) == 0;
}

/*
 * analyze the width information from vp6 binary.
 * @param prev_frame ref for prev analyzed vp6 frame.
 * @param data       vp6 data
 * @param data_size  vp6 data size
 * @param adjustment adjustment byte
 * @return 0:error or width size.
 */
uint32_t ttLibC_Vp6_getWidth(
		ttLibC_Vp6 *prev_frame,
		uint8_t *data,
		size_t data_size,
		uint8_t adjustment) {
	/*
	 * memo
	 * 1bit frameMode 0:key frame 1:inner frame
	 * 6bit qp
	 * 1bit marker
	 * 5bit version
	 * 2bit version2
	 * 1bit interlace
	 * 16bit offset(marker = 1 version2 = 0 only)
	 * 8bit dimY (x16 = height)
	 * 8bit dimX (x16 = width)
	 * 8bit ...
	 */
	ttLibC_ByteReader *reader = ttLibC_ByteReader_make(data, data_size, ByteUtilType_default);
	if(reader == NULL) {
		ERR_PRINT("failed to make byteReader.");
		return 0;
	}
	uint32_t frameMode = ttLibC_ByteReader_bit(reader, 1);
	if(frameMode == 1) {
		ttLibC_ByteReader_close(&reader);
		if(prev_frame == NULL) {
			ERR_PRINT("ref frame is missing.");
			return 0;
		}
		return prev_frame->inherit_super.width;
	}
	ttLibC_ByteReader_bit(reader, 6);
	uint32_t marker = ttLibC_ByteReader_bit(reader, 1);
	ttLibC_ByteReader_bit(reader, 5);
	uint32_t version2 = ttLibC_ByteReader_bit(reader, 2);
	ttLibC_ByteReader_bit(reader, 1);
	if(marker == 1 && version2 == 0) {
		ttLibC_ByteReader_bit(reader, 16);
	}
	ttLibC_ByteReader_bit(reader, 8);
	ttLibC_ByteReader_bit(reader, 8);
	ttLibC_ByteReader_bit(reader, 8);
	uint32_t width = ttLibC_ByteReader_bit(reader, 8) * 16;
	if(reader->error != Error_noError) {
		LOG_ERROR(reader->error);
		ttLibC_ByteReader_close(&reader);
		return 0;
	}
	ttLibC_ByteReader_close(&reader);
	width -= ((adjustment >> 4) & 0x0F);
	return width;
}

/*
 * analyze the height information from vp6 binary.
 * @param prev_frame ref for prev analyzed vp6 frame.
 * @param data       vp6 data
 * @param data_size  vp6 data size
 * @param adjustment adjustment byte
 * @return 0:error or height size.
 */
uint32_t ttLibC_Vp6_getHeight(
		ttLibC_Vp6 *prev_frame,
		uint8_t *data,
		size_t data_size,
		uint8_t adjustment) {
	ttLibC_ByteReader *reader = ttLibC_ByteReader_make(data, data_size, ByteUtilType_default);
	if(reader == NULL) {
		ERR_PRINT("failed to make byteReader.");
		return 0;
	}
	uint32_t frameMode = ttLibC_ByteReader_bit(reader, 1);
	if(frameMode == 1) {
		ttLibC_ByteReader_close(&reader);
		if(prev_frame == NULL) {
			ERR_PRINT("ref frame is missing.");
			return 0;
		}
		return prev_frame->inherit_super.height;
	}
	ttLibC_ByteReader_bit(reader, 6);
	uint32_t marker = ttLibC_ByteReader_bit(reader, 1);
	ttLibC_ByteReader_bit(reader, 5);
	uint32_t version2 = ttLibC_ByteReader_bit(reader, 2);
	ttLibC_ByteReader_bit(reader, 1);
	if(marker == 1 && version2 == 0) {
		ttLibC_ByteReader_bit(reader, 16);
	}
	ttLibC_ByteReader_bit(reader, 8);
	ttLibC_ByteReader_bit(reader, 8);
	uint32_t height = ttLibC_ByteReader_bit(reader, 8) * 16;
	ttLibC_ByteReader_bit(reader, 8);
	if(reader->error != Error_noError) {
		LOG_ERROR(reader->error);
		ttLibC_ByteReader_close(&reader);
		return 0;
	}
	ttLibC_ByteReader_close(&reader);
	height -= (adjustment & 0x0F);
	return height;
}

/*
 * make frame object from vp6 binary data.
 * @param prev_frame    ref for prev analyzed vp6 frame.
 * @param data          vp6 data
 * @param data_size     vp6 data size
 * @param non_copy_mode true:hold pointer. false:copy data.
 * @param pts           pts for vp6 frame.
 * @param timebase      timebase for pts.
 * @param adjustment    adjustment byte
 * @return vp6 frame
 */
ttLibC_Vp6 *ttLibC_Vp6_getFrame(
		ttLibC_Vp6 *prev_frame,
		uint8_t *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		uint8_t adjustment) {
	if(data_size <= 1) {
		ERR_PRINT("data size is too small for analyze.");
		return NULL;
	}
	bool is_key = ttLibC_Vp6_isKey(data, data_size);
	uint32_t width  = ttLibC_Vp6_getWidth(prev_frame, data, data_size, adjustment);
	uint32_t height = ttLibC_Vp6_getHeight(prev_frame, data, data_size, adjustment);
	if(width == 0 || height == 0) {
		return NULL;
	}
	return ttLibC_Vp6_make(
			prev_frame,
			is_key ? videoType_key : videoType_inner,
			width,
			height,
			data, data_size,
			non_copy_mode,
			pts,
			timebase);
}

/*
 * close frame
 * @param frame
 */
void ttLibC_Vp6_close(ttLibC_Vp6 **frame) {
	ttLibC_Video_close_((ttLibC_Video **)frame);
}
