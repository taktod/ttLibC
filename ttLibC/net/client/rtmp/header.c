/*
 * @file   header.c
 * @brief  
 *
 * This code is under 3-Cause BSD license
 *
 * @author taktod
 * @date   2015/09/16
 */

#include "header.h"
#include <stdio.h>
#include "netConnection.h"
#include "../../../allocator.h"
#include "../../../log.h"
#include "../../../util/ioUtil.h"

ttLibC_RtmpHeader *ttLibC_RtmpHeader_make(
		ttLibC_RtmpConnection *conn,
		uint64_t pts,
		uint32_t size,
		ttLibC_RtmpMessage_Type message_type,
		uint32_t stream_id,
		bool is_read) {
	ttLibC_RtmpConnection_ *conn_ = (ttLibC_RtmpConnection_ *)conn;
	ttLibC_RtmpHeader *header = NULL;
	if(conn_->cs_id < 64) {
		if(is_read) {
			header = conn_->r_headers[conn_->cs_id];
		}
		else {
			header = conn_->headers[conn_->cs_id];
		}
	}
	else {
		// TODO need to make for extra headers.
		ERR_PRINT("huge data size cs_id is not supported now.");
		return NULL;
	}
	bool has_prev_header = (header != NULL);
	if(header == NULL) {
		header = (ttLibC_RtmpHeader *)ttLibC_malloc(sizeof(ttLibC_RtmpHeader));
	}
	if(header == NULL) {
		ERR_PRINT("failed to allocate memory for rtmpHeader.");
		return NULL;
	}
	header->cs_id = conn_->cs_id;
	// この位置で前のheader情報と追加したいデータがどの程度にているか判断し、typeに登録しておく。
	// size timestamp stream_idが一致する場合
	if(has_prev_header && header->stream_id == stream_id) {
		if(header->size == size && header->message_type == message_type) {
			if(header->timestamp == pts) {
				header->type = RtmpHeaderType_3;
			}
			else {
				header->type = RtmpHeaderType_2;
			}
		}
		else {
			header->type = RtmpHeaderType_1;
		}
	}
	else {
		header->type = RtmpHeaderType_0;
	}
	header->message_type = message_type;
	header->size = size;
	header->timestamp = (uint32_t)pts;
	header->stream_id = stream_id;
	header->read_size = 0;
	// register header on netConnection
	if(conn_->cs_id < 64) {
		if(is_read) {
			conn_->r_headers[conn_->cs_id] = header;
		}
		else {
			conn_->headers[conn_->cs_id] = header;
		}
	}
	else {
		// TODO need to make for extra headers.
	}
	return header;
}

ttLibC_RtmpHeader *ttLibC_RtmpHeader_getCurrentHeader(
		ttLibC_RtmpConnection *conn,
		bool is_read) {
	ttLibC_RtmpConnection_ *conn_ = (ttLibC_RtmpConnection_ *)conn;
	if(conn_ == NULL) {
		return NULL;
	}
	if(conn_->cs_id < 64) {
		if(is_read) {
			return conn_->r_headers[conn_->cs_id];
		}
		else {
			return conn_->headers[conn_->cs_id];
		}
	}
	else {
		ERR_PRINT("huge data size_ cs_id is not ready.");
		return NULL;
	}
}

