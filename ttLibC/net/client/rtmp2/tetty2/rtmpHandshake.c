/*
 * rtmpHandshake.c
 *
 *  Created on: 2017/07/21
 *      Author: taktod
 */

#ifdef __ENABLE_SOCKET__

#include "rtmpHandshake.h"
#include "../data/clientObject.h"

#include "../../../../_log.h"
#include "../../../../allocator.h"
#include "../../../../util/hexUtil.h"

#include <string.h>

static tetty2_errornum RtmpHandshake_channelActive(ttLibC_Tetty2Context *ctx) {
	// send c1 c2 for server.
	ttLibC_ClientObject *client_object = (ttLibC_ClientObject *)ctx->tetty_info->ptr;
	if(client_object != NULL) {
		ERR_PRINT("already client_object is generated, unexpected.");
		ttLibC_ClientObject_close(&client_object);
	}
	client_object = ttLibC_ClientObject_make();

	ctx->tetty_info->ptr = client_object;

	// c1
	uint8_t c1[1] = {0x03};
	ttLibC_Tetty2Context_super_write(ctx, c1, 1);
	// c2
	size_t c2_size = 1536;
	uint8_t *c2 = ttLibC_malloc(c2_size);
	// TODO make random.
	for(uint32_t i = 0;i < c2_size;++ i) {
		c2[i] = (uint8_t)(i % 256);
	}
	c2[4] = 0; // for red5 handshake.
	client_object->c2_value = c2;
	ttLibC_Tetty2Context_super_write(ctx, c2, c2_size);
	ttLibC_Tetty2Context_channel_flush(ctx);
	return 0;
}

static tetty2_errornum RtmpHandshake_channelRead(
		ttLibC_Tetty2Context *ctx,
		void *data,
		size_t data_size) {
	ttLibC_ClientObject *client_object = (ttLibC_ClientObject *)ctx->tetty_info->ptr;
	if(client_object->is_handshake_done) {
		return ttLibC_Tetty2Context_super_channelRead(ctx, data, data_size);
	}
	ttLibC_DynamicBuffer_append(client_object->recv_buffer, (uint8_t *)data, data_size);
	switch(client_object->phase) {
	case phase_s1:
		if(ttLibC_DynamicBuffer_refSize(client_object->recv_buffer) < 1) {
			return 0;
		}
		else {
			uint8_t *buf = ttLibC_DynamicBuffer_refData(client_object->recv_buffer);
			if(buf[0] != 0x03) {
				ERR_PRINT("incompatible rtmp type.:%x", buf[0]);
				ctx->bootstrap->error_number = 1;
				return 0;
			}
			ttLibC_DynamicBuffer_markAsRead(client_object->recv_buffer, 1);
			client_object->phase = phase_s2;
		}
		/* no break */
	case phase_s2:
		if(ttLibC_DynamicBuffer_refSize(client_object->recv_buffer) < 1536) {
			return 0;
		}
		else {
			uint8_t *buf = ttLibC_DynamicBuffer_refData(client_object->recv_buffer);
			// c3 is same as s2.
			ttLibC_Tetty2Context_super_write(ctx, buf, 1536);

			client_object->phase = phase_s3;
			ttLibC_DynamicBuffer_markAsRead(client_object->recv_buffer, 1536);
			ttLibC_Tetty2Context_channel_flush(ctx);
		}
		/* no break */
	case phase_s3:
		if(ttLibC_DynamicBuffer_refSize(client_object->recv_buffer) < 1536) {
			return 0;
		}
		else {
			client_object->is_handshake_done = true;
			ttLibC_DynamicBuffer_empty(client_object->recv_buffer);

			ttLibC_RtmpHandshake *handshake = (ttLibC_RtmpHandshake *)ctx->channel_handler;
			if(handshake->handshake_promise != NULL) {
				ttLibC_Tetty2Promise_setSuccess(handshake->handshake_promise, NULL);
			}
			// done, call channel active for pipeline.
			ttLibC_Tetty2Context_super_channelActive(ctx);
		}
		break;
	default:
		break;
	}
	return 0;
}

static tetty2_errornum RtmpHandshake_close(ttLibC_Tetty2Context *ctx) {
	ttLibC_ClientObject *client_object = ctx->tetty_info->ptr;
	ttLibC_ClientObject_close(&client_object);
	return 0;
}

ttLibC_RtmpHandshake TT_ATTRIBUTE_INNER *ttLibC_RtmpHandshake_make() {
	ttLibC_RtmpHandshake *handshake = ttLibC_malloc(sizeof(ttLibC_RtmpHandshake));
	if(handshake == NULL) {
		return NULL;
	}
	memset(handshake, 0, sizeof(ttLibC_RtmpHandshake));
	handshake->channel_handler.channelActive = RtmpHandshake_channelActive;
	handshake->channel_handler.channelRead = RtmpHandshake_channelRead;
	handshake->channel_handler.close = RtmpHandshake_close;
	return handshake;
}

ttLibC_Tetty2Promise TT_ATTRIBUTE_INNER *ttLibC_RtmpHandshake_getHandshakePromise(
		ttLibC_Tetty2Bootstrap *bootstrap,
		ttLibC_RtmpHandshake *handshake) {
	ttLibC_Tetty2Promise *promise = ttLibC_Tetty2Bootstrap_makePromise(bootstrap);
	handshake->handshake_promise = promise;
	return promise;
}

void TT_ATTRIBUTE_INNER ttLibC_RtmpHandshake_close(ttLibC_RtmpHandshake **handshake) {
	ttLibC_RtmpHandshake *target = (ttLibC_RtmpHandshake *)*handshake;
	if(target == NULL) {
		return;
	}
	ttLibC_free(target);
	*handshake = NULL;
}

#endif