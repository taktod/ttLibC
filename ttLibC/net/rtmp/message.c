/*
 * @file   message.c
 * @brief  rtmp message
 *
 * this code is under 3-Cause BSD license
 *
 * @author taktod
 * @date   2015/09/05
 */

#include "message.h"
#include "netConnection.h"
#include <stdlib.h>
#include <string.h>
#include "../../log.h"
#include "../../allocator.h"
#include "../../util/hexUtil.h"
#include "../../util/ioUtil.h"

ttLibC_RtmpMessage *ttLibC_RtmpMessage_userControlMessage(
		ttLibC_RtmpConnection *conn,
		ttLibC_RtmpUserControlMessage_EventType event_type) {
	uint32_t size = 0;
	switch(event_type) {
	case RtmpEventType_StreamBegin:
	case RtmpEventType_StreamEof:
	case RtmpEventType_StreamDry:
		size = 6;
		break;
	case RtmpEventType_ClientBufferLength:
		size = 10;
		break;
	case RtmpEventType_RecordedStreamBegin:
		size = 6;
		break;
	case RtmpEventType_Unknown5:
		ERR_PRINT("unknown 5");
		size = 0;
		break;
	case RtmpEventType_Ping:
	case RtmpEventType_Pong:
		size = 6;
		break;
	case RtmpEventType_Unknown8:
		ERR_PRINT("unknown 8");
		size = 0;
		break;
	case RtmpEventType_PingSwfVerification:
	case RtmpEventType_PongSwfVerification:
		ERR_PRINT("swf verification ping/pong. I need to check.");
		size = 0;
		break;
	case RtmpEventType_BufferEmpty:
	case RtmpEventType_BufferFull:
		size = 6;
		break;
	}
	ttLibC_RtmpUserControlMessage *user_control_message = (ttLibC_RtmpUserControlMessage *)ttLibC_RtmpMessage_make(
			conn,
			sizeof(ttLibC_RtmpUserControlMessage),
			0,
			size,
			RtmpMessageType_userControlMessage,
			0);
	user_control_message->event_type = event_type;
	return (ttLibC_RtmpMessage *)user_control_message;
}

ttLibC_RtmpMessage *ttLibC_RtmpMessage_amf0Command(
		ttLibC_RtmpConnection *conn,
		uint64_t pts,
		const char *command_name,
		uint32_t command_id,
		ttLibC_Amf0Object *object1,
		ttLibC_Amf0Object *object2) {
	// rtmpMessageMakeの一種。
	// あとは必要な命令をつくっておかなければならない。
	ttLibC_RtmpConnection_ *conn_ = (ttLibC_RtmpConnection_ *)conn;
	conn_->cs_id = 3; // csidは強制で3にする。
	ttLibC_RtmpHeader *header = NULL;
	uint32_t size = 0;
	ttLibC_Amf0Object *amf0_command_name = ttLibC_Amf0_string(command_name);
	ttLibC_Amf0Object *amf0_command_id = ttLibC_Amf0_number(command_id);
	if(amf0_command_name == NULL || amf0_command_id == NULL) {
		// どっちかがNULLになったら必要なものが作れていない。
		ttLibC_Amf0_close(&amf0_command_name);
		ttLibC_Amf0_close(&amf0_command_id);
		return NULL;
	}
	// 必要なものがそろったので、headerとかつくっていく。
	size = amf0_command_name->data_size + amf0_command_id->data_size;
	if(object1 != NULL) {
		size += object1->data_size;
	}
	if(object2 != NULL) {
		size += object2->data_size;
	}
	// 必要なものが揃ったので、オブジェクトをつくる。
	ttLibC_RtmpAmf0Command *amf0_command = (ttLibC_RtmpAmf0Command *)ttLibC_RtmpMessage_make(
			(ttLibC_RtmpConnection *)conn,
			sizeof(ttLibC_RtmpAmf0Command),
			0,
			size,
			RtmpMessageType_amf0Command,
			0);
	// あとは追加のデータをくっつければよし。
	amf0_command->command_name = amf0_command_name;
	amf0_command->command_id = amf0_command_id;
	amf0_command->command_param1 = object1;
	amf0_command->command_param2 = object2;
	return (ttLibC_RtmpMessage *)amf0_command;
}

