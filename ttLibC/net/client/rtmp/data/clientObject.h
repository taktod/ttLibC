/**
 * @file   clientObject.h
 * @brief  data def for client.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/07
 */

#ifndef TTLIBC_NET_CLIENT_RTMP_DATA_CLIENTOBJECT_H_
#define TTLIBC_NET_CLIENT_RTMP_DATA_CLIENTOBJECT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../../rtmp.h"
#include "../tetty/rtmpHandshake.h"
#include "../../../../util/dynamicBufferUtil.h"
#include "../../../../util/stlMapUtil.h"
#include "../../../../util/stlListUtil.h"

typedef enum ttLibC_RtmpHandshake_Phase {
	phase_s1,
	phase_s2,
	phase_s3
} ttLibC_RtmpHandshake_Phase;

/**
 * definition of client_object.
 */
typedef struct ttLibC_Net_Client_Rtmp_Data_ClientObject{
	uint32_t recv_chunk_size;
	uint32_t send_chunk_size;

	tetty_errornum error_number;

	// recv
	ttLibC_DynamicBuffer *recv_buffer;
	ttLibC_StlMap *recv_headers; // int -> header map
	ttLibC_StlMap *recv_buffers; // int -> dynamicBuffer map
	// send
	ttLibC_DynamicBuffer *send_buffer;
	ttLibC_StlMap *send_headers; // int -> header map

	// handshake
	ttLibC_RtmpHandshake_Phase phase;
	uint8_t *c2_value;
//	uint8_t *s2_value;
	bool is_handshake_done;
	ttLibC_TettyPromise *handshake_promise;

	// command
	uint32_t next_command_id;

	// for _result command
	ttLibC_StlMap *commandId_command_map; // command_id -> command map.(for CommandMessage.)
	// for stream event
	ttLibC_StlMap *streamId_rtmpStream_map; // stream_id -> rtmpStream map.(for stream event.)
	// do I need to separate send and recv map?
//	ttLibC_StlMap *streamId_rtmpStream_recvMap;
//	ttLibC_StlMap *streamId_rtmpStream_sendMap;
} ttLibC_Net_Client_Rtmp_Data_ClientObject;

typedef ttLibC_Net_Client_Rtmp_Data_ClientObject ttLibC_ClientObject;

typedef struct ttLibC_Net_Client_Rtmp_Data_ClientObject_PassingObject {
	uint32_t stream_id;
	ttLibC_ClientObject *client_object;
} ttLibC_Net_Client_Rtmp_Data_ClientObject_PassingObject;

typedef ttLibC_Net_Client_Rtmp_Data_ClientObject_PassingObject ttLibC_ClientObject_PassingObject;

/**
 * make client_object obj.
 */
ttLibC_ClientObject *ttLibC_ClientObject_make();

/**
 * close client_object obj.
 */
void ttLibC_ClientObject_close(ttLibC_ClientObject **client_object);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_CLIENT_RTMP_DATA_CLIENTOBJECT_H_ */
