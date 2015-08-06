/*
 * @file   h264.c
 * @brief  h264 image frame information
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/24
 */

#include "h264.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../log.h"
#include "../../util/bitUtil.h"

/*
 * h264 analyze ref information
 */
typedef struct {
	uint32_t width;
	uint32_t height;
	void *analyze_data_ptr;
} ttLibC_H264_Ref_t;

/*
 * h264 frame definition(detail)
 */
typedef struct {
	/** inherit data from ttLibC_H264 */
	ttLibC_H264 inherit_super;
} ttLibC_Frame_Video_H264_;

typedef ttLibC_Frame_Video_H264_ ttLibC_H264_;

/*
 * make h264 frame
 * @param prev_frame    reuse frame.
 * @param type          type of h264
 * @param width         width of picture
 * @param height        height of picture
 * @param data          h264 data
 * @param data_size     h264 data size
 * @param non_copy_mode true:hold the data pointer. false:data will copy.
 * @param pts           pts for h264 data.
 * @param timebase      timebase number for pts.
 */
ttLibC_H264 *ttLibC_H264_make(
		ttLibC_H264 *prev_frame,
		ttLibC_H264_Type type,
		uint32_t width,
		uint32_t height,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	ttLibC_H264_ *h264 = (ttLibC_H264_ *)prev_frame;
	size_t buffer_size_ = data_size;
	size_t data_size_ = data_size;
	switch(type) {
	case H264Type_configData:
	case H264Type_slice:
	case H264Type_sliceIDR:
		break;
	case H264Type_unknown:
		ERR_PRINT("unknown is defined.");
		return NULL;
	default:
		ERR_PRINT("unknown h264 type.%d", type);
		return NULL;
	}
	if(h264 == NULL) {
		h264 = malloc(sizeof(ttLibC_H264_));
		if(h264 == NULL) {
			ERR_PRINT("failed to allocate memory for h264 frame.");
			return NULL;
		}
		h264->inherit_super.inherit_super.inherit_super.data = NULL;
	}
	else {
		if(!h264->inherit_super.inherit_super.inherit_super.is_non_copy) {
			if(non_copy_mode || h264->inherit_super.inherit_super.inherit_super.data_size < data_size_) {
				free(h264->inherit_super.inherit_super.inherit_super.data);
				h264->inherit_super.inherit_super.inherit_super.data = NULL;
			}
			else {
				data_size_ = h264->inherit_super.inherit_super.inherit_super.data_size;
			}
		}
	}
	h264->inherit_super.type                 = type;
	h264->inherit_super.inherit_super.width  = width;
	h264->inherit_super.inherit_super.height = height;
	switch(type) {
	case H264Type_configData:
		h264->inherit_super.inherit_super.type = videoType_info;
		break;
	case H264Type_slice:
		h264->inherit_super.inherit_super.type = videoType_inner;
		break;
	case H264Type_sliceIDR:
		h264->inherit_super.inherit_super.type = videoType_key;
		break;
	case H264Type_unknown:
		if(prev_frame == NULL) {
			free(h264);
		}
		return NULL;
	}
	h264->inherit_super.inherit_super.inherit_super.buffer_size = buffer_size_;
	h264->inherit_super.inherit_super.inherit_super.data_size   = data_size_;
	h264->inherit_super.inherit_super.inherit_super.is_non_copy = non_copy_mode;
	h264->inherit_super.inherit_super.inherit_super.pts         = pts;
	h264->inherit_super.inherit_super.inherit_super.timebase    = timebase;
	h264->inherit_super.inherit_super.inherit_super.type        = frameType_h264;
	if(non_copy_mode) {
		h264->inherit_super.inherit_super.inherit_super.data = data;
	}
	else {
		if(h264->inherit_super.inherit_super.inherit_super.data == NULL) {
			h264->inherit_super.inherit_super.inherit_super.data = malloc(data_size);
			if(h264->inherit_super.inherit_super.inherit_super.data == NULL) {
				ERR_PRINT("failed to allocate memory for data.");
				if(prev_frame == NULL) {
					free(h264);
				}
				return NULL;
			}
		}
		memcpy(h264->inherit_super.inherit_super.inherit_super.data, data, data_size);
	}
	return (ttLibC_H264 *)h264;
}

/*
 * analyze info of one nal.
 * @param info      pointer for info data.(update with data.)
 * @param data      data for analyze
 * @param data_size data size
 */
