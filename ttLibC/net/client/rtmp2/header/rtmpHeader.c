/*
 * rtmpHeader.c
 *
 *  Created on: 2017/07/22
 *      Author: taktod
 */

#include "rtmpHeader.h"
#include "../../../../_log.h"
#include "../../../../allocator.h"
#include "../../../../util/byteUtil.h"
#include "../../../../util/ioUtil.h"
#include "../../../../util/hexUtil.h"
#include <string.h>

ttLibC_RtmpHeader TT_ATTRIBUTE_INNER *ttLibC_RtmpHeader_make(
		uint32_t cs_id,
		uint64_t timestamp,
		ttLibC_RtmpMessage_Type message_type,
		uint32_t stream_id) {
	ttLibC_RtmpHeader *header = ttLibC_malloc(sizeof(ttLibC_RtmpHeader));
	if(header == NULL) {
		return NULL;
	}
	header->type = Type0;
	header->cs_id = cs_id;
	header->delta_time = 0;
	header->timestamp = timestamp;
	header->message_type = message_type;
	header->stream_id = stream_id;
	header->size = 0;
	return header;
}

ttLibC_RtmpHeader TT_ATTRIBUTE_INNER *ttLibC_RtmpHeader_copy(
		ttLibC_RtmpHeader *target,
		ttLibC_RtmpHeader *source) {
	if(source == NULL) {
		ERR_PRINT("source is NULL, nothing to do.");
		return target;
	}
	if(target == NULL) {
		target = ttLibC_RtmpHeader_make(
				source->cs_id,
				source->timestamp,
				source->message_type,
				source->stream_id);
	}
	memcpy(target, source, sizeof(ttLibC_RtmpHeader));
	return target;
}

size_t TT_ATTRIBUTE_INNER ttLibC_RtmpHeader_getData(
		ttLibC_RtmpHeader *header,
		void *data,
		size_t data_size) {
	// header object -> binary data.
	ttLibC_ByteConnector *connector = ttLibC_ByteConnector_make(data, data_size, ByteUtilType_default);
	ttLibC_ByteConnector_bit(connector, header->type, 2);
	if(header->cs_id < 64) {
		ttLibC_ByteConnector_bit(connector, header->cs_id, 6);
	}
	else if(header->cs_id < 320) {
		ttLibC_ByteConnector_bit(connector, 0, 6);
		ttLibC_ByteConnector_bit(connector, header->cs_id - 64, 8);
	}
	else {
		ttLibC_ByteConnector_bit(connector, 1, 6);
		ttLibC_ByteConnector_bit(connector, header->cs_id - 320, 16);
	}
	switch(header->type) {
	default:
	case Type0:
		{
			if(header->timestamp > 0xFFFFFFL) {
				ttLibC_ByteConnector_bit(connector, 0xFFFFFFL, 24);
			}
			else {
				ttLibC_ByteConnector_bit(connector, header->timestamp, 24);
			}
			ttLibC_ByteConnector_bit(connector, header->size, 24);
			ttLibC_ByteConnector_bit(connector, header->message_type, 8);
			ttLibC_ByteConnector_bit(connector, be_uint32_t(header->stream_id), 32);
			if(header->timestamp > 0xFFFFFFL) {
				ttLibC_ByteConnector_bit(connector, header->timestamp, 32);
			}
		}
		break;
	case Type1:
		{
			if(header->delta_time > 0xFFFFFFL) {
				ttLibC_ByteConnector_bit(connector, 0xFFFFFFL, 24);
			}
			else {
				ttLibC_ByteConnector_bit(connector, header->delta_time, 24);
			}
			ttLibC_ByteConnector_bit(connector, header->size, 24);
			ttLibC_ByteConnector_bit(connector, header->message_type, 8);
			if(header->delta_time > 0xFFFFFFL) {
				ttLibC_ByteConnector_bit(connector, header->delta_time, 32);
			}
		}
		break;
	case Type2:
		{
			if(header->delta_time > 0xFFFFFFL) {
				ttLibC_ByteConnector_bit(connector, 0xFFFFFFL, 24);
			}
			else {
				ttLibC_ByteConnector_bit(connector, header->delta_time, 24);
			}
			if(header->delta_time > 0xFFFFFFL) {
				ttLibC_ByteConnector_bit(connector, header->delta_time, 32);
			}
		}
		break;
	case Type3:
		{
			;
		}
		break;
	}
	size_t res_size = connector->write_size;
	ttLibC_ByteConnector_close(&connector);
	return res_size;
}

