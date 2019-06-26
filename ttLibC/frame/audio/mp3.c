/*
 * @file   mp3.c
 * @brief  mp3 frame information
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/21
 * TODO need to make id3 and tag frame.
 * (to read mp3 file. must make id3. tag... old one... can ignore.)
 */

#include "mp3.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../_log.h"
#include "../../allocator.h"
#include "../../util/byteUtil.h"

/*
 * mp3 frame definition(detail)
 */
typedef struct {
	/** inherit data from ttLibC_Mp3 */
	ttLibC_Mp3 inherit_super;
/*	union {
		struct { // for frame
		};
		struct { // for id3

		};
		struct { // for tag

		};
	};*/
} ttLibC_Frame_Audio_Mp3_;

typedef ttLibC_Frame_Audio_Mp3_ ttLibC_Mp3_;

/*
 * make mp3 frame
 * @param prev_frame    reuse frame.
 * @param type          type of mp3
 * @param sample_rate   sample rate of data.
 * @param sample_num    sample number of data.
 * @param channel_num   channel number of data. 1:monoral 2:stereo
 * @param data          mp3 data
 * @param data_size     mp3 data size
 * @param non_copy_mode true:hold the data pointer. false:data will copy.
 * @param pts           pts for mp3 data.
 * @param timebase      timebase number for pts.
 * @return mp3 object.
 */
ttLibC_Mp3 TT_ATTRIBUTE_API *ttLibC_Mp3_make(
		ttLibC_Mp3 *prev_frame,
		ttLibC_Mp3_Type type,
		uint32_t sample_rate,
		uint32_t sample_num,
		uint32_t channel_num,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	if(prev_frame != NULL && prev_frame->inherit_super.inherit_super.type != frameType_mp3) {
		ERR_PRINT("reuse with incompatible frame.");
		return NULL;
	}
	ttLibC_Mp3_ *mp3 = (ttLibC_Mp3_ *)prev_frame;
	size_t data_size_ = data_size;
	size_t buffer_size_ = data_size;
	switch(type) {
	case Mp3Type_frame:
	case Mp3Type_id3:
	case Mp3Type_tag:
		break;
	default:
		ERR_PRINT("unknown mp3 type.%d", type);
		return NULL;
	}
	if(mp3 == NULL) {
		mp3 = ttLibC_malloc(sizeof(ttLibC_Mp3_));
		if(mp3 == NULL) {
			ERR_PRINT("failed to allocate memory for mp3 frame.");
			return NULL;
		}
		mp3->inherit_super.inherit_super.inherit_super.data = NULL;
	}
	else {
		if(!mp3->inherit_super.inherit_super.inherit_super.is_non_copy) {
			if(non_copy_mode || mp3->inherit_super.inherit_super.inherit_super.data_size < data_size) {
				ttLibC_free(mp3->inherit_super.inherit_super.inherit_super.data);
				mp3->inherit_super.inherit_super.inherit_super.data = NULL;
			}
			else {
				data_size_ = mp3->inherit_super.inherit_super.inherit_super.data_size;
			}
		}
	}
	mp3->inherit_super.type                                    = type;
	mp3->inherit_super.inherit_super.channel_num               = channel_num;
	mp3->inherit_super.inherit_super.sample_num                = sample_num;
	mp3->inherit_super.inherit_super.sample_rate               = sample_rate;
	mp3->inherit_super.inherit_super.inherit_super.buffer_size = buffer_size_;
	mp3->inherit_super.inherit_super.inherit_super.data_size   = data_size_;
	mp3->inherit_super.inherit_super.inherit_super.is_non_copy = non_copy_mode;
	mp3->inherit_super.inherit_super.inherit_super.pts         = pts;
	mp3->inherit_super.inherit_super.inherit_super.dts         = 0;
	mp3->inherit_super.inherit_super.inherit_super.timebase    = timebase;
	mp3->inherit_super.inherit_super.inherit_super.type        = frameType_mp3;
	if(non_copy_mode) {
		mp3->inherit_super.inherit_super.inherit_super.data = data;
	}
	else {
		if(mp3->inherit_super.inherit_super.inherit_super.data == NULL) {
			mp3->inherit_super.inherit_super.inherit_super.data = ttLibC_malloc(data_size);
			if(mp3->inherit_super.inherit_super.inherit_super.data == NULL) {
				ERR_PRINT("failed to allocate memory for data.");
				if(prev_frame == NULL) {
					ttLibC_free(mp3);
				}
				return NULL;
			}
		}
		memcpy(mp3->inherit_super.inherit_super.inherit_super.data, data, data_size);
	}
	return (ttLibC_Mp3 *)mp3;
}