bool ttLibC_RtmpHeader_write(
		ttLibC_RtmpHeader_Type type,
		ttLibC_RtmpHeader *header,
		ttLibC_RtmpDataWriteFunc callback,
		void *ptr) {
	// csid:max 3byte
	// timestamp size messageType streamId = 11byte
	// timestamp_ext:4byte
	// 3 + 11 + 4:max 18 byte
	if(type == RtmpHeaderType_default) {
		// reuse動作は、前回の送信headerに対して実行可能みたいです。
		// 受け取ったデータをベースに、reuseを実行するとサーバー側からエラーがかえってくる模様。
		// うーん。
		type = header->type;
	}
	uint8_t data[18];
	uint8_t *buf = data;
	size_t data_size = 0;
	if(header->cs_id < 64) {
		buf[0] = (uint8_t)header->cs_id | (type << 6);
		++ buf;
		++ data_size;
	}
	else if(header->cs_id < 321) {
		buf[0] = type << 6;
		buf[1] = header->cs_id - 64;
		buf += 2;
		data_size += 2;
	}
	else {
		buf[0] = (type << 6) | 1;
		buf[1] = ((header->cs_id - 64) >> 8) & 0xFF;
		buf[2] = (header->cs_id - 64) & 0xFF;
		buf += 3;
		data_size += 3;
	}
	switch(type) {
	case RtmpHeaderType_0:
		// timestamp
		data_size += 11;
		if(header->timestamp >= 0xFFFFFF) {
			buf[0]  = 0xFF;
			buf[1]  = 0xFF;
			buf[2]  = 0xFF;
			buf[11] = (header->timestamp >> 24) & 0xFF;
			buf[12] = (header->timestamp >> 16) & 0xFF;
			buf[13] = (header->timestamp >> 8)  & 0xFF;
			buf[14] = (header->timestamp)       & 0xFF;
			data_size += 4;
		}
		else {
			buf[0] = (header->timestamp >> 16) & 0xFF;
			buf[1] = (header->timestamp >> 8)  & 0xFF;
			buf[2] = (header->timestamp)       & 0xFF;
		}
		// size
		buf[3] = (header->size >> 16) & 0xFF;
		buf[4] = (header->size >> 8)  & 0xFF;
		buf[5] = (header->size)       & 0xFF;
		// messageType
		buf[6] = (uint8_t)header->message_type;
		// stream_id
		*((uint32_t *)(buf + 7)) = le_uint32_t(header->stream_id);
		break;
	case RtmpHeaderType_1:
		// timestamp(delta time)
		data_size += 7;
		if(header->timestamp >= 0xFFFFFF) {
			buf[0]  = 0xFF;
			buf[1]  = 0xFF;
			buf[2]  = 0xFF;
			buf[7]  = (header->timestamp >> 24) & 0xFF;
			buf[8]  = (header->timestamp >> 16) & 0xFF;
			buf[9]  = (header->timestamp >> 8)  & 0xFF;
			buf[10] = (header->timestamp)       & 0xFF;
			data_size += 4;
		}
		else {
			buf[0] = (header->timestamp >> 16) & 0xFF;
			buf[1] = (header->timestamp >> 8)  & 0xFF;
			buf[2] = (header->timestamp)       & 0xFF;
		}
		// size
		buf[3] = (header->size >> 16) & 0xFF;
		buf[4] = (header->size >> 8)  & 0xFF;
		buf[5] = (header->size)       & 0xFF;
		// messageType
		buf[6] = (uint8_t)header->message_type;
		break;
	case RtmpHeaderType_2:
		// timestamp(delta time)
		data_size += 3;
		if(header->timestamp >= 0xFFFFFF) {
			buf[0] = 0xFF;
			buf[1] = 0xFF;
			buf[2] = 0xFF;
			buf[3] = (header->timestamp >> 24) & 0xFF;
			buf[4] = (header->timestamp >> 16) & 0xFF;
			buf[5] = (header->timestamp >> 8)  & 0xFF;
			buf[6] = (header->timestamp)       & 0xFF;
			data_size += 4;
		}
		else {
			buf[0] = (header->timestamp >> 16) & 0xFF;
			buf[1] = (header->timestamp >> 8)  & 0xFF;
			buf[2] = (header->timestamp)       & 0xFF;
		}
		break;
	case RtmpHeaderType_3: // nothing to do.
	default:
		break;
	}
	callback(ptr, data, data_size);
	return true;
}

