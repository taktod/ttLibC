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
#include "../../_log.h"
#include "../../allocator.h"
#include "../../ttLibC_common.h"
#include "../../util/ioUtil.h"
#include "../../util/byteUtil.h"
#include "../../util/hexUtil.h"
#include "../../util/crc32Util.h"

/*
 * h264 analyze ref information
 */
typedef struct {
	uint32_t width;
	uint32_t height;
	void *analyze_data_ptr;
} ttLibC_H264_Ref_t;

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
	if(prev_frame != NULL && prev_frame->inherit_super.inherit_super.type != frameType_h264) {
		ERR_PRINT("reuse with incompative frame.");
		return NULL;
	}
	ttLibC_H264 *h264 = prev_frame;
	size_t buffer_size_ = data_size;
	size_t data_size_ = data_size;
	switch(type) {
	case H264Type_configData:
	case H264Type_slice:
	case H264Type_sliceIDR:
	case H264Type_unknown:
		break;
	default:
		ERR_PRINT("unknown h264 type.%d", type);
		return NULL;
	}
	if(h264 == NULL) {
		h264 = ttLibC_malloc(sizeof(ttLibC_H264));
		if(h264 == NULL) {
			ERR_PRINT("failed to allocate memory for h264 frame.");
			return NULL;
		}
		h264->inherit_super.inherit_super.data = NULL;
	}
	else {
		if(!h264->inherit_super.inherit_super.is_non_copy) {
			if(non_copy_mode || h264->inherit_super.inherit_super.data_size < data_size_) {
				ttLibC_free(h264->inherit_super.inherit_super.data);
				h264->inherit_super.inherit_super.data = NULL;
			}
			else {
				data_size_ = h264->inherit_super.inherit_super.data_size;
			}
		}
	}
	h264->type                 = type;
	h264->inherit_super.width  = width;
	h264->inherit_super.height = height;
	switch(type) {
	case H264Type_configData:
	case H264Type_unknown:
		h264->inherit_super.type = videoType_info;
		break;
	case H264Type_slice:
		h264->inherit_super.type = videoType_inner;
		break;
	case H264Type_sliceIDR:
		h264->inherit_super.type = videoType_key;
		break;
	default:
		if(prev_frame == NULL) {
			ttLibC_free(h264);
		}
		return NULL;
	}
	h264->inherit_super.inherit_super.buffer_size = buffer_size_;
	h264->inherit_super.inherit_super.data_size   = data_size_;
	h264->inherit_super.inherit_super.is_non_copy = non_copy_mode;
	h264->inherit_super.inherit_super.pts         = pts;
	h264->inherit_super.inherit_super.dts         = 0;
	h264->inherit_super.inherit_super.timebase    = timebase;
	h264->inherit_super.inherit_super.type        = frameType_h264;
	if(non_copy_mode) {
		h264->inherit_super.inherit_super.data = data;
	}
	else {
		if(h264->inherit_super.inherit_super.data == NULL) {
			h264->inherit_super.inherit_super.data = ttLibC_malloc(data_size);
			if(h264->inherit_super.inherit_super.data == NULL) {
				ERR_PRINT("failed to allocate memory for data.");
				if(prev_frame == NULL) {
					ttLibC_free(h264);
				}
				return NULL;
			}
		}
		memcpy(h264->inherit_super.inherit_super.data, data, data_size);
	}
	switch(h264->type) {
	case H264Type_slice:
	case H264Type_sliceIDR:
		{
			uint8_t *buf = h264->inherit_super.inherit_super.data;
			size_t buf_size = h264->inherit_super.inherit_super.buffer_size;
			ttLibC_ByteReader *reader = NULL;
			if(buf[2] == 1) {
				reader = ttLibC_ByteReader_make(buf + 4, buf_size - 4, ByteUtilType_h26x);
				h264->is_disposable = (*(buf + 3) & 0x60) == 0;
			}
			else {
				reader = ttLibC_ByteReader_make(buf + 5, buf_size - 5, ByteUtilType_h26x);
				h264->is_disposable = (*(buf + 4) & 0x60) == 0;
			}
			/*uint32_t first_mb_in_slice = */ttLibC_ByteReader_expGolomb(reader, false);
			uint32_t slice_type = ttLibC_ByteReader_expGolomb(reader, false);
			h264->frame_type = slice_type % 5;
			ttLibC_ByteReader_close(&reader);
		}
		break;
	case H264Type_configData:
		h264->is_disposable = false;
		h264->frame_type = H264FrameType_unknown;
		break;
	case H264Type_unknown:
	default:
		h264->is_disposable = true;
		h264->frame_type = H264FrameType_unknown;
		break;
	}
	return h264;
}

