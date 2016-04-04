/**
 * @file   acknowledgement.c
 * @brief  rtmp message acknowledgement
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/11
 */
#ifdef __ENABLE_SOCKET__

#include "acknowledgement.h"
#include "../../../../log.h"
#include "../../../../allocator.h"
#include <string.h>

ttLibC_Acknowledgement *ttLibC_Acknowledgement_make(uint32_t size) {
	ttLibC_Acknowledgement *ack = ttLibC_malloc(sizeof(ttLibC_Acknowledgement));
	if(ack == NULL) {
		return NULL;
	}
	ttLibC_RtmpHeader *header = ttLibC_RtmpHeader_make(2, 0, RtmpMessageType_acknowledgement, 0);
	if(header == NULL) {
		ttLibC_free(ack);
		return NULL;
	}
	ack->inherit_super.header = header;
	ack->size = size;
	return ack;
}

void ttLibC_Acknowledgement_close(ttLibC_Acknowledgement **ack) {
	ttLibC_Acknowledgement *target = (ttLibC_Acknowledgement *)*ack;
	if(target == NULL) {
		return;
	}
	ttLibC_RtmpHeader_close(&target->inherit_super.header);
	ttLibC_free(target);
	*ack = NULL;
}

#endif
