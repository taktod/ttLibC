/**
 * @file   rtmp.h
 * @brief  support for rtmp client.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/06
 */

#ifndef TTLIBC_NET_CLIENT_RTMP_H_
 #define TTLIBC_NET_CLIENT_RTMP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../tetty.h"
#include "../../util/amfUtil.h"
#include "../../frame/frame.h"

/**
 * definition of rtmpConnection.
 */
typedef struct ttLibC_Net_Client_Rtmp_RtmpConnection {

} ttLibC_Net_Client_Rtmp_RtmpConnection;

typedef ttLibC_Net_Client_Rtmp_RtmpConnection ttLibC_RtmpConnection;

/**
 * callback for status event.
 * @param ptr      user def data pointer.
 * @param amf0_obj status event object.
 * @return true: success false:error.
 */
typedef bool (* ttLibC_RtmpEventFunc)(void *ptr, ttLibC_Amf0Object *amf0_obj);

/**
 * make rtmpConnection object.
 */
ttLibC_RtmpConnection *ttLibC_RtmpConnection_make();

/**
 * add event listener for rtmpConnection.
 * @param conn
 * @param callback
 * @param ptr
 */
void ttLibC_RtmpConnection_addEventListener(
		ttLibC_RtmpConnection *conn,
		ttLibC_RtmpEventFunc callback,
		void *ptr);

/**
 * connect
 * @param conn    rtmpConnnection object.
 * @param address for example: rtmp://localhost:1935/live
 */
bool ttLibC_RtmpConnection_connect(
		ttLibC_RtmpConnection *conn,
		const char *address);

/**
 * update connection event.
 * @param conn          rtmpConnection object
 * @param wait_interval interval in micro sec.
 * @return true:success false:error
 */
bool ttLibC_RtmpConnection_update(
		ttLibC_RtmpConnection* conn,
		uint32_t wait_interval);

/**
 * close connection object.
 * @param conn
 */
void ttLibC_RtmpConnection_close(ttLibC_RtmpConnection **conn);

/**
 * definition of rtmpStream.
 */
typedef struct ttLibC_Net_Client_Rtmp_RtmpStream {

} ttLibC_Net_Client_Rtmp_RtmpStream;

typedef ttLibC_Net_Client_Rtmp_RtmpStream ttLibC_RtmpStream;

/**
 * callback function to get frame object.
 * @param ptr   user def pointer.
 * @param frame analyzed frame from container.
 * @return true: work properly, false: error occured.
 */
typedef bool (* ttLibC_RtmpStream_getFrameFunc)(void *ptr, ttLibC_Frame *frame);

/**
 * make rtmpStream
 * @param conn rtmpConnection object.
 * @return rtmpStream object.
 */
ttLibC_RtmpStream *ttLibC_RtmpStream_make(ttLibC_RtmpConnection *conn);

/**
 * add event listener
 * @param stream
 * @param callback
 * @param ptr
 */
void ttLibC_RtmpStream_addEventListener(
		ttLibC_RtmpStream *stream,
		ttLibC_RtmpEventFunc callback,
		void *ptr);

/**
 * add frame listener.
 * @param stream
 * @param callback
 * @@aram ptr
 */
void ttLibC_RtmpStream_addFrameListener(
		ttLibC_RtmpStream *stream,
		ttLibC_RtmpStream_getFrameFunc callback,
		void *ptr);

/**
 * publish stream.
 * @param stream target stream
 * @param name   target name
 */
void ttLibC_RtmpStream_publish(
		ttLibC_RtmpStream *stream,
		const char *name);

/**
 * send frame object.
 * @param stream
 * @param frame
 * @return true:success false:error
 */
bool ttLibC_RtmpStream_addFrame(
		ttLibC_RtmpStream *stream,
		ttLibC_Frame *frame);

/**
 * send buffer length for play.
 */
void ttLibC_RtmpStream_setBufferLength(
		ttLibC_RtmpStream *stream,
		uint32_t buffer_length);

/**
 * play stream.
 * @param stream       target stream
 * @param name         target name
 * @param accept_video flag for video recv.
 * @param accept_audio flag for audio recv.
 */
void ttLibC_RtmpStream_play(
		ttLibC_RtmpStream *stream,
		const char *name,
		bool accept_video,
		bool accept_audio);

/**
 * pause stream.
 */
void ttLibC_RtmpStream_pause(ttLibC_RtmpStream *stream);

/**
 * close rtmpStream
 * @param stream
 */
void ttLibC_RtmpStream_close(ttLibC_RtmpStream **stream);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_CLIENT_RTMP_H_ */
