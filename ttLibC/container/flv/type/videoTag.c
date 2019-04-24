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
#include "../../../ttLibC_predef.h"
#include "../../../_log.h"
#include "../../../frame/video/h264.h"
#include "../../../frame/video/flv1.h"
#include "../../../util/hexUtil.h"
#include "../../../util/ioUtil.h"
#include "../../../util/flvFrameUtil.h"
#include "../../../util/dynamicBufferUtil.h"

ttLibC_FlvVideoTag TT_VISIBILITY_HIDDEN *ttLibC_FlvVideoTag_make(
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

ttLibC_FlvVideoTag TT_VISIBILITY_HIDDEN *ttLibC_FlvVideoTag_getTag(
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
//	uint8_t frame_type = ((data[11] >> 4) & 0x0F);
	uint8_t codec_id = data[11] & 0x0F;
//	uint32_t dts = 0;
	data += 11;
	data_size -= 11;

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

bool TT_VISIBILITY_HIDDEN ttLibC_FlvVideoTag_getFrame(
		ttLibC_FlvVideoTag *video_tag,
		ttLibC_getFrameFunc callback,
		void *ptr) {
	return ttLibC_FlvFrameManager_readVideoBinary(
			video_tag->inherit_super.frameManager,
			video_tag->inherit_super.inherit_super.inherit_super.data,
			video_tag->inherit_super.inherit_super.inherit_super.buffer_size,
			video_tag->inherit_super.inherit_super.inherit_super.pts,
			callback,
			ptr);
}

bool TT_VISIBILITY_HIDDEN ttLibC_FlvVideoTag_writeTag(
		ttLibC_FlvWriter_ *writer,
		ttLibC_Frame *frame,
		ttLibC_ContainerWriteFunc callback,
		void *ptr) {
	uint64_t pts = frame->pts;
	if(frame->type == frameType_h264) {
		ttLibC_H264 *h264 = (ttLibC_H264 *)frame;
		if(h264->type == H264Type_sliceIDR) {
			uint32_t crc32_value = ttLibC_H264_getConfigCrc32((ttLibC_H264 *)writer->video_track.configData);
			if(writer->video_track.crc32 != crc32_value) {
				writer->video_track.configData->pts = frame->pts;
				writer->video_track.configData->dts = frame->dts;
				if(!ttLibC_FlvVideoTag_writeTag(writer, writer->video_track.configData, callback, ptr)) {
					return false;
				}
				writer->video_track.crc32 = crc32_value;
			}
		}
		pts = frame->dts;
	}
	ttLibC_DynamicBuffer *buffer = ttLibC_DynamicBuffer_make();
	ttLibC_DynamicBuffer_alloc(buffer, 11);
	uint8_t *data = ttLibC_DynamicBuffer_refData(buffer);
	data[0] = 0x09;
	// size is update later.
	data[1]  = 0x00;
	data[2]  = 0x00;
	data[3]  = 0x00;
	// get pts from frame information.
	data[4]  = (pts >> 16) & 0xFF;
	data[5]  = (pts >> 8) & 0xFF;
	data[6]  = pts & 0xFF;
	data[7]  = (pts >> 24) & 0xFF;
	// streamId...
	data[8]  = 0x00;
	data[9]  = 0x00;
	data[10] = 0x00;
	// append frame data.
	ttLibC_FlvFrameManager_getData(
			frame,
			buffer);
	// update size.
	uint32_t size = ttLibC_DynamicBuffer_refSize(buffer) - 11;
	data = ttLibC_DynamicBuffer_refData(buffer);
	data[1]  = (size >> 16) & 0xFF;
	data[2]  = (size >> 8) & 0xFF;
	data[3]  = size & 0xFF;
/*	if(frame->type == frameType_h264) {
		uint32_t offset = frame->pts - frame->dts;
		data[13] = (offset >> 16) & 0xFF;
		data[14] = (offset >> 8) & 0xFF;
		data[15] = offset & 0xFF;
	}*/
	// update size 2.
	uint32_t endSize = ttLibC_DynamicBuffer_refSize(buffer);
	uint32_t be_endSize = be_uint32_t(endSize);
	ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&be_endSize, 4);

	bool result = true;
	if(callback != NULL) {
		result = callback(
				ptr,
				ttLibC_DynamicBuffer_refData(buffer),
				ttLibC_DynamicBuffer_refSize(buffer));
	}
	ttLibC_DynamicBuffer_close(&buffer);
	return result;
}
