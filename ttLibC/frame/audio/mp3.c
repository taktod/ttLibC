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
#include "../../log.h"

/*
 * mp3 frame definition(detail)
 */
typedef struct {
	/** inherit data from ttLibC_Mp3 */
	ttLibC_Mp3 inherit_super;
	union {
		struct { // for frame
			/** frame header 4byte data. */
			uint32_t frame_header;
		};
		struct { // for id3

		};
		struct { // for tag

		};
	};
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
ttLibC_Mp3 *ttLibC_Mp3_make(
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
	ttLibC_Mp3_ *mp3 = (ttLibC_Mp3_ *)prev_frame;
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
		mp3 = malloc(sizeof(ttLibC_Mp3_));
		if(mp3 == NULL) {
			ERR_PRINT("failed to allocate memory for mp3 frame.");
			return NULL;
		}
	}
	else {
		if(!mp3->inherit_super.inherit_super.inherit_super.is_non_copy) {
			if(non_copy_mode || mp3->inherit_super.inherit_super.inherit_super.data_size < data_size) {
				free(mp3->inherit_super.inherit_super.inherit_super.data);
				mp3->inherit_super.inherit_super.inherit_super.data = NULL;
			}
		}
	}
	mp3->inherit_super.type                                    = type;
	mp3->inherit_super.inherit_super.channel_num               = channel_num;
	mp3->inherit_super.inherit_super.sample_num                = sample_num;
	mp3->inherit_super.inherit_super.sample_rate               = sample_rate;
	mp3->inherit_super.inherit_super.inherit_super.buffer_size = data_size;
	mp3->inherit_super.inherit_super.inherit_super.data_size   = data_size;
	mp3->inherit_super.inherit_super.inherit_super.is_non_copy = non_copy_mode;
	mp3->inherit_super.inherit_super.inherit_super.pts         = pts;
	mp3->inherit_super.inherit_super.inherit_super.timebase    = timebase;
	mp3->inherit_super.inherit_super.inherit_super.type        = frameType_mp3;
	if(non_copy_mode) {
		mp3->inherit_super.inherit_super.inherit_super.data = data;
	}
	else {
		if(mp3->inherit_super.inherit_super.inherit_super.data == NULL) {
			mp3->inherit_super.inherit_super.inherit_super.data = malloc(data_size);
			if(mp3->inherit_super.inherit_super.inherit_super.data == NULL) {
				ERR_PRINT("failed to allocate memory for data.");
				if(prev_frame == NULL) {
					free(mp3);
				}
				return NULL;
			}
		}
		memcpy(mp3->inherit_super.inherit_super.inherit_super.data, data, data_size);
	}
	return (ttLibC_Mp3 *)mp3;
}

/*
 * check the mp3 frame type from data buffer.
 * TODO this func can be static?
 * @param data      mp3 binary data.
 * @param data_size data size.
 * @return Mp3Type value.
 */
