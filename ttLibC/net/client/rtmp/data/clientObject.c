/**
 * @file   clientObject.c
 * @brief  data def for client.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/07
 */
#ifdef __ENABLE_SOCKET__

#include "clientObject.h"
#include "../../../../_log.h"
#include "../../../../allocator.h"
#include "../header/rtmpHeader.h"
#include "../message/rtmpMessage.h"

ttLibC_ClientObject *ttLibC_ClientObject_make() {
	ttLibC_ClientObject *client_object = ttLibC_malloc(sizeof(ttLibC_ClientObject));
	if(client_object == NULL) {
		return NULL;
	}
	client_object->error_number = 0;
	client_object->recv_chunk_size = 128;
	client_object->send_chunk_size = 128;
	client_object->c2_value = NULL;
	client_object->recv_buffer  = ttLibC_DynamicBuffer_make();
	client_object->recv_headers = ttLibC_StlMap_make();
	client_object->recv_buffers = ttLibC_StlMap_make();
	client_object->send_buffer  = ttLibC_DynamicBuffer_make();
	client_object->send_headers = ttLibC_StlMap_make();

	client_object->is_handshake_done = false;
	client_object->phase = phase_s1;

	client_object->next_command_id = 1;
	client_object->commandId_command_map = ttLibC_StlMap_make();
	client_object->streamId_rtmpStream_map = ttLibC_StlMap_make();
	return client_object;
}

static bool ClientObject_commandIdCommandCloseCallback(void *ptr, void *key, void *item) {
	(void)ptr;
	(void)key;
	ttLibC_RtmpMessage_close((ttLibC_RtmpMessage **)&item);
	return true;
}

/**
 * for release recv_buffers.
 * @param ptr
 * @param key
 * @param item
 * @return true to continue forEach.
 */
static bool ClientObject_recvBuffersCloseCallback(void *ptr, void *key, void *item) {
	(void)ptr;
	(void)key;
	ttLibC_DynamicBuffer_close((ttLibC_DynamicBuffer **)&item);
	return true;
}

/**
 * for send/recv_headers
 * @param ptr
 * @param key
 * @param item
 * @return true to continue forEach.
 */
static bool ClientObject_rtmpHeadersCloseCallback(void *ptr, void *key, void *item) {
	(void)ptr;
	(void)key;
	ttLibC_RtmpHeader_close((ttLibC_RtmpHeader **)&item);
	return true;
}

void ttLibC_ClientObject_close(ttLibC_ClientObject **client_object) {
	ttLibC_ClientObject *target = (ttLibC_ClientObject *)*client_object;
	if(target == NULL) {
		return;
	}
	if(target->c2_value != NULL) {
		ttLibC_free(target->c2_value);
	}
	ttLibC_DynamicBuffer_close(&target->recv_buffer);
	ttLibC_DynamicBuffer_close(&target->send_buffer);

	ttLibC_StlMap_forEach(target->send_headers, ClientObject_rtmpHeadersCloseCallback, NULL);
	ttLibC_StlMap_close(&target->send_headers);

	ttLibC_StlMap_forEach(target->recv_headers, ClientObject_rtmpHeadersCloseCallback, NULL);
	ttLibC_StlMap_close(&target->recv_headers);

	ttLibC_StlMap_forEach(target->recv_buffers, ClientObject_recvBuffersCloseCallback, NULL);
	ttLibC_StlMap_close(&target->recv_buffers);

	ttLibC_StlMap_forEach(target->commandId_command_map, ClientObject_commandIdCommandCloseCallback, NULL);
	ttLibC_StlMap_close(&target->commandId_command_map);

	// object in streamid_promise_map is release in rtmpStream, no necessary here.
	ttLibC_StlMap_close(&target->streamId_rtmpStream_map);

	ttLibC_free(target);
	*client_object = NULL;
}

#endif