/*
 * make clone frame.
 * always make copy buffer on it.
 * @param prev_frame reuse frame object.
 * @param src_frame  source of clone.
 */
ttLibC_H264 *ttLibC_H264_clone(
		ttLibC_H264 *prev_frame,
		ttLibC_H264 *src_frame) {
	if(src_frame == NULL) {
		return NULL;
	}
	if(src_frame->inherit_super.inherit_super.type != frameType_h264) {
		ERR_PRINT("try to clone non h264 frame.");
		return NULL;
	}
	if(prev_frame != NULL && prev_frame->inherit_super.inherit_super.type != frameType_h264) {
		// if close with ttLibC_Frame_close. object size is depends on original frameType. and this will break the memory.
		// therefore, just skip it.
		ERR_PRINT("try to use non h264 frame for reuse.");
		return NULL;
	}
	ttLibC_H264 *h264 = ttLibC_H264_make(
			prev_frame,
			src_frame->type,
			src_frame->inherit_super.width,
			src_frame->inherit_super.height,
			src_frame->inherit_super.inherit_super.data,
			src_frame->inherit_super.inherit_super.buffer_size,
			false,
			src_frame->inherit_super.inherit_super.pts,
			src_frame->inherit_super.inherit_super.timebase);
	if(h264 != NULL) {
		h264->inherit_super.inherit_super.id = src_frame->inherit_super.inherit_super.id;
	}
	return h264;
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
	if(data_size == 0) {
		return false; // no more data.
	}
	uint8_t *dat = data;
//	info->pos           = 0;
	info->data_pos      = 0;
	info->nal_unit_type = H264NalType_error;
	info->nal_size      = 0;
	size_t pos = 0;
	size_t i = 0;
	for(i = 0;i < data_size; ++ i, ++ data) {
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
				dat += info->data_pos;
				if(((*data) & 0x80) != 0) {
					ERR_PRINT("forbidden zero bit is not zero.");
					return false;
				}
				info->is_disposable = ((*dat) & 0x60) == 0;
				info->nal_unit_type = (*dat) & 0x1F;
				// sliceType check
				switch(info->nal_unit_type) {
				case H264NalType_slice:
//				case H264NalType_sliceDataPartitionA:
//				case H264NalType_sliceDataPartitionB:
//				case H264NalType_sliceDataPartitionC:
				case H264NalType_sliceIDR:
					{
						ttLibC_ByteReader *reader = ttLibC_ByteReader_make(dat + 1, data_size - info->data_pos - 1, ByteUtilType_h26x);
						/*uint32_t first_mb_in_slice = */ttLibC_ByteReader_expGolomb(reader, false);
						uint32_t slice_type = ttLibC_ByteReader_expGolomb(reader, false);
						info->frame_type = slice_type % 5;
						ttLibC_ByteReader_close(&reader);
					}
					break;
				default:
					info->frame_type = H264FrameType_unknown;
					break;
				}
			}
			pos = i + 1;
		}
		else {
			if(i - pos == 3) {
				// if continue with 00 for the padding.
				pos ++;
			}
		}
	}
	if(info->nal_size == 0) {
		info->nal_size = pos;
		if(i == data_size) {
			// if hit the end, take as one nal.
			info->nal_size = i;
		}
	}
	return true;
}

/*
 * analyze info of one nal(for avcc data).
 * @param info      pointer for info data.(update with data.)
 * @param data      data for analyze
 * @param data_size data size
 * @return true:analyze success
 * @note work with 4byte size length only
 */
bool ttLibC_H264_getAvccInfo(ttLibC_H264_NalInfo* info, uint8_t *data, size_t data_size) {
	if(info == NULL) {
		return false;
	}
	if(data_size < 5) {
		// not enough data size.
		return false;
	}
	info->data_pos = 4;
	info->nal_unit_type = H264NalType_error;
	info->nal_size = ((data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3]) + 4;
	data += 4;
	//size_t pos = 0;
	if(data_size < info->nal_size) {
		// not enough data size
		return false;
	}
	if(((*data) & 0x80) != 0) {
		ERR_PRINT("forbidden zero bit is not zero.");
		return false;
	}
	// type check
	info->nal_unit_type = (*data) & 0x1F;
	return true;
}

