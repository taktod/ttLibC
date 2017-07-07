/**
 * @file   amf0Command.c
 * @brief  rtmp message amf0Command.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/08
 */
#ifdef __ENABLE_SOCKET__

#include "amf0Command.h"
#include "../../../../ttLibC_predef.h"
#include "../../../../_log.h"
#include "../../../../allocator.h"
#include <string.h>
#include "../../../../util/hexUtil.h"

ttLibC_Amf0Command *ttLibC_Amf0Command_make(const char *command_name) {
	ttLibC_Amf0Command *command = ttLibC_malloc(sizeof(ttLibC_Amf0Command));
	if(command == NULL) {
		return NULL;
	}
	// make header.
	ttLibC_RtmpHeader *header = ttLibC_RtmpHeader_make(3, 0, RtmpMessageType_amf0Command, 0);
	if(header == NULL) {
		ttLibC_free(command);
		return NULL;
	}
	command->inherit_super.header = header;
	strcpy((char *)command->command_name, command_name);
	command->command_id = -1;
	command->obj1 = NULL;
	command->obj2 = NULL;
	command->obj3 = NULL;
	command->promise = NULL;
	return command;
}

ttLibC_Amf0Command *ttLibC_Amf0Command_connect(
		const char *tcUrl,
		const char *app) {
	ttLibC_Amf0Command *connect = ttLibC_Amf0Command_make("connect");
	if(connect == NULL) {
		return NULL;
	}
	ttLibC_Amf0MapObject map_objects[] = {
			{"app",            ttLibC_Amf0_string(app)},
			{"flashVer",       ttLibC_Amf0_string("MAC 18,0,0,232")},
			{"tcUrl",          ttLibC_Amf0_string(tcUrl)},
			{"fpad",           ttLibC_Amf0_boolean(false)},
			{"audioCodecs",    ttLibC_Amf0_number(3575)},
			{"videoCodecs",    ttLibC_Amf0_number(252)},
			{"objectEncoding", ttLibC_Amf0_number(0)},
			{"capabilities",   ttLibC_Amf0_number(239)},
			{"videoFunction",  ttLibC_Amf0_number(1)},
			{NULL,             NULL}
	};
	ttLibC_Amf0Object *command_param = ttLibC_Amf0_object(map_objects);
	if(command_param == NULL) {
 		int i = 0;
		while(map_objects[i].key != NULL || map_objects[i].amf0_obj != NULL) {
			ttLibC_Amf0_close((ttLibC_Amf0Object **)&map_objects[i].amf0_obj);
			++ i;
		}
		return NULL;
	}
	connect->obj1 = command_param;
	return connect;
}

ttLibC_Amf0Command *ttLibC_Amf0Command_createStream() {
	ttLibC_Amf0Command *createStream = ttLibC_Amf0Command_make("createStream");
	if(createStream == NULL) {
		return NULL;
	}
	createStream->obj1 = ttLibC_Amf0_null();
	return createStream;
}

ttLibC_Amf0Command *ttLibC_Amf0Command_publish(
		uint32_t stream_id,
		const char *name) {
	ttLibC_Amf0Command *publish = ttLibC_Amf0Command_make("publish");
	if(publish == NULL) {
		return NULL;
	}
	publish->command_id = 0;
	publish->inherit_super.header->stream_id = stream_id;
	publish->obj1 = ttLibC_Amf0_null();
	publish->obj2 = ttLibC_Amf0_string(name);
	publish->obj3 = ttLibC_Amf0_string("live");
	return publish;
}

ttLibC_Amf0Command *ttLibC_Amf0Command_receiveAudio(
		uint32_t stream_id,
		bool accept_audio) {
	ttLibC_Amf0Command *receiveAudio = ttLibC_Amf0Command_make("receiveAudio");
	if(receiveAudio == NULL) {
		return NULL;
	}
	receiveAudio->command_id = 0;
	receiveAudio->inherit_super.header->stream_id = stream_id;
	receiveAudio->obj1 = ttLibC_Amf0_null();
	receiveAudio->obj2 = ttLibC_Amf0_boolean(accept_audio);
	return receiveAudio;
}

ttLibC_Amf0Command *ttLibC_Amf0Command_receiveVideo(
		uint32_t stream_id,
		bool accept_video) {
	ttLibC_Amf0Command *receiveVideo = ttLibC_Amf0Command_make("receiveVideo");
	if(receiveVideo == NULL) {
		return NULL;
	}
	receiveVideo->command_id = 0;
	receiveVideo->inherit_super.header->stream_id = stream_id;
	receiveVideo->obj1 = ttLibC_Amf0_null();
	receiveVideo->obj2 = ttLibC_Amf0_boolean(accept_video);
	return receiveVideo;
}

