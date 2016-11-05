/*
 * @file   simpleBlock.c
 * @brief  simple block for mkv container.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/06/30
 */

#include "simpleBlock.h"
#include "../mkvReader.h"
#include "../../../log.h"
#include "../../../util/hexUtil.h"
#include "../../../util/byteUtil.h"
#include "../../../frame/video/h264.h"
#include "../../../frame/video/h265.h"
#include "../../../frame/video/jpeg.h"
#include "../../../frame/video/theora.h"
#include "../../../frame/video/vp8.h"
#include "../../../frame/video/vp9.h"
#include "../../../frame/audio/aac.h"
#include "../../../frame/audio/adpcmImaWav.h"
#include "../../../frame/audio/mp3.h"
#include "../../../frame/audio/opus.h"
#include "../../../frame/audio/speex.h"
#include "../../../frame/audio/vorbis.h"

static void SimpleBlock_getLace0Frame(
		ttLibC_MkvReader_ *reader,
		ttLibC_MkvTrack *track,
		uint8_t *data,
		size_t data_size,
		uint32_t time_diff,
		ttLibC_getFrameFunc callback,
		void *ptr) {
	if(track->frame == NULL) {
		// for first frame
		ttLibC_MkvTag_getPrivateDataFrame((ttLibC_MkvReader *)reader, track, callback, ptr);
	}
	uint64_t pts = reader->pts + time_diff;
	uint32_t timebase = reader->timebase;
	switch(track->type) {
	case frameType_h265:
		{
			// ここはframeの解析を実施するところ。
			// あとはここで解析してれば大丈夫になるんだろうか・・・
			uint8_t *buf = data;
			size_t buf_size = data_size;
			do {
				uint32_t size = 0;
				for(int i = 1;i <= track->size_length;++ i) {
					size = (size << 8) | *buf;
					if(i != track->size_length) {
						*buf = 0x00;
					}
					else {
						*buf = 0x01;
					}
					++ buf;
					-- buf_size;
				}
				buf += size;
				buf_size -= size;
			} while(buf_size > 0);
			while(data_size > 0) {
				ttLibC_H265 *h265 = ttLibC_H265_getFrame(
						(ttLibC_H265 *)track->frame,
						data,
						data_size,
						true,
						pts,
						timebase);
				if(h265 == NULL) {
					ERR_PRINT("failed to make h265 data.");
					reader->error_number = 5;
					break;
				}
				data += h265->inherit_super.inherit_super.buffer_size;
				data_size -= h265->inherit_super.inherit_super.buffer_size;
				track->frame = (ttLibC_Frame *)h265;
				track->frame->id = track->track_number;
				if(callback != NULL) {
					if(!callback(ptr, track->frame)) {
						reader->error_number = 5;
						break;
					}
				}
			}
		}
		break;
	case frameType_h264:
		{
			uint8_t *buf = data;
			size_t buf_size = data_size;
			// sizenal -> nal
			do {
				uint32_t size = 0;
				for(int i = 1;i <= track->size_length;++ i) {
					size = (size << 8) | *buf;
					if(i != track->size_length) {
						*buf = 0x00;
					}
					else {
						*buf = 0x01;
					}
					++ buf;
					-- buf_size;
				}
				buf += size;
				buf_size -= size;
			} while(buf_size > 0);
			while(data_size > 0) {
				ttLibC_H264 *h264 = ttLibC_H264_getFrame(
						(ttLibC_H264 *)track->frame,
						data,
						data_size,
						true,
						pts,
						timebase);
				if(h264 == NULL) {
					ERR_PRINT("failed to make h264 data.");
					reader->error_number = 5;
					break;
				}
				data += h264->inherit_super.inherit_super.buffer_size;
				data_size -= h264->inherit_super.inherit_super.buffer_size;
				track->frame = (ttLibC_Frame *)h264;
				track->frame->id = track->track_number;
				if(callback != NULL) {
					if(!callback(ptr, track->frame)) {
						reader->error_number = 5;
						break;
					}
				}
			}
		}
		break;
	case frameType_jpeg:
		{
			ttLibC_Jpeg *jpeg = ttLibC_Jpeg_getFrame(
					(ttLibC_Jpeg *)track->frame,
					data,
					data_size,
					true,
					pts,
					timebase);
			if(jpeg == NULL) {
				ERR_PRINT("failed to make jpeg data.");
				reader->error_number = 5;
			}
			else {
				track->frame = (ttLibC_Frame *)jpeg;
				track->frame->id = track->track_number;
				if(callback != NULL) {
					if(!callback(ptr, track->frame)) {
						reader->error_number = 5;
					}
				}
			}
		}
		break;
	case frameType_theora:
		{
			ttLibC_Theora *theora = ttLibC_Theora_getFrame(
					(ttLibC_Theora *)track->frame,
					data,
					data_size,
					true,
					pts,
					timebase);
			if(theora == NULL) {
				ERR_PRINT("failed to make theora data.");
				reader->error_number = 5;
			}
			else {
				track->frame = (ttLibC_Frame *)theora;
				track->frame->id = track->track_number;
				if(callback != NULL) {
					if(!callback(ptr, track->frame)) {
						reader->error_number = 5;
					}
				}
			}
		}
		break;
	case frameType_vp8:
		{
			ttLibC_Vp8 *vp8 = ttLibC_Vp8_getFrame(
					(ttLibC_Vp8 *)track->frame,
					data,
					data_size,
					true,
					pts,
					timebase);
			if(vp8 == NULL) {
				ERR_PRINT("failed to make vp8 data.");
				reader->error_number = 5;
			}
			else {
				track->frame = (ttLibC_Frame *)vp8;
				track->frame->id = track->track_number;
				if(callback != NULL) {
					if(!callback(ptr, track->frame)) {
						reader->error_number = 5;
					}
				}
			}
		}
		break;
	case frameType_vp9:
		{
			ttLibC_Vp9 *vp9 = ttLibC_Vp9_getFrame(
					(ttLibC_Vp9 *)track->frame,
					data,
					data_size,
					true,
					pts,
					timebase);
			if(vp9 == NULL) {
				ERR_PRINT("failed to make vp9 data.");
				reader->error_number = 5;
			}
			else {
				track->frame = (ttLibC_Frame *)vp9;
				track->frame->id = track->track_number;
				if(callback != NULL) {
					if(!callback(ptr, track->frame)) {
						reader->error_number = 5;
					}
				}
			}
		}
		break;
	case frameType_aac:
		{
			ttLibC_Aac *aac = ttLibC_Aac_make(
					(ttLibC_Aac *)track->frame,
					AacType_raw,
					track->sample_rate,
					1024,
					track->channel_num,
					data,
					data_size,
					true,
					pts,
					timebase,
					track->dsi_info);
			if(aac == NULL) {
				ERR_PRINT("failed to make aac data.");
				reader->error_number = 5;
			}
			else {
				track->frame = (ttLibC_Frame *)aac;
				track->frame->id = track->track_number;
				if(callback != NULL) {
					if(!callback(ptr, track->frame)) {
						reader->error_number = 5;
					}
				}
			}
		}
		break;
	case frameType_adpcm_ima_wav:
		{
			// adpcmの細かいところの指定はbinaryから復元できないので、ここに書こう
			// sample_numはpcmの値から復元しなければならないわけか・・・
			// たしか、4bitが１データだっけ？
			uint32_t sample_num = 0;
			switch(track->channel_num) {
			case 1:
				sample_num = (data_size - 4) * 2 + 1;
				break;
			case 2:
				sample_num = (data_size - 8) + 1;
				break;
			default:
				ERR_PRINT("only support stereo / monoral.");
				return;
			}
			ttLibC_AdpcmImaWav *adpcm = ttLibC_AdpcmImaWav_make(
					(ttLibC_AdpcmImaWav *)track->frame,
					track->sample_rate,
					sample_num,
					track->channel_num,
					data,
					data_size,
					true,
					pts,
					timebase);
			if(adpcm == NULL) {
				ERR_PRINT("failed to make adpcm data.");
				reader->error_number = 5;
			}
			else {
				track->frame = (ttLibC_Frame *)adpcm;
				track->frame->id = track->track_number;
				if(callback != NULL) {
					if(!callback(ptr, track->frame)) {
						reader->error_number = 5;
					}
				}
			}
		}
		break;
	case frameType_opus:
		{
			ttLibC_Opus *opus = ttLibC_Opus_makeFrame(
					(ttLibC_Opus *)track->frame,
					data,
					data_size,
					pts,
					timebase);
			if(opus == NULL) {
				ERR_PRINT("failed to make opus data.");
				reader->error_number = 5;
			}
			else {
				track->frame = (ttLibC_Frame *)opus;
				track->frame->id = track->track_number;
				if(callback != NULL) {
					if(!callback(ptr, track->frame)) {
						reader->error_number = 5;
					}
				}
			}
		}
		break;
	case frameType_vorbis:
		{
			ttLibC_Vorbis *vorbis = ttLibC_Vorbis_getFrame(
					(ttLibC_Vorbis *)track->frame,
					data,
					data_size,
					true,
					pts,
					timebase);
			if(vorbis == NULL) {
				ERR_PRINT("failed to make vorbis data.");
				reader->error_number = 5;
			}
			else {
				track->frame = (ttLibC_Frame *)vorbis;
				track->frame->id = track->track_number;
				if(callback != NULL) {
					if(!callback(ptr, track->frame)) {
						reader->error_number = 5;
					}
				}
			}
		}
		break;
	case frameType_mp3:
		{
			ttLibC_Mp3 *mp3 = ttLibC_Mp3_getFrame(
					(ttLibC_Mp3 *)track->frame,
					data,
					data_size,
					true,
					pts,
					timebase);
			if(mp3 == NULL) {
				ERR_PRINT("failed to make mp3 data.");
				reader->error_number = 5;
			}
			else {
				track->frame = (ttLibC_Frame *)mp3;
				track->frame->id = track->track_number;
				if(callback != NULL) {
					if(!callback(ptr, track->frame)) {
						reader->error_number = 5;
					}
				}
			}
		}
		break;
	case frameType_speex:
		{
			ttLibC_Speex *speex = ttLibC_Speex_getFrame(
					(ttLibC_Speex *)track->frame,
					data,
					data_size,
					true,
					pts,
					timebase);
			if(speex == NULL) {
				ERR_PRINT("failed to make speex data.");
				reader->error_number = 5;
			}
			else {
				// とりあえずこれで上書きしておく。
				speex->inherit_super.channel_num = track->channel_num;
				speex->inherit_super.sample_rate = track->sample_rate;
				track->frame = (ttLibC_Frame *)speex;
				track->frame->id = track->track_number;
				if(callback != NULL) {
					if(!callback(ptr, track->frame)) {
						reader->error_number = 5;
					}
				}
			}
		}
		break;
	default:
		LOG_PRINT("frame analyze is not support yet.%d", track->type);
		reader->error_number = 5;
		break;
	}
}

