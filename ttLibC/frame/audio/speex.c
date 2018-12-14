/*
 * @file   speex.c
 * @brief  speex frame information
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/28
 */

#include "speex.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../ttLibC_predef.h"
#include "../../_log.h"
#include "../../allocator.h"
#include "../../util/ioUtil.h"
#include "../../util/byteUtil.h"

typedef ttLibC_Frame_Audio_Speex ttLibC_Speex_;

/*
 * make speex frame
 * @param prev_frame    reuse frame.
 * @param type          type of speex
 * @param sample_rate   sample rate of data.
 * @param sample_num    sample number of data.
 * @param channel_num   channel number of data. 1:monoral 2:stereo
 * @param data          speex data
 * @param data_size     speex data size
 * @param non_copy_mode true:hold the data pointer. false:data will copy.
 * @param pts           pts for speex data.
 * @param timebase      timebase number for pts.
 * @return speex object.
 */
ttLibC_Speex TT_VISIBILITY_DEFAULT *ttLibC_Speex_make(
		ttLibC_Speex *prev_frame,
		ttLibC_Speex_Type type,
		uint32_t sample_rate,
		uint32_t sample_num,
		uint32_t channel_num,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	if(prev_frame != NULL && prev_frame->inherit_super.inherit_super.type != frameType_speex) {
		ERR_PRINT("reuse with incompatible frame.");
		return NULL;
	}
	ttLibC_Speex_ *speex = (ttLibC_Speex_ *)prev_frame;
	size_t data_size_ = data_size;
	size_t buffer_size_ = data_size;
	switch(type) {
	case SpeexType_comment:
	case SpeexType_header:
	case SpeexType_frame:
		break;
	default:
		ERR_PRINT("unknown speex type:%d", type);
		return NULL;
	}
	if(speex == NULL) {
		speex = ttLibC_malloc(sizeof(ttLibC_Speex_));
		if(speex == NULL) {
			ERR_PRINT("failed to allocate memory for speex frame.");
			return NULL;
		}
		speex->inherit_super.inherit_super.data = NULL;
	}
	else {
		if(!speex->inherit_super.inherit_super.is_non_copy) {
			if(non_copy_mode || speex->inherit_super.inherit_super.data_size < data_size) {
				ttLibC_free(speex->inherit_super.inherit_super.data);
				speex->inherit_super.inherit_super.data = NULL;
			}
			else {
				data_size_ = speex->inherit_super.inherit_super.data_size;
			}
		}
	}
	speex->type                                    = type;
	speex->inherit_super.channel_num               = channel_num;
	speex->inherit_super.sample_rate               = sample_rate;
	speex->inherit_super.sample_num                = sample_num;
	speex->inherit_super.inherit_super.buffer_size = buffer_size_;
	speex->inherit_super.inherit_super.data_size   = data_size_;
	speex->inherit_super.inherit_super.is_non_copy = non_copy_mode;
	speex->inherit_super.inherit_super.pts         = pts;
	speex->inherit_super.inherit_super.dts         = 0;
	speex->inherit_super.inherit_super.timebase    = timebase;
	speex->inherit_super.inherit_super.type        = frameType_speex;
	if(non_copy_mode) {
		speex->inherit_super.inherit_super.data = data;
	}
	else {
		if(speex->inherit_super.inherit_super.data == NULL) {
			speex->inherit_super.inherit_super.data = ttLibC_malloc(data_size);
			if(speex->inherit_super.inherit_super.data == NULL) {
				ERR_PRINT("failed to allocate memory for data.");
				if(prev_frame == NULL) {
					ttLibC_free(speex);
				}
				return NULL;
			}
		}
		memcpy(speex->inherit_super.inherit_super.data, data, data_size);
	}
	return (ttLibC_Speex *)speex;
}

/**
 * make clone frame
 * always make copy buffer on it.
 * @param prev_frame reuse frame object.
 * @param src_frame  source of clone.
 */