ttLibC_Amf0Command *ttLibC_Amf0Command_play(
		uint32_t stream_id,
		const char *name) {
	ttLibC_Amf0Command *play = ttLibC_Amf0Command_make("play");
	if(play == NULL) {
		return NULL;
	}
	play->command_id = 0;
	play->inherit_super.header->stream_id = stream_id;
	play->obj1 = ttLibC_Amf0_null();
	play->obj2 = ttLibC_Amf0_string(name);
	return play;
}

ttLibC_Amf0Command *ttLibC_Amf0Command_pause(uint32_t stream_id) {
	ttLibC_Amf0Command *pause = ttLibC_Amf0Command_make("pause");
	if(pause == NULL) {
		return NULL;
	}
	pause->command_id = 0;
	pause->inherit_super.header->stream_id = stream_id;
	pause->obj1 = ttLibC_Amf0_null();
	pause->obj2 = ttLibC_Amf0_boolean(true);
	pause->obj3 = ttLibC_Amf0_number(0);
	return pause;
}

ttLibC_Amf0Command *ttLibC_Amf0Command_closeStream(uint32_t stream_id) {
	ttLibC_Amf0Command *closeStream = ttLibC_Amf0Command_make("closeStream");
	if(closeStream == NULL) {
		return NULL;
	}
	closeStream->command_id = 0;
	closeStream->inherit_super.header->stream_id = stream_id;
	closeStream->obj1 = ttLibC_Amf0_null();
	return closeStream;
}


static bool Amf0Command_readBinaryObjectCallback(void *ptr, ttLibC_Amf0Object *amf0_obj) {
	ttLibC_Amf0Command *command = (ttLibC_Amf0Command *)ptr;
	if(strlen((const char *)command->command_name) == 0) {
		if(amf0_obj->type == amf0Type_String) {
			strcpy((char *)command->command_name, (const char *)amf0_obj->object);
		}
		else {
			return false;
		}
	}
	else if(command->command_id == -1) { // could be 0, therefore set -1 for unsettled.
		if(amf0_obj->type == amf0Type_Number) {
			command->command_id = (uint32_t)(*((double *)(amf0_obj->object)));
		}
		else {
			return false;
		}
	}
	else if(command->obj1 == NULL) {
		command->obj1 = ttLibC_Amf0_clone(amf0_obj);
	}
	else if(command->obj2 == NULL) {
		command->obj2 = ttLibC_Amf0_clone(amf0_obj);
	}
	else if(command->obj3 == NULL) {
		command->obj3 = ttLibC_Amf0_clone(amf0_obj);
	}
	else {
		return false;
	}
	return true;
}

ttLibC_Amf0Command *ttLibC_Amf0Command_readBinary(
		uint8_t *data,
		size_t data_size) {
	ttLibC_Amf0Command *command = ttLibC_Amf0Command_make("");
	if(command == NULL) {
		return NULL;
	}
	// call callback until read all binary.(we will get some number of amf0 object.)
	bool result = ttLibC_Amf0_read(data, data_size, Amf0Command_readBinaryObjectCallback, command);
	if(!result) {
		ttLibC_Amf0Command_close(&command);
		return NULL;
	}
	return command;
}

static bool Amf0Command_writeCallback(void *ptr, void *data, size_t data_size) {
	ttLibC_DynamicBuffer *buffer = (ttLibC_DynamicBuffer *)ptr;
	ttLibC_DynamicBuffer_append(buffer, data, data_size);
	return true;
}

bool ttLibC_Amf0Command_getData(
		ttLibC_Amf0Command *command,
		ttLibC_DynamicBuffer *buffer) {
	// amf0Command -> binary for send message.
	ttLibC_Amf0Object *command_name = ttLibC_Amf0_string((const char *)command->command_name);
	ttLibC_Amf0Object *command_id = ttLibC_Amf0_number(command->command_id);
	ttLibC_Amf0_write(command_name, Amf0Command_writeCallback, buffer);
	ttLibC_Amf0_write(command_id, Amf0Command_writeCallback, buffer);
	if(command->obj1 != NULL) {
		ttLibC_Amf0_write(command->obj1, Amf0Command_writeCallback, buffer);
	}
	if(command->obj2 != NULL) {
		ttLibC_Amf0_write(command->obj2, Amf0Command_writeCallback, buffer);
	}
	if(command->obj3 != NULL) {
		ttLibC_Amf0_write(command->obj3, Amf0Command_writeCallback, buffer);
	}
	ttLibC_Amf0_close(&command_name);
	ttLibC_Amf0_close(&command_id);
	return true;
}

void ttLibC_Amf0Command_close(ttLibC_Amf0Command **command) {
	ttLibC_Amf0Command *target = (ttLibC_Amf0Command *)*command;
	if(target == NULL) {
		return;
	}
	ttLibC_RtmpHeader_close(&target->inherit_super.header);
	ttLibC_Amf0_close(&target->obj1);
	ttLibC_Amf0_close(&target->obj2);
	ttLibC_Amf0_close(&target->obj3);
	ttLibC_free(target);
	*command = NULL;
}

#endif
