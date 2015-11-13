/*
 * @file   videoTag.c
 * @brief  flvTag for video
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/12
 */

#include "videoTag.h"
#include "../flvTag.h"
#include "../../../log.h"
#include "../../../frame/video/h264.h"
#include "../../../frame/video/flv1.h"
#include "../../../util/hexUtil.h"
#include "../../../util/ioUtil.h"

ttLibC_FlvVideoTag *ttLibC_FlvVideoTag_make(
		ttLibC_FlvTag *prev_tag,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		uint32_t track_id,
		uint8_t codec_id) {
	ttLibC_FlvVideoTag *video_tag = (ttLibC_FlvVideoTag *)ttLibC_FlvTag_make(
			prev_tag,
			data,
			data_size,
			non_copy_mode,
			pts,
			timebase,
			FlvType_video,
			track_id);
	if(video_tag != NULL) {
		video_tag->codec_id = codec_id;
		switch(codec_id) {
		case 1: // jpeg
			video_tag->frame_type = frameType_unknown;
			break;
		case 2:
			video_tag->frame_type = frameType_flv1;
			break;
		case 3:
			// screen video
			video_tag->frame_type = frameType_unknown;
			break;
		case 4:
			// on2vp6
			video_tag->frame_type = frameType_vp6;
			break;
		case 5:
			// on2vp6 alpha
			video_tag->frame_type = frameType_vp6;
			break;
		case 6:
			// screen video 2
			video_tag->frame_type = frameType_unknown;
			break;
		case 7:
			video_tag->frame_type = frameType_h264;
			break;
		default:
			video_tag->frame_type = frameType_unknown;
			break;
		}
	}
	return video_tag;
}

ttLibC_FlvVideoTag *ttLibC_FlvVideoTag_getTag(
		ttLibC_FlvTag *prev_tag,
		uint8_t *data,
		size_t data_size) {
	/*
	 * 1byte flag
	 * 3byte size
	 * 3byte timestamp
	 * 1byte timestamp-ext
	 * 3byte track_id
	 * 4bit frame type
	 * 4bit codec type
	 *
	 * vp6
	 *  4bit horizontal adjustment
	 *  4bit vertical adjustment
	 *
	 * vp6a
	 *  4byte offset to alpha
	 *
	 * h264
	 *  1byte packet type 0:msg 1:frame 2:end
	 *  3byte dts
	 *
	 * ...
	 *
	 * 4byte post_size
	 */
	size_t size = ((data[1] << 16) & 0xFF0000) | ((data[2] << 8) & 0xFF00) | (data[3] & 0xFF);
	uint32_t timestamp = ((data[4] << 16) & 0xFF0000) | ((data[5] << 8) & 0xFF00) | (data[6] & 0xFF) | ((data[7] << 24) & 0xFF000000);
	uint32_t track_id = ((data[8] << 16) & 0xFF0000) | ((data[9] << 8) & 0xFF00) | (data[10] & 0xFF);
	uint8_t frame_type = ((data[11] >> 4) & 0x0F);
	uint8_t codec_id = data[11] & 0x0F;
	uint32_t dts = 0;
	data += 12;
	data_size -= 12;

	size_t post_size = ((*(data + data_size - 4)) << 24) | ((*(data + data_size - 3)) << 16) | ((*(data + data_size - 2)) << 8) | (*(data + data_size - 1));
	if(size + 11 != post_size) {
		ERR_PRINT("size data is crazy. out of flv format.");
		return NULL;
	}
	return ttLibC_FlvVideoTag_make(
			prev_tag,
			data,
			data_size - 4,
			true,
			timestamp,
			1000,
			track_id,
			codec_id);
}