ttLibC_RtmpMessage *ttLibC_RtmpMessage_connect(
		ttLibC_RtmpConnection *conn,
		ttLibC_Amf0Object *override_connect_params) {
	// コネクト命令発行。
	ttLibC_RtmpConnection_ *conn_ = (ttLibC_RtmpConnection_ *)conn;
	if(conn_ == NULL) {
		// コネクションがないので、処理できない。
		ERR_PRINT("conn is null.");
		return NULL;
	}
	char tcUrl[256];
	sprintf(tcUrl, "rtmp://%s:%d/%s",
			conn_->inherit_super.server,
			conn_->inherit_super.port,
			conn_->inherit_super.app);
	// リークしないようにした。
	ttLibC_Amf0MapObject map_objects[] = {
			{"app",            ttLibC_Amf0_string(conn_->inherit_super.app)},
			{"flashVer",       ttLibC_Amf0_string("MAC 18,0,0,232")},
			{"tcUrl",          ttLibC_Amf0_string(tcUrl)},
			{"fpad",           ttLibC_Amf0_boolean(false)},
			{"audioCodecs",    ttLibC_Amf0_number(3575)},
			{"videoCodecs",    ttLibC_Amf0_number(252)},
			{"objectEncoding", ttLibC_Amf0_number(0)},
			{"capabilities",   ttLibC_Amf0_number(239)},
			{"videoFunction",  ttLibC_Amf0_number(1)},
			{NULL,             NULL},
	};
	ttLibC_Amf0Object *command_param = ttLibC_Amf0_object(map_objects);
	if(command_param == NULL) {
		int i = 0;
		while(map_objects[i].key != NULL && map_objects[i].amf0_obj != NULL) {
			if(map_objects[i].key != NULL) {
				ttLibC_free(map_objects[i].key);
			}
			ttLibC_Amf0_close((ttLibC_Amf0Object **)&map_objects[i].amf0_obj);
			++ i;
		}
		return NULL;
	}
	// あとはamf0Commandの仕事
	ttLibC_RtmpMessage *message = ttLibC_RtmpMessage_amf0Command(
			(ttLibC_RtmpConnection *)conn_,
			0,
			"connect",
			conn_->command_id,
			command_param,
			NULL);
	if(message == NULL) {
		// 解放しなければならない。
		ttLibC_Amf0_close(&command_param);
	}
	return message;
}

static ttLibC_RtmpMessage *RtmpMessage_read_make(
		ttLibC_RtmpHeader *header,
		size_t object_size) {
	ttLibC_RtmpMessage *message = ttLibC_malloc(object_size);
	if(message == NULL) {
		ERR_PRINT("failed to allocate message object.");
		return NULL;
	}
	message->header = header;
	return message;
}

ttLibC_RtmpMessage *ttLibC_RtmpMessage_make(
		ttLibC_RtmpConnection *conn,
		size_t object_size,
		uint64_t pts,
		uint32_t size,
 		ttLibC_RtmpMessage_Type type,
		uint32_t stream_id) {
	ttLibC_RtmpMessage *message = ttLibC_malloc(object_size);
	if(message == NULL) {
		ERR_PRINT("failed to allocate message object.");
		return NULL;
	}
	// rtmpHeaderだけつくっておく必要がある。
	ttLibC_RtmpHeader *header = ttLibC_RtmpHeader_make(conn, pts, size, type, stream_id);
	if(header == NULL) {
		ttLibC_free(message);
		return NULL;
	}
	message->header = header;
	return message;
}

