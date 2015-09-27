/**
 * @file   rtmp.h
 * @brief  rtmp support.
 *
 * this code is under 3-Cause BSD license
 *
 * @author taktod
 * @date   2015/08/26
 */

#ifndef TTLIBC_NET_RTMP_H_
#define TTLIBC_NET_RTMP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../../util/amfUtil.h"
#include "../../frame/frame.h"

#include <stdint.h>

/**
 * rtmpMessageType
 */
typedef enum {
	RtmpMessageType_setChunkSize = 0x01,
	RtmpMessageType_abortMessage = 0x02,
	RtmpMessageType_acknowledgement = 0x03,
	RtmpMessageType_userControlMessage = 0x04,
	RtmpMessageType_windowAcknowledgementSize = 0x05,
	RtmpMessageType_setPeerBandwidth = 0x06,
	RtmpMessageType_audioMessage = 0x08,
	RtmpMessageType_videoMessage = 0x09,
	RtmpMessageType_amf3DataMessage = 0x0F,
	RtmpMessageType_amf3SharedObjectMessage = 0x10,
	RtmpMessageType_amf3Command = 0x11,
	RtmpMessageType_amf0DataMessage = 0x12,
	RtmpMessageType_amf0SharedObjectMessage = 0x13,
	RtmpMessageType_amf0Command = 0x14,
	RtmpMessageType_aggregateMessage = 0x16,
} ttLibC_RtmpMessage_Type;

/**
 * information for rtmpConnection.
 */
typedef struct {
	char *server;
	uint16_t port;
	char *app;
} ttLibC_Net_RtmpConnection;

typedef ttLibC_Net_RtmpConnection ttLibC_RtmpConnection;

/**
 * callback for status event.
 * @param ptr      user def data pointer.
 * @param amf0_obj status event object.
 * @return true: success false:error.
 */
typedef bool (* ttLibC_RtmpEventFunc)(void *ptr, ttLibC_Amf0Object *amf0_obj);

/**
 * create new rtmpConnection.
 * @return connection object.
 */
ttLibC_RtmpConnection *ttLibC_RtmpConnection_make();

/**
 * try connect.
 * @param conn     rtmpConnection object.
 * @param address  target address.
 * @param callback event callback function.
 * @param ptr      user def object pointer.
 * @return true: success false: error
 */
bool ttLibC_RtmpConnection_connect(
		ttLibC_RtmpConnection *conn,
		const char *address,
		ttLibC_RtmpEventFunc callback,
		void *ptr);

bool ttLibC_RtmpConnection_read(ttLibC_RtmpConnection *conn);

/**
 * close
 */
void ttLibC_RtmpConnection_close(ttLibC_RtmpConnection **conn);

// next to make netStream.
typedef struct {
} ttLibC_Net_RtmpStream;

typedef ttLibC_Net_RtmpStream ttLibC_RtmpStream;

typedef bool (* ttLibC_RtmpFrameFunc)(void *ptr, ttLibC_Frame *frame);

// makeすると、ttLibC_RtmpStreamのオブジェクトが帰ってくる。
ttLibC_RtmpStream *ttLibC_RtmpStream_make(ttLibC_RtmpConnection *conn);

uint32_t ttLibC_RtmpStream_play(
		ttLibC_RtmpStream *stream,
		const char *name,
		ttLibC_RtmpFrameFunc callback,
		void *ptr);

uint32_t ttLibC_RtmpStream_publish(
		ttLibC_RtmpStream *stream,
		const char *name);

bool ttLibC_RtmpStream_stop(
		ttLibC_RtmpStream *stream,
		uint32_t id);

void ttLibC_RtmpStream_feed(
		ttLibC_RtmpStream *stream,
		uint32_t id,
		ttLibC_Frame *frame);

void ttLibC_RtmpStream_close(ttLibC_RtmpStream **stream);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_RTMP_H_ */