bool ttLibC_FlvVideoTag_getFrame(
		ttLibC_FlvVideoTag *video_tag,
		ttLibC_getFrameFunc callback,
		void *ptr) {
	uint8_t *buffer = video_tag->inherit_super.inherit_super.inherit_super.data;
	size_t left_size = video_tag->inherit_super.inherit_super.inherit_super.buffer_size;
	switch(video_tag->frame_type) {
	case frameType_flv1:
		{
			ttLibC_Flv1 *flv1 = ttLibC_Flv1_getFrame(
					(ttLibC_Flv1 *)video_tag->inherit_super.frame,
					buffer,
					left_size,
					true,
					video_tag->inherit_super.inherit_super.inherit_super.pts,
					video_tag->inherit_super.inherit_super.inherit_super.timebase);
			if(flv1 == NULL) {
				return false;
			}
			video_tag->inherit_super.frame = (ttLibC_Frame *)flv1;
			return callback(ptr, video_tag->inherit_super.frame);
		}
		break;
	case frameType_h264:
		{
			uint32_t dts = (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
			if(dts != 0) {
				LOG_PRINT("found dts is non zero h264 data.");
			}
			if(buffer[0] == 0x00) {
				// avcC information
				buffer += 4;
				left_size -= 4;
				uint32_t length_size = 0;
				ttLibC_H264 *h264 = ttLibC_H264_analyzeAvccTag((ttLibC_H264 *)video_tag->inherit_super.frame, buffer, left_size, &length_size);
				if(h264 == NULL) {
					ERR_PRINT("failed to make h264 configdata frame.");
					return false;
				}
				if(length_size < 3) {
					ERR_PRINT("avcc size length is too small.");
					return false;
				}
				// h264_length_size can be changed, however, usually 4byte
				video_tag->h264_length_size = length_size;
				video_tag->inherit_super.frame = (ttLibC_Frame *)h264;
				return callback(ptr, video_tag->inherit_super.frame);
			}
			else if(buffer[0] == 0x01) {
				// normal frame.
				// first 4 byte for type and 3byte dts.(just ignore.)
				buffer += 4;
				left_size -= 4;
				uint8_t *buf = buffer;
				size_t buf_size = left_size;
				// change sizenal -> nal
				do {
					uint32_t size = 0;
					for(int i = 1;i <= video_tag->h264_length_size;++ i) {
						size = (size << 8) | *buf;
						if(i != video_tag->h264_length_size) {
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
				ttLibC_H264 *h264 = ttLibC_H264_getFrame(
						(ttLibC_H264 *)video_tag->inherit_super.frame,
						buffer,
						left_size,
						true,
						video_tag->inherit_super.inherit_super.inherit_super.pts,
						video_tag->inherit_super.inherit_super.inherit_super.timebase);
				if(h264 == NULL) {
					ERR_PRINT("failed to make h264 data.");
					return false;
				}
				video_tag->inherit_super.frame = (ttLibC_Frame *)h264;
				return callback(ptr, video_tag->inherit_super.frame);
			}
			else if(buffer[0] == 0x02) {
				// flag for end of h264 stream.
				return true;
			}
			else {
				// others, unknown
				LOG_PRINT("found unknown data. h264 end sign?");
				LOG_DUMP(buffer, left_size, true);
				return false;
			}
		}
		break;
	case frameType_vp6:
		// need to make
		break;
	default:
		break;
	}
	return false;
}

bool ttLibC_FlvVideoTag_writeTag(
		ttLibC_FlvWriter_ *writer,
		ttLibC_Frame *frame,
		ttLibC_ContainerWriteFunc callback,
		void *ptr) {
	uint8_t buf[256]; // buffer for writing.
	switch(frame->type) {
	case frameType_flv1:
		{
			ttLibC_Video *video = (ttLibC_Video *)frame;
			uint8_t *buffer = video->inherit_super.data;
			uint32_t left_size = video->inherit_super.buffer_size;
			buf[0]  = 0x09;
			uint32_t pre_size = 1 + left_size;
			buf[1]  = (pre_size >> 16) & 0xFF;
			buf[2]  = (pre_size >> 8) & 0xFF;
			buf[3]  = pre_size & 0xFF;
			// pts
			buf[4]  = (video->inherit_super.pts >> 16) & 0xFF;
			buf[5]  = (video->inherit_super.pts >> 8) & 0xFF;
			buf[6]  = video->inherit_super.pts & 0xFF;
			buf[7]  = (video->inherit_super.pts >> 24) & 0xFF;
			// track
			buf[8]  = 0x00;
			buf[9]  = 0x00;
			buf[10] = 0x00;
			// tag
			ttLibC_Flv1 *flv1 = (ttLibC_Flv1 *)video;
			switch(flv1->type) {
			case Flv1Type_disposableInner:
				buf[11] = 0x32;
				break;
			case Flv1Type_inner:
				buf[11] = 0x22;
				break;
			case Flv1Type_intra:
				buf[11] = 0x12;
				break;
			}
			// header
			if(!callback(ptr, buf, 12)) {
				return false;
			}
			// data_body
			if(!callback(ptr, buffer, left_size)) {
				return false;
			}
			// post size
			uint32_t post_size = pre_size + 11;
			buf[0] = (post_size >> 24) & 0xFF;
			buf[1] = (post_size >> 16) & 0xFF;
			buf[2] = (post_size >> 8) & 0xFF;
			buf[3] = post_size & 0xFF;
			if(!callback(ptr, buf, 4)) {
				return false;
			}
			// done.
			return true;
		}
		break;
	case frameType_h264:
		{
			ttLibC_H264 *h264 = (ttLibC_H264*)frame;
			switch(h264->type) {
			case H264Type_configData:
				// need to make media sequence header.
				// if crc32 value is same, the data is same.
				{
					uint32_t crc32_value = ttLibC_H264_getConfigCrc32(h264);
					if(writer->video_track.crc32 == 0 || writer->video_track.crc32 != crc32_value) {
						/*
						 * 09 s  i  ze ti me st mp tr ck id 17 00 00 00 00 avccTag
						 */
						uint8_t avcc[256];
						size_t size = ttLibC_H264_readAvccTag(h264, avcc, sizeof(avcc));
						// make up buf.
						buf[0]  = 0x09;
						uint32_t pre_size = size + 4 + 1;
						buf[1]  = (pre_size >> 16) & 0xFF;
						buf[2]  = (pre_size >> 8) & 0xFF;
						buf[3]  = pre_size & 0xFF;
						// pts
						buf[4]  = (h264->inherit_super.inherit_super.pts >> 16) & 0xFF;
						buf[5]  = (h264->inherit_super.inherit_super.pts >> 8) & 0xFF;
						buf[6]  = h264->inherit_super.inherit_super.pts & 0xFF;
						buf[7]  = (h264->inherit_super.inherit_super.pts >> 24) & 0xFF;
						// track
						buf[8]  = 0x00;
						buf[9]  = 0x00;
						buf[10] = 0x00;
						// tag
						buf[11] = 0x17;
						// type
						buf[12] = 0x00;
						// dts
						buf[13] = 0x00;
						buf[14] = 0x00;
						buf[15] = 0x00;
						if(!callback(ptr, buf, 16)) {
							return false;
						}
						// data
						if(!callback(ptr, avcc, size)) {
							return false;
						}
						// post size
						uint32_t post_size = pre_size + 11;
						buf[0] = (post_size >> 24) & 0xFF;
						buf[1] = (post_size >> 16) & 0xFF;
						buf[2] = (post_size >> 8) & 0xFF;
						buf[3] = post_size & 0xFF;
						if(!callback(ptr, buf, 4)) {
							return false;
						}
						// all ok.
						writer->video_track.crc32 = crc32_value;
					}
					return true;
				}
				break;
			case H264Type_slice:
			case H264Type_sliceIDR:
				{
					uint32_t pre_size = 5; // codec + type + dts(3byte)
					ttLibC_H264_NalInfo nal_info;
					uint8_t *data = h264->inherit_super.inherit_super.data;
					size_t data_size = h264->inherit_super.inherit_super.buffer_size;
					while(ttLibC_H264_getNalInfo(&nal_info, data, data_size)) {
						pre_size += 4; // size;
						pre_size += nal_info.nal_size - nal_info.data_pos;
						data += nal_info.nal_size;
						data_size -= nal_info.nal_size;
					}
					buf[0]  = 0x09;
					buf[1]  = (pre_size >> 16) & 0xFF;
					buf[2]  = (pre_size >> 8) & 0xFF;
					buf[3]  = pre_size & 0xFF;
					// pts
					buf[4]  = (h264->inherit_super.inherit_super.pts >> 16) & 0xFF;
					buf[5]  = (h264->inherit_super.inherit_super.pts >> 8) & 0xFF;
					buf[6]  = h264->inherit_super.inherit_super.pts & 0xFF;
					buf[7]  = (h264->inherit_super.inherit_super.pts >> 24) & 0xFF;
					// track
					buf[8]  = 0x00;
					buf[9]  = 0x00;
					buf[10] = 0x00;
					// tag
					if(h264->type == H264Type_slice) {
						buf[11] = 0x27;
					}
					else {
						buf[11] = 0x17;
					}
					// type
					buf[12] = 0x01;
					// dts
					buf[13] = 0x00;
					buf[14] = 0x00;
					buf[15] = 0x00;
					if(!callback(ptr, buf, 16)) {
						return false;
					}
					// now ready to make data.
					data = h264->inherit_super.inherit_super.data;
					data_size = h264->inherit_super.inherit_super.buffer_size;
					// nal -> sizenal.
					while(ttLibC_H264_getNalInfo(&nal_info, data, data_size)) {
						uint32_t size = nal_info.nal_size - nal_info.data_pos;
						buf[0] = (size >> 24) & 0xFF;
						buf[1] = (size >> 16) & 0xFF;
						buf[2] = (size >> 8) & 0xFF;
						buf[3] = size & 0xFF;
						if(!callback(ptr, buf, 4)) {
							return false;
						}
						if(!callback(ptr, data + nal_info.data_pos, size)) {
							return false;
						}
						data += nal_info.nal_size;
						data_size -= nal_info.nal_size;
					}
					// post size
					uint32_t post_size = pre_size + 11;
					buf[0] = (post_size >> 24) & 0xFF;
					buf[1] = (post_size >> 16) & 0xFF;
					buf[2] = (post_size >> 8) & 0xFF;
					buf[3] = post_size & 0xFF;
					if(!callback(ptr, buf, 4)) {
						return false;
					}
					return true;
				}
				break;
			default:
				ERR_PRINT("unexpected h264 type.:%d", h264->type);
				return false;
			}
		}
		break;
	case frameType_vp6:
		break;
	default:
		ERR_PRINT("unexpected frame type of flv.");
		return false;
	}
	return false;
}
