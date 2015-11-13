/*
 * @file   mp3Writer.c
 * @brief  
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/23
 */

#include "mp3Writer.h"
#include "../../log.h"
#include "../../allocator.h"
#include "../../frame/audio/mp3.h"
#include <stdlib.h>


ttLibC_Mp3Writer *ttLibC_Mp3Writer_make() {
	ttLibC_Mp3Writer_ *writer = (ttLibC_Mp3Writer_ *)ttLibC_ContainerWriter_make(containerType_mp3, sizeof(ttLibC_Mp3Writer_), 44100);
	if(writer != NULL) {
		writer->is_first = true;
	}
	return (ttLibC_Mp3Writer *)writer;
}

bool ttLibC_Mp3Writer_write(
		ttLibC_Mp3Writer *writer,
		ttLibC_Frame *frame,
		ttLibC_ContainerWriteFunc callback,
		void *ptr) {
	ttLibC_Mp3Writer_ *writer_ = (ttLibC_Mp3Writer_ *)writer;
	if(writer_ == NULL) {
		ERR_PRINT("writer is not ready.");
		return false;
	}
	if(frame->type != frameType_mp3) {
		ERR_PRINT("only support mp3.");
		return false;
	}
	// frameのptsデータをsample_rateベースに書き直します。
	ttLibC_Mp3 *mp3 = (ttLibC_Mp3 *)frame;
	mp3->inherit_super.inherit_super.pts = mp3->inherit_super.inherit_super.pts * mp3->inherit_super.sample_rate / mp3->inherit_super.inherit_super.timebase;
	mp3->inherit_super.inherit_super.timebase = mp3->inherit_super.sample_rate;
	// timestampの値を確認しておきたい。
	return callback(ptr, mp3->inherit_super.inherit_super.data, mp3->inherit_super.inherit_super.buffer_size);
}

void ttLibC_Mp3Writer_close(ttLibC_Mp3Writer **writer) {
	ttLibC_Mp3Writer_ *target = (ttLibC_Mp3Writer_ *)*writer;
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.inherit_super.type != containerType_mp3) {
		ERR_PRINT("try to close non mp3 writer.");
		return;
	}
	ttLibC_free(target);
	*writer = NULL;
}
