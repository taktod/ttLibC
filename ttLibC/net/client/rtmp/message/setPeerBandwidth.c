/**
 * @file   setPeerBandwidth.c
 * @brief  rtmp message setPeerBandwidth
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/11
 */
#ifdef __ENABLE_SOCKET__

#include "setPeerBandwidth.h"
#include "../../../../ttLibC_predef.h"
#include "../../../../_log.h"
#include "../../../../allocator.h"
#include <string.h>

ttLibC_SetPeerBandwidth *ttLibC_SetPeerBandwidth_make(
		uint32_t size,
		ttLibC_SetPeerBandwidth_LimitType limit_type) {
	ttLibC_SetPeerBandwidth *bandwidth = ttLibC_malloc(sizeof(ttLibC_SetPeerBandwidth));
	if(bandwidth == NULL) {
		return NULL;
	}
	ttLibC_RtmpHeader *header = ttLibC_RtmpHeader_make(2, 0, RtmpMessageType_setPeerBandwidth, 0);
	if(header == NULL) {
		ttLibC_free(bandwidth);
		return NULL;
	}
	bandwidth->inherit_super.header = header;
	bandwidth->size = size;
	bandwidth->limit_type = limit_type;
	return bandwidth;
}

void ttLibC_SetPeerBandwidth_close(ttLibC_SetPeerBandwidth **bandwidth) {
	ttLibC_SetPeerBandwidth *target = (ttLibC_SetPeerBandwidth *)*bandwidth;
	if(target == NULL) {
		return;
	}
	ttLibC_RtmpHeader_close(&target->inherit_super.header);
	ttLibC_free(target);
	*bandwidth = NULL;
}

#endif
