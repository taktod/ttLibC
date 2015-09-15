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
mesageTypeも前回と同じ
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

typedef bool (* ttLibC_RtmpHeaderWriteFunc)(void *ptr, void *data, size_t data_size);

typedef enum {
	RtmpHeaderType_0 = 0x00,
	RtmpHeaderType_1 = 0x01,
	RtmpHeaderType_2 = 0x02,
	RtmpHeaderType_3 = 0x03,
} ttLibC_RtmpHeader_Type;

typedef struct {
//	uint8_t type; // typeはheaderをbinary化するときに決定されるものとする。
	uint32_t cs_id;
	uint32_t timestamp;
	uint32_t size;
	ttLibC_RtmpMessage_Type message_type;
	uint32_t stream_id;

	uint32_t read_size; // 読み込み時に利用したデータサイズ
} ttLibC_Net_RtmpHeader;

typedef ttLibC_Net_RtmpHeader ttLibC_RtmpHeader;

ttLibC_RtmpHeader *ttLibC_RtmpHeader_make(
		ttLibC_RtmpConnection *conn,
		uint64_t pts,
		uint32_t size,
		ttLibC_RtmpMessage_Type type,
		uint32_t stream_id);

/**
 * 現在処理中のrtmpHeaderを取得する。
 * netConnectionがcs_idを保持しているため動作可能。
 * あとマルチスレッドで動作してない・・・というのもある。
 */
ttLibC_RtmpHeader *ttLibC_RtmpHeader_getCurrentHeader(ttLibC_RtmpConnection *conn);

bool ttLibC_RtmpHeader_write(
		ttLibC_RtmpHeader_Type type,
		ttLibC_RtmpHeader *header,
		ttLibC_RtmpHeaderWriteFunc callback,
		void *ptr);

// rtmpHeaderをつくって応答する。
// この動作はboolにして、応答をcallback化した方がいいか？
// そもそもrtmpMessageのreadに吸収されそうだけど。
ttLibC_RtmpHeader *ttLibC_RtmpHeader_read(
		ttLibC_RtmpConnection *conn,
		void *data,
		size_t data_size);

void ttLibC_RtmpHeader_close(ttLibC_RtmpHeader **header);

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
