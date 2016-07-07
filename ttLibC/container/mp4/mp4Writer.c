/**
 * @file   mp4Writer.c
 * @brief  
 * @author taktod
 * @date   2016/07/03
 */

#include "../mp4.h"

ttLibC_Mp4Writer *ttLibC_Mp4Writer_make() {
	return NULL;
}

bool ttLibC_Mp4Writer_write(
		ttLibC_Mp4Writer *writer,
		ttLibC_Frame *frame,
		ttLibC_ContainerWriteFunc callback,
		void *ptr) {
	return false;
}

void ttLibC_Mp4Writer_close(ttLibC_Mp4Writer **writer) {
}


