/**
 * @file   rtmpEncoder.c
 * @brief  tetty handler for rtmpEncoder.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/07
 */
#ifdef __ENABLE_SOCKET__

#include "rtmpEncoder.h"
#include "../../../../log.h"
#include "../../../../allocator.h"
#include <string.h>
#include "../message/rtmpMessage.h"
#include "../data/clientObject.h"
#include "../../../../util/hexUtil.h"

static tetty_errornum RtmpEncoder_write(
		ttLibC_TettyContext *ctx,
		void *data,
		size_t data_size) {
	// message obj -> binary stream.
	ttLibC_RtmpMessage *message = (ttLibC_RtmpMessage *)data;
	ttLibC_ClientObject *client_object = (ttLibC_ClientObject *)ctx->socket_info->ptr;
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
		ttLibC_TettyContext_super_write(ctx, header, size);

		// write data
		size_t write_size = (buffer_size > client_object->send_chunk_size ? client_object->send_chunk_size : buffer_size);
		ttLibC_TettyContext_super_write(ctx, buffer, write_size);
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

ttLibC_RtmpEncoder *ttLibC_RtmpEncoder_make() {
	ttLibC_RtmpEncoder *encoder = ttLibC_malloc(sizeof(ttLibC_RtmpEncoder));
	if(encoder == NULL) {
		return NULL;
	}
	memset(encoder, 0, sizeof(ttLibC_RtmpEncoder));
	encoder->channel_handler.write = RtmpEncoder_write;
	return encoder;
}

void ttLibC_RtmpEncoder_close(ttLibC_RtmpEncoder **encoder) {
	ttLibC_RtmpEncoder *target = (ttLibC_RtmpEncoder *)*encoder;
	if(target == NULL) {
		return;
	}
	ttLibC_free(target);
	*encoder = NULL;
}

#endif
