/*
 * acknowledgement.c
 *
 *  Created on: 2017/07/22
 *      Author: taktod
 */

#ifdef __ENABLE_SOCKET__

#include "acknowledgement.h"
#include "../../../../_log.h"
#include "../../../../allocator.h"
#include <string.h>

ttLibC_Acknowledgement TT_ATTRIBUTE_INNER *ttLibC_Acknowledgement_make(uint32_t size) {
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

bool TT_ATTRIBUTE_INNER ttLibC_Acknowledgement_getData(
		ttLibC_Acknowledgement *acknowledgement,
		ttLibC_DynamicBuffer *buffer) {
	uint32_t size = be_uint32_t(acknowledgement->size);
	ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&size, sizeof(size));
	return true;
}

void TT_ATTRIBUTE_INNER ttLibC_Acknowledgement_close(ttLibC_Acknowledgement **ack) {
	ttLibC_Acknowledgement *target = (ttLibC_Acknowledgement *)*ack;
	if(target == NULL) {
		return;
	}
	ttLibC_RtmpHeader_close(&target->inherit_super.header);
	ttLibC_free(target);
	*ack = NULL;
}

#endif