bool ttLibC_RtmpMessage_write(
		ttLibC_RtmpConnection *conn,
		ttLibC_RtmpMessage *message,
		ttLibC_RtmpHeaderWriteFunc callback,
		void *ptr) {
	ttLibC_RtmpConnection_ *conn_ = (ttLibC_RtmpConnection_ *)conn;
	if(message == NULL) {
		return false;
	}
	// messageがなにものであるかで、書き込みの方法がかわるはず。
	// 同じデータを使い回す場合は、似たデータがあるか確認して、type1や2を使えばデータサイズは小さくできます。
	if(!ttLibC_RtmpHeader_write(RtmpHeaderType_0, message->header, callback, ptr)) {
		ERR_PRINT("failed to write header message.");
		return false;
	}
	conn_->send_size = 0;
	bool result = true;
	switch(message->header->message_type) {
	case RtmpMessageType_setChunkSize:
	case RtmpMessageType_abortMessage:
	case RtmpMessageType_acknowledgement:
		break;
	case RtmpMessageType_userControlMessage:
		{
			// callback(ptr, buffer, size); // を送ればよい
			// headerを書いたので、あとは情報のみ。
			ttLibC_RtmpUserControlMessage *user_control_message = (ttLibC_RtmpUserControlMessage *)message;
			switch(user_control_message->event_type) {
			case RtmpEventType_StreamBegin:
			case RtmpEventType_StreamEof:
			case RtmpEventType_StreamDry:
			case RtmpEventType_ClientBufferLength:
			case RtmpEventType_RecordedStreamBegin:
			case RtmpEventType_Unknown5:
			case RtmpEventType_Ping:
				break;
			case RtmpEventType_Pong:
				{
					uint8_t data[6];
					uint8_t *buf = data;
					*((uint16_t *)buf) = be_uint16_t(user_control_message->event_type);
					buf += 2;
					*((uint32_t *)buf) = be_uint32_t(user_control_message->value);
					callback(ptr, data, 6);
				}
				return true;
			case RtmpEventType_Unknown8:
			case RtmpEventType_PingSwfVerification:
			case RtmpEventType_PongSwfVerification:
			case RtmpEventType_BufferEmpty:
			case RtmpEventType_BufferFull:
				break;
			default:
				break;
			}
		}
		return false;
	case RtmpMessageType_windowAcknowledgementSize:
	case RtmpMessageType_setPeerBandwidth:
	case RtmpMessageType_audioMessage:
	case RtmpMessageType_videoMessage:
	case RtmpMessageType_amf3DataMessage:
	case RtmpMessageType_amf3SharedObjectMessage:
	case RtmpMessageType_amf3Command:
	case RtmpMessageType_amf0DataMessage:
	case RtmpMessageType_amf0SharedObjectMessage:
		break;
	case RtmpMessageType_amf0Command:
		{
			conn_->send_size = 0;
			ttLibC_RtmpAmf0Command *amf0_command = (ttLibC_RtmpAmf0Command *)message;
			if(!ttLibC_Amf0_write(amf0_command->command_name, callback, ptr)) {
				result = false;
			}
			if(!ttLibC_Amf0_write(amf0_command->command_id, callback, ptr)) {
				result = false;
			}
			if(amf0_command->command_param1 != NULL) {
				if(!ttLibC_Amf0_write(amf0_command->command_param1, callback, ptr)) {
					result = false;
				}
			}
			if(amf0_command->command_param2 != NULL) {
				if(!ttLibC_Amf0_write(amf0_command->command_param2, callback, ptr)) {
					result = false;
				}
			}
		}
		break;
	case RtmpMessageType_aggregateMessage:
		break;
	}
	if(!result) {
		ERR_PRINT("failed to write body message.");
		return false;
	}
	return true;
}

static bool RtmpMessage_read_amf0ReadCallback(void *ptr, ttLibC_Amf0Object *amf0_obj) {
	ttLibC_RtmpMessage *message = (ttLibC_RtmpMessage *)ptr;
	// this amf0_obj will be close after callback. so need to use clone to hold.
	switch(message->header->message_type) {
	case RtmpMessageType_amf0Command:
		{
			ttLibC_RtmpAmf0Command *command = (ttLibC_RtmpAmf0Command *)message;
			if(command->command_name == NULL) {
				command->command_name = ttLibC_Amf0_clone(amf0_obj);
			}
			else if(command->command_id == NULL) {
				command->command_id = ttLibC_Amf0_clone(amf0_obj);
			}
			else if(command->command_param1 == NULL) {
				command->command_param1 = ttLibC_Amf0_clone(amf0_obj);
			}
			else if(command->command_param2 == NULL) {
				command->command_param2 = ttLibC_Amf0_clone(amf0_obj);
			}
		}
		break;
	default:
		LOG_PRINT("acquire invalid data.:%d", message->header->message_type);
		return false;
	}
	return true;
}