bool ttLibC_H264_getAvccInfo_ex(
		ttLibC_H264_NalInfo *info,
		uint8_t *data,
		size_t data_size,
		uint32_t length_size) {
	if(info == NULL) {
		return false;
	}
	if(data_size < length_size + 1) {
		// not enough data size.
		return false;
	}
	info->data_pos = length_size;
	info->nal_unit_type = H264NalType_error;
	info->nal_size = 0;
	for(uint32_t i = 0;i < length_size;++ i) {
		info->nal_size = (info->nal_size << 8) | *data;
		++ data;
	}
	if(data_size < info->nal_size) {
		// not enough data size.
		return false;
	}
	if(((*data) & 0x80) != 0) {
		ERR_PRINT("forbidden zero bit is not zero.");
	}
	info->nal_unit_type = (*data) & 0x1F;
	// sliceType check
	switch(info->nal_unit_type) {
	case H264NalType_slice:
//	case H264NalType_sliceDataPartitionA:
//	case H264NalType_sliceDataPartitionB:
//	case H264NalType_sliceDataPartitionC:
	case H264NalType_sliceIDR:
		{
			ttLibC_ByteReader *reader = ttLibC_ByteReader_make(data + 1, data_size - length_size - 1, ByteUtilType_h26x);
			/*uint32_t first_mb_in_slice = */ttLibC_ByteReader_expGolomb(reader, false);
			uint32_t slice_type = ttLibC_ByteReader_expGolomb(reader, false);
			info->frame_type = slice_type % 5;
			ttLibC_ByteReader_close(&reader);
		}
		break;
	default:
		info->frame_type = H264FrameType_unknown;
	}
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
 * @note work with sizelength = 4 only.
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

// TODO fix this. this is not good for multi thread.
//static ttLibC_H264_Ref_t ref = {0};

static Error_e H264_analyzeSequenceParameterSet(
		ttLibC_H264_Ref_t *ref,
		uint8_t *data,
		size_t data_size) {
	ttLibC_ByteReader *reader = ttLibC_ByteReader_make(data, data_size, ByteUtilType_h26x);
	if(reader == NULL) {
		return ttLibC_updateError(Target_On_VideoFrame, Error_MemoryAllocate);
	}
	ttLibC_ByteReader_bit(reader, 1);
	ttLibC_ByteReader_bit(reader, 2);
	uint8_t type = ttLibC_ByteReader_bit(reader, 5);
	if(type != H264NalType_sequenceParameterSet) {
		ERR_PRINT("get non sps data.");
		ttLibC_ByteReader_close(&reader);
		return ttLibC_updateError(Target_On_VideoFrame, Error_InvalidOperation);
	}
	uint8_t profile_idc = ttLibC_ByteReader_bit(reader, 8);
	uint32_t ChromaArrayType = 1;
	ttLibC_ByteReader_bit(reader, 8);
	ttLibC_ByteReader_bit(reader, 8);
	ttLibC_ByteReader_expGolomb(reader, false);
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
			uint32_t chroma_format_idc = ttLibC_ByteReader_expGolomb(reader, false);
			uint32_t separate_colour_plane_flag = 0;
			if(chroma_format_idc == 3) {
				separate_colour_plane_flag = ttLibC_ByteReader_bit(reader, 1);
			}
			if(separate_colour_plane_flag == 0) {
				ChromaArrayType = chroma_format_idc;
			}
			else {
				ChromaArrayType = 0;
			}
			ttLibC_ByteReader_expGolomb(reader, false);
			ttLibC_ByteReader_expGolomb(reader, false);
			ttLibC_ByteReader_bit(reader, 1);
			uint8_t seq_scaling_matrix_present_flag = ttLibC_ByteReader_bit(reader, 1);
			if(seq_scaling_matrix_present_flag == 1) {
				ERR_PRINT("not imprement for seq_scaling_matrix_present");
				return ttLibC_updateError(Target_On_VideoFrame, Error_InvalidOperation);
			}
		}
		break;
	default:
		break;
	}
	ttLibC_ByteReader_expGolomb(reader, false);
	uint32_t pic_order_cnt_type = ttLibC_ByteReader_expGolomb(reader, false);
	if(pic_order_cnt_type == 0) {
		ttLibC_ByteReader_expGolomb(reader, false);
	}
	else if(pic_order_cnt_type == 1){
		ttLibC_ByteReader_bit(reader, 1);
		ttLibC_ByteReader_expGolomb(reader, true);
		ttLibC_ByteReader_expGolomb(reader, true);
		uint32_t num_ref_frames_in_pic_order_cnt_cycle = ttLibC_ByteReader_expGolomb(reader, false);
		for(uint32_t i = 0;i < num_ref_frames_in_pic_order_cnt_cycle;++ i) {
			ttLibC_ByteReader_expGolomb(reader, true);
		}
	}
	ttLibC_ByteReader_expGolomb(reader, false);
	ttLibC_ByteReader_bit(reader, 1);
	uint32_t pic_width_in_mbs_minus1 = ttLibC_ByteReader_expGolomb(reader, false);
	uint32_t pic_height_in_map_units_minus1 = ttLibC_ByteReader_expGolomb(reader, false);
	uint8_t frame_mbs_only_flag = ttLibC_ByteReader_bit(reader, 1);
	if(!frame_mbs_only_flag) {
		ttLibC_ByteReader_bit(reader, 1);
	}
	ttLibC_ByteReader_bit(reader, 1);
	uint8_t frame_cropping_flag = ttLibC_ByteReader_bit(reader, 1);
	uint32_t frame_crop_left_offset   = 0;
	uint32_t frame_crop_right_offset  = 0;
	uint32_t frame_crop_top_offset    = 0;
	uint32_t frame_crop_bottom_offset = 0;
	if(frame_cropping_flag == 1) {
		frame_crop_left_offset   = ttLibC_ByteReader_expGolomb(reader, false);
		frame_crop_right_offset  = ttLibC_ByteReader_expGolomb(reader, false);
		frame_crop_top_offset    = ttLibC_ByteReader_expGolomb(reader, false);
		frame_crop_bottom_offset = ttLibC_ByteReader_expGolomb(reader, false);
	}
	uint32_t cropUnitX, cropUnitY;
	if(ChromaArrayType == 0) {
		cropUnitX = 1;
		cropUnitY = 2 - frame_mbs_only_flag;
	}
	else {
		if(ChromaArrayType <= 2) {
			cropUnitX = 2;
		}
		else {
			cropUnitX = 1;
		}
		if(ChromaArrayType <= 1) {
			cropUnitY = 2 * (2 - frame_mbs_only_flag);
		}
		else {
			cropUnitY = 1 * (2 - frame_mbs_only_flag);
		}
	}
	ref->width = ((pic_width_in_mbs_minus1 + 1) * 16) - (frame_crop_left_offset * cropUnitX) - (frame_crop_right_offset * cropUnitX);
	ref->height = ((2 - frame_mbs_only_flag)* (pic_height_in_map_units_minus1 +1) * 16) - (frame_crop_top_offset * cropUnitY) - (frame_crop_bottom_offset * cropUnitY);
	Error_e result = reader->error;
	ttLibC_ByteReader_close(&reader);
	return ttLibC_updateError(Target_On_VideoFrame, result);
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
	uint8_t *check_data = data;
	size_t check_size = data_size;
	ttLibC_H264_Ref_t ref;
	ref.analyze_data_ptr = NULL;
	ref.width  = 0;
	ref.height = 0;
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
				{
					// analyze data.
					Error_e result = H264_analyzeSequenceParameterSet(&ref, check_data + info.data_pos, check_size - info.data_pos);
					if(result != Error_noError) {
						LOG_ERROR(result);
						return 0;
					}
				}
				ref.analyze_data_ptr = data;
				return ref.width;
			default:
				if(prev_frame == NULL) {
					return 0;
				}
				// return ref value
