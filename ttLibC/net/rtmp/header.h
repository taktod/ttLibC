/**
 * @file   header.h
 * @brief  
 *
 * This code is under 3-Cause BSD license
 *
 * @author taktod
 * @date   2015/09/16
 */

#ifndef TTLIBC_NET_RTMP_TYPE_HEADER_H_
#define TTLIBC_NET_RTMP_TYPE_HEADER_H_

#include "../rtmp.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef bool (* ttLibC_RtmpHeaderWriteFunc)(void *ptr, void *data, size_t data_size);

typedef enum {
	RtmpHeaderType_0 = 0x00,
	RtmpHeaderType_1 = 0x01,
	RtmpHeaderType_2 = 0x02,
	RtmpHeaderType_3 = 0x03,
	RtmpHeaderType_default = 0xFF,
} ttLibC_RtmpHeader_Type;

typedef struct {
	ttLibC_RtmpHeader_Type type; // typeはreadもしくはmakeするときに前のデータとの差分で決定するものとする。
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
		uint32_t stream_id,
		bool is_read);

/**
 * 現在処理中のrtmpHeaderを取得する。
 * netConnectionがcs_idを保持しているため動作可能。
 * あとマルチスレッドで動作してない・・・というのもある。
 */
ttLibC_RtmpHeader *ttLibC_RtmpHeader_getCurrentHeader(
		ttLibC_RtmpConnection *conn,
		bool is_read);

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

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_RTMP_TYPE_HEADER_H_ */