ttLibC_Speex TT_VISIBILITY_DEFAULT *ttLibC_Speex_clone(
		ttLibC_Speex *prev_frame,
		ttLibC_Speex *src_frame) {
	if(src_frame == NULL) {
		return NULL;
	}
	if(src_frame->inherit_super.inherit_super.type != frameType_speex) {
		ERR_PRINT("try to clone non speex frame.");
		return NULL;
	}
	if(prev_frame != NULL && prev_frame->inherit_super.inherit_super.type != frameType_speex) {
		ERR_PRINT("try to use non speex frame for reuse.");
		return NULL;
	}
	ttLibC_Speex *speex = ttLibC_Speex_make(
			prev_frame,
			src_frame->type,
			src_frame->inherit_super.sample_rate,
			src_frame->inherit_super.sample_num,
			src_frame->inherit_super.channel_num,
			src_frame->inherit_super.inherit_super.data,
			src_frame->inherit_super.inherit_super.buffer_size,
			false,
			src_frame->inherit_super.inherit_super.pts,
			src_frame->inherit_super.inherit_super.timebase);
	if(speex != NULL) {
		speex->inherit_super.inherit_super.id = src_frame->inherit_super.inherit_super.id;
	}
	return speex;
}

static int inBandSize[] = {1, 1, 4, 4, 4, 4, 4, 4, 8, 8, 16, 16, 32, 32, 64, 64};
static int narrowSize[] = {5, 43, 119, 160, 220, 300, 364, 492, 79};
static int wideSize[] = {4, 36, 112, 192, 352};
typedef enum SpeexBand {
	unknown = 0x00,
	narrow = 0x01,
	wide = 0x02,
	super_wide = 0x03
} SpeexBand;

/**
 * analyze speex band frames.
 * @param reader    target byte reader.
 * @param data_size total applyed data size.
 * @param frame_num result of contain frame #.
 * @param bandtype  result of contain speex band type.
 * @return true:ok false:error
 * should I return Error_t?
 */
static bool Speex_analyzeFrameBuffer(
		ttLibC_ByteReader *reader,
		size_t data_size,
		uint32_t *frame_num,
		SpeexBand *bandtype) {
	uint32_t readingBitSize = 0;
	*frame_num = 0;
	*bandtype = unknown;
	do {
		// check first bit to understand narrow or wide.
		if(ttLibC_ByteReader_bit(reader, 1) == 0) {
			++ *frame_num;
			if(*frame_num == 1) {
				++ *bandtype;
			}
			// next 4bit for type.
			uint32_t type = ttLibC_ByteReader_bit(reader, 4);
			if(type <= 8) {
				readingBitSize = narrowSize[type] - 5;
			}
			else if(type == 14) {
				-- *frame_num;
				uint32_t inner_type = ttLibC_ByteReader_bit(reader, 4);
				readingBitSize = inBandSize[inner_type];
			}
			else {
				*frame_num = 0;
				*bandtype = unknown;
				return false;
			}
			while(readingBitSize > 0) {
				if(readingBitSize > 32) {
					ttLibC_ByteReader_bit(reader, 32);
					readingBitSize -= 32;
				}
				else {
					ttLibC_ByteReader_bit(reader, readingBitSize);
					readingBitSize = 0;
				}
			}
		}
		else {
			if(*frame_num == 1) {
				++ *bandtype;
			}
			// next 3bit for type.
			uint32_t type = ttLibC_ByteReader_bit(reader, 3);
			if(type > 4) {
				*frame_num = 0;
				*bandtype = unknown;
				return false;
			}
			// update bit size to skip.
			readingBitSize = wideSize[type] - 4;
			while(readingBitSize > 0) {
				if(readingBitSize > 32) {
					ttLibC_ByteReader_bit(reader, 32);
					readingBitSize -= 32;
				}
				else {
					ttLibC_ByteReader_bit(reader, readingBitSize);
					readingBitSize = 0;
				}
			}
		}
		if(reader->error != Error_noError) {
			// in the case of error.
			*frame_num = 0;
			*bandtype = unknown;
			return false;
		}
	} while(reader->read_size < data_size - 1); // speex frame can be end without full byte.
	// ok
	return true;
}

/*
 * make speex frame from byte data.
 * @param prev_frame    reuse speex frame.
 * @param data          speex binary data.
 * @param data_size     data size
 * @param non_copy_mode true:hold the data pointer. false:data will copy.
 * @param pts           pts for speex frame.
 * @param timebase      timebase for speex frame.
 * @return speex object.
 */