bool ttLibC_H264_getNalInfo(ttLibC_H264_NalInfo* info, uint8_t *data, size_t data_size) {
	if(info == NULL) {
		return false;
	}
//	info->pos           = 0;
	info->data_pos      = 0;
	info->nal_unit_type = H264NalType_error;
	info->nal_size      = 0;
	size_t pos = 0;
	for(size_t i = 0;i < data_size; ++ i, ++ data) {
		if(i == 0 && (*data) != 0) {
			return false;
		}
		if((*data) != 0) {
			if(i - pos == 2 || i - pos == 3) {
				if((*data) == 1) {
					if(info->nal_unit_type != H264NalType_error) {
//						info->nal_size = pos - info->pos;
						info->nal_size = pos;
						break;
					}
//					info->pos = pos;
					info->data_pos = i + 1;
				}
			}
			else if(info->nal_unit_type == H264NalType_error && info->data_pos != 0) {
				if(((*data) & 0x80) != 0) {
					ERR_PRINT("forbidden zero bit is not zero.");
					return false;
				}
				else {
					info->nal_unit_type = (*data) & 0x1F;
				}
			}
			pos = i + 1;
		}
	}
	if(info->nal_size == 0) {
		info->nal_size = pos;
	}
	return true;
}

/*
 * analyez info of one nal(for avcc data).
 * @param info      pointer for info data.(update with data.)
 * @param data      data for analyze
 * @param data_size data size
 * @return true:analyze success
 */
bool ttLibC_H264_getAvccInfo(ttLibC_H264_NalInfo* info, uint8_t *data, size_t data_size) {
	if(info == NULL) {
		return false;
	}
	if(data_size < 5) {
		// not enough data size.
		return false;
	}
	info->data_pos = 0;
	info->nal_unit_type = H264NalType_error;
	info->nal_size = ((data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3]) + 4;
	data += 4;
	size_t pos = 0;
	if(data_size < info->nal_size) {
		// not enough data size
		return false;
	}
	if(((*data) & 0x80) != 0) {
		ERR_PRINT("forbidden zero bit is not zero.");
		return false;
	}
	info->nal_unit_type = (*data) & 0x1F;
	return true;
}

/**
 * check data type is nal or not.
 */
bool ttLibC_H264_isNal(uint8_t *data, size_t data_size) {
	if(data_size < 4) {
		return false;
	}
	if(*data != 0) {
		return false;
	}
	++ data;
	if(*data != 0) {
		return false;
	}
	++ data;
	if(*data == 1) {
		return true;
	}
	if(*data != 0) {
		return false;
	}
	++ data;
	if(*data == 1) {
		return true;
	}
	return false;
}

/**
 * check data type is avcc or not.
 */
bool ttLibC_H264_isAvcc(uint8_t *data, size_t data_size) {
	if(data_size < 4) {
		return false;
	}
	uint32_t size = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
	if(data_size < size) {
		return false;
	}
	if(size == 1) {
		// nal
		return false;
	}
	return true;
}

/*
 * check data which is key or not.
 * @param data      target data
 * @param data_size target data size
 * @return true: data is key frame. false: not key frame.
 */
bool ttLibC_H264_isKey(uint8_t *data, size_t data_size) {
	ttLibC_H264_NalInfo info;
	if(ttLibC_H264_isAvcc(data, data_size)) {
		if(!ttLibC_H264_getAvccInfo(&info, data, data_size)) {
			return false;
		}
	}
	else if(ttLibC_H264_isNal(data, data_size)) {
		if(!ttLibC_H264_getNalInfo(&info, data, data_size)) {
			return false;
		}
	}
	else {
		return false;
	}
	switch(info.nal_unit_type) {
	case H264NalType_sliceIDR:
	case H264NalType_sequenceParameterSet:
		return true;
	default:
		return false;
	}
}

static ttLibC_H264_Ref_t ref = {0};

