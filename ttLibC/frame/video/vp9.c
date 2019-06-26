/*
 * @file   vp9.c
 * @brief  vp9 image frame information.
 *
 * this code is under 3-Cause BSD license
 *
 * @author taktod
 * @date   2015/08/06
 */

#include "vp9.h"
#include "../../_log.h"
#include "../../util/byteUtil.h"

typedef ttLibC_Frame_Video_Vp9 ttLibC_Vp9_;

/*
 * make vp9 frame
 * @param prev_frame    reuse frame
 * @param video_type    video type of vp9
 * @param width         width
 * @param height        height
 * @param data          vp9 data
 * @param data_size     vp9 data size
 * @param non_copy_mode true:hold the data pointer. false:data will copy.
 * @param pts           pts for vp9 data.
 * @param timebase      timebase number for pts.
 */
ttLibC_Vp9 TT_ATTRIBUTE_API *ttLibC_Vp9_make(
		ttLibC_Vp9 *prev_frame,
		ttLibC_Video_Type video_type,
		uint32_t width,
		uint32_t height,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	return (ttLibC_Vp9 *)ttLibC_Video_make(
			(ttLibC_Video *)prev_frame,
			sizeof(ttLibC_Vp9_),
			frameType_vp9,
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
ttLibC_Vp9 TT_ATTRIBUTE_API *ttLibC_Vp9_clone(
		ttLibC_Vp9 *prev_frame,
		ttLibC_Vp9 *src_frame) {
	if(src_frame == NULL) {
		return NULL;
	}
	if(src_frame->inherit_super.inherit_super.type != frameType_vp9) {
		ERR_PRINT("try to clone non vp9 frame.");
		return NULL;
	}
	if(prev_frame != NULL && prev_frame->inherit_super.inherit_super.type != frameType_vp9) {
		ERR_PRINT("try to use non vp9 frame for reuse.");
		return NULL;
	}
	ttLibC_Vp9 *vp9 = ttLibC_Vp9_make(
			prev_frame,
			src_frame->inherit_super.type,
			src_frame->inherit_super.width,
			src_frame->inherit_super.height,
			src_frame->inherit_super.inherit_super.data,
			src_frame->inherit_super.inherit_super.buffer_size,
			false,
			src_frame->inherit_super.inherit_super.pts,
			src_frame->inherit_super.inherit_super.timebase);
	if(vp9 != NULL) {
		vp9->inherit_super.inherit_super.id = src_frame->inherit_super.inherit_super.id;
	}
	return vp9;
}

/*
 * check if the vp9 binary is key frame.
 * @param data      vp9 data
 * @param data_size vp9 data size
 * @return true: key frame false:inter frame
 */
bool TT_ATTRIBUTE_API ttLibC_Vp9_isKey(void *data, size_t data_size) {
	ttLibC_ByteReader *reader = ttLibC_ByteReader_make(data, data_size, ByteUtilType_default);
	ttLibC_ByteReader_bit(reader, 2);
	ttLibC_ByteReader_bit(reader, 1);
	ttLibC_ByteReader_bit(reader, 1);
	uint32_t ref_flag = ttLibC_ByteReader_bit(reader, 1);
	if(ref_flag == 1) {
		ERR_PRINT("ref func is not implemented yet.");
		ttLibC_ByteReader_bit(reader, 3);
	}
	uint32_t key_frame_flag = ttLibC_ByteReader_bit(reader, 1);
	ttLibC_ByteReader_close(&reader);
	return (key_frame_flag == 0);
}

/*
 * analyze the width information from vp9 binary.
 * @param prev_frame ref for prev analyzed vp9 frame.
 * @param data       vp9 data
 * @param data_size  vp9 data size
 * @return 0:error or width size.
 */
uint32_t TT_ATTRIBUTE_API ttLibC_Vp9_getWidth(ttLibC_Vp9 *prev_frame, uint8_t *data, size_t data_size) {
	/*
	 * 2bit frame marker
	 * 1bit profile
	 * 1bit reservedbit
	 * 1bit ref flag
//	 * 3bit ref       what's this?
	 * 1bit keyFrame flag
	 * 1bit invisible flag
	 * 1bit error res
	 * 24bit start code 0x49 0x83 0x42
	 * 3bit color space
	 * 1bit full range
	 * 16bit width - 1
	 * 16bit height - 1
	 */
	ttLibC_ByteReader *reader = ttLibC_ByteReader_make(data, data_size, ByteUtilType_default);
	if(reader == NULL) {
		ERR_PRINT("failed to make byteReader.");
		return 0;
	}
	ttLibC_ByteReader_bit(reader, 2);
	ttLibC_ByteReader_bit(reader, 1);
	ttLibC_ByteReader_bit(reader, 1);
	uint32_t ref_flag = ttLibC_ByteReader_bit(reader, 1);
	if(ref_flag == 1) {
		ERR_PRINT("ref func is not implemented yet.");
		ttLibC_ByteReader_bit(reader, 3);
	}
	uint32_t key_frame_flag = ttLibC_ByteReader_bit(reader, 1);
	ttLibC_ByteReader_bit(reader, 1);
	ttLibC_ByteReader_bit(reader, 1);
	if(key_frame_flag != 0) {
		// not key frame, use prev_frame in order to ref width
		ttLibC_ByteReader_close(&reader);
		if(prev_frame == NULL) {
			ERR_PRINT("ref frame is missing.");
			return 0;
		}
		return prev_frame->inherit_super.width;
	}
	uint32_t startCode1 = ttLibC_ByteReader_bit(reader, 8);
	uint32_t startCode2 = ttLibC_ByteReader_bit(reader, 8);
	uint32_t startCode3 = ttLibC_ByteReader_bit(reader, 8);
	if(startCode1 != 0x49
	|| startCode2 != 0x83
	|| startCode3 != 0x42) {
		ERR_PRINT("invalid start code for keyframe.");
		ttLibC_ByteReader_close(&reader);
		return 0;
	}
	ttLibC_ByteReader_bit(reader, 3);
	ttLibC_ByteReader_bit(reader, 1);
	uint32_t width_minus_1  = ttLibC_ByteReader_bit(reader, 16);
/*	uint32_t height_minus_1 = ttLibC_ByteReader_bit(reader, 16);*/
	if(reader->error != Error_noError) {
		LOG_ERROR(reader->error);
		ttLibC_ByteReader_close(&reader);
		return 0;
	}
	ttLibC_ByteReader_close(&reader);
	return width_minus_1 + 1;
}

/*
 * analyze the height information from vp9 binary.
 * @param prev_frame ref for prev analyzed vp9 frame.
 * @param data       vp9 data
 * @param data_size  vp9 data size
 * @return 0:error or height size.
 */
uint32_t TT_ATTRIBUTE_API ttLibC_Vp9_getHeight(ttLibC_Vp9 *prev_frame, uint8_t *data, size_t data_size) {
	ttLibC_ByteReader *reader = ttLibC_ByteReader_make(data, data_size, ByteUtilType_default);
	if(reader == NULL) {
		ERR_PRINT("failed to make byteReader.");
		return 0;
	}
	ttLibC_ByteReader_bit(reader, 2);
	ttLibC_ByteReader_bit(reader, 1);
	ttLibC_ByteReader_bit(reader, 1);
	uint32_t ref_flag = ttLibC_ByteReader_bit(reader, 1);
	if(ref_flag == 1) {
		ERR_PRINT("ref func is not implemented yet.");
		ttLibC_ByteReader_bit(reader, 3);
	}
	uint32_t key_frame_flag = ttLibC_ByteReader_bit(reader, 1);
	ttLibC_ByteReader_bit(reader, 1);
	ttLibC_ByteReader_bit(reader, 1);
	if(key_frame_flag != 0) {
		// not key frame, use prev_frame in order to ref width
		ttLibC_ByteReader_close(&reader);
		if(prev_frame == NULL) {
			ERR_PRINT("ref frame is missing.");
			return 0;
		}
		return prev_frame->inherit_super.height;
	}
	uint32_t startCode1 = ttLibC_ByteReader_bit(reader, 8);
	uint32_t startCode2 = ttLibC_ByteReader_bit(reader, 8);
	uint32_t startCode3 = ttLibC_ByteReader_bit(reader, 8);
	if(startCode1 != 0x49
	|| startCode2 != 0x83
	|| startCode3 != 0x42) {
		ERR_PRINT("invalid start code for keyframe.");
		ttLibC_ByteReader_close(&reader);
		return 0;
	}
	ttLibC_ByteReader_bit(reader, 3);
	ttLibC_ByteReader_bit(reader, 1);
	/*uint32_t width_minus_1  = */ttLibC_ByteReader_bit(reader, 16);
	uint32_t height_minus_1 = ttLibC_ByteReader_bit(reader, 16);
	if(reader->error != Error_noError) {
		LOG_ERROR(reader->error);
		ttLibC_ByteReader_close(&reader);
		return 0;
	}
	ttLibC_ByteReader_close(&reader);
	return height_minus_1 + 1;
}

/*
 * make frame object from vp9 binary data.
 * @param prev_frame    ref for prev analyzed vp9 frame.
 * @param data          vp9 data
 * @param data_size     vp9 data size
 * @param non_copy_mode true:hold pointer. false:copy data.
 * @param pts           pts for vp9 frame.
 * @param timebase      timebase for pts.
 * @return vp9 frame
 */
ttLibC_Vp9 TT_ATTRIBUTE_API *ttLibC_Vp9_getFrame(
		ttLibC_Vp9 *prev_frame,
		uint8_t *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	if(data_size < 2) {
		ERR_PRINT("data size is too small for analyze.");
		return NULL;
	}
	bool isKey = ttLibC_Vp9_isKey(data, data_size);
	uint32_t width = ttLibC_Vp9_getWidth(prev_frame, data, data_size);
	uint32_t height = ttLibC_Vp9_getHeight(prev_frame, data, data_size);
	if(width == 0 || height == 0) {
		return NULL;
	}
	return ttLibC_Vp9_make(
			prev_frame,
			isKey ? videoType_key : videoType_inner,
			width,
			height,
			data, data_size,
			non_copy_mode,
			pts,
			timebase);
}

/**
 * close frame
 * @param frame
 */
void TT_ATTRIBUTE_API ttLibC_Vp9_close(ttLibC_Vp9 **frame) {
	ttLibC_Video_close_((ttLibC_Video **)frame);
}
