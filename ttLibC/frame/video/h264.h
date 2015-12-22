/**
 * @file   h264.h
 * @brief  h264 image frame information
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/24
 */

#ifndef TTLIBC_FRAME_VIDEO_H264_H_
#define TTLIBC_FRAME_VIDEO_H264_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "video.h"

/**
 * h264 nal type.
 */
typedef enum ttLibC_H264_NalType {
	H264NalType_unspecified1                       = 0x00,
	H264NalType_slice                              = 0x01,
	H264NalType_sliceDataPartitionA                = 0x02,
	H264NalType_sliceDataPartitionB                = 0x03,
	H264NalType_sliceDataPartitionC                = 0x04,
	H264NalType_sliceIDR                           = 0x05,
	H264NalType_supplementalEnhancementInformation = 0x06,
	H264NalType_sequenceParameterSet               = 0x07,
	H264NalType_pictureParameterSet                = 0x08,
	H264NalType_accessUnitDelimiter                = 0x09,
	H264NalType_endOfSequence                      = 0x0A,
	H264NalType_endOfStream                        = 0x0B,
	H264NalType_filterData                         = 0x0C,
	H264NalType_sequenceParameterSetExtension      = 0x0D,
	H264NalType_prefixNalUnit                      = 0x0E,
	H264NalType_subsetSequenceParameterSet         = 0x0F,
	H264NalType_reserved1                          = 0x10,
	H264NalType_reserved2                          = 0x11,
	H264NalType_reserved3                          = 0x12,
	H264NalType_codedSliceAuxiliary                = 0x13,
	H264NalType_codedSliceExtension                = 0x14,
	H264NalType_codedSliceForDepthView             = 0x15,
	H264NalType_reserved4                          = 0x16,
	H264NalType_reserved5                          = 0x17,
	H264NalType_unspecified2                       = 0x18,
	H264NalType_unspecified3                       = 0x19,
	H264NalType_unspecified4                       = 0x1A,
	H264NalType_unspecified5                       = 0x1B,
	H264NalType_unspecified6                       = 0x1C,
	H264NalType_unspecified7                       = 0x1D,
	H264NalType_unspecified8                       = 0x1E,
	H264NalType_unspecified9                       = 0x1F,

	H264NalType_error = 0xFF,
} ttLibC_H264_NalType;

/**
 * h264 object type.
 */
typedef enum ttLibC_H264_Type {
	/** sps and pps(spsext also?) */
	H264Type_configData,
	/** sliceIdr frame data. */
	H264Type_sliceIDR,
	/** slice frame data. */
	H264Type_slice,

	/** for unknown data. */
	H264Type_unknown = 0xFF,
} ttLibC_H264_Type;

/**
 * h264 nalinfo definition.
 * for analyze result.
 */
typedef struct ttLibC_H264_NalInfo {
	/* nal start position of buffer. now it's always 0, so removed. */
//	size_t pos;
	/** data_pos from buffer start pos. point the xx(00 00 01 xx) */
	size_t data_pos;
	/** nal unit type */
	ttLibC_H264_NalType nal_unit_type;
	/** size of nal */
	size_t nal_size;
} ttLibC_H264_NalInfo;

/**
 * h264 frame definition
 */
typedef struct ttLibC_Frame_Video_H264 {
	/** inherit data from ttLibC_Video */
	ttLibC_Video inherit_super;
	/** frame type */
	ttLibC_H264_Type type;
} ttLibC_Frame_Video_H264;

typedef ttLibC_Frame_Video_H264 ttLibC_H264;

/**
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
		uint32_t timebase);

/**
 * make clone frame.
 * always make copy buffer on it.
 * @param prev_frame reuse frame object.
 * @param src_frame  source of clone.
 */
ttLibC_H264 *ttLibC_H264_clone(
		ttLibC_H264 *prev_frame,
		ttLibC_H264 *src_frame);

/**
 * analyze info of one nal.
 * @param info      pointer for info data.(update with data.)
 * @param data      data for analyze
 * @param data_size data size
 * @return true:analyze success
 */
bool ttLibC_H264_getNalInfo(ttLibC_H264_NalInfo* info, uint8_t *data, size_t data_size);

/**
 * analyez info of one nal(for avcc data).
 * @param info      pointer for info data.(update with data.)
 * @param data      data for analyze
 * @param data_size data size
 * @return true:analyze success
 */
bool ttLibC_H264_getAvccInfo(ttLibC_H264_NalInfo* info, uint8_t *data, size_t data_size);

/**
 * check data type is nal or not.
 */
bool ttLibC_H264_isNal(uint8_t *data, size_t data_size);

/**
 * check data type is avcc or not.
 */
bool ttLibC_H264_isAvcc(uint8_t *data, size_t data_size);

/**
 * check data which is key or not.
 * @param data      target data
 * @param data_size target data size
 * @return true: data is key frame. false: not key frame.
 */
bool ttLibC_H264_isKey(uint8_t *data, size_t data_size);

/**
 * analyze width from data.
 * @param prev_frame ref for frame before make.
 * @param data       target data
 * @param data_size  target data size
 * @return width siz
 */
uint32_t ttLibC_H264_getWidth(ttLibC_H264 *prev_frame, uint8_t *data, size_t data_size);

/**
 * analyze height from data.
 * @param prev_frame ref for frame before make.
 * @param data       target data
 * @param data_size  target data size
 * @return height size
 */
uint32_t ttLibC_H264_getHeight(ttLibC_H264 *prev_frame, uint8_t *data, size_t data_size);

/**
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
		uint32_t timebase);

/**
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
		uint32_t *length_size);

/**
 * calcurate crc32 value for configdata.
 * @param h264 target h264 object
 * @return value of crc32. 0 for error.
 */
uint32_t ttLibC_H264_getConfigCrc32(ttLibC_H264 *h264);

/**
 * read avcc buffer for config data.
 * @param h264      target h264 object. this must be config data.
 * @param data      buffer to put data.
 * @param data_size buffer size.
 * @return write size. 0 for error.
 */
size_t ttLibC_H264_readAvccTag(
		ttLibC_H264 *h264,
		void *data,
		size_t data_size);

/**
 * close frame
 * @param frame
 */
void ttLibC_H264_close(ttLibC_H264 **frame);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_FRAME_VIDEO_H264_H_ */