static void H264_analyzeSequenceParameterSet(ttLibC_H264_Ref_t *ref, uint8_t *data, size_t data_size) {
	ttLibC_BitReader *reader = ttLibC_BitReader_make(data, data_size, BitReaderType_h26x);

	ttLibC_BitReader_bit(reader, 1);
	ttLibC_BitReader_bit(reader, 2);
	uint8_t type = ttLibC_BitReader_bit(reader, 5);
	if(type != H264NalType_sequenceParameterSet) {
		ERR_PRINT("get non sps data.");
		ttLibC_BitReader_close(&reader);
		return;
	}
	uint8_t profile_idc = ttLibC_BitReader_bit(reader, 8);
	ttLibC_BitReader_bit(reader, 8);
	ttLibC_BitReader_bit(reader, 8);
	ttLibC_BitReader_expGolomb(reader, false);
	switch(profile_idc) {
	case 100:
	case 110:
	case 122:
	case 244:
	case 44:
	case 83:
	case 86:
	case 118:
	case 128:
	case 138:
	case 139:
	case 134:
		{
			ERR_PRINT("non check code.");
			uint32_t chroma_format_idc = ttLibC_BitReader_expGolomb(reader, false);
			if(chroma_format_idc == 3) {
				ttLibC_BitReader_bit(reader, 1);
			}
			ttLibC_BitReader_expGolomb(reader, false);
			ttLibC_BitReader_expGolomb(reader, false);
			ttLibC_BitReader_bit(reader, 1);
			uint8_t seq_scaling_matrix_present_flag = ttLibC_BitReader_bit(reader, 1);
			if(seq_scaling_matrix_present_flag == 1) {
				ERR_PRINT("not imprement for seq_scaling_matrix_present");
				return;
			}
		}
		break;
	default:
		break;
	}
	ttLibC_BitReader_expGolomb(reader, false);
	uint32_t pic_order_cnt_type = ttLibC_BitReader_expGolomb(reader, false);
	if(pic_order_cnt_type == 0) {
		ttLibC_BitReader_expGolomb(reader, false);
	}
	else {
		ttLibC_BitReader_bit(reader, 1);
		ttLibC_BitReader_expGolomb(reader, true);
		ttLibC_BitReader_expGolomb(reader, true);
		uint32_t num_ref_frames_in_pic_order_cnt_cycle = ttLibC_BitReader_expGolomb(reader, false);
		for(int i = 0;i < num_ref_frames_in_pic_order_cnt_cycle;++ i) {
			ttLibC_BitReader_expGolomb(reader, true);
		}
	}
	ttLibC_BitReader_expGolomb(reader, false);
	ttLibC_BitReader_bit(reader, 1);
	uint32_t pic_width_in_mbs_minus1 = ttLibC_BitReader_expGolomb(reader, false);
	uint32_t pic_height_in_map_units_minus1 = ttLibC_BitReader_expGolomb(reader, false);
	uint8_t frame_mbs_only_flag = ttLibC_BitReader_bit(reader, 1);
	if(!frame_mbs_only_flag) {
		ttLibC_BitReader_bit(reader, 1);
	}
	ttLibC_BitReader_bit(reader, 1);
	uint8_t frame_cropping_flag = ttLibC_BitReader_bit(reader, 1);
	uint32_t frame_crop_left_offset   = 0;
	uint32_t frame_crop_right_offset  = 0;
	uint32_t frame_crop_top_offset    = 0;
	uint32_t frame_crop_bottom_offset = 0;
	if(frame_cropping_flag == 1) {
		frame_crop_left_offset   = ttLibC_BitReader_expGolomb(reader, false);
		frame_crop_right_offset  = ttLibC_BitReader_expGolomb(reader, false);
		frame_crop_top_offset    = ttLibC_BitReader_expGolomb(reader, false);
		frame_crop_bottom_offset = ttLibC_BitReader_expGolomb(reader, false);
	}
	ref->width = ((pic_width_in_mbs_minus1 + 1) * 16) - frame_crop_left_offset * 2 - frame_crop_right_offset * 2;
	ref->height = ((2 - frame_mbs_only_flag)* (pic_height_in_map_units_minus1 +1) * 16) - (frame_crop_top_offset * 2) - (frame_crop_bottom_offset * 2);

	ttLibC_BitReader_close(&reader);
}

/*
 * analyze width from data.
 * @param prev_frame ref for frame before make.
 * @param data       target data
 * @param data_size  target data size
 * @return width size
 */
uint32_t ttLibC_H264_getWidth(ttLibC_H264 *prev_frame, uint8_t *data, size_t data_size) {
	// for pps aud -> analyze next nal.
	// for sps -> analyze to get width and height.
	// for others -> get ref from prev_frame.
	ttLibC_H264_NalInfo info;
	if(ref.analyze_data_ptr == data) {
		return ref.width;
	}
	uint8_t *check_data = data;
	size_t check_size = data_size;
	if(ttLibC_H264_isAvcc(check_data, check_size)) {
		do {
			if(!ttLibC_H264_getAvccInfo(&info, check_data, check_size)) {
				return 0;
			}
			switch(info.nal_unit_type) {
			case H264NalType_accessUnitDelimiter:
			case H264NalType_pictureParameterSet:
				// continue to do.
				check_data += info.nal_size;
				check_size -= info.nal_size;
				break;
			case H264NalType_sequenceParameterSet:
				// analyze data.
				H264_analyzeSequenceParameterSet(&ref, check_data + info.data_pos, check_size - info.data_pos);
				ref.analyze_data_ptr = data;
				return ref.width;
			default:
				if(prev_frame == NULL) {
					return 0;
				}
				// return ref value
				ref.analyze_data_ptr = data;
				ref.width  = prev_frame->inherit_super.width;
				ref.height = prev_frame->inherit_super.height;
				return ref.width;
			}
		} while(check_size > 0);
	}
	else if(ttLibC_H264_isNal(check_data, check_size)) {
		do {
			if(!ttLibC_H264_getNalInfo(&info, check_data, check_size)) {
				return 0;
			}
			switch(info.nal_unit_type) {
			case H264NalType_accessUnitDelimiter:
			case H264NalType_pictureParameterSet:
				// continue to do.
				check_data += info.nal_size;
				check_size -= info.nal_size;
				break;
			case H264NalType_sequenceParameterSet:
				// analyze data.
				H264_analyzeSequenceParameterSet(&ref, check_data + info.data_pos, check_size - info.data_pos);
				ref.analyze_data_ptr = data;
				return ref.width;
			default:
				if(prev_frame == NULL) {
					return 0;
				}
				// return ref value
				ref.analyze_data_ptr = data;
				ref.width  = prev_frame->inherit_super.width;
				ref.height = prev_frame->inherit_super.height;
				return ref.width;
			}
		} while(check_size > 0);
	}
	return 0;
}