static bool RtmpMessage_readHeader(
		ttLibC_RtmpConnection_ *conn,
		uint8_t *data,
		size_t data_size,
		ttLibC_RtmpMessageReadFunc callback,
		void *ptr) {
	ttLibC_RtmpHeader *header = NULL;
	uint32_t left_data_start_pos = 0;
	if(conn->tmp_header_buffer_next_pos != 0) {
		// 前からのデータの追記状態。
		// 必要と思われる分だけ、足してやる
		// header作成上で必要と思われたサイズもしくは、18byteつくるのに必要なサイズ
		// dataから取得できる大きな方を採用する。
		uint8_t *buf = conn->tmp_header_buffer;
		if(data_size >= conn->tmp_header_buffer_size - conn->tmp_header_buffer_next_pos) {
			// data_sizeが十分にあり、tmp_header_buffer_sizeをつくるのに十分な量がある場合
			// データをコピーしてheader作成する。
			// こっちは、差分量をコピーして処理する。
			memcpy(buf + conn->tmp_header_buffer_next_pos, data, conn->tmp_header_buffer_size - conn->tmp_header_buffer_next_pos);
			// これでコピー完了。
			// あとはheader読み込む
			header = ttLibC_RtmpHeader_read(
					(ttLibC_RtmpConnection *)conn,
					conn->tmp_header_buffer,
					conn->tmp_header_buffer_size);
			if(header == NULL) {
				// 読み込めない。
				ERR_PRINT("failed to get header. cannot be.");
				return false;
			}
		}
		else if(data_size >= conn->tmp_header_buffer_target_size - conn->tmp_header_buffer_next_pos) {
			// data_sizeが前回の処理でrequireされた量以上ある場合
			// データをコピーしてheaderを作成する。
			// こっちはdata_sizeにある分だけコピーして処理する。
			memcpy(buf + conn->tmp_header_buffer_next_pos, data, data_size);
			header = ttLibC_RtmpHeader_read(
					(ttLibC_RtmpConnection *)conn,
					conn->tmp_header_buffer,
					conn->tmp_header_buffer_next_pos + data_size);
			if(header == NULL) {
				conn->tmp_header_buffer_next_pos += data_size;
 				return true; // もっかいやる。
			}
		}
		else {
			// コピーしてもデータが足りないので、追記コピーだけして、headerの読み込み処理トライはしない。
			memcpy(buf + conn->tmp_header_buffer_next_pos, data, data_size);
			conn->tmp_header_buffer_next_pos += data_size;
			return true; // もっかいやる。
		}
		// ここにきた場合はheaderができたことになる。
		left_data_start_pos = header->read_size - conn->tmp_header_buffer_next_pos;
	}
	else {
		// 普通にはじめから読み込む場合
		// とりあえずそのまま進む
		header = ttLibC_RtmpHeader_read(
				(ttLibC_RtmpConnection *)conn,
				data,
				data_size);
		if(header == NULL) {
			// データが足りてないので、現状のデータをtmp_header_bufferにコピーして、次の読み込みに託す。
			if(conn->tmp_header_buffer_next_pos == 0) { // next_posが0ではない場合は、すでにbufferにデータがはいっている。
				memcpy(conn->tmp_header_buffer, data, data_size);
			}
			conn->tmp_header_buffer_next_pos = data_size;
			return true;
		}
		// 読み込み成功したので、フェーズをbodyに変更する。
		left_data_start_pos = header->read_size;
	}
	uint32_t left_data_size = data_size - left_data_start_pos;
	conn->read_type = readType_body; // 次の読み込み指示をbodyに変更。
	conn->tmp_header_buffer_next_pos = 0;
	conn->tmp_header_buffer_target_size = 0;
	if(left_data_size > 0) {
		// データが残っている場合は、処理にまわす。
		return ttLibC_RtmpMessage_read(
				(ttLibC_RtmpConnection *)conn,
				data + left_data_start_pos,
				left_data_size,
				callback,
				ptr);
	}
	return true;
}