ttLibC_RtmpHeader TT_ATTRIBUTE_INNER *ttLibC_RtmpHeader_readBinary(
		ttLibC_DynamicBuffer *buffer,
		ttLibC_ClientObject *client_object) {
	// binary -> header object
	uint8_t *data = ttLibC_DynamicBuffer_refData(buffer);
	size_t data_size = ttLibC_DynamicBuffer_refSize(buffer);
	if(data_size == 0) {
		return NULL;
	}
	ttLibC_ByteReader *reader = ttLibC_ByteReader_make(data, data_size, ByteUtilType_default);
	ttLibC_RtmpHeader_Type type = ttLibC_ByteReader_bit(reader, 2);
	uint32_t cs_id = ttLibC_ByteReader_bit(reader, 6);
	if(cs_id == 0) {
		cs_id = ttLibC_ByteReader_bit(reader, 8);
		if(data_size < 2) {
			ttLibC_ByteReader_close(&reader);
			return NULL;
		}
	}
	else if(cs_id == 1) {
		cs_id = ttLibC_ByteReader_bit(reader, 16);
		if(data_size < 3) {
			ttLibC_ByteReader_close(&reader);
			return NULL;
		}
	}
	ttLibC_RtmpHeader *prev_header = ttLibC_StlMap_get(client_object->recv_headers, (void *)((long)cs_id));
	ttLibC_DynamicBuffer *target_buffer = ttLibC_StlMap_get(client_object->recv_buffers, (void *)((long)cs_id));
	size_t target_buffer_size = 0;
	if(target_buffer != NULL) {
		target_buffer_size = ttLibC_DynamicBuffer_refSize(target_buffer);
	}
	uint64_t timestamp = 0;
	int32_t  delta_time = 0;
	uint32_t size = 0;
	ttLibC_RtmpMessage_Type message_type = 0;
	uint32_t stream_id = 0;
	if(prev_header != NULL) {
		timestamp    = prev_header->timestamp;
		size         = prev_header->size;
		message_type = prev_header->message_type;
		stream_id    = prev_header->stream_id;
	}
	switch(type) {
	default:
	case Type0:
		{
			if(data_size < reader->read_size + 12) {
				ttLibC_ByteReader_close(&reader);
				return NULL;
			}
			timestamp = ttLibC_ByteReader_bit(reader, 24);
			size = ttLibC_ByteReader_bit(reader, 24);
			message_type = ttLibC_ByteReader_bit(reader, 8);
			uint32_t stid = ttLibC_ByteReader_bit(reader, 32);
			stream_id = be_uint32_t(stid);
			if(timestamp == 0xFFFFFFL) {
				if(data_size < reader->read_size + 16) {
					ttLibC_ByteReader_close(&reader);
					return NULL;
				}
				timestamp = ttLibC_ByteReader_bit(reader, 32);
			}
		}
		break;
	case Type1:
		{
			if(prev_header == NULL) {
				ERR_PRINT("prev header is missing. type1 require prev header.");
				ttLibC_ByteReader_close(&reader);
				return NULL;
			}
			if(data_size < reader->read_size + 8) {
				ttLibC_ByteReader_close(&reader);
				return NULL;
			}
			delta_time = ttLibC_ByteReader_bit(reader, 24);
			size = ttLibC_ByteReader_bit(reader, 24);
			message_type = ttLibC_ByteReader_bit(reader, 8);
			stream_id = prev_header->stream_id;
			if(delta_time == 0xFFFFFFL) {
				if(data_size < reader->read_size + 12) {
					ttLibC_ByteReader_close(&reader);
					return NULL;
				}
				delta_time = ttLibC_ByteReader_bit(reader, 32);
			}
		}
		break;
	case Type2:
		{
			if(prev_header == NULL) {
				ERR_PRINT("prev header is missing. type2 require prev header.");
				ttLibC_ByteReader_close(&reader);
				return NULL;
			}
			if(data_size < reader->read_size + 3) {
				ttLibC_ByteReader_close(&reader);
				return NULL;
			}
			delta_time = ttLibC_ByteReader_bit(reader, 24);
			size = prev_header->size;
			message_type = prev_header->message_type;
			stream_id = prev_header->stream_id;
			if(delta_time == 0xFFFFFFL) {
				if(data_size < reader->read_size + 7) {
					ttLibC_ByteReader_close(&reader);
					return NULL;
				}
				delta_time = ttLibC_ByteReader_bit(reader, 32);
			}
		}
		break;
	case Type3:
		{
			if(prev_header == NULL) {
				ERR_PRINT("prev header is missing. type3 require prev header.");
				ttLibC_ByteReader_close(&reader);
				return NULL;
			}
			delta_time = prev_header->delta_time;
			size = prev_header->size;
			message_type = prev_header->message_type;
			stream_id = prev_header->stream_id;
//			if(delta_time == 0xFFFFFFL) {
//				delta_time = ttLibC_ByteReader_bit(reader, 32);
//			}
		}
		break;
	}
	// target_buffer_size is 0 -> this header is first header for message unit.
	if(type != Type0 && target_buffer_size == 0) {
		timestamp = delta_time + prev_header->timestamp;
	}
	bool success = reader->error_number == 0;
	uint32_t header_size = reader->read_size;
	ttLibC_ByteReader_close(&reader);
	if(!success) {
		ERR_PRINT("error happen during byte reading.");
		return NULL;
	}
	// check data buffer, if we have enough size, do next. otherwise return NULL to stop process.
	uint32_t chunk_size = (size - target_buffer_size) > client_object->recv_chunk_size ? client_object->recv_chunk_size : (size - target_buffer_size);
	if(data_size - header_size < chunk_size) {
		return NULL;
	}
	// have enough size, make header and reply it.
	ttLibC_RtmpHeader *header = prev_header;
	if(header == NULL) {
		header = ttLibC_RtmpHeader_make(cs_id, timestamp, message_type, stream_id);
	}
	header->cs_id = cs_id;
	header->delta_time = delta_time;
	header->message_type = message_type;
	header->size = size;
	header->stream_id = stream_id;
	header->timestamp = timestamp;
	header->type = Type0; // force to set type0.
	ttLibC_StlMap_put(client_object->recv_headers, (void *)((long)cs_id), header);
	// update reading pointer for dynamic buffer.
	ttLibC_DynamicBuffer_markAsRead(buffer, header_size);
	return header;
}

void TT_ATTRIBUTE_INNER ttLibC_RtmpHeader_close(ttLibC_RtmpHeader **header) {
	ttLibC_RtmpHeader *target = (ttLibC_RtmpHeader *)*header;
	if(target == NULL) {
		return;
	}
	ttLibC_free(target);
	*header = NULL;
}

