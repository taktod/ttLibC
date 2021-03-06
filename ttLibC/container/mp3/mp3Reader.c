/*
 * @file   mp3Reader.c
 * @brief  mp3Frame reader from binary data.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/23
 */

#include "mp3Reader.h"
#include "../../ttLibC_predef.h"
#include "../../_log.h"
#include "../../allocator.h"
#include "../../frame/audio/mp3.h"
#include <stdlib.h>
#include <string.h>

ttLibC_Mp3Reader TT_VISIBILITY_DEFAULT *ttLibC_Mp3Reader_make() {
	ttLibC_Mp3Reader_ *reader = (ttLibC_Mp3Reader_ *)ttLibC_ContainerReader_make(containerType_mp3, sizeof(ttLibC_Mp3Reader_));
	if(reader == NULL) {
		ERR_PRINT("failed to allocate memory for reader.");
		return NULL;
	}
	reader->current_pts = 0;
	// fill data for example data.
	reader->timebase = 44100;
	reader->frame = NULL;

	reader->tmp_buffer = ttLibC_DynamicBuffer_make();
	reader->is_reading = false;
	return (ttLibC_Mp3Reader *)reader;
}

bool TT_VISIBILITY_HIDDEN Mp3Reader_updateMp3Frame(
		ttLibC_Mp3Reader_ *reader,
		ttLibC_Mp3 *mp3) {
	ttLibC_Mp3Frame *mp3_frame = ttLibC_Mp3Frame_make(
			reader->frame,
			NULL,
			0,
			true,
			mp3->inherit_super.inherit_super.pts,
			mp3->inherit_super.inherit_super.timebase);
	if(mp3_frame == NULL) {
		return false;
	}
	mp3_frame->frame = (ttLibC_Frame *)mp3;
	reader->frame = mp3_frame;
	return true;
}

ttLibC_Mp3 TT_VISIBILITY_HIDDEN *Mp3Reader_readMp3FromBinary(
		ttLibC_Mp3Reader_ *reader,
		uint8_t *data,
		size_t data_size) {
	ttLibC_Mp3 *mp3 = NULL;
	ttLibC_Mp3 *prev_frame = NULL;
	if(reader->frame != NULL) {
		prev_frame = (ttLibC_Mp3 *)reader->frame->frame;
	}
	mp3 = ttLibC_Mp3_getFrame(prev_frame, data, data_size, true, 0, 1000);
	if(mp3 != NULL) {
		// fix timestamp information.
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

bool TT_VISIBILITY_DEFAULT ttLibC_Mp3Reader_read(
		ttLibC_Mp3Reader *reader,
		void *data,
		size_t data_size,
		ttLibC_Mp3ReadFunc callback,
		void *ptr) {
	ttLibC_Mp3Reader_ *reader_ = (ttLibC_Mp3Reader_ *)reader;
	ttLibC_Mp3 *mp3 = NULL;
	ttLibC_DynamicBuffer_append(reader_->tmp_buffer, data, data_size);
	if(reader_->is_reading) {
		return true;
	}
	reader_->is_reading = true;
	do {
		mp3 = Mp3Reader_readMp3FromBinary(reader_, ttLibC_DynamicBuffer_refData(reader_->tmp_buffer), ttLibC_DynamicBuffer_refSize(reader_->tmp_buffer));
		if(mp3 == NULL) {
			// if mp3 is NULL, need more data.
			ttLibC_DynamicBuffer_clear(reader_->tmp_buffer);
			reader_->is_reading = false;
			return true;
		}
		ttLibC_DynamicBuffer_markAsRead(reader_->tmp_buffer, mp3->inherit_super.inherit_super.buffer_size);
		if(!Mp3Reader_updateMp3Frame(
				reader_,
				mp3)) {
			ERR_PRINT("failed to make mp3 frame");
			ttLibC_DynamicBuffer_clear(reader_->tmp_buffer);
			reader_->is_reading = false;
			return false;
		}
		if(!callback(ptr, (ttLibC_Container_Mp3 *)reader_->frame)) {
			ttLibC_DynamicBuffer_clear(reader_->tmp_buffer);
			reader_->is_reading = false;
			return false;
		}
	} while(true);
}

void TT_VISIBILITY_DEFAULT ttLibC_Mp3Reader_close(ttLibC_Mp3Reader **reader) {
	ttLibC_Mp3Reader_ *target = (ttLibC_Mp3Reader_ *)*reader;
	if(target == NULL) {
		return;
	}
	ttLibC_Mp3Frame_close(&target->frame);
	ttLibC_DynamicBuffer_close(&target->tmp_buffer);
	ttLibC_free(target);
	*reader = NULL;
}