ttLibC_Speex TT_VISIBILITY_DEFAULT *ttLibC_Speex_getFrame(
		ttLibC_Speex *prev_frame,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	if(data_size < 8) {
		return NULL;
	}
	if(prev_frame == NULL) {
		uint8_t *buf = data;
		// first frame can be speex header.
		if(buf[0] == 'S'
		&& buf[1] == 'p'
		&& buf[2] == 'e'
		&& buf[3] == 'e'
		&& buf[4] == 'x'
		&& buf[5] == ' '
		&& buf[6] == ' '
		&& buf[7] == ' ') {
			if(data_size < 0x44) {
				return NULL;
			}
			buf += 0x24;
			uint32_t *buf_int = (uint32_t *)buf;
			uint32_t sample_rate = le_uint32_t(*buf_int);
			buf_int += 3;
			uint32_t channel_num = le_uint32_t(*buf_int);
			buf_int += 2;
			uint32_t sample_num = le_uint32_t(*buf_int);
			buf_int += 2;
			sample_num *= le_uint32_t(*buf_int);
			return ttLibC_Speex_make(
					NULL,
					SpeexType_header,
					sample_rate,
					sample_num,
					channel_num,
					data,
					data_size,
					non_copy_mode,
					pts,
					timebase);
		}
		else {
			// if not header, try to use as frame data.
			ttLibC_ByteReader *reader = ttLibC_ByteReader_make(data, data_size, ByteUtilType_default);
			uint32_t frame_num;
			SpeexBand band;
			uint32_t sample_rate = 0;
			uint32_t sample_num = 0;
			if(!Speex_analyzeFrameBuffer(reader, data_size, &frame_num, &band)) {
				if((reader->error & 0x000FFFFF) != Error_NeedMoreInput) {
					ERR_PRINT("1st speex frame is unknown.");
				}
			}
			else {
				switch(band) {
				case narrow:
					sample_rate = 8000;
					break;
				case wide:
					sample_rate = 16000;
					break;
				case super_wide:
					sample_rate = 32000;
					break;
				default:
					// something wrong.
					break;
				}
				sample_num = sample_rate / 50 * frame_num;
			}
			ttLibC_ByteReader_close(&reader);
			// if sample_rate is not updated, frame is unknown.
			if(sample_rate == 0) {
				return NULL;
			}
			else {
				return ttLibC_Speex_make(
						NULL,
						SpeexType_frame,
						sample_rate,
						sample_num,
						1,
						data,
						data_size,
						non_copy_mode,
						pts,
						timebase);
			}
		}
	}
	else {
		if(prev_frame->type == SpeexType_header) {
			// in the case of 1st frame is header. next frame can be comment.
			// check the data is frame or not.
			ttLibC_ByteReader *reader = ttLibC_ByteReader_make(data, data_size, ByteUtilType_default);
			uint32_t frame_num;
			SpeexBand band;
			Speex_analyzeFrameBuffer(reader, data_size, &frame_num, &band);
			ttLibC_ByteReader_close(&reader);
			if(band == unknown) {
				// if it's not frame, treat as comment.
				return ttLibC_Speex_make(
						prev_frame,
						SpeexType_comment,
						prev_frame->inherit_super.sample_rate,
						prev_frame->inherit_super.sample_num,
						prev_frame->inherit_super.channel_num,
						data,
						data_size,
						non_copy_mode,
						pts,
						timebase);
			}
		}
		return ttLibC_Speex_make(
				prev_frame,
				SpeexType_frame,
				prev_frame->inherit_super.sample_rate,
				prev_frame->inherit_super.sample_num,
				prev_frame->inherit_super.channel_num,
				data,
				data_size,
				non_copy_mode,
				pts,
				timebase);
	}
}

/*
 * close frame
 * @param frame
 */
void TT_VISIBILITY_DEFAULT ttLibC_Speex_close(ttLibC_Speex **frame) {
	ttLibC_Speex_ *target = (ttLibC_Speex_ *)*frame;
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.inherit_super.type != frameType_speex) {
		ERR_PRINT("found non speex frame in speex_close.");
		return;
	}
	if(!target->inherit_super.inherit_super.is_non_copy) {
		ttLibC_free(target->inherit_super.inherit_super.data);
	}
	ttLibC_free(target);
	*frame = NULL;

}
