/*
 * @file   mkvTag.c
 * @brief  mkv container support.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/09/04
 */

#include "mkvTag.h"
#include "type/simpleBlock.h"
#include "mkvReader.h"
#include "../../log.h"
#include "../../util/hexUtil.h"
#include "../../allocator.h"
#include "../../frame/video/h264.h"
#include "../../frame/video/h265.h"
#include "../../frame/video/theora.h"
#include "../../frame/audio/aac.h"
#include "../../frame/audio/speex.h"
#include "../../frame/audio/vorbis.h"
#include "../../util/ioUtil.h"
#include "../../util/byteUtil.h"
#include <string.h>

ttLibC_MkvTag *ttLibC_MkvTag_make(
		ttLibC_MkvTag *prev_tag,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		ttLibC_Mkv_Type type) {
	ttLibC_MkvTag *tag = (ttLibC_MkvTag *)ttLibC_Container_make(
			(ttLibC_Container *)prev_tag,
			sizeof(union{
				ttLibC_MkvTag tag;
			}),
			containerType_mkv,
			data,
			data_size,
			non_copy_mode,
			pts,
			timebase);
	if(tag != NULL) {
		tag->inherit_super.type = type;
	}
	return tag;
}

bool ttLibC_Mkv_getFrame(ttLibC_Mkv *mkv, ttLibC_getFrameFunc callback, void *ptr) {
	switch(mkv->type) {
	case MkvType_SimpleBlock:
		// just now, only simpleBlock support to get frame.
		return ttLibC_SimpleBlock_getFrame((ttLibC_MkvTag *)mkv, callback, ptr);
	case MkvType_Block:
//		return ttLibC_Block_getFrame()
		return ttLibC_SimpleBlock_getFrame((ttLibC_MkvTag *)mkv, callback, ptr);
	default:
		return true;
	}
	return true;
}

/**
 * analyze frames in private data.
 * @param reader
 * @param track
 * @param callback
 * @param ptr
 * @note in the case of first reply of simple block, we will return private data information.
 * this code will be move to mkvTag.c
 */
