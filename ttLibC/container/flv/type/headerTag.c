/*
 * @file   headerTag.c
 * @brief  flvTag for file header.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/12
 */

#include "headerTag.h"

#include "../../../log.h"
#include "../../../util/ioUtil.h"
#include "../../../util/hexUtil.h"

ttLibC_FlvHeaderTag *ttLibC_FlvHeaderTag_make(
		ttLibC_FlvTag *prev_tag,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		uint32_t track_id,
		bool has_audio,
		bool has_video) {
	ttLibC_FlvHeaderTag *headerTag = (ttLibC_FlvHeaderTag *)ttLibC_FlvTag_make(
			prev_tag,
			data,
			data_size,
			non_copy_mode,
			pts,
			timebase,
			FlvType_header,
			track_id);
	if(headerTag != NULL) {
		headerTag->has_audio = has_audio;
		headerTag->has_video = has_video;
	}
	return headerTag;
}

ttLibC_FlvHeaderTag *ttLibC_FlvHeaderTag_getTag(
		ttLibC_FlvTag *prev_tag,
		uint8_t *data,
		size_t data_size) {
	if(data_size < 13) {
		ERR_PRINT("size is too small.");
		return NULL;
	}
	if(data[0] != 'F'
	|| data[1] != 'L'
	|| data[2] != 'V') {
		ERR_PRINT("start is not 'FLV', this is not flv format.");
		return NULL;
	}
	if(data[3] != 1) {
		ERR_PRINT("version is not 1.");
		return NULL;
	}
	bool has_audio = (data[4] & 0x04) != 0;
	bool has_video = (data[4] & 0x01) != 0;
	data += 5;
	if(be_uint32_t(*((uint32_t *)data)) != 9) {
		ERR_PRINT("data size is not 9, out of flv format?");
		return NULL;
	}
	return ttLibC_FlvHeaderTag_make(prev_tag, NULL, 0, true, 0, 1000, 1, has_audio, has_video);
 }

bool ttLibC_FlvHeaderTag_writeTag(
		ttLibC_FlvWriter_ *writer,
		ttLibC_ContainerWriteFunc callback,
		void *ptr) {
	uint8_t buf[13];
	size_t size = ttLibC_HexUtil_makeBuffer("464C5601000000000900000000", buf, 13);
	if(writer->audio_track.frame_type != frameType_unknown) {
		buf[4] |= 0x04;
	}
	if(writer->video_track.frame_type != frameType_unknown) {
		buf[4] |= 0x01;
	}
	callback(ptr, buf, 13);
	return true;
}
