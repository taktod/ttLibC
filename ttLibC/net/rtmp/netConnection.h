/**
 * @file   netConnection.h
 * @brief  netConnection functions.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/09/05
 */

#ifndef TTLIBC_NET_RTMP_NETCONNECTION_H_
#define TTLIBC_NET_RTMP_NETCONNECTION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../rtmp.h"
#include "header.h"
//#include "message.h"

typedef enum {
	readType_header,   // header取得動作中
	readType_body,     // body取得動作中
	readType_innerHeader, // chunkの途切れ目の中途headerを確認
} ttLibC_RtmpRead_Type;

typedef struct {
	ttLibC_RtmpConnection inherit_super;
	// for send
	ttLibC_RtmpHeader *headers[64]; // 64 headers for often use.
	ttLibC_RtmpHeader **ex_headers; // left 65536 headers, use with alloc

	// for recv
	ttLibC_RtmpHeader *r_headers[64]; // 64 headers for often use.
	ttLibC_RtmpHeader **r_ex_headers; // left 65536 headers, use with alloc
	uint32_t cs_id;
	uint32_t command_id; // command id will be incremented.
	int32_t sock;
	uint32_t chunk_size;
	size_t send_size; // これは、chunk_sizeのコントロールをするのに必要となる、すでに送信したデータみたいです。

	// windowの読み込みサイズ ack_sizeを超えたらwindow_acknowledgementを送信する必要あり。
	size_t read_size; // ackを応答するかどうかを判定するようの読み込んだデータ量保持
	// ackしたら、%で割り、あまりをだすことにする。
	// この他にacknowledgeを送るために、今までに送った、受け取ったデータ量を保持する定数が必要です。
	size_t byte_read_window;
	size_t byte_write_window;

	ttLibC_RtmpEventFunc event_callback;
	void *event_ptr;

	// tmp buffer for server response.
	ttLibC_RtmpRead_Type read_type;
	uint8_t *tmp_buffer;
	size_t tmp_buffer_size;
	size_t tmp_buffer_next_pos;
	size_t tmp_buffer_target_size;

	uint8_t tmp_header_buffer[18];
	size_t tmp_header_buffer_size; // 18 fixed
	size_t tmp_header_buffer_next_pos;
	size_t tmp_header_buffer_target_size;
} ttLibC_Net_RtmpConnection_;

typedef ttLibC_Net_RtmpConnection_ ttLibC_RtmpConnection_;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_RTMP_NETCONNECTION_H_ */