//				ref.analyze_data_ptr = data;
//				ref.width  = prev_frame->inherit_super.width;
//				ref.height = prev_frame->inherit_super.height;
//				return ref.width;
				return prev_frame->inherit_super.width;
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
				{
					Error_e result = H264_analyzeSequenceParameterSet(&ref, check_data + info.data_pos, check_size - info.data_pos);
					if(result != Error_noError) {
						LOG_ERROR(result);
						return 0;
					}
				}
				ref.analyze_data_ptr = data;
				return ref.width;
			default:
				if(prev_frame == NULL) {
					return 0;
				}
/*				// return ref value
				ref.analyze_data_ptr = data;
				ref.width  = prev_frame->inherit_super.width;
				ref.height = prev_frame->inherit_super.height;
				return ref.width;*/
				return prev_frame->inherit_super.width;
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
//	if(ref.analyze_data_ptr == data) {
//		return ref.height;
//	}
	uint8_t *check_data = data;
	size_t check_size = data_size;
	ttLibC_H264_Ref_t ref;
	ref.analyze_data_ptr = NULL;
	ref.width  = 0;
	ref.height = 0;
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
				{
					Error_e result = H264_analyzeSequenceParameterSet(&ref, check_data + info.data_pos, check_size - info.data_pos);
					if(result != Error_noError) {
						LOG_ERROR(result);
						return 0;
					}
				}
				ref.analyze_data_ptr = data;
				return ref.height;
			default:
				if(prev_frame == NULL) {
					return 0;
				}
				// return ref value
