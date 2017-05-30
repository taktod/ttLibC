/*
 * @file   metaTag.c
 * @brief  flvTag for meta.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/12
 */

#include "metaTag.h"
#include "../flvTag.h"
#include "../../../_log.h"
#include "../../../util/hexUtil.h"
#include "../../../util/ioUtil.h"

ttLibC_FlvMetaTag *ttLibC_FlvMetaTag_make(
		ttLibC_FlvTag *prev_tag,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		uint32_t track_id) {
	ttLibC_FlvMetaTag *metaTag = (ttLibC_FlvMetaTag *)ttLibC_FlvTag_make(
			prev_tag,
			data,
			data_size,
			non_copy_mode,
			pts,
			timebase,
			FlvType_meta,
			track_id);
	if(metaTag != NULL) {
	}
	return metaTag;
}

// if need to get meta information, hold binary data and read the binary.
ttLibC_FlvMetaTag *ttLibC_FlvMetaTag_getTag(
		ttLibC_FlvTag *prev_tag,
		uint8_t *data,
		size_t data_size) {
	/**
	 * 1byte flag
	 * 3byte size
	 * 3byte timestamp
	 * 1byte timestamp-ext
	 * 3byte track_id
	 */
	size_t size = ((data[1] << 16) & 0xFF0000) | ((data[2] << 8) & 0xFF00) | (data[3] & 0xFF);
	uint32_t timestamp = ((data[4] << 16) & 0xFF0000) | ((data[5] << 8) & 0xFF00) | (data[6] & 0xFF) | ((data[7] << 24) & 0xFF000000);
	uint32_t track_id = ((data[8] << 16) & 0xFF0000) | ((data[9] << 8) & 0xFF00) | (data[10] & 0xFF);
	data += 11;
	data_size -= 11;
	size_t post_size = ((*(data + data_size - 4)) << 24) | ((*(data + data_size - 3)) << 16) | ((*(data + data_size - 2)) << 8) | (*(data + data_size - 1));
	if(size + 11 != post_size) {
		ERR_PRINT("crazy size data, out of flv format.");
		return NULL;
	}
	return ttLibC_FlvMetaTag_make(
			prev_tag,
			data,
			data_size - 4,
			true,
			timestamp,
			1000,
			track_id);
}

