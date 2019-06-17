/*
 * rtmpDecoder.c
 *
 *  Created on: 2017/07/21
 *      Author: taktod
 */

#include "rtmpDecoder.h"
#include "../../../../_log.h"
#include "../../../../allocator.h"
#include <string.h>
#include "../data/clientObject.h"
#include "../../../../util/hexUtil.h"
#include "../message/rtmpMessage.h"

static tetty2_errornum RtmpDecoder_channelRead(
		ttLibC_Tetty2Context *ctx,
		void *data,
		size_t data_size) {
	// decode recv data.
	// append on recv buffer.
	ttLibC_ClientObject *client_object = (ttLibC_ClientObject *)ctx->tetty_info->ptr;
	ttLibC_DynamicBuffer_append(client_object->recv_buffer, data, data_size);
	// binary -> rtmpMessage.
	ttLibC_RtmpMessage *message = NULL;
	while((message = ttLibC_RtmpMessage_readBinary(client_object->recv_buffer, client_object)) != NULL) {
		// success to make message. pass to next handler.
		tetty2_errornum err = ttLibC_Tetty2Context_super_channelRead(ctx, message, sizeof(ttLibC_RtmpMessage));
		ttLibC_RtmpMessage_close(&message);
		if(err != 0) {
			return err;
		}
	}
	return 0;
}

ttLibC_RtmpDecoder TT_ATTRIBUTE_INNER *ttLibC_RtmpDecoder_make() {
	ttLibC_RtmpDecoder *decoder = ttLibC_malloc(sizeof(ttLibC_RtmpDecoder));
	if(decoder == NULL) {
		return NULL;
	}
	memset(decoder, 0, sizeof(ttLibC_RtmpDecoder));
	decoder->channel_handler.channelRead = RtmpDecoder_channelRead;
	return decoder;
}
void TT_ATTRIBUTE_INNER ttLibC_RtmpDecoder_close(ttLibC_RtmpDecoder **decoder) {
	ttLibC_RtmpDecoder *target = (ttLibC_RtmpDecoder *)*decoder;
	if(target == NULL) {
		return;
	}
	ttLibC_free(target);
	*decoder = NULL;
}