/*				ref.analyze_data_ptr = data;
				ref.width  = prev_frame->inherit_super.width;
				ref.height = prev_frame->inherit_super.height;
				return ref.height;*/
				return prev_frame->inherit_super.height;
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
				{
					Error_e result = H264_analyzeSequenceParameterSet(&ref, check_data + info.data_pos, check_size - info.data_pos);
					if(result != Error_noError) {
						LOG_ERROR(result);
						return 0;
					}
				}
				ref.analyze_data_ptr = data;
				return ref.height;
			default:
				if(prev_frame == NULL) {
					return 0;
				}
/*				// return ref value
				ref.analyze_data_ptr = data;
				ref.width  = prev_frame->inherit_super.width;
				ref.height = prev_frame->inherit_super.height;
				return ref.height;*/
				return prev_frame->inherit_super.height;
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
ttLibC_H264 *ttLibC_H264_getFrame(
		ttLibC_H264 *prev_frame,
		uint8_t *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	// analyze data as nal. annex B.
	ttLibC_H264_NalInfo nal_info;
	if(!ttLibC_H264_getNalInfo(&nal_info, data, data_size)) {
		ERR_PRINT("failed to get nal info.");
		return NULL;
	}
	size_t target_size = 0;
	uint32_t width = 0, height = 0;
	if(prev_frame != NULL) {
		width  = prev_frame->inherit_super.width;
		height = prev_frame->inherit_super.height;
	}
	switch(nal_info.nal_unit_type) {
	default:
	case H264NalType_filterData:
		{
			if(nal_info.nal_unit_type != H264NalType_filterData) {
				ERR_PRINT("unknown nal is found:%d", nal_info.nal_unit_type);
			}
			else if(!nal_info.is_disposable){
				ERR_PRINT("non disposable filterData nal is found, we take this data as unknown.");
			}
		}
		/* no break */
	case H264NalType_accessUnitDelimiter:
	case H264NalType_supplementalEnhancementInformation:
	case H264NalType_pictureParameterSet:
		// type unknown.
		return ttLibC_H264_make(
				prev_frame,
				H264Type_unknown,
				width,
				height,
				data,
				nal_info.nal_size,
				non_copy_mode,
				pts,
				timebase);
	case H264NalType_sequenceParameterSet:
		// assume sps, pps.. order.
		target_size = nal_info.nal_size;
		width = ttLibC_H264_getWidth(prev_frame, data, data_size);
		height = ttLibC_H264_getHeight(prev_frame, data, data_size);
		if(width == 0 || height == 0) {
			return NULL;
		}
		// try to get next data.
		if(!ttLibC_H264_getNalInfo(&nal_info, data + target_size, data_size - target_size)) {
			ERR_PRINT("failed to get nal info.");
			return NULL;
		}
		if(nal_info.nal_unit_type != H264NalType_pictureParameterSet) {
			ERR_PRINT("expect sps and pps chunk. however, find other nal:%x", nal_info.nal_unit_type);
			return NULL;
		}
		// now sps and pps is ready.
		// TODO I have to improve this code, h264 config data can be consist with 2 or more sps, pps.
		// sometimes possible to include sps-ext.
		target_size += nal_info.nal_size;
		return ttLibC_H264_make(
				prev_frame,
				H264Type_configData,
				width,
				height,
				data,
				target_size,
				non_copy_mode,
				pts,
				timebase);
	case H264NalType_sliceIDR:
		// TODO to improve this code. it is better to check, first mb in slice.
		return ttLibC_H264_make(
				prev_frame,
				H264Type_sliceIDR,
				width,
				height,
				data,
				data_size,
				non_copy_mode,
				pts,
				timebase);
	case H264NalType_slice:
		// TODO to improve this code. it is better to check, first mb in slice.
		return ttLibC_H264_make(
				prev_frame,
				H264Type_slice,
				width,
				height,
				data,
				data_size,
				non_copy_mode,
				pts,
				timebase);
	}
	return NULL;
}

