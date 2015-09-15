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

#include "../util/amfUtil.h"
#include <stdint.h>

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

/**
 * close
 */
void ttLibC_RtmpConnection_close(ttLibC_RtmpConnection **conn);

// next to make netStream.
typedef struct {

} ttLibC_Net_RtmpStream;

typedef ttLibC_Net_RtmpStream ttLibC_RtmpStream;

void ttLibC_RtmpStream_make();
void ttLibC_RtmpStream_play();
void ttLibC_RtmpStream_publish();

void ttLibC_Rtmp_NetStream_feed(); // feed frame data for publish.


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_RTMP_H_ */