static ttLibC_RtmpMessage *RtmpMessage_readBody_complete(
		ttLibC_RtmpHeader *header,
		uint8_t *data,
		size_t data_size) {
	switch(header->message_type) {
	case RtmpMessageType_abortMessage:
	case RtmpMessageType_acknowledgement:
		break;
	case RtmpMessageType_userControlMessage:
		{
			ttLibC_RtmpUserControlMessage *message = (ttLibC_RtmpUserControlMessage *)RtmpMessage_read_make(header, sizeof(ttLibC_RtmpUserControlMessage));
			message->event_type = be_uint16_t(*((uint16_t*)data));
			data += 2;
			data_size -= 2;
			switch(message->event_type) {
			case RtmpEventType_StreamBegin:
				// これは・・little endianなのかもしれない。
				message->value = be_uint32_t(*((uint32_t *)data));
				data += 4;
				data_size -= 4;
				break;
//			case RtmpEventType_StreamEof:
//			case RtmpEventType_StreamDry:
//			case RtmpEventType_ClientBufferLength:
//			case RtmpEventType_RecordedStreamBegin:
//			case RtmpEventType_Unknown5:
			case RtmpEventType_Ping: // pingとpongの処理をしなければならない。
			case RtmpEventType_Pong:
				// 4byte time
				message->value = be_uint32_t(*((uint32_t *)data));
				data += 4;
				data_size -= 4;
				break;
//			case RtmpEventType_Unknown8:
//			case RtmpEventType_PingSwfVerification:
//			case RtmpEventType_PongSwfVerification:
//			case RtmpEventType_BufferEmpty:
//			case RtmpEventType_BufferFull:
				break;
			default:
				ERR_PRINT("unknown event type:%d", message->event_type);
			}
			return (ttLibC_RtmpMessage *)message;
		}
		break;
	case RtmpMessageType_setChunkSize:
	case RtmpMessageType_windowAcknowledgementSize:
		{
			if(header->size != 4) {
				ERR_PRINT("data_size is invalid.");
				return NULL;
			}
			ttLibC_Rtmp4ByteMessage *message = (ttLibC_Rtmp4ByteMessage *)RtmpMessage_read_make(
					header,
					sizeof(ttLibC_Rtmp4ByteMessage));
			message->value = be_uint32_t(*((uint32_t *)data));
			return (ttLibC_RtmpMessage *)message;
		}
		break;
	case RtmpMessageType_setPeerBandwidth:
		{
			if(header->size != 5) {
				ERR_PRINT("data_size is invalid.");
				return NULL;
			}
			ttLibC_RtmpSetPeerBandwidth *message = (ttLibC_RtmpSetPeerBandwidth *)RtmpMessage_read_make(
					header,
					sizeof(ttLibC_RtmpSetPeerBandwidth));
			message->window_acknowledge_size = be_uint32_t(*((uint32_t *)data));
			data += 4;
			data_size -= 4;
			message->type = *data;
			return (ttLibC_RtmpMessage *)message;
		}
		break;
	case RtmpMessageType_audioMessage:
	case RtmpMessageType_videoMessage:
	case RtmpMessageType_amf3DataMessage:
	case RtmpMessageType_amf3SharedObjectMessage:
	case RtmpMessageType_amf3Command:
	case RtmpMessageType_amf0DataMessage:
	case RtmpMessageType_amf0SharedObjectMessage:
		break;
	case RtmpMessageType_amf0Command:
		{
			ttLibC_RtmpAmf0Command *message = (ttLibC_RtmpAmf0Command *)RtmpMessage_read_make(
					header,
					sizeof(ttLibC_RtmpAmf0Command));
			message->command_name = NULL;
			message->command_id = NULL;
			message->command_param1 = NULL;
			message->command_param2 = NULL;
			ttLibC_Amf0_read(data, header->size, RtmpMessage_read_amf0ReadCallback, message);
			return (ttLibC_RtmpMessage *)message;
		}
		break;
	case RtmpMessageType_aggregateMessage:
		break;
	}
	return NULL;
}