/*
 * analyze mp4 avcc tag.
 * @param prev_frame  reuse frame object.
 * @param data        data
 * @param data_size   data_size
 * @param length_size size of length information on avcc.
 */
ttLibC_H264 *ttLibC_H264_analyzeAvccTag(
		ttLibC_H264 *prev_frame,
		uint8_t *data,
		size_t data_size,
		uint32_t *length_size) {
	size_t buffer_size = data_size; // assume sps is one pps is one.
	uint8_t *buffer = NULL;
	bool alloc_flag = false;
	if(prev_frame != NULL) {
		if(!prev_frame->inherit_super.inherit_super.is_non_copy) {
			if(prev_frame->inherit_super.inherit_super.data_size > buffer_size) {
				// memory do have enough size.
				buffer = prev_frame->inherit_super.inherit_super.data;
				buffer_size = prev_frame->inherit_super.inherit_super.data_size;
			}
			else {
				// need more size.
				ttLibC_free(prev_frame->inherit_super.inherit_super.data);
			}
		}
		prev_frame->inherit_super.inherit_super.is_non_copy = true;
	}
	if(buffer == NULL) {
		buffer = ttLibC_malloc(buffer_size);
		if(buffer == NULL) {
			ERR_PRINT("failed to alloc.");
			return NULL;
		}
		alloc_flag = true;
	}
	uint8_t *buf = buffer;
	size_t buf_pos = 0;
	/*
	 * 1byte version
	 * 1byte profile
	 * 1byte compatibility
	 * 1byte level
	 * 6bit reserved
	 * 2bit nalLengthSize - 1
	 * 3bit reserved
	 * 5bit sps num
	 * 2byte sps size;
	 * nbyte sps
	 * 1byte pps num
	 * 2byte pps size
	 * nbyte pps
	 * profile_idc = 100 110 122 144 only
	 * 1byte 0xFC | chroma_format
	 * 1byte 0xF8 | (bit_depth_luma - 8)
	 * 1byte 0xF8 | (bit_depth_chroma - 8)
	 * 1byte spsext num
	 * 2byte length spsext size
	 * nbyte spsext
	 * done...
	 */
	if(data[0] != 1) {
		ERR_PRINT("avcc version is not 1.");
	}
	uint8_t profile_idc = data[1];
	*length_size = (data[4] & 0x03) + 1;
	uint32_t sps_count = data[5] & 0x1F;
	if(sps_count != 1) {
		ERR_PRINT("sps count is not 1.:%d", sps_count);
		if(alloc_flag) {
			ttLibC_free(buffer);
		}
		return NULL;
	}
	data += 6;
	data_size -= 6 ;
	uint32_t sps_size = be_uint16_t(*((uint16_t *)data));
	data += 2;
	data_size -= 2;
	*((uint32_t *)buf) = be_uint32_t(1);
	buf += 4;
	buf_pos += 4;
	memcpy(buf, data, sps_size);
	data += sps_size;
	data_size -= sps_size;
	buf += sps_size;
	buf_pos += sps_size;
	uint32_t pps_count = data[0];
	if(pps_count != 1) {
		ERR_PRINT("pps count is not 1.:%d", pps_count);
		if(alloc_flag) {
			ttLibC_free(buffer);
		}
		return NULL;
	}
	uint32_t pps_size = be_uint16_t(*((uint16_t *)(data + 1)));
	data += 3;
	data_size -= 3;
	*((uint32_t *)buf) = be_uint32_t(1);
	buf += 4;
	buf_pos += 4;
	memcpy(buf, data, pps_size);
	data += pps_size;
	data_size -= pps_size;
	buf += pps_size;
	buf_pos += pps_size;
	if(data_size != 0) {
		if(profile_idc != 100 && profile_idc != 110 && profile_idc != 122 && profile_idc != 144) {
			ERR_PRINT("invalid profile_idc for extended data.");
			if(alloc_flag) {
				ttLibC_free(buffer);
			}
			return NULL;
		}
		// in the case of support spsext need to append buf with annexB style nal.
		// just now not support spsext, so check spsext num == 0 or not.
		if(data[3] != 0) {
			ERR_PRINT("spsext num is not 0, not support now.");
			if(alloc_flag) {
				ttLibC_free(buffer);
			}
			return NULL;
		}
	}
	// now make frame.
	ttLibC_H264 *h264 = ttLibC_H264_getFrame(
			prev_frame,
			buffer,
			buf_pos,
			true,
			0,
			1000);
	if(h264 == NULL) {
		if(alloc_flag) {
			ttLibC_free(buffer);
		}
		return NULL;
	}
	// ok.
	// update buffer size
	h264->inherit_super.inherit_super.data_size = buffer_size;
	// set the data non copy.
	h264->inherit_super.inherit_super.is_non_copy = false;
	// done.
	return h264;
}

