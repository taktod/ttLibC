/*
 * @file   dynamicBufferUtil.c
 * @brief  expandable data buffer.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/10/06
 */

#include "dynamicBufferUtil.h"
#include "../ttLibC_predef.h"
#include "../allocator.h"
#include <string.h>
#include "../_log.h"
#include "../ttLibC_common.h"

typedef struct {
	ttLibC_DynamicBuffer inherit_super;
	uint8_t *buffer;
	size_t read_pos;
	size_t buffer_size;
	size_t target_size;
} ttLibC_Util_DynamicBuffer_;

typedef ttLibC_Util_DynamicBuffer_ ttLibC_DynamicBuffer_;

ttLibC_DynamicBuffer TT_VISIBILITY_DEFAULT *ttLibC_DynamicBuffer_make() {
	ttLibC_DynamicBuffer_ *buffer = ttLibC_malloc(sizeof(ttLibC_DynamicBuffer_));
	if(buffer == NULL) {
		return NULL;
	}
	buffer->buffer = NULL;
	buffer->inherit_super.buffer_size = 0;
	buffer->inherit_super.target_size = 0;
	buffer->inherit_super.error = Error_noError;
	buffer->buffer_size = 0;
	buffer->target_size = 0;
	buffer->read_pos = 0;
	return (ttLibC_DynamicBuffer *)buffer;
}

bool TT_VISIBILITY_DEFAULT ttLibC_DynamicBuffer_append(
		ttLibC_DynamicBuffer *buffer,
		uint8_t *data,
		size_t data_size) {
	ttLibC_DynamicBuffer_ *buffer_ = (ttLibC_DynamicBuffer_ *)buffer;
	if(buffer_ == NULL) {
		return false;
	}
	if(buffer_->buffer == NULL) {
		// no data. make new one.
		buffer_->buffer = ttLibC_malloc(data_size);
		if(buffer_->buffer == NULL) {
			ERR_PRINT("failed to allocate memory for buffer.");
			buffer_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_MemoryAllocate);
			return false;
		}
		memcpy(buffer_->buffer, data, data_size);
		buffer_->target_size = data_size;
		buffer_->buffer_size = data_size;
		buffer_->inherit_super.target_size = buffer_->target_size;
		buffer_->inherit_super.buffer_size = buffer_->buffer_size;
		return true;
	}
	else {
		// already have data.
		if(buffer_->target_size + data_size < buffer_->buffer_size) {
			// have enough memory size, append on the end.
			memcpy(buffer_->buffer + buffer_->target_size, data, data_size);
			buffer_->target_size += data_size;
			buffer_->inherit_super.target_size = buffer_->target_size;
			return true;
		}
		else {
			// not enough memory size.
			// reallocate and append data.
			size_t target_size = buffer_->target_size + data_size;
			uint8_t *new_buffer = ttLibC_malloc(target_size);
			if(new_buffer == NULL) {
				ERR_PRINT("failed to allocate memory for expand buffer.");
				buffer_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_MemoryAllocate);
				return false;
			}
			memcpy(new_buffer, buffer_->buffer, buffer_->target_size);
			memcpy(new_buffer + buffer_->target_size, data, data_size);
			ttLibC_free(buffer_->buffer);
			buffer_->buffer = new_buffer;
			buffer_->buffer_size = target_size;
			buffer_->target_size = target_size;
			buffer_->inherit_super.buffer_size = buffer_->buffer_size;
			buffer_->inherit_super.target_size = buffer_->target_size;
			return true;
 		}
	}
}

bool TT_VISIBILITY_DEFAULT ttLibC_DynamicBuffer_markAsRead(ttLibC_DynamicBuffer *buffer, size_t read_size) {
	ttLibC_DynamicBuffer_ *buffer_ = (ttLibC_DynamicBuffer_ *)buffer;
	if(buffer_ == NULL) {
		return false;
	}
	if(buffer_->read_pos + read_size > buffer_->target_size) {
		ERR_PRINT("read_size is bigger than target_size, overflowed.");
		buffer_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_TtLibCError);
		return false;
	}
	buffer_->read_pos += read_size;
	return true;
}

uint8_t TT_VISIBILITY_DEFAULT *ttLibC_DynamicBuffer_refData(ttLibC_DynamicBuffer *buffer) {
	ttLibC_DynamicBuffer_ *buffer_ = (ttLibC_DynamicBuffer_ *)buffer;
	if(buffer_ == NULL) {
		return NULL;
	}
	return buffer_->buffer + buffer_->read_pos;
}

size_t TT_VISIBILITY_DEFAULT ttLibC_DynamicBuffer_refSize(ttLibC_DynamicBuffer *buffer) {
	ttLibC_DynamicBuffer_ *buffer_ = (ttLibC_DynamicBuffer_ *)buffer;
	if(buffer_ == NULL) {
		return 0;
	}
	return buffer_->target_size - buffer_->read_pos;
}