/*
 * analyze height from data.
 * @param prev_frame ref for frame before make.
 * @param data       target data
 * @param data_size  target data size
 * @return height size
 */
uint32_t ttLibC_H264_getHeight(ttLibC_H264 *prev_frame, uint8_t *data, size_t data_size) {
	// for pps aud -> analyze next nal.
	// for sps -> analyze to get width and height.
	// for others -> get ref from prev_frame.
	ttLibC_H264_NalInfo info;
	if(ref.analyze_data_ptr == data) {
		return ref.height;
	}
	uint8_t *check_data = data;
	size_t check_size = data_size;
	if(ttLibC_H264_isAvcc(check_data, check_size)) {
		do {
			if(!ttLibC_H264_getAvccInfo(&info, check_data, check_size)) {
				return 0;
			}
			switch(info.nal_unit_type) {
			case H264NalType_accessUnitDelimiter:
			case H264NalType_pictureParameterSet:
				// continue to do.
				check_data += info.nal_size;
				check_size -= info.nal_size;
				break;
			case H264NalType_sequenceParameterSet:
				// analyze data.
				H264_analyzeSequenceParameterSet(&ref, check_data + info.data_pos, check_size - info.data_pos);
				ref.analyze_data_ptr = data;
				return ref.height;
			default:
				if(prev_frame == NULL) {
					return 0;
				}
				// return ref value
				ref.analyze_data_ptr = data;
				ref.width  = prev_frame->inherit_super.width;
				ref.height = prev_frame->inherit_super.height;
				return ref.height;
			}
		} while(check_size > 0);
	}
	else if(ttLibC_H264_isNal(check_data, check_size)) {
		do {
			if(!ttLibC_H264_getNalInfo(&info, check_data, check_size)) {
				return 0;
			}
			switch(info.nal_unit_type) {
			case H264NalType_accessUnitDelimiter:
			case H264NalType_pictureParameterSet:
				// continue to do.
				check_data += info.nal_size;
				check_size -= info.nal_size;
				break;
			case H264NalType_sequenceParameterSet:
				// analyze data.
				H264_analyzeSequenceParameterSet(&ref, check_data + info.data_pos, check_size - info.data_pos);
				ref.analyze_data_ptr = data;
				return ref.height;
			default:
				if(prev_frame == NULL) {
					return 0;
				}
				// return ref value
				ref.analyze_data_ptr = data;
				ref.width  = prev_frame->inherit_super.width;
				ref.height = prev_frame->inherit_super.height;
				return ref.height;
			}
		} while(check_size > 0);
	}
	return 0;
}

/*
 * get frame object from data.
 * @param prev_frame reuse frame object.
 * @param data       target data
 * @param data_size  target data size
 * @return h264 frame object.
 */
ttLibC_H264 *ttLibC_H264_getFrame(ttLibC_H264 *prev_frame, uint8_t *data, size_t data_size) {
	// TODO make this function.
}

/*
 * close frame
 * @param frame
 */
void ttLibC_H264_close(ttLibC_H264 **frame) {
	ttLibC_H264_ *target = (ttLibC_H264_ *)*frame;
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.inherit_super.inherit_super.type != frameType_h264) {
		ERR_PRINT("found non h264 frame in h264_close.");
		return;
	}
	if(!target->inherit_super.inherit_super.inherit_super.is_non_copy) {
		free(target->inherit_super.inherit_super.inherit_super.data);
	}
	free(target);
	*frame = NULL;
}