ttLibC_Mp3_Type ttLibC_Mp3_getMp3Type(
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
		ERR_PRINT("unknown mp3 frame. start width:%x", *dat);
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

static uint16_t getSyncBit(uint32_t header) {
	return ((header >> 21) & 0x7FF);
}
/*
 * @param header first 4 byte of frame.
 * @return 0:mpeg2.5 2:mpeg2 3:mpeg1
 */
static uint8_t getMpegVersion(uint32_t header) {
	return ((header >> 19) & 0x03);
}

/*
 * @param header first 4 byte of frame.
 * @return 1:layer3 2:layer2 3:layer1
 */
static uint8_t getLayer(uint32_t header) {
	return ((header >> 17) & 0x03);
}

/**
 * @param header first 4 byte of frame.
 * @return 0:protected by CRC16
 */
static uint8_t getProtectionBit(uint32_t header) {
	return ((header >> 16) & 0x01);
}

static uint8_t getBitrateIndex(uint32_t header) {
	return ((header >> 12) & 0x0F);
}

static uint8_t getSampleRateIndex(uint32_t header) {
	return ((header >> 10) & 0x03);
}

static uint8_t getPaddingBit(uint32_t header) {
	return ((header >> 9) & 0x01);
}

static uint8_t getPrivateBit(uint32_t header) {
	return ((header >> 8) & 0x01);
}

/*
 * @param header first 4 byte of frame.
 * @return 0:stereo 1:joint stereo 2:dual channel 3:monoral
 */
static uint8_t getChannelMode(uint32_t header) {
	return ((header >> 6) & 0x03);
}

static uint8_t getModeExtension(uint32_t header) {
	// for joint stereo.
	return ((header >> 4) & 0x03);
}

static uint8_t getCopyRight(uint32_t header) {
	return ((header >> 3) & 0x01);
}
static uint8_t getOriginalFlg(uint32_t header) {
	return ((header >> 2) & 0x01);
}
static uint8_t getEmphasis(uint32_t header) {
	return (header & 3);
}

static ttLibC_Mp3 *makeFrame(ttLibC_Mp3 *prev_frame, void *data, size_t data_size, uint64_t pts, uint32_t timebase) {
	if(data_size < 4) {
		return NULL;
	}
	uint8_t *dat = data;
	uint32_t header = (dat[0] << 24) | (dat[1] << 16) | (dat[2] << 8) | dat[3];
	// check the sync bit.
	if(getSyncBit(header) != 0x7FF) {
		ERR_PRINT("syncBit is invalid.");
		return NULL;
	}
	uint8_t mpeg_version = getMpegVersion(header);
	uint8_t layer = getLayer(header);
	uint8_t bitrate_index = getBitrateIndex(header);
	uint8_t sample_rate_index = getSampleRateIndex(header);
	uint8_t padding_bit = getPaddingBit(header);

	uint32_t bitrate = 0;
	uint32_t sample_rate = 0;
	uint32_t channel_num = getChannelMode(header) == 3 ? 1 : 2;
	uint32_t frame_size = 0;
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
	// if data_size is more than frame_size, we can make mp3 frame object.
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
	ttLibC_Mp3_ *mp3 = (ttLibC_Mp3_ *)ttLibC_Mp3_make(prev_frame, Mp3Type_frame, sample_rate, sample_num, channel_num, data, frame_size, true, pts, timebase);
	if(mp3 == NULL) {
		return NULL;
	}
	// save the extra data for Mp3Type_frame.
	mp3->frame_header = header;
	return (ttLibC_Mp3 *)mp3;
}

static ttLibC_Mp3 *makeId3Frame(ttLibC_Mp3 *prev_frame, void *data, size_t data_size, uint64_t pts, uint32_t timebase) {
	// pts maybe 0.
	return NULL;
}

static ttLibC_Mp3 *makeTagFrame(ttLibC_Mp3 *prev_frame, void *data, size_t data_size, uint64_t pts, uint32_t timebase) {
	return NULL;
}

/*
 * make mp3 frame from byte data.
 * @param prev_frame reuse mp3 frame.
 * @param data       mp3 binary data
 * @param data_size  data size
 * @param pts        pts for mp3 frame.
 * @param timebase   timebase for mp3 frame.
 * @return mp3 object.
 */
ttLibC_Mp3 *ttLibC_Mp3_makeFrame(ttLibC_Mp3 *prev_frame, void *data, size_t data_size, uint64_t pts, uint32_t timebase) {
	ttLibC_Mp3_Type type = ttLibC_Mp3_getMp3Type(data, data_size);
	switch(type) {
	case Mp3Type_frame:
		return makeFrame(prev_frame, data, data_size, pts, timebase);
	case Mp3Type_id3:
		return makeId3Frame(prev_frame, data, data_size, pts, timebase);
	case Mp3Type_tag:
		return makeTagFrame(prev_frame, data, data_size, pts, timebase);
	default:
		// unknown mp3 frame. can be here, in the case of too short data. less than 1byte.
		return NULL;
	}
}

/*
 * close frame
 * @param frame
 */
void ttLibC_Mp3_close(ttLibC_Mp3 **frame) {
	if(*frame == NULL) {
		return;
	}
}