bool TT_VISIBILITY_DEFAULT ttLibC_DynamicBuffer_clear(ttLibC_DynamicBuffer *buffer) {
	ttLibC_DynamicBuffer_ *buffer_ = (ttLibC_DynamicBuffer_ *)buffer;
	if(buffer_ == NULL) {
		return false;
	}
	if(buffer_->read_pos == 0) {
		// if read_pos is 0, no need to shift.
		return true;
	}
	uint8_t *buf = buffer_->buffer;
	for(int i = 0,max = buffer_->target_size - buffer_->read_pos;i < max;++ i) {
		*buf = *(buf + buffer_->read_pos);
		++ buf;
	}
	buffer_->target_size -= buffer_->read_pos;
	buffer_->inherit_super.target_size = buffer_->target_size;
	buffer_->read_pos = 0;
	return true;
}

bool TT_VISIBILITY_DEFAULT ttLibC_DynamicBuffer_reset(ttLibC_DynamicBuffer *buffer) {
	ttLibC_DynamicBuffer_ *buffer_ = (ttLibC_DynamicBuffer_ *)buffer;
	if(buffer_ == NULL) {
		return false;
	}
	buffer_->read_pos = 0; // just to reset to 0.
	return true;
}

/**
 * set empty for writing buffer.
 */
bool TT_VISIBILITY_DEFAULT ttLibC_DynamicBuffer_empty(ttLibC_DynamicBuffer *buffer) {
	ttLibC_DynamicBuffer_ *buffer_ = (ttLibC_DynamicBuffer_ *)buffer;
	if(buffer_ == NULL) {
		return false;
	}
	buffer_->read_pos = 0;
	buffer_->target_size = 0;
	buffer_->inherit_super.target_size = 0;
	return true;
}

bool TT_VISIBILITY_DEFAULT ttLibC_DynamicBuffer_alloc(
		ttLibC_DynamicBuffer *buffer,
		size_t size) {
	ttLibC_DynamicBuffer_ *buffer_ = (ttLibC_DynamicBuffer_ *)buffer;
	if(buffer_ == NULL) {
		return false;
	}
	if(buffer_->buffer == NULL) {
		// if no data. alloc memory.
		buffer_->buffer = ttLibC_malloc(size);
		if(buffer_->buffer == NULL) {
			ERR_PRINT("failed to allocate memory for buffer.");
			buffer_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_MemoryAllocate);
			return false;
		}
		buffer_->buffer_size = size;
		buffer_->target_size = size;
		buffer_->inherit_super.buffer_size = buffer_->buffer_size;
		buffer_->inherit_super.target_size = buffer_->target_size;
		return true;
	}
	else {
		if(buffer_->buffer_size > size) {
			// if already have enough size. nothing to do.
			buffer_->target_size = size;
			buffer_->inherit_super.target_size = buffer_->target_size;
			return true;
		}
		else {
			// if already have not enough size. re-allocate.
			uint8_t *new_buffer = ttLibC_malloc(size);
			if(new_buffer == NULL) {
				ERR_PRINT("failed to allocate memory for new size.");
				buffer_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_MemoryAllocate);
				return false;
			}
			// copy previous data.
			memcpy(new_buffer, buffer_->buffer, buffer_->target_size);
			// free prev data.
			ttLibC_free(buffer_->buffer);
			buffer_->buffer = new_buffer;
			buffer_->buffer_size = size;
			buffer_->inherit_super.buffer_size = buffer_->buffer_size;
			buffer_->target_size = size;
			buffer_->inherit_super.target_size = buffer_->target_size;
		}
	}
	return true;
}

bool TT_VISIBILITY_DEFAULT ttLibC_DynamicBuffer_write(
		ttLibC_DynamicBuffer *buffer,
		size_t write_pos,
		uint8_t *data,
		size_t data_size) {
	ttLibC_DynamicBuffer_ *buffer_ = (ttLibC_DynamicBuffer_ *)buffer;
	if(buffer_ == NULL) {
		return false;
	}
	if(buffer_->buffer == NULL) {
		buffer_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_MemoryAllocate);
		return false;
	}
	if(write_pos + data_size > buffer_->target_size) {
		ERR_PRINT("data is overflowed.");
		buffer_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_NeedMoreOutput);
		return false;
	}
	memcpy(buffer_->buffer + write_pos, data, data_size);
	return true;
}

void TT_VISIBILITY_DEFAULT ttLibC_DynamicBuffer_close(ttLibC_DynamicBuffer **buffer) {
	ttLibC_DynamicBuffer_ *target = (ttLibC_DynamicBuffer_ *)*buffer;
	if(target == NULL) {
		return;
	}
	if(target->buffer != NULL) {
		ttLibC_free(target->buffer);
	}
	ttLibC_free(target);
	*buffer = NULL;
}
