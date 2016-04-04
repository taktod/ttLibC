/**
 * @file   amf0DataMessage.c
 * @brief  rtmp message amf0Data message.(meta data?)
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/21
 */
#ifdef __ENABLE_SOCKET__

#include "amf0DataMessage.h"
#include "../../../../log.h"
#include "../../../../allocator.h"
#include <string.h>
#include "../../../../util/hexUtil.h"

ttLibC_Amf0DataMessage *ttLibC_Amf0DataMessage_make(const char *message_name) {
	ttLibC_Amf0DataMessage *message = ttLibC_malloc(sizeof(ttLibC_Amf0DataMessage));
	if(message == NULL) {
		return NULL;
	}
	// make header
	ttLibC_RtmpHeader *header = ttLibC_RtmpHeader_make(3, 0, RtmpMessageType_amf0DataMessage, 0);
	if(header == NULL) {
		ttLibC_free(message);
		return NULL;
	}
	message->inherit_super.header = header;
	strcpy((char *)message->message_name, message_name);
	message->obj1 = NULL;
	message->obj2 = NULL;
	return message;
}

static bool Amf0DataMessage_readBinaryObjectCallback(void *ptr, ttLibC_Amf0Object *amf0_obj) {
	ttLibC_Amf0DataMessage *message = (ttLibC_Amf0DataMessage *)ptr;
	if(strlen((const char *)message->message_name) == 0) {
		if(amf0_obj->type == amf0Type_String) {
			strcpy((char *)message->message_name, (const char *)amf0_obj->object);
		}
		else {
			return false;
		}
	}
	else if(message->obj1 == NULL) {
		message->obj1 = ttLibC_Amf0_clone(amf0_obj);
	}
	else if(message->obj2 == NULL) {
		message->obj2 = ttLibC_Amf0_clone(amf0_obj);
	}
	else {
		return false;
	}
	return true;
}

ttLibC_Amf0DataMessage *ttLibC_Amf0DataMessage_readBinary(
		uint8_t *data,
		size_t data_size) {
	ttLibC_Amf0DataMessage *message = ttLibC_Amf0DataMessage_make("");
	if(message == NULL) {
		return NULL;
	}
	bool result = ttLibC_Amf0_read(data, data_size, Amf0DataMessage_readBinaryObjectCallback, message);
	if(!result) {
		ttLibC_Amf0DataMessage_close(&message);
		return NULL;
	}
	return message;
}

void ttLibC_Amf0DataMessage_close(ttLibC_Amf0DataMessage **message) {
	ttLibC_Amf0DataMessage *target = (ttLibC_Amf0DataMessage *)*message;
	if(target == NULL) {
		return;
	}
	ttLibC_RtmpHeader_close(&target->inherit_super.header);
	ttLibC_Amf0_close(&target->obj1);
	ttLibC_Amf0_close(&target->obj2);
	ttLibC_free(target);
	*message = NULL;
}

#endif