static bool RtmpMessage_readBody(
		ttLibC_RtmpConnection_ *conn,
		uint8_t *data,
		size_t data_size,
		ttLibC_RtmpMessageReadFunc callback,
		void *ptr) {
	ttLibC_RtmpHeader *header = ttLibC_RtmpHeader_getCurrentHeader((ttLibC_RtmpConnection *)conn);
	uint8_t *buf = NULL;
	bool callback_result = true;
	if(conn->tmp_buffer_next_pos != 0) {
		// 追記で動作する場合
		// データを追記させて動作させなければならない場合
		// 普通の追記
		size_t size_to_chunk_border = conn->chunk_size - (conn->tmp_buffer_next_pos % conn->chunk_size);
		size_t size_to_data = conn->tmp_buffer_target_size - conn->tmp_buffer_next_pos;
		// data_size // 今回処理にまわってきた実データ
		if(size_to_data > size_to_chunk_border) {
			// chunk分いれたい。
			if(data_size >= size_to_chunk_border) {
				// chunk_sizeより大きいので
				// chunkまでのサイズをいれて、再度この関数を呼び出す。
				memcpy(conn->tmp_buffer + conn->tmp_buffer_next_pos, data, size_to_chunk_border);
				conn->tmp_buffer_next_pos += size_to_chunk_border;
				conn->read_type = readType_innerHeader; // 通常のheaderでもいいかも
				if(data_size - size_to_chunk_border > 0) {
					// 呼び出しを実施する必要あり。
					return ttLibC_RtmpMessage_read(
							(ttLibC_RtmpConnection *)conn,
							data + size_to_chunk_border,
							data_size -= size_to_chunk_border,
							callback,
							ptr);
				}
				return true;
			}
			else {
				// chunkまでより小さいのでdata_size分追加すればそれでよい。
				memcpy(conn->tmp_buffer + conn->tmp_buffer_next_pos, data, data_size);
				conn->tmp_buffer_next_pos += data_size;
				return true;
			}
		}
		else {
			if(data_size > size_to_data) {
				// sizeよりデータが大きいので、今回データを作るのに必要なデータがあるはず。
				// 単に追記すればよい。
				memcpy(conn->tmp_buffer + conn->tmp_buffer_next_pos, data, size_to_data);
				// これでデータが完成した。
				ttLibC_RtmpMessage *rtmpMessage = RtmpMessage_readBody_complete(
						header,
						conn->tmp_buffer,
						conn->tmp_buffer_target_size);
				if(rtmpMessage == NULL) {
					return false;
				}
				callback_result = callback(ptr, rtmpMessage);
				ttLibC_RtmpMessage_close(&rtmpMessage);
				if(!callback_result) {
					return false;
				}
				conn->read_type = readType_header; // headerの読み込みに戻す。
				conn->tmp_buffer_next_pos = 0;
				conn->tmp_buffer_target_size = 0;
				// 残りのデータがある場合は処理にまわす。
				if(data_size - size_to_data > 0) {
					return ttLibC_RtmpMessage_read(
							(ttLibC_RtmpConnection *)conn,
							data + size_to_data,
							data_size - size_to_data,
							callback,
							ptr);
				}
				return true;
			}
			else {
				// sizeよりデータが小さいので、単に足しておわり。
				// 次のデータ追記がこないとなにもできない。
				memcpy(conn->tmp_buffer + conn->tmp_buffer_next_pos, data, data_size);
				conn->tmp_buffer_next_pos += data_size;
				return true;
			}
		}
		return false;
	}
	else {
		// headerのサイズがchunk_sizeより大きかったら、chunkをきにする必要がでてくる
		// とりあえず、conn->tmp_bufferにコピーして、あとでcompleteで動作するように・・・っていうかしなくてもいいか・・・
		// currentHeaderをベースにぽいぽいする。
		if(header->size > conn->chunk_size) {
			// chunkより大きなデータを扱うので、chunk_size分追加することにする。
			// この時点でbufferは決定できそう。
			if(conn->tmp_buffer != NULL && conn->tmp_buffer_size < header->size) {
				ttLibC_free(conn->tmp_buffer);
				conn->tmp_buffer = NULL;
			}
			if(conn->tmp_buffer == NULL) {
				conn->tmp_buffer = ttLibC_malloc(header->size);
				conn->tmp_buffer_size = header->size;
			}
			conn->tmp_buffer_target_size = header->size;
			if(data_size >= conn->chunk_size) {
				// chunk_sizeより大きいので、chunk_size分追加してから、再度この関数にかけなおす。
				// ここだけまだ作ってない感じか？
				memcpy(conn->tmp_buffer, data, conn->chunk_size);
				conn->tmp_buffer_next_pos = conn->chunk_size;
				conn->read_type = readType_innerHeader;
				if(data_size - conn->chunk_size > 0) {
					// 呼び出しを実施する必要あり。
					return ttLibC_RtmpMessage_read(
							(ttLibC_RtmpConnection *)conn,
							data + conn->chunk_size,
							data_size -= conn->chunk_size,
							callback,
							ptr);
				}
				return true;
			}
			else {
				// chunk_size分より小さいので、そのまま追加する。
				memcpy(conn->tmp_buffer, data, data_size);
				conn->tmp_buffer_next_pos = data_size;
				return true;
			}
			return false;
		}
		else {
			// そのまま処理する。
			if(data_size < header->size) {
				// tmpBufferに記録して、使い回す。
				if(conn->tmp_buffer != NULL && conn->tmp_buffer_size < header->size) {
					ttLibC_free(conn->tmp_buffer);
					conn->tmp_buffer = NULL;
				}
				if(conn->tmp_buffer == NULL) {
					conn->tmp_buffer = ttLibC_malloc(header->size);
					conn->tmp_buffer_size = header->size;
				}
				conn->tmp_buffer_target_size = header->size;
				memcpy(conn->tmp_buffer, data, data_size);
				conn->tmp_buffer_next_pos = data_size;
				return true;
			}
			else {
				// 普通に処理する。
				ttLibC_RtmpMessage *rtmpMessage = RtmpMessage_readBody_complete(
						header,
						data,
						data_size);
				if(rtmpMessage == NULL) {
					return false;
				}
				callback_result = callback(ptr, rtmpMessage);
				ttLibC_RtmpMessage_close(&rtmpMessage);
				if(!callback_result) {
					return false;
				}
				// 読み込んでメッセージが残っていたら次の処理にまわす。
				conn->read_type = readType_header; // headerの読み込みに戻す。
				conn->tmp_buffer_next_pos = 0;
				conn->tmp_buffer_target_size = 0;
				if(data_size - header->size > 0) {
					return ttLibC_RtmpMessage_read(
							(ttLibC_RtmpConnection *)conn,
							data + header->size,
							data_size - header->size,
							callback,
							ptr);
				}
			}
		}
	}
	return true;
}