void ttLibC_MkvTag_getPrivateDataFrame(
		ttLibC_MkvReader *reader,
		ttLibC_MkvTrack *track,
		ttLibC_getFrameFunc callback,
		void *ptr) {
	ttLibC_MkvReader_ *reader_ = (ttLibC_MkvReader_ *)reader;
	uint8_t *private_data = track->private_data;
	size_t private_data_size = track->private_data_size;
	if(private_data == NULL || private_data_size == 0) {
		// in the case of no private data. skip this task.
		return;
	}
	switch(track->type) {
	case frameType_h265:
		{
			uint32_t size_length = 0;
			ttLibC_H265 *h265 = ttLibC_H265_analyzeHvccTag(NULL, private_data, private_data_size, &size_length);
			if(h265 == NULL) {
				ERR_PRINT("failed to analyze hvccTag");
				reader_->error_number = 3;
				return;
			}
			track->frame = (ttLibC_Frame *)h265;
			track->frame->id = track->track_number;
			track->size_length = size_length;
			if(size_length < 3) {
				ERR_PRINT("hvcc size is too small.");
				reader_->error_number = 1;
				return;
			}
			if(callback != NULL) {
				if(!callback(ptr, track->frame)) {
					reader_->error_number = 5;
				}
			}
		}
		break;
	case frameType_h264:
		{
			uint32_t size_length = 0;
			ttLibC_H264 *h264 = ttLibC_H264_analyzeAvccTag(NULL, private_data, private_data_size, &size_length);
			if(h264 == NULL) {
				ERR_PRINT("failed to analyze avccTag");
				reader_->error_number = 3;
				return;
			}
			track->frame = (ttLibC_Frame *)h264;
			track->frame->id = track->track_number;
			track->size_length = size_length;
			if(size_length < 3) {
				ERR_PRINT("avcc size is too small.");
				reader_->error_number = 1;
				return;
			}
			if(callback != NULL) {
				if(!callback(ptr, track->frame)) {
					reader_->error_number = 5;
				}
			}
		}
		break;
	case frameType_theora:
		{
			uint32_t diff[4];
			diff[0] = private_data[0];
			diff[1] = private_data[1];
			diff[2] = private_data[2];
			diff[3] = private_data_size - 3 - diff[1] - diff[2];
			ttLibC_Theora *theora = NULL;
			private_data += 3;
			ttLibC_Theora *t = ttLibC_Theora_getFrame(
					NULL,
					private_data,
					diff[1],
					true,
					reader_->pts,
					reader_->timebase);
			if(t == NULL) {
				ERR_PRINT("failed to get theora identification frame.");
				reader_->error_number = 5;
				return;
			}
			theora = t;
			theora->inherit_super.inherit_super.id = track->track_number;
			if(callback != NULL) {
				if(!callback(ptr, (ttLibC_Frame *)theora)) {
					reader_->error_number = 5;
					ttLibC_Theora_close(&theora);
					return;
				}
			}
			private_data += diff[1];
			t = ttLibC_Theora_getFrame(
					theora,
					private_data,
					diff[2],
					true,
					reader_->pts,
					reader_->timebase);
			if(t == NULL) {
				ERR_PRINT("failed to get theora identification frame.");
				reader_->error_number = 5;
				return;
			}
			theora = t;
			theora->inherit_super.inherit_super.id = track->track_number;
			if(callback != NULL) {
				if(!callback(ptr, (ttLibC_Frame *)theora)) {
					reader_->error_number = 5;
					ttLibC_Theora_close(&theora);
					return;
				}
			}
			private_data += diff[2];
			t = ttLibC_Theora_getFrame(
					theora,
					private_data,
					diff[3],
					true,
					reader_->pts,
					reader_->timebase);
			if(t == NULL) {
				ERR_PRINT("failed to get theora identification frame.");
				reader_->error_number = 5;
				return;
			}
			theora = t;
			theora->inherit_super.inherit_super.id = track->track_number;
			if(callback != NULL) {
				if(!callback(ptr, (ttLibC_Frame *)theora)) {
					reader_->error_number = 5;
					ttLibC_Theora_close(&theora);
					return;
				}
			}
			track->frame = (ttLibC_Frame *)theora;
		}
		break;
	case frameType_aac:
		{
			memcpy(&track->dsi_info, private_data, private_data_size);
			ttLibC_Aac *aac = ttLibC_Aac_getFrame(
					NULL,
					private_data,
					private_data_size,
					true,
					0,
					reader_->timebase);
			if(aac != NULL) {
				aac->inherit_super.inherit_super.id = track->track_number;
				track->frame = (ttLibC_Frame *)aac;
				if(callback != NULL) {
					if(!callback(ptr, track->frame)) {
						reader_->error_number = 5;
					}
				}
			}
		}
		break;
	case frameType_vorbis:
		{
			uint32_t diff[4];
			diff[0] = private_data[0];
			diff[1] = private_data[1];
			diff[2] = private_data[2];
			diff[3] = private_data_size - 3 - diff[1] - diff[2];

			private_data += 3;
			ttLibC_Vorbis *v = ttLibC_Vorbis_getFrame(
					(ttLibC_Vorbis *)track->frame,
					private_data,
					diff[1],
					true,
					reader_->pts,
					reader_->timebase);
			if(v == NULL) {
				ERR_PRINT("failed to get vorbis identification frame.");
				reader_->error_number = 5;
				return;
			}
			track->frame = (ttLibC_Frame *)v;
			track->frame->id = track->track_number;
			if(callback != NULL) {
				if(!callback(ptr, track->frame)) {
					reader_->error_number = 5;
					return;
				}
			}
			private_data += diff[1];
			v = ttLibC_Vorbis_getFrame(
					(ttLibC_Vorbis *)track->frame,
					private_data,
					diff[2],
					true,
					reader_->pts,
					reader_->timebase);
			if(v == NULL) {
				ERR_PRINT("failed to get vorbis comment frame.");
				reader_->error_number = 5;
				return;
			}
			track->frame = (ttLibC_Frame *)v;
			track->frame->id = track->track_number;
			if(callback != NULL) {
				if(!callback(ptr, track->frame)) {
					reader_->error_number = 5;
					return;
				}
			}
			private_data += diff[2];
			v = ttLibC_Vorbis_getFrame(
					(ttLibC_Vorbis *)track->frame,
					private_data,
					diff[3],
					true,
					reader_->pts,
					reader_->timebase);
			if(v == NULL) {
				ERR_PRINT("failed to get vorbis setup frame.");
				reader_->error_number = 5;
				return;
			}
			track->frame = (ttLibC_Frame *)v;
			track->frame->id = track->track_number;
			if(callback != NULL) {
				if(!callback(ptr, track->frame)) {
					reader_->error_number = 5;
					return;
				}
			}
		}
		break;
	case frameType_unknown:
		// in the case of riff, we will be here. such as speex, adpcmimawav
		{
			// try to read binary as wave format ex.
			if(!track->is_video) {
				// must be audio
				ttLibC_ByteReader *reader = ttLibC_ByteReader_make(private_data, private_data_size, ByteUtilType_default);
				uint16_t be_tag = ttLibC_ByteReader_bit(reader, 16);
				uint16_t be_channels = ttLibC_ByteReader_bit(reader, 16);
				uint32_t be_sample_rate = ttLibC_ByteReader_bit(reader, 32);
				uint16_t tag = be_uint16_t(be_tag);
				uint16_t channels = be_uint16_t(be_channels);
				uint32_t sample_rate = be_uint32_t(be_sample_rate);
				ttLibC_ByteReader_close(&reader);
				// take as ok, if sample rate and channel number are same.
				if(channels == track->channel_num && sample_rate == track->sample_rate) {
					switch(tag) {
					default:
					case 0x0000: // unknown
//					case 0x0001: // pcm
//					case 0x0002: // ms adpcm
//					case 0x0005: // ibm csvd
//					case 0x0006: // pcmalaw
//					case 0x0007: // pcmmulaw
//					case 0x0010: // oki adpcm
					case 0x0011: // ima adpcm
						{
							track->type = frameType_adpcm_ima_wav;
						}
						break;
//					case 0x0055: // mp3
//					case 0x00FF: // aac
					case 0xA109: // speex
						{
							track->type = frameType_speex;
							// we already have enough information, however  it is better to analyze speex header information to check.
							ttLibC_Speex *speex = ttLibC_Speex_getFrame(
									(ttLibC_Speex *)track->frame,
									private_data + 18,
									private_data_size - 18,
									true,
									reader_->pts,
									reader_->timebase);
							if(speex == NULL) {
								ERR_PRINT("failed to read speex frame.");
								reader->error_number = 5;
							}
							else {
								track->frame = (ttLibC_Frame *)speex;
								track->frame->id = track->track_number;
								if(callback != NULL) {
									if(!callback(ptr, track->frame)) {
										reader_->error_number = 5;
										return;
									}
								}
							}
						}
						break;
//					case 0x566F: // vorbis
					}
				}
			}
//			reader_->error_number = 3;
		}
		break;
	default:
		// if we need to do something, add here...
		break;
	}
}

void ttLibC_MkvTag_close(ttLibC_MkvTag **tag) {
	ttLibC_MkvTag *target = *tag;
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.inherit_super.type != containerType_mkv) {
		ERR_PRINT("container type is not mkv");
		return;
	}
	if(!target->inherit_super.inherit_super.is_non_copy) {
		ttLibC_free(target->inherit_super.inherit_super.data);
	}
	ttLibC_free(target);
	*tag = NULL;
}
