/*
 * rtmpEncoder.c
 *
 *  Created on: 2017/07/21
 *      Author: taktod
 */

#include "rtmpEncoder.h"
#include "../../../../_log.h"
#include "../../../../allocator.h"
#include <string.h>
#include "../message/rtmpMessage.h"
#include "../data/clientObject.h"
#include "../../../../util/hexUtil.h"

static tetty2_errornum RtmpEncoder_write(
		ttLibC_Tetty2Context *ctx,
		void *data,
		size_t data_size) {
	(void)data_size;
	// message obj -> binary stream.
	ttLibC_RtmpMessage *message = (ttLibC_RtmpMessage *)data;
	ttLibC_ClientObject *client_object = (ttLibC_ClientObject *)ctx->tetty_info->ptr;
	ttLibC_DynamicBuffer_empty(client_object->send_buffer);
	// update sendBuffer
	if(!ttLibC_RtmpMessage_getData(
			client_object,
			message,
			client_object->send_buffer)) {
		// something happen.
		ctx->bootstrap->error_number = -1;
		return -1;
	}
	// now ready to send.
	uint8_t *buffer = ttLibC_DynamicBuffer_refData(client_object->send_buffer);
	size_t buffer_size = ttLibC_DynamicBuffer_refSize(client_object->send_buffer);
	if(buffer_size == 0) {
		return 0;
	}

	// update rtmpHeader.
	ttLibC_RtmpHeader *prev_header = ttLibC_StlMap_get(client_object->send_headers, (void *)((long)message->header->cs_id));
	if(prev_header != NULL) {
		if(prev_header->stream_id != message->header->stream_id) {
			// if streamId is changed -> type0
			message->header->type = Type0;
		}
		else {
			// type 1 or 2 or 3.
			message->header->delta_time = message->header->timestamp - prev_header->timestamp;
			if(prev_header->size != message->header->size
			|| prev_header->message_type != message->header->message_type) {
				message->header->type = Type1;
			}
			else if(prev_header->delta_time != message->header->delta_time) {
				message->header->type = Type2;
			}
			else {
				message->header->type = Type3;
			}
		}
	}

	uint8_t header[20];
	message->header->size = buffer_size;
	do {
		// write header.
		size_t size = ttLibC_RtmpHeader_getData(message->header, header, 20);
		ttLibC_Tetty2Context_super_write(ctx, header, size);

		// write data
		size_t write_size = (buffer_size > client_object->send_chunk_size ? client_object->send_chunk_size : buffer_size);
		ttLibC_Tetty2Context_super_write(ctx, buffer, write_size);
		buffer += write_size;
		buffer_size -= write_size;
		// if remain, use type3 header.
		message->header->type = Type3;
	} while(buffer_size > 0); // loop until complete.
	// hold header for next message.
	prev_header = ttLibC_RtmpHeader_copy(prev_header, message->header);
	ttLibC_StlMap_put(client_object->send_headers, (void *)((long)prev_header->cs_id), prev_header);
	return 0;
}

ttLibC_RtmpEncoder TT_ATTRIBUTE_INNER *ttLibC_RtmpEncoder_make() {
	ttLibC_RtmpEncoder *encoder = ttLibC_malloc(sizeof(ttLibC_RtmpEncoder));
	if(encoder == NULL) {
		return NULL;
	}
	memset(encoder, 0, sizeof(ttLibC_RtmpEncoder));
	encoder->channel_handler.write = RtmpEncoder_write;
	return encoder;
}

void TT_ATTRIBUTE_INNER ttLibC_RtmpEncoder_close(ttLibC_RtmpEncoder **encoder) {
	ttLibC_RtmpEncoder *target = (ttLibC_RtmpEncoder *)*encoder;
	if(target == NULL) {
		return;
	}
	ttLibC_free(target);
	*encoder = NULL;
}
