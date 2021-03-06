/**
 * @file   amf0Command.h
 * @brief  rtmp message amf0Command.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/07
 */

#ifndef TTLIBC_NET_CLIENT_RTMP_MESSAGE_AMF0COMMAND_H_
#define TTLIBC_NET_CLIENT_RTMP_MESSAGE_AMF0COMMAND_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "../../../../util/amfUtil.h"
#include "rtmpMessage.h"

/*
 * header
 * Amf0String commandName
 * Amf0Number id
 * Amf0Object
 * Amf0Object
 * Amf0Object (only for publish?)
 */

typedef struct ttLibC_Net_Client_Rtmp_Message_Amf0Command {
	ttLibC_RtmpMessage inherit_super;
	char *command_name[256];
	int32_t command_id;
	ttLibC_Amf0Object *obj1;
	ttLibC_Amf0Object *obj2;
	ttLibC_Amf0Object *obj3;
	ttLibC_TettyPromise *promise;
} ttLibC_Net_Client_Rtmp_Message_Amf0Command;

typedef ttLibC_Net_Client_Rtmp_Message_Amf0Command ttLibC_Amf0Command;

ttLibC_Amf0Command *ttLibC_Amf0Command_make(const char *command_name);
ttLibC_Amf0Command *ttLibC_Amf0Command_connect(
		const char *tcUrl,
		const char *app);
ttLibC_Amf0Command *ttLibC_Amf0Command_createStream();
ttLibC_Amf0Command *ttLibC_Amf0Command_publish(
		uint32_t stream_id,
		const char *name);

ttLibC_Amf0Command *ttLibC_Amf0Command_receiveAudio(
		uint32_t stream_id,
		bool accept_audio);
ttLibC_Amf0Command *ttLibC_Amf0Command_receiveVideo(
		uint32_t stream_id,
		bool accept_video);
ttLibC_Amf0Command *ttLibC_Amf0Command_play(
		uint32_t stream_id,
		const char *name);
ttLibC_Amf0Command *ttLibC_Amf0Command_pause(uint32_t stream_id);

ttLibC_Amf0Command *ttLibC_Amf0Command_closeStream(uint32_t stream_id);

ttLibC_Amf0Command *ttLibC_Amf0Command_readBinary(
		uint8_t *data,
		size_t data_size);

bool ttLibC_Amf0Command_getData(
		ttLibC_Amf0Command *command,
		ttLibC_DynamicBuffer *buffer);

void ttLibC_Amf0Command_close(ttLibC_Amf0Command **command);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_CLIENT_RTMP_MESSAGE_AMF0COMMAND_H_ */
