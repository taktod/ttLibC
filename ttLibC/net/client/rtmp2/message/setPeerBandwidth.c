/*
 * setPeerBandwidth.c
 *
 *  Created on: 2017/07/22
 *      Author: taktod
 */

#include "setPeerBandwidth.h"
#include "../../../../ttLibC_predef.h"
#include "../../../../_log.h"
#include "../../../../allocator.h"
#include <string.h>

ttLibC_SetPeerBandwidth TT_VISIBILITY_HIDDEN *ttLibC_SetPeerBandwidth_make(
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

void TT_VISIBILITY_HIDDEN ttLibC_SetPeerBandwidth_close(ttLibC_SetPeerBandwidth **bandwidth) {
	ttLibC_SetPeerBandwidth *target = (ttLibC_SetPeerBandwidth *)*bandwidth;
	if(target == NULL) {
		return;
	}
	ttLibC_RtmpHeader_close(&target->inherit_super.header);
	ttLibC_free(target);
	*bandwidth = NULL;
}

