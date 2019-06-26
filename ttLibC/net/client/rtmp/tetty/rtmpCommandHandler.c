/**
 * @file   rtmpCommandHandler.c
 * @brief  tetty handler for amf0/amf3command.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/07
 */
#ifdef __ENABLE_SOCKET__

#include "rtmpCommandHandler.h"
#include "../../../../_log.h"
#include "../../../../allocator.h"
#include "../message/rtmpMessage.h"
#include "../message/amf0Command.h"
#include "../message/amf3Command.h"
#include "../data/clientObject.h"
#include "../../../../util/hexUtil.h"
#include "../rtmpStream.h"
#include <string.h>

static tetty_errornum RtmpCommandHandler_channelRead(
		ttLibC_TettyContext *ctx,
		void *data,
		size_t data_size) {
	ttLibC_RtmpMessage *rtmp_message = (ttLibC_RtmpMessage *)data;
	ttLibC_ClientObject *client_object = (ttLibC_ClientObject *)ctx->socket_info->ptr;
	switch(rtmp_message->header->message_type) {
	case RtmpMessageType_amf0Command:
		{
			ttLibC_Amf0Command *command = (ttLibC_Amf0Command *)rtmp_message;
			if(strcmp((const char *)command->command_name, "_result") == 0) {
				// find corresponds command message.
				ttLibC_Amf0Command *send_command = (ttLibC_Amf0Command *)ttLibC_StlMap_get(client_object->commandId_command_map, (void *)((long)command->command_id));
				if(send_command->promise != NULL) {
					if(strcmp((const char *)send_command->command_name, "createStream") == 0) {
						if(command->obj2 != NULL && command->obj2->type == amf0Type_Number) {
							ttLibC_ClientObject_PassingObject passData;
							passData.client_object = client_object;
							passData.stream_id = (uint32_t)(*((double *)(command->obj2->object)));
							ttLibC_TettyPromise_setSuccess(send_command->promise, &passData);
						}
					}
					else {
						ttLibC_TettyPromise_setSuccess(send_command->promise, command);
					}
				}
			}
			else if(strcmp((const char *)command->command_name, "onStatus") == 0){
				// check stream_id
				// TODO for unpublish or stop, should I remove target streamId from map?
				ttLibC_RtmpStream_ *rtmpStream = (ttLibC_RtmpStream_ *)ttLibC_StlMap_get(client_object->streamId_rtmpStream_map, (void *)((long)rtmp_message->header->stream_id));
				if(rtmpStream != NULL && rtmpStream->promise != NULL) {
					ttLibC_TettyPromise_setSuccess(rtmpStream->promise, command);
				}
			}
			else {
				return ttLibC_TettyContext_super_channelRead(ctx, data, data_size);
			}
		}
		break;
	case RtmpMessageType_userControlMessage:
		// TODO support userControlMessage? (NetStream.Buffer.Empty, NetStream.Buffer.Full and so on...)
		return ttLibC_TettyContext_super_channelRead(ctx, data, data_size);
	case RtmpMessageType_amf3Command:
		LOG_PRINT("");
		break;
	default:
		return ttLibC_TettyContext_super_channelRead(ctx, data, data_size);
	}
	return 0;
}

static tetty_errornum RtmpCommandHandler_write(
		ttLibC_TettyContext *ctx,
		void *data,
		size_t data_size) {
	ttLibC_RtmpMessage *message = (ttLibC_RtmpMessage *)data;
	switch(message->header->message_type) {
	case RtmpMessageType_amf0Command:
		{
			ttLibC_ClientObject *client_object = (ttLibC_ClientObject *)ctx->socket_info->ptr;
			ttLibC_Amf0Command *amf0_command = (ttLibC_Amf0Command *)message;
			if(amf0_command->command_id == -1) {
				// if command_id is -1(init value), apply id and wait for _result.
				amf0_command->command_id = client_object->next_command_id;
				++ client_object->next_command_id; // increment for next command.
				// clone command for result check.
				ttLibC_Amf0Command *cloned_command = ttLibC_Amf0Command_make((const char *)amf0_command->command_name);
				cloned_command->command_id = amf0_command->command_id;
				if(amf0_command->obj1 != NULL) {
					cloned_command->obj1 = ttLibC_Amf0_clone(amf0_command->obj1);
				}
				if(amf0_command->obj2 != NULL) {
					cloned_command->obj2 = ttLibC_Amf0_clone(amf0_command->obj2);
				}
				cloned_command->promise = amf0_command->promise;
				ttLibC_StlMap_put(client_object->commandId_command_map, (void *)((long)amf0_command->command_id), cloned_command);
			}
			return ttLibC_TettyContext_super_write(ctx, data, data_size);
		}
		break;
	case RtmpMessageType_amf3Command:
		break;
	default:
		return ttLibC_TettyContext_super_write(ctx, data, data_size);
	}
	return 0;
}

ttLibC_RtmpCommandHandler TT_ATTRIBUTE_INNER *ttLibC_RtmpCommandHandler_make() {
	ttLibC_RtmpCommandHandler *handler = ttLibC_malloc(sizeof(ttLibC_RtmpCommandHandler));
	if(handler == NULL) {
		return NULL;
	}
	memset(handler, 0, sizeof(ttLibC_RtmpCommandHandler));
	handler->channel_handler.channelRead = RtmpCommandHandler_channelRead;
	handler->channel_handler.write = RtmpCommandHandler_write;
	return handler;
}

void TT_ATTRIBUTE_INNER ttLibC_RtmpCommandHandler_close(ttLibC_RtmpCommandHandler **handler) {
	ttLibC_RtmpCommandHandler *target = (ttLibC_RtmpCommandHandler *)*handler;
	if(target == NULL) {
		return;
	}
	ttLibC_free(target);
	*handler = NULL;
}

#endif
