/*
 * @file   h265.c
 * @brief  h265 image frame information
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/06
 */

#include "h265.h"
#include "../../_log.h"
#include "../../allocator.h"
#include "../../util/hexUtil.h"
#include "../../util/byteUtil.h"
#include "../../util/crc32Util.h"
#include <string.h>

typedef struct {
	uint32_t width;
	uint32_t height;
	void *analyze_data_ptr;
} ttLibC_H265_Ref_t;

ttLibC_H265 TT_ATTRIBUTE_API *ttLibC_H265_make(
		ttLibC_H265 *prev_frame,
		ttLibC_H265_Type type,
		uint32_t width,
		uint32_t height,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	if(prev_frame != NULL && prev_frame->inherit_super.inherit_super.type != frameType_h265) {
		ERR_PRINT("reuse with incompatible frame.");
		return NULL;
	}
	ttLibC_Video_Type video_type = videoType_info;
	switch(type) {
	case H265Type_configData:
		break;
	case H265Type_slice:
		video_type = videoType_inner;
		break;
	case H265Type_sliceIDR:
		video_type = videoType_key;
		break;
	case H265Type_unknown:
		break;
	default:
		ERR_PRINT("unknown h265 type.%d", type);
		return NULL;
	}
	ttLibC_H265 *h265 = (ttLibC_H265 *)ttLibC_Video_make(
			(ttLibC_Video *)prev_frame,
			sizeof(ttLibC_H265),
			frameType_h265,
			video_type,
			width,
			height,
			data,
			data_size,
			non_copy_mode,
			pts,
			timebase);
	if(h265 == NULL) {
		return NULL;
	}
	h265->type = type;
	// check if disposable and frame type.
	switch(h265->type) {
	case H265Type_slice:
		{
			uint8_t *buf = h265->inherit_super.inherit_super.data;
			size_t buf_size = h265->inherit_super.inherit_super.buffer_size;
			ttLibC_ByteReader *reader = NULL;
			ttLibC_H265_NalType nal_unit_type = H265NalType_error;
			if(buf[2] == 1) {
				reader = ttLibC_ByteReader_make(buf + 5, buf_size - 5, ByteUtilType_h26x);
				nal_unit_type = ((*(buf + 3)) >> 1) & 0x3F;
			}
			else {
				reader = ttLibC_ByteReader_make(buf + 6, buf_size - 6, ByteUtilType_h26x);
				nal_unit_type = ((*(buf + 4)) >> 1) & 0x3F;
			}
			h265->is_disposable = false;
			switch(nal_unit_type) {
			case H265NalType_trailN:
			case H265NalType_tsaN:
			case H265NalType_stsaN:
			case H265NalType_radlN:
			case H265NalType_raslN:
			case H265NalType_rsvVclN10:
			case H265NalType_rsvVclN12:
			case H265NalType_rsvVclN14:
				h265->is_disposable = true;
				/* no break */
			case H265NalType_trailR:
			case H265NalType_tsaR:
			case H265NalType_stsaR:
			case H265NalType_radlR:
			case H265NalType_raslR:
			case H265NalType_rsvVclR11:
			case H265NalType_rsvVclR13:
			case H265NalType_rsvVclR15:
				{
					/*uint32_t first_slice_segment_in_pic_flag = */ttLibC_ByteReader_bit(reader, 1);
					/*uint32_t slice_pic_parameter_set_id = */ttLibC_ByteReader_expGolomb(reader, false);
					h265->frame_type = ttLibC_ByteReader_expGolomb(reader, false);
				}
				break;
			default:
				ERR_PRINT("unexpected nal type.:%d", nal_unit_type);
				break;
			}
			ttLibC_ByteReader_close(&reader);
		}
		break;
	case H265Type_sliceIDR:
		h265->is_disposable = false;
		h265->frame_type = H265FrameType_I;
		break;
	case H265Type_configData:
		h265->is_disposable = false;
		h265->frame_type = H265FrameType_unknown;
		break;
	default:
		h265->is_disposable = true;
		h265->frame_type = H265FrameType_unknown;
		break;
	}
	return h265;
}

