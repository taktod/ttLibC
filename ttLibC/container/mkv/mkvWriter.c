/*
 * @file   mkvWriter.c
 * @brief  mkv container writer.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/09/04
 */

#include "mkvWriter.h"
#include "../../log.h"
#include "../../allocator.h"
#include "../../util/hexUtil.h"

ttLibC_MkvWriter *ttLibC_MkvWriter_make() {
	return NULL;
}

bool ttLibC_MkvWriter_write(
		ttLibC_MkvWriter *writer,
		ttLibC_Frame *frame,
		ttLibC_ContainerWriteFunc callback,
		void *ptr) {
	return false;
}

void ttLibC_MkvWriter_close(ttLibC_MkvWriter **writer) {
	ttLibC_MkvWriter_ *target = (ttLibC_MkvWriter_ *)*writer;
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.inherit_super.type != containerType_mkv) {
		ERR_PRINT("try to close non mkvWriter.");
		return;
	}
	ttLibC_free(target);
	*writer = NULL;
}


