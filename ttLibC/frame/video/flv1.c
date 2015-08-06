/*
 * @file   flv1.c
 * @brief  flv1 image frame information
 *
 * this code is under 3-Cause BSD license
 *
 * @author taktod
 * @date   2015/08/06
 */

#include "flv1.h"
#include "../../log.h"

#include "../../util/bitUtil.h"

typedef ttLibC_Frame_Video_Flv1 ttLibC_Flv1_;

/**
 * make flv1 frame
 * @param prev_frame    reuse frame
 * @param video_type    video type of flv1
 * @param width         width
 * @param height        height
 * @param data          flv1 data
 * @param data_size     flv1 data size
 * @param non_copy_mode true:hold the data pointer. false:data will copy.
 * @param pts           pts for flv1 data.
 * @param timebase      timebase number for pts.
 */
ttLibC_Flv1 *ttLibC_Flv1_make(
		ttLibC_Flv1 *prev_frame,
		ttLibC_Video_Type video_type,
		uint32_t width,
		uint32_t height,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	return (ttLibC_Flv1 *)ttLibC_Video_make(
			(ttLibC_Video *)prev_frame,
			sizeof(ttLibC_Flv1_),
			frameType_flv1,
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
 * check if the flv1 binary is key frame.
 * @param data      flv1 data
 * @param data_size flv1 data size
 * @return true: key frame false:inter frame
 */
bool ttLibC_Flv1_isKey(void *data, size_t data_size) {
/*
 * 17bit picture Start code 0000 0000 0000 0000 1
 * 5bit version
 * 8bit temporal Reference
 * 3bit picture size :0 custom 8bit 8bit
 *                    1 custom 16bit 16bit
 *                    2 352 x 288
 *                    3 176 x 144
 *                    4 128 x 96
 *                    5 320 x 240
 *                    6 160 x 120
 *                    7 reserved.
 * 2bit picture Type :0 intra frame(keyframe)
 *                    1 inner frame
 *                    2 disposable inner frame
 * 1bit deblocking flag
 * 5bit quantizer
 * 1bit extra information flag -> 8bit extra information
 */
	ttLibC_BitReader *reader = ttLibC_BitReader_make(data, data_size, BitReaderType_default);
	if(ttLibC_BitReader_bit(reader, 17) != 1) {
		ERR_PRINT("invalid flv1 data.");
		ttLibC_BitReader_close(&reader);
		return false;
	}
	ttLibC_BitReader_bit(reader, 5);
	ttLibC_BitReader_bit(reader, 8);
	uint32_t picture_size = ttLibC_BitReader_bit(reader, 3);
	switch(picture_size) {
	case 0:
		ttLibC_BitReader_bit(reader, 8);
		ttLibC_BitReader_bit(reader, 8);
		break;
	case 1:
		ttLibC_BitReader_bit(reader, 16);
		ttLibC_BitReader_bit(reader, 16);
		break;
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
		break;
	default:
	case 7:
		ERR_PRINT("picture type = 7 is reserved.");
		ttLibC_BitReader_close(&reader);
		return false;
	}
	uint32_t picture_type = ttLibC_BitReader_bit(reader, 2);
	ttLibC_BitReader_close(&reader);
	switch(picture_type) {
	case 0:
		return true;
	default:
	case 1:
	case 2:
		return false;
	}
}

/**
 * analyze the width.
 * @param data      flv1 data
 * @param data_size flv1 data size
 * @return width  0 for error.
 */
uint32_t ttLibC_Flv1_getWidth(void *data, size_t data_size) {
	ttLibC_BitReader *reader = ttLibC_BitReader_make(data, data_size, BitReaderType_default);
	if(ttLibC_BitReader_bit(reader, 17) != 1) {
		ERR_PRINT("invalid flv1 data.");
		ttLibC_BitReader_close(&reader);
		return 0;
	}
	ttLibC_BitReader_bit(reader, 5);
	ttLibC_BitReader_bit(reader, 8);
	uint32_t picture_size = ttLibC_BitReader_bit(reader, 3);
	uint32_t width = 0;
	switch(picture_size) {
	case 0:
		width = ttLibC_BitReader_bit(reader, 8);
		break;
	case 1:
		width = ttLibC_BitReader_bit(reader, 16);
		break;
	case 2:
		width = 352;
		break;
	case 3:
		width = 176;
		break;
	case 4:
		width = 128;
		break;
	case 5:
		width = 320;
		break;
	case 6:
		width = 160;
		break;
	default:
	case 7:
		ERR_PRINT("picture type = 7 is reserved.");
		ttLibC_BitReader_close(&reader);
		return 0;
	}
	ttLibC_BitReader_close(&reader);
	return width;
}

/**
 * analyze the height.
 * @param data      flv1 data
 * @param data_size flv1 data size
 * @return height  0 for error.
 */
uint32_t ttLibC_Flv1_getHeight(void *data, size_t data_size) {
	ttLibC_BitReader *reader = ttLibC_BitReader_make(data, data_size, BitReaderType_default);
	if(ttLibC_BitReader_bit(reader, 17) != 1) {
		ERR_PRINT("invalid flv1 data.");
		ttLibC_BitReader_close(&reader);
		return 0;
	}
	ttLibC_BitReader_bit(reader, 5);
	ttLibC_BitReader_bit(reader, 8);
	uint32_t picture_size = ttLibC_BitReader_bit(reader, 3);
	uint32_t height = 0;
	switch(picture_size) {
	case 0:
		ttLibC_BitReader_bit(reader, 8);
		height = ttLibC_BitReader_bit(reader, 8);
		break;
	case 1:
		ttLibC_BitReader_bit(reader, 16);
		height = ttLibC_BitReader_bit(reader, 16);
		break;
	case 2:
		height = 288;
		break;
	case 3:
		height = 144;
		break;
	case 4:
		height = 96;
		break;
	case 5:
		height = 240;
		break;
	case 6:
		height = 120;
		break;
	default:
	case 7:
		ERR_PRINT("picture type = 7 is reserved.");
		ttLibC_BitReader_close(&reader);
		return 0;
	}
	ttLibC_BitReader_close(&reader);
	return height;
}

/**
 * analyze the frame information from flv1 binary
 * @param prev_frame    reuse frame
 * @param data          flv1 data
 * @param data_size     flv1 data size
 * @param non_copy_mode true:hold pointer. false:copy data.
 * @param pts           pts for flv1 frame
 * @param timebase      timebase for pts
 * @return flv1 frame
 */
ttLibC_Flv1 *ttLibC_Flv1_getFrame(
		ttLibC_Flv1 *prev_frame,
		uint8_t *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	bool key_frame = ttLibC_Flv1_isKey(data, data_size);
	uint32_t width = ttLibC_Flv1_getWidth(data, data_size);
	uint32_t height = ttLibC_Flv1_getHeight(data, data_size);
	if(width == 0 || height == 0) {
		return NULL;
	}
	return ttLibC_Flv1_make(
			prev_frame,
			key_frame ? videoType_key : videoType_inner,
			width,
			height,
			data,
			data_size,
			non_copy_mode,
			pts,
			timebase);
}

/**
 * close frame
 * @param frame
 */
void ttLibC_Flv1_close(ttLibC_Flv1 **frame) {
	ttLibC_Video_close_((ttLibC_Video **)frame);
}
