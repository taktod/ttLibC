/*
 * @file   mp3Reader.c
 * @brief  
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/23
 */

#include "mp3Reader.h"
#include "../../log.h"
#include "../../allocator.h"
#include "../../frame/audio/mp3.h"
#include <stdlib.h>
#include <string.h>

ttLibC_Mp3Reader *ttLibC_Mp3Reader_make() {
	ttLibC_Mp3Reader_ *reader = (ttLibC_Mp3Reader_ *)ttLibC_ContainerReader_make(containerType_mp3, sizeof(ttLibC_Mp3Reader_));
	if(reader == NULL) {
		ERR_PRINT("failed to allocate memory for reader.");
		return NULL;
	}
	// これは一度作成すれば、それを使い回すことができるっぽいですね。

	reader->current_pts = 0;
	reader->timebase = 44100;
	reader->frame = NULL;
	reader->tmp_buffer_size = 512;
	reader->tmp_buffer = ttLibC_malloc(reader->tmp_buffer_size);
	reader->tmp_buffer_next_pos = 0;
	return (ttLibC_Mp3Reader *)reader;
}

bool Mp3Reader_updateMp3Frame(
		ttLibC_Mp3Reader_ *reader,
		ttLibC_Mp3 *mp3) {
	ttLibC_Mp3Frame *mp3_frame = ttLibC_Mp3Frame_make(
			reader->frame,
			NULL,
			0,
			true,
			mp3->inherit_super.inherit_super.pts,
			mp3->inherit_super.inherit_super.timebase);;
	if(mp3_frame == NULL) {
		return false;
	}
	mp3_frame->frame = (ttLibC_Frame *)mp3;
	reader->frame = mp3_frame;
	return true;
}

ttLibC_Mp3 *Mp3Reader_readMp3FromBinary(
		ttLibC_Mp3Reader_ *reader,
		uint8_t *data,
		size_t data_size) {
	ttLibC_Mp3 *mp3 = NULL;
	ttLibC_Mp3 *prev_frame = NULL;
	if(reader->frame != NULL) {
		prev_frame = (ttLibC_Mp3 *)reader->frame->frame;
	}
	mp3 = ttLibC_Mp3_getFrame(prev_frame, data, data_size, 0, 1000);
	if(mp3 != NULL) {
		switch(mp3->type) {
		case Mp3Type_frame:
			mp3->inherit_super.inherit_super.pts = reader->current_pts;
			mp3->inherit_super.inherit_super.timebase = mp3->inherit_super.sample_rate;
			reader->current_pts += mp3->inherit_super.sample_num;
			reader->timebase = mp3->inherit_super.sample_rate;
			break;
		default:
			break;
		}
	}
	return mp3;
}

bool ttLibC_Mp3Reader_read(
		ttLibC_Mp3Reader *reader,
		void *data,
		size_t data_size,
		ttLibC_Mp3ReaderFunc callback,
		void *ptr) {
	// mp3lame encoderのときと違いこっちでは、データサイズが足りない場合がある。
	// mp3readerで読み込めばまったく同じになっているので・・・そうしようか・・・
	ttLibC_Mp3Reader_ *reader_ = (ttLibC_Mp3Reader_ *)reader;
	ttLibC_Mp3 *mp3 = NULL;
	if(reader_->tmp_buffer_next_pos != 0) {
		// 現在のデータをtmp_bufferに追記する。
		size_t copy_size = reader_->tmp_buffer_size - reader_->tmp_buffer_next_pos;
		if(copy_size > data_size) {
			copy_size = data_size;
		}
		memcpy(reader_->tmp_buffer + reader_->tmp_buffer_next_pos, data, copy_size);
		mp3 = Mp3Reader_readMp3FromBinary(reader_, reader_->tmp_buffer, reader_->tmp_buffer_next_pos + copy_size);
		if(mp3 == NULL) {
			ERR_PRINT("failed to get mp3 frame. something fatal happen?");
			return false;
		}
		data += mp3->inherit_super.inherit_super.buffer_size - reader_->tmp_buffer_next_pos;
		data_size -= mp3->inherit_super.inherit_super.buffer_size - reader_->tmp_buffer_next_pos;

		if(!Mp3Reader_updateMp3Frame(
				reader_,
				mp3)) {
			return false;
		}
		// このmp3_frameをcallbackで応答すればよい。
		if(!callback(ptr, (ttLibC_Container_Mp3 *)reader_->frame)) {
			return false;
		}
	}
	// ここからdo whileループにする。
	do {
		mp3 = Mp3Reader_readMp3FromBinary(reader_, data, data_size);
		if(mp3 == NULL) {
			memcpy(reader_->tmp_buffer, data, data_size);
			reader_->tmp_buffer_next_pos = data_size;
			// NULLになった場合はデータが足りてないから。
			// だいたいがサイズが足りないので、ここでおわり。
			return true;
		}
		data += mp3->inherit_super.inherit_super.buffer_size;
		data_size -= mp3->inherit_super.inherit_super.buffer_size;
		if(!Mp3Reader_updateMp3Frame(
				reader_,
				mp3)) {
			ERR_PRINT("failed to make mp3 frame");
			return false;
		}
		// このmp3_frameをcallbackで応答すればよい。
		if(!callback(ptr, (ttLibC_Container_Mp3 *)reader_->frame)) {
			return false;
		}
	} while(true);
}

void ttLibC_Mp3Reader_close(ttLibC_Mp3Reader **reader) {
	ttLibC_Mp3Reader_ *target = (ttLibC_Mp3Reader_ *)*reader;
	if(target == NULL) {
		return;
	}
	ttLibC_Mp3Frame_close(&target->frame);
	if(target->tmp_buffer != NULL) {
		ttLibC_free(target->tmp_buffer);
	}
	ttLibC_free(target);
	*reader = NULL;
}

