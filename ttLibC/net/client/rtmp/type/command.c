/*
 * @file   command.c
 * @brief  
 *
 * This code is under 3-Cause BSD license
 *
 * @author taktod
 * @date   2015/09/06
 */

#include "command.h"
#include <stdio.h>
#include <stdbool.h>
#include "../../../../log.h"
#include "../../../../allocator.h"

ttLibC_RtmpMessage *ttLibC_RtmpCommandMessage_amf0Command(
		ttLibC_RtmpConnection *conn,
		uint64_t pts,
		const char *command_name,
		uint32_t command_id,
		ttLibC_Amf0Object *object1,
		ttLibC_Amf0Object *object2) {
	// rtmpMessageMakeの一種。
	// あとは必要な命令をつくっておかなければならない。
	ttLibC_RtmpConnection_ *conn_ = (ttLibC_RtmpConnection_ *)conn;
	conn_->cs_id = 3; // csidは強制で3にする。
	ttLibC_RtmpHeader *header = NULL;
	uint32_t size = 0;
	ttLibC_Amf0Object *amf0_command_name = ttLibC_Amf0_string(command_name);
	ttLibC_Amf0Object *amf0_command_id = ttLibC_Amf0_number(command_id);
	if(amf0_command_name == NULL || amf0_command_id == NULL) {
		// どっちかがNULLになったら必要なものが作れていない。
		ttLibC_Amf0_close(&amf0_command_name);
		ttLibC_Amf0_close(&amf0_command_id);
		return NULL;
	}
	// 必要なものがそろったので、headerとかつくっていく。
	size = amf0_command_name->data_size + amf0_command_id->data_size;
	if(object1 != NULL) {
		size += object1->data_size;
	}
	if(object2 != NULL) {
		size += object2->data_size;
	}
	// 必要なものが揃ったので、オブジェクトをつくる。
	ttLibC_RtmpAmf0Command *amf0_command = (ttLibC_RtmpAmf0Command *)ttLibC_RtmpMessage_make(
			(ttLibC_RtmpConnection *)conn,
			sizeof(ttLibC_RtmpAmf0Command),
			0,
			size,
			RtmpMessageType_amf0Command,
			0);
	// あとは追加のデータをくっつければよし。
	amf0_command->command_name = amf0_command_name;
	amf0_command->command_id = amf0_command_id;
	amf0_command->command_param1 = object1;
	amf0_command->command_param2 = object2;
	return (ttLibC_RtmpMessage *)amf0_command;
}

static ttLibC_RtmpMessage *RtmpCommandMessage_createStream(
		ttLibC_RtmpConnection *conn) {
	ttLibC_RtmpConnection_ *conn_ = (ttLibC_RtmpConnection_ *)conn;
	if(conn_ == NULL) {
		ERR_PRINT("conn is null.");
		return NULL;
	}
	return NULL;
}

static ttLibC_RtmpMessage *RtmpCommandMessage_connect(
		ttLibC_RtmpConnection *conn,
		ttLibC_Amf0Object *override_connect_params) {
	// コネクト命令発行。
	ttLibC_RtmpConnection_ *conn_ = (ttLibC_RtmpConnection_ *)conn;
	if(conn_ == NULL) {
		// コネクションがないので、処理できない。
		ERR_PRINT("conn is null.");
		return NULL;
	}
	char tcUrl[256];
	sprintf(tcUrl, "rtmp://%s:%d/%s",
			conn_->inherit_super.server,
			conn_->inherit_super.port,
			conn_->inherit_super.app);
	// リークしないようにした。
	ttLibC_Amf0MapObject map_objects[] = {
			{"app",            ttLibC_Amf0_string(conn_->inherit_super.app)},
			{"flashVer",       ttLibC_Amf0_string("MAC 18,0,0,232")},
			{"tcUrl",          ttLibC_Amf0_string(tcUrl)},
			{"fpad",           ttLibC_Amf0_boolean(false)},
			{"audioCodecs",    ttLibC_Amf0_number(3575)},
			{"videoCodecs",    ttLibC_Amf0_number(252)},
			{"objectEncoding", ttLibC_Amf0_number(0)},
			{"capabilities",   ttLibC_Amf0_number(239)},
			{"videoFunction",  ttLibC_Amf0_number(1)},
			{NULL,             NULL},
	};
	ttLibC_Amf0Object *command_param = ttLibC_Amf0_object(map_objects);
	if(command_param == NULL) {
		int i = 0;
		while(map_objects[i].key != NULL && map_objects[i].amf0_obj != NULL) {
			if(map_objects[i].key != NULL) {
				ttLibC_free(map_objects[i].key);
			}
			ttLibC_Amf0_close((ttLibC_Amf0Object **)&map_objects[i].amf0_obj);
			++ i;
		}
		return NULL;
	}
	// あとはamf0Commandの仕事
	ttLibC_RtmpMessage *message = ttLibC_RtmpCommandMessage_amf0Command(
			(ttLibC_RtmpConnection *)conn_,
			0,
			"connect",
			conn_->command_id,
			command_param,
			NULL);
	if(message == NULL) {
		// 解放しなければならない。
		ttLibC_Amf0_close(&command_param);
	}
	return message;
}

bool ttLibC_RtmpCommandMessage_sendCreateStream(
		ttLibC_RtmpConnection *conn,
		ttLibC_RtmpDataWriteFunc callback,
		void *ptr) {
	ttLibC_RtmpMessage *createStream = RtmpCommandMessage_createStream(
			conn);
	bool result = ttLibC_RtmpMessage_write(
			conn,
			createStream,
			callback,
			ptr);
	ttLibC_RtmpMessage_close(&createStream);
	return result;
}

bool ttLibC_RtmpCommandMessage_sendConnect(
		ttLibC_RtmpConnection *conn,
		ttLibC_Amf0Object *override_connect_params,
		ttLibC_RtmpDataWriteFunc callback,
		void *ptr) {
	ttLibC_RtmpMessage *connect = RtmpCommandMessage_connect(
			conn,
			override_connect_params);
	bool result = ttLibC_RtmpMessage_write(
			conn,
			connect,
			callback,
			ptr);
	ttLibC_RtmpMessage_close(&connect);
	return result;
}

void ttLibC_RtmpCommandMessage_close(ttLibC_RtmpMessage **message) {
	ttLibC_RtmpMessage *target = *message;
	if(target == NULL) {
		return;
	}
	if(target->header != NULL) {
		switch(target->header->message_type) {
		case RtmpMessageType_amf0Command:
			{
				ttLibC_RtmpAmf0Command *amf0_command = (ttLibC_RtmpAmf0Command *)target;
				ttLibC_Amf0_close(&amf0_command->command_name);
				ttLibC_Amf0_close(&amf0_command->command_id);
				ttLibC_Amf0_close(&amf0_command->command_param1);
				ttLibC_Amf0_close(&amf0_command->command_param2);
			}
			break;
		default:
			break;
		}
	}
	ttLibC_free(target);
	*message = NULL;
}



