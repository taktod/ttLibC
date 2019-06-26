/*
 * @file   vorbis.c
 * @brief  vorbis frame information
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/06
 */

#include "vorbis.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../_log.h"
#include "../../util/ioUtil.h"

typedef ttLibC_Frame_Audio_Vorbis ttLibC_Vorbis_;

/*
 * make vorbis frame.
 * @param prev_frame    reuse frame.
 * @param type          vorbis frame type.
 * @param sample_rate   sample rate of data
 * @param sample_num    sample num of data
 * @param channel_num   channel number of data
 * @param data          vorbis data
 * @param data_size     vorbis data size
 * @param non_copy_mode true:hold the data pointer. false:data will copy.
 * @param pts           pts for vorbis data.
 * @param timebase      timebase number for pts.
 * @return vorbis object.
 */
ttLibC_Vorbis TT_ATTRIBUTE_API *ttLibC_Vorbis_make(
		ttLibC_Vorbis *prev_frame,
		ttLibC_Vorbis_Type type,
		uint32_t sample_rate,
		uint32_t sample_num,
		uint32_t channel_num,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	ttLibC_Vorbis_ *vorbis = (ttLibC_Vorbis_ *)ttLibC_Audio_make(
			(ttLibC_Audio *)prev_frame,
			sizeof(ttLibC_Vorbis_),
			frameType_vorbis,
			sample_rate,
			sample_num,
			channel_num,
			data,
			data_size,
			non_copy_mode,
			pts,
			timebase);
	if(vorbis != NULL) {
		vorbis->type = type;
		vorbis->block0 = 0;
		vorbis->block1 = 0;
		vorbis->block_type = 0;
	}
	return (ttLibC_Vorbis *)vorbis;
}

/**
 * make clone frame
 * always make copy buffer on it.
 * @param prev_frame reuse frame object.
 * @param src_frame  source of clone.
 */
ttLibC_Vorbis TT_ATTRIBUTE_API *ttLibC_Vorbis_clone(
		ttLibC_Vorbis *prev_frame,
		ttLibC_Vorbis *src_frame) {
	if(src_frame == NULL) {
		return NULL;
	}
	if(src_frame->inherit_super.inherit_super.type != frameType_vorbis) {
		ERR_PRINT("try to clone non vorbis frame.");
		return NULL;
	}
	if(prev_frame != NULL && prev_frame->inherit_super.inherit_super.type != frameType_vorbis) {
		ERR_PRINT("try to use non vorbis frame for reuse.");
		return NULL;
	}
	ttLibC_Vorbis *vorbis = ttLibC_Vorbis_make(
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
	if(vorbis != NULL) {
		vorbis->inherit_super.inherit_super.id = src_frame->inherit_super.inherit_super.id;
		vorbis->block_type = src_frame->block_type;
		vorbis->block0 = src_frame->block0;
		vorbis->block1 = src_frame->block1;
	}
	return vorbis;
}

/**
 * make vorbis frame from byte data.
 */
ttLibC_Vorbis TT_ATTRIBUTE_API *ttLibC_Vorbis_getFrame(
		ttLibC_Vorbis *prev_frame,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	uint8_t *buf = (uint8_t *)data;
	if(data_size > 7){
		// check is header or not.
		if(buf[1] == 'v'
		&& buf[2] == 'o'
		&& buf[3] == 'r'
		&& buf[4] == 'b'
		&& buf[5] == 'i'
		&& buf[6] == 's') {
			// maybe header.
			switch(buf[0]) {
			case 0x01: // identification
				{
					if(data_size < 29) {
						// too small data size.
						return NULL;
					}
					uint32_t channel_num = buf[11];
					uint32_t *buf32 = (uint32_t *)(buf + 12);
					uint32_t sample_rate = le_uint32_t(*buf32);
					uint32_t block0 = buf[28] & 0x0F;
					uint32_t block1 = (buf[28] >> 4) & 0x0F;
					uint32_t block0_value = (1 << block0);
					uint32_t block1_value = (1 << block1);
					ttLibC_Vorbis *vorbis = ttLibC_Vorbis_make(
							prev_frame,
							VorbisType_identification,
							sample_rate,
							0,
							channel_num,
							data,
							data_size,
							non_copy_mode,
							pts,
							timebase);
					if(vorbis == NULL) {
						ERR_PRINT("failed to make vorbisFrame.");
						return NULL;
					}
					vorbis->block0 = block0_value;
					vorbis->block1 = block1_value;
					vorbis->block_type = 0;
					return vorbis;
				}
				break;
			case 0x03: // comment
				{
					if(prev_frame == NULL) {
						return NULL;
					}
					uint32_t block0 = prev_frame->block0;
					uint32_t block1 = prev_frame->block1;
					ttLibC_Vorbis *vorbis = ttLibC_Vorbis_make(
							prev_frame,
							VorbisType_comment,
							prev_frame->inherit_super.sample_rate,
							0,
							prev_frame->inherit_super.channel_num,
							data,
							data_size,
							non_copy_mode,
							pts,
							timebase);
					if(vorbis == NULL) {
						ERR_PRINT("failed to make vorbisFrame.");
						return NULL;
					}
					vorbis->block0 = block0;
					vorbis->block1 = block1;
					vorbis->block_type = 0;
					return vorbis;
				}
				break;
			case 0x05: // setup
				{
					if(prev_frame == NULL) {
						return NULL;
					}
					uint32_t block0 = prev_frame->block0;
					uint32_t block1 = prev_frame->block1;
					ttLibC_Vorbis *vorbis = ttLibC_Vorbis_make(
							prev_frame,
							VorbisType_setup,
							prev_frame->inherit_super.sample_rate,
							0,
							prev_frame->inherit_super.channel_num,
							data,
							data_size,
							non_copy_mode,
							pts,
							timebase);
					if(vorbis == NULL) {
						ERR_PRINT("failed to make vorbisFrame.");
						return NULL;
					}
					vorbis->block0 = block0;
					vorbis->block1 = block1;
					vorbis->block_type = 0;
					return vorbis;
				}
				break;
			default:
				return NULL;
			}
		}
	}
	if(prev_frame == NULL) {
		return NULL;
	}
	uint32_t block0 = prev_frame->block0;
	uint32_t block1 = prev_frame->block1;
	uint32_t sample_num = 0;
	if(prev_frame->block_type == 0) {
		sample_num += block0;
	}
	else {
		sample_num += block1;
	}
	uint8_t block_type = (buf[0] & 0x02) != 0;
	if(block_type == 0) {
		sample_num += block0;
	}
	else {
		sample_num += block1;
	}
	sample_num = sample_num >> 2;
	// normal frame.
	ttLibC_Vorbis *vorbis = ttLibC_Vorbis_make(
			prev_frame,
			VorbisType_frame,
			prev_frame->inherit_super.sample_rate,
			sample_num,
			prev_frame->inherit_super.channel_num,
			data,
			data_size,
			non_copy_mode,
			pts,
			timebase);
	if(vorbis == NULL) {
		ERR_PRINT("failed to make vorbisFrame.");
		return NULL;
	}
	vorbis->block0 = block0;
	vorbis->block1 = block1;
	vorbis->block_type = block_type;
	return vorbis;
}

/*
 * close frame
 * @param frame
 */
void TT_ATTRIBUTE_API ttLibC_Vorbis_close(ttLibC_Vorbis **frame) {
	ttLibC_Audio_close_((ttLibC_Audio **)frame);
}


