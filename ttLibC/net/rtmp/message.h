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

/*
 * rtmpのheader情報をコントロールするクラス
 * とします。
 * type0〜type3まであって、後者になるほど情報量が小さくなります。
 * 前のデータを参照する必要があるので、rtmpHeaderをどこかにcacheする必要があるのですが
 * headerのリストを出すと65600個も必要になるみたいです。
 * これはちょっといただけない・・・
 * どうするかな・・・
type0の場合
3byte timestamp
3byte size
1byte messageType
4byte streamId(これだけlittle endian,)
timestsampが0xFFFFFFになっている場合は
  このあとに4byteでtimestampをいれる。

type1の場合
csIdまでは同じ
3byte deltatime
3byte size
1byte messageType
streamIdはなし(csidが一致する命令が前回の動作なので、それを参照する。)
deltaTimeが0xFFFFFFの場合は・・・
   4byte timestamp

type2の場合
3byte deltatime
sizeは前回と同じ
messageTypeも前回と同じ
streamIdも前回の使い回し
deltaTimeが0xFFFFFFの場合は
  4byte timestamp

type3の場合
deltaTime使い回し
size使い回し
messageType使い回し
streamId使い回し
deltaTimeの件もなし
 *
 */

#include "header.h"

typedef struct {
	ttLibC_RtmpHeader *header; // header情報
} ttLibC_RtmpMessage;

typedef struct {
	ttLibC_RtmpMessage inherit_super;
	ttLibC_Amf0Object *command_name;
	ttLibC_Amf0Object *command_id;
	ttLibC_Amf0Object *command_param1;
	ttLibC_Amf0Object *command_param2;
} ttLibC_RtmpAmf0Command;

typedef struct {
	ttLibC_RtmpMessage inherit_super;
	uint32_t value;
} ttLibC_Rtmp4ByteMessage;

typedef ttLibC_Rtmp4ByteMessage ttLibC_RtmpWindowAcknowledgementSize;
typedef ttLibC_Rtmp4ByteMessage ttLibC_RtmpSetChunkSize;

typedef struct {
	ttLibC_RtmpMessage inherit_super;
	uint32_t window_acknowledge_size;
	uint8_t type;
} ttLibC_RtmpSetPeerBandwidth;

typedef enum {
	RtmpEventType_StreamBegin = 0,
	RtmpEventType_StreamEof = 1,
	RtmpEventType_StreamDry = 2,
	RtmpEventType_ClientBufferLength = 3,
	RtmpEventType_RecordedStreamBegin = 4,
	RtmpEventType_Unknown5 = 5,
	RtmpEventType_Ping = 6,
	RtmpEventType_Pong = 7,
	RtmpEventType_Unknown8 = 8,
	RtmpEventType_PingSwfVerification = 26,
	RtmpEventType_PongSwfVerification = 27,
	RtmpEventType_BufferEmpty = 31,
	RtmpEventType_BufferFull = 32,
} ttLibC_RtmpUserControlMessage_EventType;

typedef struct {
	ttLibC_RtmpMessage inheerit_super;
	ttLibC_RtmpUserControlMessage_EventType event_type;
	uint32_t value;
	uint32_t buffer_length;
} ttLibC_RtmpUserControlMessage;

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

// ping用のpongを応答するのにつかう予定。
ttLibC_RtmpMessage *ttLibC_RtmpMessage_userControlMessage(
		ttLibC_RtmpConnection *conn,
		ttLibC_RtmpUserControlMessage_EventType event_type);

// これでコマンドを作る形にしよう。
ttLibC_RtmpMessage *ttLibC_RtmpMessage_amf0Command(
		ttLibC_RtmpConnection *conn,
		uint64_t pts,
		const char *command_name,
		uint32_t command_id,
		ttLibC_Amf0Object *object1,
		ttLibC_Amf0Object *object2);

ttLibC_RtmpMessage *ttLibC_RtmpMessage_connect(
		ttLibC_RtmpConnection *conn,
		ttLibC_Amf0Object *override_connect_params);

bool ttLibC_RtmpMessage_write(
		ttLibC_RtmpConnection *conn,
		ttLibC_RtmpMessage *message,
		ttLibC_RtmpHeaderWriteFunc callback,
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