ttLibC_RtmpHeader *ttLibC_RtmpHeader_read(
		ttLibC_RtmpConnection *conn,
		void *data,
		size_t data_size) {
	if(data == NULL) {
		return NULL;
	}
	if(data_size <= 0) {
		return NULL;
	}
	ttLibC_RtmpConnection_ *conn_ = (ttLibC_RtmpConnection_ *)conn;
	uint8_t *dat = (uint8_t *)data;
	size_t read_size = 0;
	ttLibC_RtmpHeader_Type type = ((*dat) >> 6) & 0x03;
	uint32_t cs_id = (*dat) & 0x3F;
	++ dat;
	-- data_size;
	if(cs_id == 0) {
		if(data_size < 2) {
			conn_->tmp_header_buffer_target_size = 2;
			return NULL;
		}
		cs_id = (*dat) + 64;
		++ dat;
		-- data_size;
	}
	else if(cs_id == 1) {
		if(data_size < 3) {
			conn_->tmp_header_buffer_target_size = 3;
			return NULL;
		}
		cs_id = ((((*dat) & 0xFF) << 8) | ((*(dat + 1)) & 0xFF)) + 64;
		dat += 2;
		data_size -= 2;
	}
	// ref_prev data;
	// innerHeaderの場合は、ここでcs_idをつけるのではなく、出来上がったデータとcs_idが一致しているか確認しないといけない。
	// が、面倒なので、とりあえずほっとく。
	conn_->cs_id = cs_id;
	ttLibC_RtmpHeader *prev_header = ttLibC_RtmpHeader_getCurrentHeader(conn, true);
	if(prev_header == NULL && type != RtmpHeaderType_0) {
		ERR_PRINT("acquire non type0 without and prev_header. something wrong.");
		return NULL;
	}
	switch(type) {
	case RtmpHeaderType_0:
		{
			// at least 11 byte needed.
			if(data_size < 11) {
				conn_->tmp_header_buffer_target_size = 11 + (dat - (uint8_t *)data);
				return NULL;
			}
			uint32_t timestamp = (dat[0] << 16) | (dat[1] << 8) | dat[2];
			uint32_t size = (dat[3] << 16) | (dat[4] << 8) | dat[5];
			ttLibC_RtmpMessage_Type message_type = dat[6];
			uint32_t stream_id = dat[7] | (dat[8] << 8) | (dat[9] << 16) | (dat[10] << 24);
			dat += 11;
			data_size -= 11;
			if(timestamp == 0xFFFFFF) {
				// extratimestamp
				if(data_size < 4) {
					conn_->tmp_header_buffer_target_size = 4 + (dat - (uint8_t *)data);
					return NULL;
				}
				timestamp = (dat[0] << 24) | (dat[1] << 16) | (dat[2] << 8) | dat[3];
				dat += 4;
				data_size -= 4;
			}
			ttLibC_RtmpHeader *header = ttLibC_RtmpHeader_make(conn, timestamp, size, message_type, stream_id, true);
			if(header == NULL) {
				return NULL;
			}
			header->read_size = dat - (uint8_t *)data;
			return header;
		}
		break;
	case RtmpHeaderType_1:
		{
			// at least 7 byte needed.
			if(data_size < 7) {
				conn_->tmp_header_buffer_target_size = 7 + (dat - (uint8_t *)data);
				return NULL;
			}
			uint32_t timestamp = (dat[0] << 16) | (dat[1] << 8) | dat[2];
			uint32_t size = (dat[3] << 16) | (dat[4] << 8) | dat[5];
			ttLibC_RtmpMessage_Type message_type = dat[6];
			uint32_t stream_id = prev_header->stream_id;
			dat += 7;
			data_size -= 7;
			if(timestamp == 0xFFFFFF) {
				// extratimestamp
				if(data_size < 4) {
					conn_->tmp_header_buffer_target_size = 4 + (dat - (uint8_t *)data);
					return NULL;
				}
				timestamp = (dat[0] << 24) | (dat[1] << 16) | (dat[2] << 8) | dat[3];
				dat += 4;
				data_size -= 4;
			}
			ttLibC_RtmpHeader *header = ttLibC_RtmpHeader_make(conn, timestamp, size, message_type, stream_id, true);
			if(header == NULL) {
				return NULL;
			}
			header->read_size = dat - (uint8_t *)data;
			return header;
		}
		break;
	case RtmpHeaderType_2:
		{
			// at least 3 byte needed.
			if(data_size < 3) {
				conn_->tmp_header_buffer_target_size = 3 + (dat - (uint8_t *)data);
				return NULL;
			}
			uint32_t timestamp = (dat[0] << 16) | (dat[1] << 8) | dat[2];
			uint32_t size = prev_header->size;
			ttLibC_RtmpMessage_Type message_type = prev_header->message_type;
			uint32_t stream_id = prev_header->stream_id;
			dat += 3;
			data_size -= 3;
			if(timestamp == 0xFFFFFF) {
				// extratimestamp
				if(data_size < 4) {
					conn_->tmp_header_buffer_target_size = 4 + (dat - (uint8_t *)data);
					return NULL;
				}
				timestamp = (dat[0] << 24) | (dat[1] << 16) | (dat[2] << 8) | dat[3];
				dat += 4;
				data_size -= 4;
			}
			ttLibC_RtmpHeader *header = ttLibC_RtmpHeader_make(conn, timestamp, size, message_type, stream_id, true);
			if(header == NULL) {
				return NULL;
			}
			header->read_size = dat - (uint8_t *)data;
			return header;
		}
		break;
	case RtmpHeaderType_3:
		{
			prev_header->read_size = dat - (uint8_t *)data;
			return prev_header; // use prev_header it self.
		}
		break;
	default:
		break;
	}
	return NULL;
}

void ttLibC_RtmpHeader_close(ttLibC_RtmpHeader **header) {
	ttLibC_RtmpHeader *target = *header;
	if(target == NULL) {
		return;
	}
	ttLibC_free(target);
	*header = NULL;
}