bool ttLibC_RtmpMessage_read(
		ttLibC_RtmpConnection *conn,
		uint8_t *data,
		size_t data_size,
		ttLibC_RtmpMessageReadFunc callback,
		void *ptr) {
	ttLibC_RtmpConnection_ *conn_ = (ttLibC_RtmpConnection_ *)conn;
	switch(conn_->read_type) {
	case readType_header:
		return RtmpMessage_readHeader(
				conn_,
				data,
				data_size,
				callback,
				ptr);
	case readType_body:
		return RtmpMessage_readBody(
				conn_,
				data,
				data_size,
				callback,
				ptr);
	case readType_innerHeader:
		return RtmpMessage_readHeader(
				conn_,
				data,
				data_size,
				callback,
				ptr);
	default:
		break;
	}
	return true;
}

void ttLibC_RtmpMessage_close(ttLibC_RtmpMessage **message) {
	ttLibC_RtmpMessage *target = *message;
	if(target == NULL) {
		return;
	}
	if(target->header != NULL) {
		switch(target->header->message_type) {
		case RtmpMessageType_setChunkSize:
			break; // checked
		case RtmpMessageType_abortMessage:
		case RtmpMessageType_acknowledgement:
		case RtmpMessageType_userControlMessage:
			break;
		case RtmpMessageType_windowAcknowledgementSize:
			break; // checked
		case RtmpMessageType_setPeerBandwidth:
			break; // checked
		case RtmpMessageType_audioMessage:
		case RtmpMessageType_videoMessage:
		case RtmpMessageType_amf3DataMessage:
		case RtmpMessageType_amf3SharedObjectMessage:
		case RtmpMessageType_amf3Command:
		case RtmpMessageType_amf0DataMessage:
		case RtmpMessageType_amf0SharedObjectMessage:
			break;
		case RtmpMessageType_amf0Command:
			{
				ttLibC_RtmpAmf0Command *amf0_command = (ttLibC_RtmpAmf0Command *)target;
				ttLibC_Amf0_close(&amf0_command->command_name);
				ttLibC_Amf0_close(&amf0_command->command_id);
				ttLibC_Amf0_close(&amf0_command->command_param1);
				ttLibC_Amf0_close(&amf0_command->command_param2);
			}
			break;
		case RtmpMessageType_aggregateMessage:
			break;
		}
	}
	// header will be close during netConnection close.
//	ttLibC_RtmpHeader_close(&target->header);
	ttLibC_free(target);
	*message = NULL;
}
