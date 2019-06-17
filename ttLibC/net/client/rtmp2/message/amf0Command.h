/*
 * amf0Command.h
 *
 *  Created on: 2017/07/22
 *      Author: taktod
 */

#ifndef TTLIBC_NET_CLIENT_RTMP2_MESSAGE_AMF0COMMAND_H_
#define TTLIBC_NET_CLIENT_RTMP2_MESSAGE_AMF0COMMAND_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../../../../ttLibC_predef.h"
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

typedef struct ttLibC_Net_Client_Rtmp2_Message_Amf0Command {
	ttLibC_RtmpMessage inherit_super;
	char *command_name[256];
	int32_t command_id;
	ttLibC_Amf0Object *obj1;
	ttLibC_Amf0Object *obj2;
	ttLibC_Amf0Object *obj3;
	ttLibC_Tetty2Promise *promise;
} ttLibC_Net_Client_Rtmp2_Message_Amf0Command;

typedef ttLibC_Net_Client_Rtmp2_Message_Amf0Command ttLibC_Amf0Command;

ttLibC_Amf0Command TT_ATTRIBUTE_INNER *ttLibC_Amf0Command_make(const char *command_name);
ttLibC_Amf0Command TT_ATTRIBUTE_INNER *ttLibC_Amf0Command_connect(
		const char *tcUrl,
		const char *app);
ttLibC_Amf0Command TT_ATTRIBUTE_INNER *ttLibC_Amf0Command_createStream();
ttLibC_Amf0Command TT_ATTRIBUTE_INNER *ttLibC_Amf0Command_publish(
		uint32_t stream_id,
		const char *name);

ttLibC_Amf0Command TT_ATTRIBUTE_INNER *ttLibC_Amf0Command_receiveAudio(
		uint32_t stream_id,
		bool accept_audio);
ttLibC_Amf0Command TT_ATTRIBUTE_INNER *ttLibC_Amf0Command_receiveVideo(
		uint32_t stream_id,
		bool accept_video);
ttLibC_Amf0Command TT_ATTRIBUTE_INNER *ttLibC_Amf0Command_play(
		uint32_t stream_id,
		const char *name);
ttLibC_Amf0Command TT_ATTRIBUTE_INNER *ttLibC_Amf0Command_pause(uint32_t stream_id);

ttLibC_Amf0Command TT_ATTRIBUTE_INNER *ttLibC_Amf0Command_closeStream(uint32_t stream_id);

ttLibC_Amf0Command TT_ATTRIBUTE_INNER *ttLibC_Amf0Command_readBinary(
		uint8_t *data,
		size_t data_size);

bool TT_ATTRIBUTE_INNER ttLibC_Amf0Command_getData(
		ttLibC_Amf0Command *command,
		ttLibC_DynamicBuffer *buffer);

void TT_ATTRIBUTE_INNER ttLibC_Amf0Command_close(ttLibC_Amf0Command **command);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_CLIENT_RTMP2_MESSAGE_AMF0COMMAND_H_ */
