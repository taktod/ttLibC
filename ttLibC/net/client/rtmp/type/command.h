/**
 * @file   command.h
 * @brief  
 *
 * This code is under 3-Cause BSD license
 *
 * @author taktod
 * @date   2015/09/06
 */

#ifndef TTLIBC_NET_RTMP_MESSAGE_COMMAND_H_
#define TTLIBC_NET_RTMP_MESSAGE_COMMAND_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../netConnection.h"
#include "../message.h"

typedef struct {
	ttLibC_RtmpMessage inherit_super;
	ttLibC_Amf0Object *command_name;
	ttLibC_Amf0Object *command_id;
	ttLibC_Amf0Object *command_param1;
	ttLibC_Amf0Object *command_param2;
} ttLibC_RtmpAmf0Command;

// すでに送っているコネクト命令はこっちにもってきた方がよさそうですね。
// そうすれば、netConnectionにいれている命令集をはずすことができる。

// これでコマンドを作る形にしよう。
/*
// for future use, comment here.
ttLibC_RtmpMessage *ttLibC_RtmpCommandMessage_amf0Command(
		ttLibC_RtmpConnection *conn,
		uint64_t pts,
		const char *command_name,
		uint32_t command_id,
		ttLibC_Amf0Object *object1,
		ttLibC_Amf0Object *object2);
*/

bool ttLibC_RtmpCommandMessage_sendCreateStream(
		ttLibC_RtmpConnection *conn,
		ttLibC_RtmpDataWriteFunc callback,
		void *ptr);

bool ttLibC_RtmpCommandMessage_sendConnect(
		ttLibC_RtmpConnection *conn,
		ttLibC_Amf0Object *override_connect_params,
		ttLibC_RtmpDataWriteFunc callback,
		void *ptr);

void ttLibC_RtmpCommandMessage_close(ttLibC_RtmpMessage **message);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_RTMP_MESSAGE_COMMAND_H_ */