bool ttLibC_SimpleBlock_getFrame(
		ttLibC_MkvTag *tag,
		ttLibC_getFrameFunc callback,
		void *ptr) {
	ttLibC_ByteReader *byte_reader = ttLibC_ByteReader_make(tag->inherit_super.inherit_super.data, tag->inherit_super.inherit_super.data_size, ByteUtilType_default);
	// get information.
	uint32_t type          = ttLibC_ByteReader_ebml(byte_reader, true);
	uint64_t size          = ttLibC_ByteReader_ebml(byte_reader, false);
	uint32_t track_id      = ttLibC_ByteReader_ebml(byte_reader, false);
	uint32_t timecode_diff = ttLibC_ByteReader_bit(byte_reader, 16);

	bool is_key            = ttLibC_ByteReader_bit(byte_reader, 1) == 1;
	ttLibC_ByteReader_bit(byte_reader, 3);
	bool is_invisible      = ttLibC_ByteReader_bit(byte_reader, 1) == 1;
	uint32_t lacing        = ttLibC_ByteReader_bit(byte_reader, 2);
	ttLibC_ByteReader_bit(byte_reader, 1);

	ttLibC_MkvReader_ *reader = (ttLibC_MkvReader_ *)tag->reader;
	uint8_t *data = tag->inherit_super.inherit_super.data;
	data += byte_reader->read_size;
	size_t data_size = tag->inherit_super.inherit_super.data_size - byte_reader->read_size;

	// get track information from reader.
	ttLibC_MkvTrack *track = ttLibC_StlMap_get(reader->tracks, (void *)track_id);
	if(track == NULL) {
		ERR_PRINT("failed to get track information.");
		reader->error_number = 1;
	}
	else {
		switch(lacing) {
		case 0:
			SimpleBlock_getLace0Frame(
					reader,
					track,
					data,
					data_size,
					timecode_diff,
					callback,
					ptr);
			break;
		case 1:
		case 2:
		case 3:
		default:
			ERR_PRINT("unknown lacing. make later.");
			reader->error_number = 5;
			break;
		}
	}
	ttLibC_ByteReader_close(&byte_reader);
	return reader->error_number == 0;
}
