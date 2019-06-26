/**
 * @file   setChunkSize.c
 * @brief  rtmp message setChunkSize
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/11
 */
#ifdef __ENABLE_SOCKET__

#include "setChunkSize.h"
#include "../../../../_log.h"
#include "../../../../allocator.h"
#include <string.h>

ttLibC_SetChunkSize TT_ATTRIBUTE_INNER *ttLibC_SetChunkSize_make(uint32_t size) {
	ttLibC_SetChunkSize *chunk_size = ttLibC_malloc(sizeof(ttLibC_SetChunkSize));
	if(chunk_size == NULL) {
		return NULL;
	}
	ttLibC_RtmpHeader *header = ttLibC_RtmpHeader_make(2, 0, RtmpMessageType_setChunkSize, 0);
	if(header == NULL) {
		ttLibC_free(chunk_size);
		return NULL;
	}
	chunk_size->inherit_super.header = header;
	chunk_size->size = size;
//	ttLibC_RtmpMessage *rtmpMsg = (ttLibC_RtmpMessage *)chunk_size;
	return chunk_size;
}

void TT_ATTRIBUTE_INNER ttLibC_SetChunkSize_close(ttLibC_SetChunkSize **chunk_size) {
	ttLibC_SetChunkSize *target = (ttLibC_SetChunkSize *)*chunk_size;
	if(target == NULL) {
		return;
	}
	ttLibC_RtmpHeader_close(&target->inherit_super.header);
	ttLibC_free(target);
	*chunk_size = NULL;
}

#endif
