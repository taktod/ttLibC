/**
 * @file   rtmpMessage.c
 * @brief  rtmp message
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/08
 */
#ifdef __ENABLE_SOCKET__

#include "rtmpMessage.h"
#include "../../../../ttLibC_predef.h"
#include "../../../../_log.h"
#include "acknowledgement.h"
#include "aggregateMessage.h"
#include "amf0Command.h"
#include "amf0DataMessage.h"
#include "audioMessage.h"
#include "setChunkSize.h"
#include "setPeerBandwidth.h"
#include "userControlMessage.h"
#include "videoMessage.h"
#include "windowAcknowledgementSize.h"
#include "../data/clientObject.h"
#include "../../../../util/hexUtil.h"
#include "../../../../util/ioUtil.h"
#include "../../../../util/stlMapUtil.h"

bool TT_VISIBILITY_HIDDEN ttLibC_RtmpMessage_getData(
		ttLibC_ClientObject *client_object,
		ttLibC_RtmpMessage *message,
		ttLibC_DynamicBuffer *buffer) {
	(void)client_object;
	ttLibC_RtmpHeader *header = message->header;
	// rtmpMessage obj -> binary
	switch(header->message_type) {
	case RtmpMessageType_abortMessage:
		return false;
	case RtmpMessageType_acknowledgement:
		return ttLibC_Acknowledgement_getData((ttLibC_Acknowledgement *)message, buffer);
	case RtmpMessageType_aggregateMessage:
		return false;
	case RtmpMessageType_amf0Command:
		return ttLibC_Amf0Command_getData((ttLibC_Amf0Command *)message, buffer);
	case RtmpMessageType_amf0DataMessage:
	case RtmpMessageType_amf0SharedObjectMessage:
	case RtmpMessageType_amf3Command:
	case RtmpMessageType_amf3DataMessage:
	case RtmpMessageType_amf3SharedObjectMessage:
		return false;
	case RtmpMessageType_audioMessage:
		return ttLibC_AudioMessage_getData((ttLibC_AudioMessage *)message, buffer);
	case RtmpMessageType_setChunkSize:
	case RtmpMessageType_setPeerBandwidth:
		return false;
	case RtmpMessageType_userControlMessage:
		return ttLibC_UserControlMessage_getData((ttLibC_UserControlMessage *)message, buffer);
	case RtmpMessageType_videoMessage:
		return ttLibC_VideoMessage_getData((ttLibC_VideoMessage *)message, buffer);
	case RtmpMessageType_windowAcknowledgementSize:
		return ttLibC_WindowAcknowledgementSize_getData((ttLibC_WindowAcknowledgementSize *)message, buffer);
	default:
		return false;
	}
}