/*
 * make clone frame
 * always make copy buffer on it.
 * @param prev_frame reuse frame object.
 * @param src_frame  source of clone.
 */
ttLibC_Mp3 TT_ATTRIBUTE_API *ttLibC_Mp3_clone(
		ttLibC_Mp3 *prev_frame,
		ttLibC_Mp3 *src_frame) {
	if(src_frame == NULL) {
		return NULL;
	}
	if(src_frame->inherit_super.inherit_super.type != frameType_mp3) {
		ERR_PRINT("try to clone non mp3 frame.");
		return NULL;
	}
	if(prev_frame != NULL && prev_frame->inherit_super.inherit_super.type != frameType_mp3) {
		ERR_PRINT("try to use non mp3 frame for reuse.");
		return NULL;
	}
	ttLibC_Mp3 *mp3 = ttLibC_Mp3_make(
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
	if(mp3 != NULL) {
		mp3->inherit_super.inherit_super.id = src_frame->inherit_super.inherit_super.id;
	}
	return mp3;
}

/*
 * check the mp3 frame type from data buffer.
 * TODO this func can be static?
 * @param data      mp3 binary data.
 * @param data_size data size.
 * @return Mp3Type value.
 */
ttLibC_Mp3_Type TT_ATTRIBUTE_API ttLibC_Mp3_getMp3Type(
		void *data,
		size_t data_size) {
	if(data_size < 1) {
		return -1;
	}
	uint8_t *dat = (uint8_t *)data;
	switch(*dat) {
	case 'I': // ID3v2
		return Mp3Type_id3;
	case 'T': // ID3v1
		return Mp3Type_tag;
	case 0xFF:
		return Mp3Type_frame;
	default:
		LOG_PRINT("unknown mp3 frame. start width:%x", *dat);
		return -1;
	}
}

/*
 * table for bitrate index.
 */
static int bitrateIndexV1L1[]  = {-1, 32000, 64000, 96000, 128000, 160000, 192000, 224000, 256000, 288000, 320000, 352000, 384000, 416000, 448000, -1};
static int bitrateIndexV1L2[]  = {-1, 32000, 48000, 56000,  64000,  80000,  96000, 112000, 128000, 160000, 192000, 224000, 256000, 320000, 384000, -1};
static int bitrateIndexV1L3[]  = {-1, 32000, 40000, 48000,  56000,  64000,  80000,  96000, 112000, 128000, 160000, 192000, 224000, 256000, 320000, -1};
static int bitrateIndexV2L1[]  = {-1, 32000, 48000, 56000,  64000,  80000,  96000, 112000, 128000, 144000, 160000, 176000, 192000, 224000, 256000, -1};
static int bitrateIndexV2L23[] = {-1,  8000, 16000, 24000,  32000,  40000,  48000,  56000,  64000,  80000,  96000, 112000, 128000, 144000, 160000, -1};
static int sampleRateTableV1[]  = {44100, 48000, 32000};
static int sampleRateTableV2[]  = {22050, 24000, 16000};
static int sampleRateTableV25[] = {11025, 12000,  8000};

static ttLibC_Mp3 *Mp3_makeFrame(
		ttLibC_Mp3 *prev_frame,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	if(data_size < 4) {
		return NULL;
	}
	ttLibC_ByteReader *reader = ttLibC_ByteReader_make(data, data_size, ByteUtilType_default);
	if(reader == NULL) {
		ERR_PRINT("failed to allocate byteReader.");
		return NULL;
	}
	if(ttLibC_ByteReader_bit(reader, 11) != 0x07FF) {
		ERR_PRINT("syncbit is invalid.");
		ttLibC_ByteReader_close(&reader);
		return 0;
	}
	uint8_t mpeg_version = ttLibC_ByteReader_bit(reader, 2);
	uint8_t layer        = ttLibC_ByteReader_bit(reader, 2);
	ttLibC_ByteReader_bit(reader, 1);
	uint8_t bitrate_index     = ttLibC_ByteReader_bit(reader, 4);
	uint8_t sample_rate_index = ttLibC_ByteReader_bit(reader, 2);
	uint8_t padding_bit       = ttLibC_ByteReader_bit(reader, 1);
	ttLibC_ByteReader_bit(reader, 1);
	uint8_t channel_mode = ttLibC_ByteReader_bit(reader, 2);
	ttLibC_ByteReader_bit(reader, 2);
	ttLibC_ByteReader_bit(reader, 1);
	ttLibC_ByteReader_bit(reader, 1);
	ttLibC_ByteReader_bit(reader, 2);
	if(reader->error != Error_noError) {
		LOG_ERROR(reader->error);
		ttLibC_ByteReader_close(&reader);
		return 0;
	}
	ttLibC_ByteReader_close(&reader);

	uint32_t bitrate = 0;
	uint32_t sample_rate = 0;
	size_t frame_size = 0;
	uint32_t channel_num = channel_mode == 3 ? 1 : 2;
	switch(mpeg_version) {
	case 0:
	case 2:
		switch(layer) {
		case 1:
		case 2:
			bitrate = bitrateIndexV2L23[bitrate_index];
			break;
		case 3:
			bitrate = bitrateIndexV2L1[bitrate_index];
			break;
		default:
			ERR_PRINT("unknown layer index:%d", layer);
			return NULL;
		}
		break;
	case 3:
		switch(layer) {
		case 1: // layer3
			bitrate = bitrateIndexV1L3[bitrate_index];
			break;
		case 2: // layer2
			bitrate = bitrateIndexV1L2[bitrate_index];
			break;
		case 3: // layer1
			bitrate = bitrateIndexV1L1[bitrate_index];
			break;
		default:
			ERR_PRINT("unknown layer index:%d", layer);
			return NULL;
		}
		break;
	default:
		ERR_PRINT("unknown mpeg_version:%d", mpeg_version);
		return NULL;
	}
	switch(mpeg_version) {
	case 0:
		sample_rate = sampleRateTableV25[sample_rate_index];
		break;
	case 2:
		sample_rate = sampleRateTableV2[sample_rate_index];
		break;
	case 3:
		sample_rate = sampleRateTableV1[sample_rate_index];
		break;
	default:
		ERR_PRINT("unknown mpeg_version:%d", mpeg_version);
		return NULL;
	}
	switch(layer) {
	case 3:
		frame_size = ((uint32_t)(12.0f * bitrate / sample_rate + padding_bit)) * 4;
		break;
	case 2:
		frame_size = (uint32_t)(144.0f * bitrate / sample_rate + padding_bit);
		break;
	case 1:
		switch(mpeg_version) {
		case 0:
		case 2:
			frame_size = (uint32_t)(72.0f * bitrate / sample_rate + padding_bit);
			break;
		case 3:
			frame_size = (uint32_t)(144.0f * bitrate / sample_rate + padding_bit);
			break;
		default:
			ERR_PRINT("unknown mpeg_version:%d", mpeg_version);
			return NULL;
		}
		break;
	default:
		ERR_PRINT("unknown layer index:%d", layer);
		return NULL;
	}
	if(data_size < frame_size) {
		// data size is too short. need more.
		return NULL;
	}

	uint32_t sample_num = 0;
	switch(layer) {
	case 3:
		sample_num = 384;
		break;
	case 2:
		sample_num = 1152;
		break;
	case 1:
		switch(mpeg_version) {
		case 0:
		case 2:
			sample_num = 576;
			break;
		case 3:
			sample_num = 1152;
			break;
		default:
			ERR_PRINT("unknown mpeg_version:%d", mpeg_version);
			return NULL;
		}
		break;
	default:
		ERR_PRINT("unknown layer index:%d", layer);
		return NULL;
	}
	// now make mp3 object.
	ttLibC_Mp3_ *mp3 = (ttLibC_Mp3_ *)ttLibC_Mp3_make(
			prev_frame,
			Mp3Type_frame,
			sample_rate,
			sample_num,
			channel_num,
			data,
			frame_size,
			non_copy_mode,
			pts,
			timebase);
	if(mp3 == NULL) {
		return NULL;
	}
	return (ttLibC_Mp3 *)mp3;
}

static ttLibC_Mp3 *Mp3_makeId3Frame(
		ttLibC_Mp3 *prev_frame,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	if(data_size < 10) {
		return NULL;
	}
	// pts maybe 0.
	ttLibC_ByteReader *reader = ttLibC_ByteReader_make(data, data_size, ByteUtilType_default);
	if(reader == NULL) {
		ERR_PRINT("failed to make byteReader object.");
		return NULL;
	}
	/*
	 * recipe
	 * 24bit ID3
	 * 16bit version
	 * 8bit flag
	 * 1bit dummy
	 * 7bit size1
	 * 1bit dummy
	 * 7bit size2
	 * 1bit dummy
	 * 7bit size3
	 * 1bit dummy
	 * 7bit size4
	 *
	 * 10 + size1size2size3size4 = size
	 */
	if(ttLibC_ByteReader_bit(reader, 8) != 'I'
	|| ttLibC_ByteReader_bit(reader, 8) != 'D'
	|| ttLibC_ByteReader_bit(reader, 8) != '3') {
		ERR_PRINT("tag is not ID3.");
		ttLibC_ByteReader_close(&reader);
		return NULL;
	}
	ttLibC_ByteReader_bit(reader, 16);
	ttLibC_ByteReader_bit(reader, 8);
	ttLibC_ByteReader_bit(reader, 1);
	uint32_t size = ttLibC_ByteReader_bit(reader, 7);
	ttLibC_ByteReader_bit(reader, 1);
	size = (size << 7) | ttLibC_ByteReader_bit(reader, 7);
	ttLibC_ByteReader_bit(reader, 1);
	size = (size << 7) | ttLibC_ByteReader_bit(reader, 7);
	ttLibC_ByteReader_bit(reader, 1);
	size = (size << 7) | ttLibC_ByteReader_bit(reader, 7);
	size += 10; // this is frame size.
	if(reader->error != Error_noError) {
		LOG_ERROR(reader->error);
		ttLibC_ByteReader_close(&reader);
		return NULL;
	}
	ttLibC_ByteReader_close(&reader);
	if(size > data_size) {
		return NULL;
	}
	ttLibC_Mp3_ *mp3 = (ttLibC_Mp3_ *)ttLibC_Mp3_make(
			prev_frame,
			Mp3Type_id3,
			0,
			0,
			0,
			data,
			size,
			non_copy_mode,
			pts,
			timebase);
	if(mp3 == NULL) {
		return NULL;
	}
	return (ttLibC_Mp3 *)mp3;
}

static ttLibC_Mp3 *Mp3_makeTagFrame(
		ttLibC_Mp3 *prev_frame,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	(void)prev_frame;
	(void)data;
	(void)data_size;
	(void)non_copy_mode;
	(void)pts;
	(void)timebase;
	return NULL;
}

/*
 * make mp3 frame from byte data.
 * @param prev_frame    reuse mp3 frame.
 * @param data          mp3 binary data
 * @param data_size     data size
 * @param non_copy_mode true:hold the data pointer. false:data will copy.
 * @param pts           pts for mp3 frame.
 * @param timebase      timebase for mp3 frame.
 * @return mp3 object.
 */
ttLibC_Mp3 TT_ATTRIBUTE_API *ttLibC_Mp3_getFrame(
		ttLibC_Mp3 *prev_frame,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	ttLibC_Mp3_Type type = ttLibC_Mp3_getMp3Type(data, data_size);
	switch(type) {
	case Mp3Type_frame:
		return Mp3_makeFrame(
				prev_frame,
				data,
				data_size,
				non_copy_mode,
				pts,
				timebase);
	case Mp3Type_id3:
		return Mp3_makeId3Frame(
				prev_frame,
				data,
				data_size,
				non_copy_mode,
				pts,
				timebase);
	case Mp3Type_tag:
		return Mp3_makeTagFrame(
				prev_frame,
				data,
				data_size,
				non_copy_mode,
				pts,
				timebase);
	default:
		// unknown mp3 frame. can be here, in the case of too short data. less than 1byte.
		return NULL;
	}
}

/*
 * close frame
 * @param frame
 */
void TT_ATTRIBUTE_API ttLibC_Mp3_close(ttLibC_Mp3 **frame) {
	ttLibC_Mp3_ *target = (ttLibC_Mp3_ *)*frame;
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.inherit_super.inherit_super.type != frameType_mp3) {
		ERR_PRINT("found non mp3 frame in mp3_close.");
		return;
	}
	if(!target->inherit_super.inherit_super.inherit_super.is_non_copy) {
		ttLibC_free(target->inherit_super.inherit_super.inherit_super.data);
	}
	ttLibC_free(target);
	*frame = NULL;
}
