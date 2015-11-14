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
#include "../../allocator.h"
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
	case H264Type_unknown:
		break;
	default:
		ERR_PRINT("unknown h264 type.%d", type);
		return NULL;
	}
	if(h264 == NULL) {
		h264 = ttLibC_malloc(sizeof(ttLibC_H264_));
		if(h264 == NULL) {
			ERR_PRINT("failed to allocate memory for h264 frame.");
			return NULL;
		}
		h264->inherit_super.inherit_super.inherit_super.data = NULL;
	}
	else {
		if(!h264->inherit_super.inherit_super.inherit_super.is_non_copy) {
			if(non_copy_mode || h264->inherit_super.inherit_super.inherit_super.data_size < data_size_) {
				ttLibC_free(h264->inherit_super.inherit_super.inherit_super.data);
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
	case H264Type_unknown:
		h264->inherit_super.inherit_super.type = videoType_info;
		break;
	case H264Type_slice:
		h264->inherit_super.inherit_super.type = videoType_inner;
		break;
	case H264Type_sliceIDR:
		h264->inherit_super.inherit_super.type = videoType_key;
		break;
	default:
		if(prev_frame == NULL) {
			ttLibC_free(h264);
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
			h264->inherit_super.inherit_super.inherit_super.data = ttLibC_malloc(data_size);
			if(h264->inherit_super.inherit_super.inherit_super.data == NULL) {
				ERR_PRINT("failed to allocate memory for data.");
				if(prev_frame == NULL) {
					ttLibC_free(h264);
				}
				return NULL;
			}
		}
		memcpy(h264->inherit_super.inherit_super.inherit_super.data, data, data_size);
	}
	return (ttLibC_H264 *)h264;
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
	return ttLibC_H264_make(
			prev_frame,
			src_frame->type,
			src_frame->inherit_super.width,
			src_frame->inherit_super.height,
			src_frame->inherit_super.inherit_super.data,
			src_frame->inherit_super.inherit_super.buffer_size,
			false,
			src_frame->inherit_super.inherit_super.pts,
			src_frame->inherit_super.inherit_super.timebase);
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
	ttLibC_ByteReader *reader = ttLibC_ByteReader_make(data, data_size, ByteUtilType_h26x);

	ttLibC_ByteReader_bit(reader, 1);
	ttLibC_ByteReader_bit(reader, 2);
	uint8_t type = ttLibC_ByteReader_bit(reader, 5);
	if(type != H264NalType_sequenceParameterSet) {
		ERR_PRINT("get non sps data.");
		ttLibC_ByteReader_close(&reader);
		return;
	}
	uint8_t profile_idc = ttLibC_ByteReader_bit(reader, 8);
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
			ERR_PRINT("non check code.");
			uint32_t chroma_format_idc = ttLibC_ByteReader_expGolomb(reader, false);
			if(chroma_format_idc == 3) {
				ttLibC_ByteReader_bit(reader, 1);
			}
			ttLibC_ByteReader_expGolomb(reader, false);
			ttLibC_ByteReader_expGolomb(reader, false);
			ttLibC_ByteReader_bit(reader, 1);
			uint8_t seq_scaling_matrix_present_flag = ttLibC_ByteReader_bit(reader, 1);
			if(seq_scaling_matrix_present_flag == 1) {
				ERR_PRINT("not imprement for seq_scaling_matrix_present");
				return;
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
		for(int i = 0;i < num_ref_frames_in_pic_order_cnt_cycle;++ i) {
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
	ref->width = ((pic_width_in_mbs_minus1 + 1) * 16) - frame_crop_left_offset * 2 - frame_crop_right_offset * 2;
	ref->height = ((2 - frame_mbs_only_flag)* (pic_height_in_map_units_minus1 +1) * 16) - (frame_crop_top_offset * 2) - (frame_crop_bottom_offset * 2);

	ttLibC_ByteReader_close(&reader);
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
	ttLibC_H264 *h264 = NULL;
	size_t target_size = 0;
	uint32_t width = 0, height = 0;
	if(prev_frame != NULL) {
		width  = prev_frame->inherit_super.width;
		height = prev_frame->inherit_super.height;
	}
	switch(nal_info.nal_unit_type) {
	case H264NalType_accessUnitDelimiter:
	case H264NalType_supplementalEnhancementInformation:
	default:
		// type unknown.
		return ttLibC_H264_make(
				prev_frame,
				H264Type_unknown,
				width,
				height,
				data,
				nal_info.nal_size,
				true,
				pts,
				timebase);
	case H264NalType_sequenceParameterSet:
		// assume sps, pps.. order.
		target_size = nal_info.nal_size;
		width = ttLibC_H264_getWidth(prev_frame, data, data_size);
		height = ttLibC_H264_getHeight(prev_frame, data, data_size);
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
				true,
				pts,
				timebase);
	case H264NalType_pictureParameterSet:
		ERR_PRINT("unexpected.. to have pps first.");
		return NULL;
	case H264NalType_sliceIDR:
		// TODO to improve this code. it is better to check, first mb in slice.
		return ttLibC_H264_make(
				prev_frame,
				H264Type_sliceIDR,
				width,
				height,
				data,
				data_size,
				true,
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
				true,
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
	bool is_alloc_flg = false;
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
		is_alloc_flg = true;
	}
	uint8_t *buf = buffer;
	size_t buf_pos = 0;
	/**
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
	 * done...
	 */
	if(data[0] != 1) {
		ERR_PRINT("avcc version is not 1.");
	}
	*length_size = (data[4] & 0x03) + 1;
	uint32_t sps_count = data[5] & 0x1F;
	if(sps_count != 1) {
		ERR_PRINT("sps count is not 1.:%d", sps_count);
		if(is_alloc_flg) {
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
		if(is_alloc_flg) {
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
		ERR_PRINT("data loading is not complete, there is some more.");
		if(is_alloc_flg) {
			ttLibC_free(buffer);
		}
		return NULL;
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
		if(is_alloc_flg) {
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
	 * that's all.
	 */
	size_t target_size = 11 + sps_info.nal_size - sps_info.data_pos + pps_info.nal_size - pps_info.data_pos;
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
	return target_size;
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
		ttLibC_free(target->inherit_super.inherit_super.inherit_super.data);
	}
	ttLibC_free(target);
	*frame = NULL;
}
