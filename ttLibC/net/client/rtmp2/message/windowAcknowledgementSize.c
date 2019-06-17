/*
 * windowAcknowledgementSize.c
 *
 *  Created on: 2017/07/22
 *      Author: taktod
 */

#include "windowAcknowledgementSize.h"
#include "../../../../_log.h"
#include "../../../../allocator.h"
#include "../../../../util/ioUtil.h"
#include <string.h>

ttLibC_WindowAcknowledgementSize TT_ATTRIBUTE_INNER *ttLibC_WindowAcknowledgementSize_make(uint32_t size) {
	ttLibC_WindowAcknowledgementSize *win_ack = ttLibC_malloc(sizeof(ttLibC_WindowAcknowledgementSize));
	if(win_ack == NULL) {
		return NULL;
	}
	ttLibC_RtmpHeader *header = ttLibC_RtmpHeader_make(2, 0, RtmpMessageType_windowAcknowledgementSize, 0);
	if(header == NULL) {
		ttLibC_free(win_ack);
		return NULL;
	}
	win_ack->inherit_super.header = header;
	win_ack->size = size;
	return win_ack;
}

bool TT_ATTRIBUTE_INNER ttLibC_WindowAcknowledgementSize_getData(
		ttLibC_WindowAcknowledgementSize *win_ack,
		ttLibC_DynamicBuffer *buffer) {
	uint32_t size = be_uint32_t(win_ack->size);
	ttLibC_DynamicBuffer_append(buffer, (uint8_t *)&size, sizeof(size));
	return true;
}

void TT_ATTRIBUTE_INNER ttLibC_WindowAcknowledgementSize_close(ttLibC_WindowAcknowledgementSize **win_ack) {
	ttLibC_WindowAcknowledgementSize *target = (ttLibC_WindowAcknowledgementSize *)*win_ack;
	if(target == NULL) {
		return;
	}
	ttLibC_RtmpHeader_close(&target->inherit_super.header);
	ttLibC_free(target);
	*win_ack = NULL;
}

