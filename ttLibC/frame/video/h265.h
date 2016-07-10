/**
 * @file   h265.h
 * @brief  h265 image frame information
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/06
 */

#ifndef TTLIBC_FRAME_VIDEO_H265_H_
#define TTLIBC_FRAME_VIDEO_H265_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "video.h"

/**
 * h265 nal type.
 */
typedef enum ttLibC_H265_NalType {
	H265NalType_trailN       = 0x00,
	H265NalType_trailR       = 0x01,
	H265NalType_tsaN         = 0x02,
	H265NalType_tsaR         = 0x03,
	H265NalType_stsaN        = 0x04,
	H265NalType_stsaR        = 0x05,
	H265NalType_radlN        = 0x06,
	H265NalType_radlR        = 0x07,
	H265NalType_raslN        = 0x08,
	H265NalType_raslR        = 0x09,
	H265NalType_rsvVclN10    = 0x0A,
	H265NalType_rsvVclR11    = 0x0B,
	H265NalType_rsvVclN12    = 0x0C,
	H265NalType_rsvVclR13    = 0x0D,
	H265NalType_rsvVclN14    = 0x0E,
	H265NalType_rsvVclR15    = 0x0F,
	H265NalType_blaWLp       = 0x10,
	H265NalType_blaWRadl     = 0x11,
	H265NalType_blaNLp       = 0x12,
	H265NalType_idrWRadl     = 0x13,
	H265NalType_idrNLp       = 0x14,
	H265NalType_craNut       = 0x15,
	H265NalType_rsvIrapVcl22 = 0x16,
	H265NalType_rsvIrapVcl23 = 0x17,
	H265NalType_rsvVcl24     = 0x18,
	H265NalType_rsvVcl25     = 0x19,
	H265NalType_rsvVcl26     = 0x1A,
	H265NalType_rsvVcl27     = 0x1B,
	H265NalType_rsvVcl28     = 0x1C,
	H265NalType_rsvVcl29     = 0x1D,
	H265NalType_rsvVcl30     = 0x1E,
	H265NalType_rsvVcl31     = 0x1F,
	H265NalType_vpsNut       = 0x20,
	H265NalType_spsNut       = 0x21,
	H265NalType_ppsNut       = 0x22,
	H265NalType_audNut       = 0x23,
	H265NalType_eosNut       = 0x24,
	H265NalType_eobNut       = 0x25,
	H265NalType_fdNut        = 0x26,
	H265NalType_prefixSeiNut = 0x27,
	H265NalType_suffixSeiNut = 0x28,
	H265NalType_rsvNvcl41    = 0x29,
	H265NalType_rsvNvcl42    = 0x2A,
	H265NalType_rsvNvcl43    = 0x2B,
	H265NalType_rsvNvcl44    = 0x2C,
	H265NalType_rsvNvcl45    = 0x2D,
	H265NalType_rsvNvcl46    = 0x2E,
	H265NalType_rsvNvcl47    = 0x2F,
	H265NalType_unspeC48     = 0x30,
	H265NalType_unspec49     = 0x31,
	H265NalType_unspec50     = 0x32,
	H265NalType_unspec51     = 0x33,
	H265NalType_unspec52     = 0x34,
	H265NalType_unspec53     = 0x35,
	H265NalType_unspec54     = 0x36,
	H265NalType_unspec55     = 0x37,
	H265NalType_unspec56     = 0x38,
	H265NalType_unspec57     = 0x39,
	H265NalType_unspec58     = 0x3A,
	H265NalType_unspec59     = 0x3B,
	H265NalType_unspec60     = 0x3C,
	H265NalType_unspec61     = 0x3D,
	H265NalType_unspec62     = 0x3E,
	H265NalType_unspec63     = 0x3F,

	H265NalType_error = 0xFF,
} ttLibC_H265_NalType;

/**
 * h265 object type
 */
typedef enum ttLibC_H265_Type {
	/** sps pps vps chunk */
	H265Type_configData,
	/** idr nut */
	H265Type_sliceIDR,
	/** trail nut? something for inner frame. */
	H265Type_slice,

	/** for unknown data. */
	H265Type_unknown = 0xFF,
} ttLibC_H265_Type;

/**
 * h265 nalinfo definition.
 * for analyze result.
 */
typedef struct ttLibC_H265_NalInfo {
	/** data_pos from buffer start pos */
	size_t data_pos;
	ttLibC_H265_NalType nal_unit_type;
	size_t nal_size; // size of nal(include 00 00 01 or avcc size data.)
} ttLibC_H265_NalInfo;

/**
 * h265 frame definition.
 */
typedef struct ttLibC_Frame_Video_H265 {
	/** inherit data from ttLibC_Video */
	ttLibC_Video inherit_super;
	/** frame type */
	ttLibC_H265_Type type;
} ttLibC_Frame_Video_H265;

typedef ttLibC_Frame_Video_H265 ttLibC_H265;

ttLibC_H265 *ttLibC_H265_make(
		ttLibC_H265 *prev_frame,
		ttLibC_H265_Type type,
		uint32_t width,
		uint32_t height,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase);

bool ttLibC_H265_getNalInfo(ttLibC_H265_NalInfo* info, uint8_t *data, size_t data_size);

bool ttLibC_H265_getAvccInfo(ttLibC_H265_NalInfo* info, uint8_t *data, size_t data_size);

bool ttLibC_H265_isNal(uint8_t *data, size_t data_size);

bool ttLibC_H265_isAvcc(uint8_t *data, size_t data_size);

bool ttLibC_H265_isKey(uint8_t *data, size_t data_size);

uint32_t ttLibC_H265_getWidth(ttLibC_H265 *prev_frame, uint8_t *data, size_t data_size);

uint32_t ttLibC_H265_getHeight(ttLibC_H265 *prev_frame, uint8_t *data, size_t data_size);

ttLibC_H265 *ttLibC_H265_getFrame(
		ttLibC_H265 *prev_frame,
		uint8_t *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase);

void ttLibC_H265_close(ttLibC_H265 **frame);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_FRAME_VIDEO_H265_H_ */