ttLibC_RtmpMessage TT_VISIBILITY_HIDDEN *ttLibC_RtmpMessage_readBinary(
		ttLibC_DynamicBuffer *buffer,
		ttLibC_ClientObject *client_object) {
	// binary -> rtmpMessage.
	ttLibC_DynamicBuffer *data_buffer = NULL;
	ttLibC_RtmpHeader *header = NULL;
	do {
		header = ttLibC_RtmpHeader_readBinary(buffer, client_object);
		if(header == NULL) {
			return NULL;
		}
		// get reuse data buffer.
		data_buffer = ttLibC_StlMap_get(client_object->recv_buffers, (void *)((long)header->cs_id));
		if(data_buffer == NULL) {
			data_buffer = ttLibC_DynamicBuffer_make();
			ttLibC_StlMap_put(client_object->recv_buffers, (void *)((long)header->cs_id), data_buffer);
		}
		// check the size to get. header->size or chunk_size.
		uint32_t target_size = header->size - ttLibC_DynamicBuffer_refSize(data_buffer);
		if(target_size > client_object->recv_chunk_size) {
			target_size = client_object->recv_chunk_size;
		}
		// update data buffer.
		ttLibC_DynamicBuffer_append(data_buffer, ttLibC_DynamicBuffer_refData(buffer), target_size);
		ttLibC_DynamicBuffer_markAsRead(buffer, target_size);
		ttLibC_DynamicBuffer_clear(buffer);
	} while(ttLibC_DynamicBuffer_refSize(data_buffer) < header->size); // if complete, do next.
	ttLibC_RtmpMessage *rtmp_message = NULL;

	// make message from data.
	switch(header->message_type) {
	case RtmpMessageType_setChunkSize:
		{
			uint32_t *buf = (uint32_t *)ttLibC_DynamicBuffer_refData(data_buffer);
			rtmp_message = (ttLibC_RtmpMessage *)ttLibC_SetChunkSize_make(be_uint32_t(*buf));
		}
		break;
//	case RtmpMessageType_abortMessage:
	case RtmpMessageType_acknowledgement:
		{
			uint32_t *buf = (uint32_t *)ttLibC_DynamicBuffer_refData(data_buffer);
			rtmp_message = (ttLibC_RtmpMessage *)ttLibC_Acknowledgement_make(be_uint32_t(*buf));
		}
		break;
	case RtmpMessageType_userControlMessage:
		{
			rtmp_message = (ttLibC_RtmpMessage *)ttLibC_UserControlMessage_readBinary(
					ttLibC_DynamicBuffer_refData(data_buffer),
					ttLibC_DynamicBuffer_refSize(data_buffer));
		}
		break;
	case RtmpMessageType_windowAcknowledgementSize:
		{
			uint32_t *buf = (uint32_t *)ttLibC_DynamicBuffer_refData(data_buffer);
			rtmp_message = (ttLibC_RtmpMessage *)ttLibC_WindowAcknowledgementSize_make(be_uint32_t(*buf));
		}
		break;
	case RtmpMessageType_setPeerBandwidth:
		{
			uint8_t *buf = ttLibC_DynamicBuffer_refData(data_buffer);
			uint32_t size = be_uint32_t(*((uint32_t *)buf));
			rtmp_message = (ttLibC_RtmpMessage *)ttLibC_SetPeerBandwidth_make(size, *(buf + 4));
		}
		break;
	case RtmpMessageType_audioMessage:
		{
			rtmp_message = (ttLibC_RtmpMessage *)ttLibC_AudioMessage_readBinary(
					ttLibC_DynamicBuffer_refData(data_buffer),
					ttLibC_DynamicBuffer_refSize(data_buffer));
		}
		break;
	case RtmpMessageType_videoMessage:
		{
			rtmp_message = (ttLibC_RtmpMessage *)ttLibC_VideoMessage_readBinary(
					ttLibC_DynamicBuffer_refData(data_buffer),
					ttLibC_DynamicBuffer_refSize(data_buffer));
		}
		break;
/*	case RtmpMessageType_amf3DataMessage:
	case RtmpMessageType_amf3SharedObjectMessage:
	case RtmpMessageType_amf3Command:*/
	case RtmpMessageType_amf0DataMessage:
		{
			rtmp_message = (ttLibC_RtmpMessage *)ttLibC_Amf0DataMessage_readBinary(
					ttLibC_DynamicBuffer_refData(data_buffer),
					ttLibC_DynamicBuffer_refSize(data_buffer));
		}
		break;
//	case RtmpMessageType_amf0SharedObjectMessage:
	case RtmpMessageType_amf0Command:
		{
			rtmp_message = (ttLibC_RtmpMessage *)ttLibC_Amf0Command_readBinary(
					ttLibC_DynamicBuffer_refData(data_buffer),
					ttLibC_DynamicBuffer_refSize(data_buffer));
		}
		break;
	case RtmpMessageType_aggregateMessage:
		{
			rtmp_message = (ttLibC_RtmpMessage *)ttLibC_AggregateMessage_readBinary(ttLibC_DynamicBuffer_refData(data_buffer));
		}
		break;
	default:
		ERR_PRINT("unknown/unimplemented message.:%d", header->message_type);
		// empty data_buffer, use next time.
		ttLibC_DynamicBuffer_empty(data_buffer);
		ttLibC_StlMap_put(client_object->recv_buffers, (void *)((long)header->cs_id), data_buffer);
		if(rtmp_message != NULL) {
			ttLibC_RtmpHeader_copy(rtmp_message->header, header);
		}
		return rtmp_message;
	}
	// empty data_buffer, use next time.
	ttLibC_DynamicBuffer_empty(data_buffer);
	ttLibC_StlMap_put(client_object->recv_buffers, (void *)((long)header->cs_id), data_buffer);
	if(rtmp_message == NULL) {
		return ttLibC_RtmpMessage_readBinary(buffer, client_object);
	}
	ttLibC_RtmpHeader_copy(rtmp_message->header, header);
	return rtmp_message;
}

void TT_VISIBILITY_HIDDEN ttLibC_RtmpMessage_close(ttLibC_RtmpMessage **message) {
	ttLibC_RtmpMessage *msg = *message;
	ttLibC_RtmpHeader *header = msg->header;
	switch(header->message_type) {
	case RtmpMessageType_setChunkSize:
		ttLibC_SetChunkSize_close((ttLibC_SetChunkSize **)message);
		break;
//	case RtmpMessageType_abortMessage:
	case RtmpMessageType_acknowledgement:
		ttLibC_Acknowledgement_close((ttLibC_Acknowledgement **)message);
		break;
	case RtmpMessageType_userControlMessage:
		ttLibC_UserControlMessage_close((ttLibC_UserControlMessage **)message);
		break;
	case RtmpMessageType_windowAcknowledgementSize:
		ttLibC_WindowAcknowledgementSize_close((ttLibC_WindowAcknowledgementSize **)message);
		break;
	case RtmpMessageType_setPeerBandwidth:
		ttLibC_SetPeerBandwidth_close((ttLibC_SetPeerBandwidth **)message);
		break;
	case RtmpMessageType_audioMessage:
		ttLibC_AudioMessage_close((ttLibC_AudioMessage **)message);
		break;
	case RtmpMessageType_videoMessage:
		ttLibC_VideoMessage_close((ttLibC_VideoMessage **)message);
		break;
/*	case RtmpMessageType_amf3DataMessage:
	case RtmpMessageType_amf3SharedObjectMessage:
	case RtmpMessageType_amf3Command:*/
	case RtmpMessageType_amf0DataMessage:
		ttLibC_Amf0DataMessage_close((ttLibC_Amf0DataMessage **)message);
		break;
//	case RtmpMessageType_amf0SharedObjectMessage:
	case RtmpMessageType_amf0Command:
		ttLibC_Amf0Command_close((ttLibC_Amf0Command **)message);
		break;
	case RtmpMessageType_aggregateMessage:
		ttLibC_AggregateMessage_close((ttLibC_AggregateMessage **)message);
		break;
	default:
		ERR_PRINT("unknown / unimplemented message.:%d", header->message_type);
		break;
	}
}

#endif
