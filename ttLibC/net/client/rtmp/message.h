/**
 * @file   message.h
 * @brief  rtmp message
 *
 * this code is under 3-Cause BSD license
 *
 * @author taktod
 * @date   2015/09/05
 */

#ifndef TTLIBC_NET_RTMP_MESSAGE_H_
#define TTLIBC_NET_RTMP_MESSAGE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../rtmp.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "header.h"

typedef struct ttLibC_RtmpMessage {
	ttLibC_RtmpHeader *header; // header情報
} ttLibC_RtmpMessage;

typedef struct ttLibC_Rtmp4ByteMessage {
	ttLibC_RtmpMessage inherit_super;
	uint32_t value;
} ttLibC_Rtmp4ByteMessage;

/**
 * messageクラスはいくつかに分けておきたい。
 * commandメッセージ amf0Command amf3Command
 * // sharedObjectMessage
 * // DataMessage
 * AudioMessage audio
 * VideoMessage video
 * AggregateMessage aggregate
 * SystemMessageこんなところか・・・その他いろいろ
 */

typedef bool (* ttLibC_RtmpMessageReadFunc)(void *ptr, ttLibC_RtmpMessage *message);


ttLibC_RtmpMessage *ttLibC_RtmpMessage_make(
		ttLibC_RtmpConnection *conn,
		size_t object_size,
		uint64_t pts,
		uint32_t size,
 		ttLibC_RtmpMessage_Type type,
		uint32_t stream_id);

bool ttLibC_RtmpMessage_write(
		ttLibC_RtmpConnection *conn,
		ttLibC_RtmpMessage *message,
		ttLibC_RtmpDataWriteFunc callback,
		void *ptr); // bufferに書き込むためのbyteデータをcallbackで呼び出す。

bool ttLibC_RtmpMessage_read(
		ttLibC_RtmpConnection *conn,
		uint8_t *data,
		size_t data_size,
		ttLibC_RtmpMessageReadFunc callback,
		void *ptr); // byteデータからrtmpMessageを読み込む

// とりあえずcloseがいるはず。
void ttLibC_RtmpMessage_close(ttLibC_RtmpMessage **message);

// ここにメッセージマクロをいれておきたい。

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_RTMP_MESSAGE_H_ */