/*
 * calcurate crc32 value for configdata.
 * @param h264 target h264 object
 * @return value of crc32. 0 for error.
 */
uint32_t ttLibC_H264_getConfigCrc32(ttLibC_H264 *h264) {
	if(h264->type != H264Type_configData) {
		return 0;
	}
	ttLibC_Crc32 *crc32 = ttLibC_Crc32_make(0);
	uint8_t *data = h264->inherit_super.inherit_super.data;
	for(uint32_t i = 0;i < h264->inherit_super.inherit_super.buffer_size;++ i) {
		ttLibC_Crc32_update(crc32, *data);
		++ data;
	}
	uint32_t value = ttLibC_Crc32_getValue(crc32);
	ttLibC_Crc32_close(&crc32);
	return value;
}

/*
 * read avcc buffer for config data.
 * @param h264      target h264 object. this must be config data.
 * @param data      buffer to put data.
 * @param data_size buffer size.
 * @return write size. 0 for error.
 */
size_t ttLibC_H264_readAvccTag(
		ttLibC_H264 *h264,
		void *data,
		size_t data_size) {
	if(h264->type != H264Type_configData) {
		return 0;
	}
	// need to know the size of sps and pps.
	ttLibC_H264_NalInfo nal_info;
	uint8_t *buf = h264->inherit_super.inherit_super.data;
	size_t buf_size = h264->inherit_super.inherit_super.buffer_size;
	ttLibC_H264_NalInfo sps_info, pps_info;
	uint8_t *sps_buf = NULL;
	uint8_t *pps_buf = NULL;
	while(ttLibC_H264_getNalInfo(&nal_info, buf, buf_size)) {
		switch(nal_info.nal_unit_type) {
		case H264NalType_sequenceParameterSet:
			if(sps_buf != NULL) {
				ERR_PRINT("found more than 2 sps. not supported.");
				return 0;
			}
			sps_buf = buf;
			sps_info.data_pos      = nal_info.data_pos;
			sps_info.nal_size      = nal_info.nal_size;
			sps_info.nal_unit_type = nal_info.nal_unit_type;
			break;
		case H264NalType_pictureParameterSet:
			if(pps_buf != NULL) {
				ERR_PRINT("found more than 2 pps. not supported.");
				return 0;
			}
			pps_buf = buf;
			pps_info.data_pos      = nal_info.data_pos;
			pps_info.nal_size      = nal_info.nal_size;
			pps_info.nal_unit_type = nal_info.nal_unit_type;
			break;
		default:
			ERR_PRINT("unexpected nal is found.");
			return 0;
		}
		buf += nal_info.nal_size;
		buf_size -= nal_info.nal_size;
	}
	// now ready.
	/*
	 * note
	 * 01 version
	 * 4D 40 1E(sps copy 3 byte from sps)
	 * FF size length (size_length = 4, so ff is ok.)
	 * E1 lower 4bit is depends on the number of sps.
	 * xx xx 2byte for sps size
	 * sps body data.
	 * 01 1byte for the number of pps.
	 * xx xx 2byte for pps size
	 * pps body data
	 * in the case profile == 100 110 122 144
	 * 1byte 0xFC | chroma_format (chroma_format_idc?)
	 * 1byte 0xF8 | (bit_depth_luma - 8)
	 * 1byte 0xF8 | (bit_depth_chroma - 8)
	 * 1byte spsext num
	 * 2byte length spsext size
	 * nbyte spsext
	 * that's all.
	 */
	uint8_t profile_idc = sps_buf[1 + sps_info.data_pos];
	size_t target_size = 11 + sps_info.nal_size - sps_info.data_pos + pps_info.nal_size - pps_info.data_pos;
	if(profile_idc == 100 || profile_idc == 110 || profile_idc == 122 || profile_idc == 144) {
		target_size += 4; // only spsext is not exist is support, so only 4byte.
	}
	if(data_size < target_size) {
		ERR_PRINT("input buffer is too small:%zx required size:%zx", data_size, target_size);
		return 0;
	}
	uint8_t *dat = data;
	dat[0] = 0x01;
	dat[1] = sps_buf[1 + sps_info.data_pos];
	dat[2] = sps_buf[2 + sps_info.data_pos];
	dat[3] = sps_buf[3 + sps_info.data_pos];
	dat[4] = 0xFF;
	dat[5] = 0xE1;
	uint16_t sps_size = sps_info.nal_size - sps_info.data_pos;
	dat[6] = (sps_size >> 8) & 0xFF;
	dat[7] = sps_size & 0xFF;
	dat += 8;
	data_size -= 8;
	memcpy(dat, sps_buf + sps_info.data_pos, sps_size);
	dat += sps_size;
	data_size -= sps_size;
	dat[0] = 0x01;
	uint16_t pps_size = pps_info.nal_size - pps_info.data_pos;
	dat[1] = (pps_size >> 8) & 0xFF;
	dat[2] = pps_size & 0xFF;
	dat += 3;
	data_size -= 3;
	memcpy(dat, pps_buf + pps_info.data_pos, pps_size);
	dat += pps_size;
	data_size -= pps_size;
	if(profile_idc == 100 || profile_idc == 110 || profile_idc == 122 || profile_idc == 144) {
//		LOG_PRINT("need to make spse info for avcC.");
		ttLibC_ByteReader *reader = ttLibC_ByteReader_make(sps_buf + sps_info.data_pos, sps_info.nal_size, ByteUtilType_h26x);
		ttLibC_ByteReader_bit(reader, 1);
		ttLibC_ByteReader_bit(reader, 2);
		ttLibC_ByteReader_bit(reader, 5); // type
		ttLibC_ByteReader_bit(reader, 8); // profile idc
		ttLibC_ByteReader_bit(reader, 8); // constraint_set flags
		ttLibC_ByteReader_bit(reader, 8); // level_idc
		ttLibC_ByteReader_expGolomb(reader, true);
		uint8_t chroma_format_idc = ttLibC_ByteReader_expGolomb(reader, true);
		if(chroma_format_idc == 3) {
			ttLibC_ByteReader_bit(reader, 1); // separate color plane flag
		}
		uint8_t bit_depth_luma_minus8 = ttLibC_ByteReader_expGolomb(reader, true);
		uint8_t bit_depth_chroma_minus8 = ttLibC_ByteReader_expGolomb(reader, true);
		ttLibC_ByteReader_close(&reader);
		dat[0] = 0xFC | chroma_format_idc;
		dat[1] = 0xF8 | bit_depth_luma_minus8;
		dat[2] = 0xF8 | bit_depth_chroma_minus8;
		dat[3] = 0;
	}
	return target_size;
}

/*
 * close frame
 * @param frame
 */
void ttLibC_H264_close(ttLibC_H264 **frame) {
	ttLibC_H264 *target = *frame;
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.inherit_super.type != frameType_h264) {
		ERR_PRINT("found non h264 frame in h264_close.");
		return;
	}
	if(!target->inherit_super.inherit_super.is_non_copy) {
		ttLibC_free(target->inherit_super.inherit_super.data);
	}
	ttLibC_free(target);
	*frame = NULL;
}