ttLibC_H265 TT_ATTRIBUTE_API *ttLibC_H265_clone(
		ttLibC_H265 *prev_frame,
		ttLibC_H265 *src_frame) {
	if(src_frame == NULL) {
		return NULL;
	}
	if(src_frame->inherit_super.inherit_super.type != frameType_h265) {
		ERR_PRINT("try to clone non h265 frame.");
		return NULL;
	}
	if(prev_frame != NULL && prev_frame->inherit_super.inherit_super.type != frameType_h265) {
		ERR_PRINT("try to use non h265 frame for reuse.");
		return NULL;
	}
	ttLibC_H265 *h265 = ttLibC_H265_make(
			prev_frame,
			src_frame->type,
			src_frame->inherit_super.width,
			src_frame->inherit_super.height,
			src_frame->inherit_super.inherit_super.data,
			src_frame->inherit_super.inherit_super.buffer_size,
			false,
			src_frame->inherit_super.inherit_super.pts,
			src_frame->inherit_super.inherit_super.timebase);
	if(h265 != NULL) {
		h265->inherit_super.inherit_super.id = src_frame->inherit_super.inherit_super.id;
	}
	return h265;
}

bool TT_ATTRIBUTE_API ttLibC_H265_getNalInfo(
		ttLibC_H265_NalInfo* info,
		uint8_t *data,
		size_t data_size) {
	if(info == NULL) {
		return false;
	}
	if(data_size == 0) {
		return false;
	}
	uint8_t *dat = data;
	info->data_pos = 0;
	info->nal_unit_type = H265NalType_error;
	info->nal_size = 0;
	size_t pos = 0;
	for(size_t i = 0;i < data_size;++ i, ++ data) {
		if(i == 0 && (*data) != 0) {
			return false;
		}
		if((*data) != 0) {
			if(i - pos == 2 || i - pos == 3) {
				if((*data) == 1) {
					if(info->nal_unit_type != H265NalType_error) {
						info->nal_size = pos;
						break;
					}
					info->data_pos = i + 1;
				}
			}
			else if(info->nal_unit_type == H265NalType_error && info->data_pos != 0) {
				dat += info->data_pos;
				if(((*dat) & 0x80) != 0) {
					ERR_PRINT("forbidden zero bit is not zero.");
					return false;
				}
				info->nal_unit_type = ((*dat) >> 1) & 0x3F;
				info->is_disposable = false;
				switch(info->nal_unit_type) {
				case H265NalType_trailN:
				case H265NalType_tsaN:
				case H265NalType_stsaN:
				case H265NalType_radlN:
				case H265NalType_raslN:
				case H265NalType_rsvVclN10:
				case H265NalType_rsvVclN12:
				case H265NalType_rsvVclN14:
					info->is_disposable = true;
					/* no break */
				case H265NalType_trailR:
				case H265NalType_tsaR:
				case H265NalType_stsaR:
				case H265NalType_radlR:
				case H265NalType_raslR:
				case H265NalType_rsvVclR11:
				case H265NalType_rsvVclR13:
				case H265NalType_rsvVclR15:
					{
						ttLibC_ByteReader *reader = ttLibC_ByteReader_make(dat + 2, data_size - info->data_pos, ByteUtilType_h26x);
						/*uint32_t first_slice_segment_in_pic_flag = */ttLibC_ByteReader_bit(reader, 1);
						/*uint32_t slice_pic_parameter_set_id = */ttLibC_ByteReader_expGolomb(reader, false);
						info->frame_type = ttLibC_ByteReader_expGolomb(reader, false);
						ttLibC_ByteReader_close(&reader);
					}
					break;
					// idr?
				case H265NalType_blaWLp:
				case H265NalType_blaWRadl:
				case H265NalType_blaNLp:
				case H265NalType_idrWRadl:
				case H265NalType_idrNLp:
				case H265NalType_craNut:
					info->frame_type = H265FrameType_I;
					break;
				default:
					info->frame_type = H265FrameType_unknown;
					break;
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

bool TT_ATTRIBUTE_API ttLibC_H265_getHvccInfo(ttLibC_H265_NalInfo* info, uint8_t *data, size_t data_size) {
	if(info == NULL) {
		return false;
	}
	if(data_size < 5) {
		return false;
	}
	info->data_pos = 4;
	info->nal_unit_type = H265NalType_error;
	info->nal_size = ((data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3]) + 4;
	data += 4;
//	size_t pos = 0;
	if(data_size < info->nal_size) {
		// not enough data size
		return false;
	}
	if(((*data) & 0x80) != 0) {
		ERR_PRINT("forbidden zero bit is not zero.");
		return false;
	}
	info->nal_unit_type = ((*data) >> 1) & 0x3F;
	return true;
}
bool TT_ATTRIBUTE_API ttLibC_H265_getHvccInfo_ex(
		ttLibC_H265_NalInfo* info,
		uint8_t *data,
		size_t data_size,
		uint32_t length_size) {
	if(info == NULL) {
		return false;
	}
	if(data_size < length_size +1) {
		return false;
	}
	info->data_pos = length_size;
	info->nal_unit_type = H265NalType_error;
	info->nal_size = 0;
	for(uint32_t i = 0;i < length_size;++ i) {
		info->nal_size = (info->nal_size << 8) | *data;
		++ data;
	}
	if(data_size < info->nal_size) {
		return false;
	}
	if(((*data) & 0x80) != 0) {
		ERR_PRINT("forbidden zero bit is not zero.");
		return false;
	}
	info->nal_unit_type = ((*data) >> 1) & 0x3F;
	info->is_disposable = false;
	switch(info->nal_unit_type) {
	case H265NalType_trailN:
	case H265NalType_tsaN:
	case H265NalType_stsaN:
	case H265NalType_radlN:
	case H265NalType_raslN:
	case H265NalType_rsvVclN10:
	case H265NalType_rsvVclN12:
	case H265NalType_rsvVclN14:
		info->is_disposable = true;
		/* no break */
	case H265NalType_trailR:
	case H265NalType_tsaR:
	case H265NalType_stsaR:
	case H265NalType_radlR:
	case H265NalType_raslR:
	case H265NalType_rsvVclR11:
	case H265NalType_rsvVclR13:
	case H265NalType_rsvVclR15:
		{
			ttLibC_ByteReader *reader = ttLibC_ByteReader_make(data + 2, data_size - 2, ByteUtilType_h26x);
			/*uint32_t first_slice_segment_in_pic_flag = */ttLibC_ByteReader_bit(reader, 1);
			/*uint32_t slice_pic_parameter_set_id = */ttLibC_ByteReader_expGolomb(reader, false);
			info->frame_type = ttLibC_ByteReader_expGolomb(reader, false);
			ttLibC_ByteReader_close(&reader);
		}
		break;
	case H265NalType_blaWLp:
	case H265NalType_blaWRadl:
	case H265NalType_blaNLp:
	case H265NalType_idrWRadl:
	case H265NalType_idrNLp:
	case H265NalType_craNut:
		info->frame_type = H265FrameType_I;
		break;
	default:
		info->frame_type = H265FrameType_unknown;
		break;
	}
	return true;
}

bool TT_ATTRIBUTE_API ttLibC_H265_isNal(uint8_t *data, size_t data_size) {
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

bool TT_ATTRIBUTE_API ttLibC_H265_isHvcc(uint8_t *data, size_t data_size) {
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

bool TT_ATTRIBUTE_API ttLibC_H265_isKey(uint8_t *data, size_t data_size) {
	ttLibC_H265_NalInfo info;
	if(ttLibC_H265_isHvcc(data, data_size)) {
		if(!ttLibC_H265_getHvccInfo(&info, data, data_size)) {
			return false;
		}
	}
	else if(ttLibC_H265_isNal(data, data_size)) {
		if(!ttLibC_H265_getNalInfo(&info, data, data_size)) {
			return false;
		}
	}
	switch(info.nal_unit_type) {
	case H265NalType_idrNLp:
	case H265NalType_idrWRadl:
		return true;
	default:
		return false;
	}
}

static Error_e H265_analyzeSPSNut(
		ttLibC_H265_Ref_t *ref,
		uint8_t *data,
		size_t data_size) {
//	LOG_DUMP(data, data_size, true);
	// try to analyze.
	ttLibC_ByteReader *reader = ttLibC_ByteReader_make(data, data_size, ByteUtilType_h26x);
	// for bidden 1 bit sync用
	ttLibC_ByteReader_bit(reader, 1);
	// nal type
	ttLibC_ByteReader_bit(reader, 6);
	// nuh_layer_id (this should be 0?)
	ttLibC_ByteReader_bit(reader, 6);
	// nuh temporal id + 1
	ttLibC_ByteReader_bit(reader, 3);

	// sps Video Parameter set id
	ttLibC_ByteReader_bit(reader, 4);
	// sps max sublayers - 1
	ttLibC_ByteReader_bit(reader, 3);
	// sps temporal id nesting flag.
	ttLibC_ByteReader_bit(reader, 1);
	// profile tier level
		// general profile space
		ttLibC_ByteReader_bit(reader, 2);
		// general tier flag
		ttLibC_ByteReader_bit(reader, 1);
		// general profile idc
		ttLibC_ByteReader_bit(reader, 5);
		// general profile compatibility flags (32 information)
		ttLibC_ByteReader_bit(reader, 32);
		// general progressive source flag
		ttLibC_ByteReader_bit(reader, 1);
		// general interlaced source flag
		ttLibC_ByteReader_bit(reader, 1);
		// general non packed constraint flag
		ttLibC_ByteReader_bit(reader, 1);
		// general frame only constraint flag
		ttLibC_ByteReader_bit(reader, 1);
		// genral reservved zero 44bit
		ttLibC_ByteReader_bit(reader, 44);
		// general level idc
		ttLibC_ByteReader_bit(reader, 8);

	// sps seq parameter set id
	ttLibC_ByteReader_expGolomb(reader, false);
	// chroma format idc
	uint32_t chroma_format_idc = ttLibC_ByteReader_expGolomb(reader, false);
	if(chroma_format_idc == 3) {
		// separate colour plane flag
		ttLibC_ByteReader_bit(reader, 1);
	}
	uint32_t width = ttLibC_ByteReader_expGolomb(reader, false);
	uint32_t height = ttLibC_ByteReader_expGolomb(reader, false);
	ref->width = width;
	ref->height = height;
	ttLibC_ByteReader_close(&reader);
	return 0;
}

uint32_t TT_ATTRIBUTE_API ttLibC_H265_getWidth(ttLibC_H265 *prev_frame, uint8_t *data, size_t data_size) {
	uint32_t width = 0;
	if(prev_frame != 0) {
		width = prev_frame->inherit_super.width;
	}
	if(width != 0) {
		return width;
	}
	ttLibC_H265_NalInfo info;
	uint8_t *check_data = data;
	size_t check_size = data_size;
	ttLibC_H265_Ref_t ref;
	ref.width = 0;
	ref.height = 0;
	ref.analyze_data_ptr = NULL;
	if(ttLibC_H265_isHvcc(check_data, check_size)) {
		do {
			if(!ttLibC_H265_getHvccInfo(&info, check_data, check_size)) {
				return 0;
			}
			if(info.nal_unit_type == H265NalType_spsNut) {
				H265_analyzeSPSNut(
						&ref,
						check_data + info.data_pos,
						info.nal_size - info.data_pos);
				return ref.width;
			}
			check_data += info.nal_size;
			check_size -= info.nal_size;
		} while(check_size > 0);
	}
	else if(ttLibC_H265_isNal(check_data, check_size)) {
		do {
			if(!ttLibC_H265_getNalInfo(&info, check_data, check_size)) {
				return 0;
			}
			if(info.nal_unit_type ==H265NalType_spsNut) {
				H265_analyzeSPSNut(
						&ref,
						check_data + info.data_pos,
						info.nal_size - info.data_pos);
				return ref.width;
			}
			check_data += info.nal_size;
			check_size -= info.nal_size;
		} while(check_size > 0);
	}
	return 0;
}

uint32_t TT_ATTRIBUTE_API ttLibC_H265_getHeight(ttLibC_H265 *prev_frame, uint8_t *data, size_t data_size) {
	uint32_t height = 0;
	if(prev_frame != 0) {
		height = prev_frame->inherit_super.height;
	}
	if(height != 0) {
		return height;
	}
	ttLibC_H265_NalInfo info;
	uint8_t *check_data = data;
	size_t check_size = data_size;
	ttLibC_H265_Ref_t ref;
	ref.width = 0;
	ref.height = 0;
	ref.analyze_data_ptr = NULL;
	if(ttLibC_H265_isHvcc(check_data, check_size)) {
		do {
			if(!ttLibC_H265_getHvccInfo(&info, check_data, check_size)) {
				return 0;
			}
			if(info.nal_unit_type == H265NalType_spsNut) {
				H265_analyzeSPSNut(
						&ref,
						check_data + info.data_pos,
						info.nal_size - info.data_pos);
				return ref.height;
			}
			check_data += info.nal_size;
			check_size -= info.nal_size;
		} while(check_size > 0);
	}
	else if(ttLibC_H265_isNal(check_data, check_size)) {
		do {
			if(!ttLibC_H265_getNalInfo(&info, check_data, check_size)) {
				return 0;
			}
			if(info.nal_unit_type ==H265NalType_spsNut) {
				H265_analyzeSPSNut(
						&ref,
						check_data + info.data_pos,
						info.nal_size - info.data_pos);
				return ref.height;
			}
			check_data += info.nal_size;
			check_size -= info.nal_size;
		} while(check_size > 0);
	}
	return 0;
}

ttLibC_H265 TT_ATTRIBUTE_API *ttLibC_H265_getFrame(
		ttLibC_H265 *prev_frame,
		uint8_t *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	// analyze data as nal. annex B.
	ttLibC_H265_NalInfo nal_info;
	if(!ttLibC_H265_getNalInfo(&nal_info, data, data_size)) {
		ERR_PRINT("failed to get nal info.");
		return NULL;
	}
	uint32_t width = 0, height = 0;
	width = ttLibC_H265_getWidth(prev_frame, data, data_size);
	height = ttLibC_H265_getHeight(prev_frame, data, data_size);
	switch(nal_info.nal_unit_type) {
	case H265NalType_trailN:
	case H265NalType_trailR:
		return ttLibC_H265_make(
				prev_frame,
				H265Type_slice,
				width,
				height,
				data,
				data_size,
				non_copy_mode,
				pts,
				timebase);
//	case H265NalType_tsaN:
//	case H265NalType_tsaR:
//	case H265NalType_stsaN:
//	case H265NalType_stsaR:
//	case H265NalType_radlN:
//	case H265NalType_radlR:
	case H265NalType_raslN:
	case H265NalType_raslR:
		return ttLibC_H265_make(
				prev_frame,
				H265Type_slice,
				width,
				height,
				data,
				data_size,
				non_copy_mode,
				pts,
				timebase);
//	case H265NalType_rsvVclN10:
//	case H265NalType_rsvVclR11:
//	case H265NalType_rsvVclN12:
//	case H265NalType_rsvVclR13:
//	case H265NalType_rsvVclN14:
//	case H265NalType_rsvVclR15:

	// keyFrame begin?
//	case H265NalType_blaWLp:
//	case H265NalType_blaWRadl:
//	case H265NalType_blaNLp:
	case H265NalType_idrWRadl:
		return ttLibC_H265_make(
				prev_frame,
				H265Type_sliceIDR,
				width,
				height,
				data,
				data_size,
				non_copy_mode,
				pts,
				timebase);
//	case H265NalType_idrNLp:
	case H265NalType_craNut:
		return ttLibC_H265_make(
				prev_frame,
				H265Type_sliceIDR,
				width,
				height,
				data,
				data_size,
				non_copy_mode,
				pts,
				timebase);
		// -- keyFrame end?
//	case H265NalType_rsvIrapVcl22:
//	case H265NalType_rsvIrapVcl22:
//	case H265NalType_rsvVcl24:
//	case H265NalType_rsvVcl25:
//	case H265NalType_rsvVcl26:
//	case H265NalType_rsvVcl27:
//	case H265NalType_rsvVcl28:
//	case H265NalType_rsvVcl29:
//	case H265NalType_rsvVcl30:
//	case H265NalType_rsvVcl31:

	case H265NalType_vpsNut:
	case H265NalType_spsNut:
	case H265NalType_ppsNut:
		{
			uint8_t *dat = data;
			size_t dat_size = 0;
			bool has_more = true;
			do {
				switch(nal_info.nal_unit_type) {
				case H265NalType_spsNut:
				case H265NalType_vpsNut:
				case H265NalType_ppsNut:
					dat += nal_info.nal_size;
					dat_size += nal_info.nal_size;
					break;
				default:
					// if we found anything else, stop.
					has_more = false;
					break;
				}
				if(!has_more) {
					break;
				}
				if(!ttLibC_H265_getNalInfo(&nal_info, dat, data_size - dat_size)) {
					break;
				}
			} while(true);
			if(dat_size != 0) {
				return ttLibC_H265_make(
						prev_frame,
						H265Type_configData,
						width,
						height,
						data,
						dat_size,
						non_copy_mode,
						pts,
						timebase);
			}
		}
		break;
//	case H265NalType_audNut:

//	case H265NalType_eosNut:
//	case H265NalType_eobNut:
//	case H265NalType_fdNut:
	case H265NalType_prefixSeiNut:
		return ttLibC_H265_make(
				prev_frame,
				H265Type_unknown,
				width,
				height,
				data,
				nal_info.nal_size,
				non_copy_mode,
				pts,
				timebase);
//	case H265NalType_suffixSeiNut:
//	case H265NalType_rsvNvcl41:
//	case H265NalType_rsvNvcl42:
//	case H265NalType_rsvNvcl43:
//	case H265NalType_rsvNvcl44:
//	case H265NalType_rsvNvcl45:
//	case H265NalType_rsvNvcl46:
//	case H265NalType_rsvNvcl47:
//	case H265NalType_unspec48:
//	case H265NalType_unspec49:
//	case H265NalType_unspec50:
//	case H265NalType_unspec51:
//	case H265NalType_unspec52:
//	case H265NalType_unspec53:
//	case H265NalType_unspec54:
//	case H265NalType_unspec55:
//	case H265NalType_unspec56:
//	case H265NalType_unspec57:
//	case H265NalType_unspec58:
//	case H265NalType_unspec59:
//	case H265NalType_unspec60:
//	case H265NalType_unspec61:
//	case H265NalType_unspec62:
//	case H265NalType_unspec63:
	default:
		ERR_PRINT("found unknown nal:%d", nal_info.nal_unit_type);
		return NULL;
	}
	return NULL;
}

ttLibC_H265 TT_ATTRIBUTE_API *ttLibC_H265_analyzeHvccTag(
		ttLibC_H265 *prev_frame,
		uint8_t *data,
		size_t data_size,
		uint32_t *length_size) {
	size_t buffer_size = data_size;
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
	size_t buf_size = 0;
	/*
	 * 1byte version
	 * from vps (sps could have version info, however, sometimes missing in sps.)
	 * 2bit general profile space
	 * 1bit general tier flag
	 * 5bit general profile idc
	 * 4byte general profile compatibility flags
	 * 6btye general constraint indicator flags
	 * 1byte general level idc
	 * 4bit reserved
	 * 12bit spatial segmentation idc(...)
	 * 6bit reserved
	 * 2bit parallelism type.(...)
	 * from sps
	 *
	 * 6bit reserved
	 * 2bit chroma format.
	 *
	 * 5bit reserved
	 * 3bit bit depth luma minux 8
	 *
	 * 5bit reserved
	 * 3bit bit depth chroma minus 8
	 *
	 * 2byte framerate(...)
	 * 2bit constant frame rate(...)
	 * 3bit num temporal layers(from vps)
	 * 1bit temporal id nested(from vps)
	 * 2bit length size minus 1(should be 3, I want 4byte length size.)
	 * 1byte (element num)
	 *
	 * loop for each nal
	 *  1bit array complete ness.
	 *  1bit reserved(0 fixed)
	 *  6bit nalUnitType
	 *  2byte nal count.
	 *  2byte nal unit size.
	 *  nbyte data.
	 */
	if(data[0] != 1) {
		ERR_PRINT("hvcc version is not 1.");
	}
	*length_size = (data[21] & 0x03) + 1;
	uint32_t nal_count = data[22];
	uint8_t *dat = data;
	dat += 23;
	for(uint32_t i = 0;i < nal_count;++ i) {
		ttLibC_H265_NalType nal_type = dat[0] & 0x3F;
		uint32_t nal_count = (dat[1] << 8) | dat[2];
		if(nal_count != 1) {
			ERR_PRINT("warning, find nal count is not 1, I need to check data.");
		}
		uint32_t nal_size  = (dat[3] << 8) | dat[4];
		switch(nal_type) {
		case H265NalType_spsNut:
		case H265NalType_ppsNut:
		case H265NalType_vpsNut:
			{
				buf[0] = 0x00;
				buf[1] = 0x00;
				buf[2] = 0x00;
				buf[3] = 0x01;
				memcpy(buf + 4, dat + 5, nal_size);
				buf += 4 + nal_size;
				buf_size += 4 + nal_size;
			}
			break;
		default:
			// skip frames not for configData.
			break;
		}
		dat += 5 + nal_size;
	}
//	LOG_DUMP(buffer, buf_size, true);
	// now make frame.
	ttLibC_H265 *h265 = ttLibC_H265_getFrame(
			prev_frame,
			buffer,
			buf_size,
			true,
			0,
			1000);
	if(h265 == NULL) {
		if(alloc_flag) {
			ttLibC_free(buffer);
		}
		return NULL;
	}
	h265->inherit_super.inherit_super.data_size = buffer_size;
	h265->inherit_super.inherit_super.is_non_copy = false;
	// done.
	return h265;
}

uint32_t TT_ATTRIBUTE_API ttLibC_H265_getConfigCrc32(ttLibC_H265 *h265) {
	if(h265->type != H265Type_configData) {
		return 0;
	}
	ttLibC_Crc32 *crc32 = ttLibC_Crc32_make(0);
	uint8_t *data = h265->inherit_super.inherit_super.data;
	for(uint32_t i = 0;i < h265->inherit_super.inherit_super.buffer_size;++ i) {
		ttLibC_Crc32_update(crc32, *data);
		++ data;
	}
	uint32_t value = ttLibC_Crc32_getValue(crc32);
	ttLibC_Crc32_close(&crc32);
	return value;
}

size_t TT_ATTRIBUTE_API ttLibC_H265_readHvccTag(
		ttLibC_H265 *h265,
		void *data,
		size_t data_size) {
	if(h265->type != H265Type_configData) {
		return 0;
	}
	ttLibC_H265_NalInfo nal_info;
	uint8_t *buf = h265->inherit_super.inherit_super.data;
	size_t buf_size = h265->inherit_super.inherit_super.buffer_size;
	// from vps
	uint8_t  generalInfo[12] = {0};
	uint32_t num_temporal_layers = 1;
	uint32_t temporal_id_nested_flag = 1;

	// from sps
	uint32_t chroma_idc = 0;
	uint32_t bitdepth_luna_minus8 = 0;
	uint32_t bitdepth_chroma_minus8 = 0;

	uint32_t nal_count = 0;

	while(ttLibC_H265_getNalInfo(&nal_info, buf, buf_size)) {
		nal_count ++;
		switch(nal_info.nal_unit_type) {
		case H265NalType_vpsNut:
			{
				ttLibC_ByteReader *reader = ttLibC_ByteReader_make(buf + nal_info.data_pos, nal_info.nal_size - nal_info.data_pos, ByteUtilType_h26x);
				ttLibC_ByteReader_bit(reader, 28); // 16bit + 12bit
				num_temporal_layers = ttLibC_ByteReader_bit(reader, 3) + 1;
				temporal_id_nested_flag = ttLibC_ByteReader_bit(reader, 1);
				ttLibC_ByteReader_bit(reader, 16);
				for(int i = 0;i < 12;++ i) {
					generalInfo[i] = ttLibC_ByteReader_bit(reader, 8);
				}
				ttLibC_ByteReader_close(&reader);
			}
			break;
		case H265NalType_spsNut:
			{
				ttLibC_ByteReader *reader = ttLibC_ByteReader_make(buf + nal_info.data_pos, nal_info.nal_size - nal_info.data_pos, ByteUtilType_h26x);
				// forbidden - temporal id nesting flag
				ttLibC_ByteReader_bit(reader, 24);
				// tier level profile
				ttLibC_ByteReader_bit(reader, 8);
				ttLibC_ByteReader_bit(reader, 32);
				ttLibC_ByteReader_bit(reader, 16);
				ttLibC_ByteReader_bit(reader, 32);
				ttLibC_ByteReader_bit(reader, 8);
				// sps seq parameter set id
				ttLibC_ByteReader_expGolomb(reader, false);
				chroma_idc = ttLibC_ByteReader_expGolomb(reader, false);
				if(chroma_idc == 3) {
					ttLibC_ByteReader_bit(reader, 1);
				}
				// width height
				uint32_t width, height;
				width = ttLibC_ByteReader_expGolomb(reader, false);
				height = ttLibC_ByteReader_expGolomb(reader, false);
				if(ttLibC_ByteReader_bit(reader, 1) == 1) {
					ttLibC_ByteReader_expGolomb(reader, false);
					ttLibC_ByteReader_expGolomb(reader, false);
					ttLibC_ByteReader_expGolomb(reader, false);
					ttLibC_ByteReader_expGolomb(reader, false);
				}
				bitdepth_luna_minus8 = ttLibC_ByteReader_expGolomb(reader, false);
				bitdepth_chroma_minus8 = ttLibC_ByteReader_expGolomb(reader, false);
				ttLibC_ByteReader_close(&reader);
			}
			break;
		default:
			break;
		}
		buf += nal_info.nal_size;
		buf_size -= nal_info.nal_size;
	}
	ttLibC_ByteConnector *connector = ttLibC_ByteConnector_make(data, data_size, ByteUtilType_default);
	// start with 01
	ttLibC_ByteConnector_bit(connector, 1, 8);
	// TODO binary data from vps(4 - 16byte(however, remove nal syntax like 00 00 03))  
	ttLibC_ByteConnector_string(connector, (const char *)generalInfo, 12);
	// F0 00
	ttLibC_ByteConnector_bit(connector, 0xF000, 16);
	// FC
	ttLibC_ByteConnector_bit(connector, 0xFC, 8);
	// 6bit chroma
	ttLibC_ByteConnector_bit(connector, 0x3F, 6);
	ttLibC_ByteConnector_bit(connector, chroma_idc & 0x3, 2);
	// 5bit bitdepth luma minus 8
	ttLibC_ByteConnector_bit(connector, 0x1F, 5);
	ttLibC_ByteConnector_bit(connector, bitdepth_luna_minus8 & 0x7, 3);
	// 5bit bitdepth chroa minus 8
	ttLibC_ByteConnector_bit(connector, 0x1F, 5);
	ttLibC_ByteConnector_bit(connector, bitdepth_chroma_minus8 & 0x7, 3);
	// 00 00
	ttLibC_ByteConnector_bit(connector, 0x0000, 16);
	// 2bit 00
	ttLibC_ByteConnector_bit(connector, 0x0, 2);
	//  3bit temporal layers(if not 1, be careful)
	ttLibC_ByteConnector_bit(connector, num_temporal_layers, 3);
	// temporal id nested(1?)
	ttLibC_ByteConnector_bit(connector, temporal_id_nested_flag, 1);
	// 2bit 11(size length = 4byte, I want to use this always.)
	ttLibC_ByteConnector_bit(connector, 3, 2);
	ttLibC_ByteConnector_bit(connector, nal_count, 8);

	buf = h265->inherit_super.inherit_super.data;
	buf_size = h265->inherit_super.inherit_super.buffer_size;
	while(ttLibC_H265_getNalInfo(&nal_info, buf, buf_size)) {
		nal_count ++;
		ttLibC_ByteConnector_bit(connector, nal_info.nal_unit_type, 8);
		ttLibC_ByteConnector_bit(connector, 1, 16);
		ttLibC_ByteConnector_bit(connector, nal_info.nal_size - nal_info.data_pos, 16);
		ttLibC_ByteConnector_string(connector, (const char *)(buf + nal_info.data_pos), nal_info.nal_size - nal_info.data_pos);
		buf += nal_info.nal_size;
		buf_size -= nal_info.nal_size;
	}
	uint32_t write_size = connector->write_size;
	ttLibC_ByteConnector_close(&connector);
	return write_size;
}

void TT_ATTRIBUTE_API ttLibC_H265_close(ttLibC_H265 **frame) {
	ttLibC_H265 *target = *frame;
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.inherit_super.type != frameType_h265) {
		ERR_PRINT("found non h265 frame in h265_close.");
		return;
	}
	if(!target->inherit_super.inherit_super.is_non_copy) {
		ttLibC_free(target->inherit_super.inherit_super.data);
	}
	ttLibC_free(target);
	*frame = NULL;
}

