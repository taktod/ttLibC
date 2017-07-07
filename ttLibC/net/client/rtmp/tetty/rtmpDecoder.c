/**
 * @file   rtmpDecoder.c
 * @brief  tetty handler for rtmpDecoder.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/07
 */
#ifdef __ENABLE_SOCKET__

#include "rtmpDecoder.h"
#include "../../../../ttLibC_predef.h"
#include "../../../../_log.h"
#include "../../../../allocator.h"
#include <string.h>
#include "../data/clientObject.h"
#include "../../../../util/hexUtil.h"
#include "../message/rtmpMessage.h"

static tetty_errornum RtmpDecoder_channelRead(
		ttLibC_TettyContext *ctx,
		void *data,
		size_t data_size) {
	// decode recv data.
	// append on recv buffer.
	ttLibC_ClientObject *client_object = (ttLibC_ClientObject *)ctx->socket_info->ptr;
	ttLibC_DynamicBuffer_append(client_object->recv_buffer, data, data_size);
	// binary -> rtmpMessage.
	ttLibC_RtmpMessage *message = NULL;
	while((message = ttLibC_RtmpMessage_readBinary(client_object->recv_buffer, client_object)) != NULL) {
		// success to make message. pass to next handler.
		tetty_errornum err = ttLibC_TettyContext_super_channelRead(ctx, message, sizeof(ttLibC_RtmpMessage));
		ttLibC_RtmpMessage_close(&message);
		if(err != 0) {
			return err;
		}
	}
	return 0;
}

ttLibC_RtmpDecoder TT_VISIBILITY_HIDDEN *ttLibC_RtmpDecoder_make() {
	ttLibC_RtmpDecoder *decoder = ttLibC_malloc(sizeof(ttLibC_RtmpDecoder));
	if(decoder == NULL) {
		return NULL;
	}
	memset(decoder, 0, sizeof(ttLibC_RtmpDecoder));
	decoder->channel_handler.channelRead = RtmpDecoder_channelRead;
	return decoder;
}
void TT_VISIBILITY_HIDDEN ttLibC_RtmpDecoder_close(ttLibC_RtmpDecoder **decoder) {
	ttLibC_RtmpDecoder *target = (ttLibC_RtmpDecoder *)*decoder;
	if(target == NULL) {
		return;
	}
	ttLibC_free(target);
	*decoder = NULL;
}

#endif
