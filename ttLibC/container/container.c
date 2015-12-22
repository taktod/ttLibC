/*
 * @file   container.c
 * @brief  container support.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/12
 */

#include "container.h"
#include "containerCommon.h"

#include <stdlib.h>
#include <string.h>

#include "flv.h"
#include "mp3.h"
#include "mpegts.h"
#include "../log.h"
#include "../allocator.h"

/*
 * common function for container make.
 * @param prev_container reuse container object.
 * @param container_size memory allocate size for container object.
 * @param container_type target container_type
 * @param data           data
 * @param data_size      data_size
 * @param non_copy_mode  true:hold the pointer / false:copy memory data.
 * @param pts            pts for container
 * @param timebase       timebase for pts.
 */
ttLibC_Container *ttLibC_Container_make(
		ttLibC_Container *prev_container,
		size_t container_size,
		ttLibC_Container_Type container_type,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	ttLibC_Container *container = prev_container;
	size_t buffer_size_ = data_size;
	size_t data_size_ = data_size;
	switch(container_type) {
	case containerType_flv:
	case containerType_mkv:
	case containerType_mp3:
	case containerType_mp4:
	case containerType_mpegts:
	case containerType_riff:
	case containerType_wav:
		break;
	default:
		ERR_PRINT("unknown container type.%d", container_type);
		return NULL;
	}
	if(container_size == 0) {
		ERR_PRINT("invalid container_size = 0;");
		return NULL;
	}
	if(container == NULL) {
		container = ttLibC_malloc(container_size);
		if(container == NULL) {
			ERR_PRINT("failed to allocate memory for container.");
			return NULL;
		}
		container->data = NULL;
	}
	else {
		if(!container->is_non_copy) {
			if(non_copy_mode || container->data_size < data_size) {
				ttLibC_free(container->data);
				container->data = NULL;
			}
			else {
				data_size_ = container->data_size;
			}
		}
	}
	container->buffer_size = buffer_size_;
	container->data_size   = data_size_;
	container->is_non_copy = non_copy_mode;
	container->pts         = pts;
	container->timebase    = timebase;
	container->type        = container_type;
	if(non_copy_mode) {
		container->data = data;
	}
	else {
		if(container->data == NULL) {
			container->data = ttLibC_malloc(data_size);
			if(container->data == NULL) {
				ERR_PRINT("failed to allocate memory for data.");
				if(prev_container == NULL) {
					ttLibC_free(container);
				}
				return NULL;
			}
		}
		memcpy(container->data, data, data_size);
	}
	return container;
}

/**
 * get frame from container object.
 * @param container container object.
 * @param callback  callback function
 * @param ptr       user def pointer object.
 * @return true:success false:error
 */
bool ttLibC_Container_getFrame(
		ttLibC_Container *container,
		ttLibC_getFrameFunc callback,
		void *ptr) {
	switch(container->type) {
	case containerType_flv:
		return ttLibC_Flv_getFrame((ttLibC_Flv *)container, callback, ptr);
//	case containerType_mkv:
	case containerType_mp3:
		return ttLibC_Container_Mp3_getFrame((ttLibC_Container_Mp3 *)container, callback, ptr);
//	case containerType_mp4:
	case containerType_mpegts:
		return ttLibC_Mpegts_getFrame((ttLibC_Mpegts *)container, callback, ptr);
//	case containerType_riff:
//	case containerType_wav:
	default:
		return false;
	}
}

/*
 * close container
 * /
void ttLibC_Container_close(ttLibC_Container **container) {
	ttLibC_Container *target = *container;
	if(target == NULL) {
		return;
	}
	switch(target->type) {
	case containerType_flv:
		ttLibC_Flv_close((ttLibC_Flv **)container);
		break;
	case containerType_mp3:
		ttLibC_Container_Mp3_close((ttLibC_Container_Mp3 **)container);
		break;
	case containerType_mpegts:
		ttLibC_Mpegts_close((ttLibC_Mpegts **)container);
		break;
	case containerType_mkv:
	case containerType_mp4:
	case containerType_riff:
	case containerType_wav:
		ERR_PRINT("not make yet.:%d", target->type);
		break;
	default:
		ERR_PRINT("unknown container type:%d", target->type);
		break;
	}
}
*/

/*
 * common work for containerReader make.
 * use inner only.
 * @param container_type target container type.
 * @param reader_size    sizeof object.
 * @return reader object.
 */
ttLibC_ContainerReader *ttLibC_ContainerReader_make(
		ttLibC_Container_Type container_type,
		size_t reader_size) {
	if(reader_size == 0) {
		ERR_PRINT("0 is invalid size for malloc.");
		return NULL;
	}
	ttLibC_ContainerReader *reader = ttLibC_malloc(reader_size);
	if(reader == NULL) {
		ERR_PRINT("failed to allocate memory for reader.");
		return NULL;
	}
	reader->type = container_type;
	return reader;
}

bool ttLibC_ContainerReader_read(
		ttLibC_ContainerReader *reader,
		void *data,
		size_t data_size,
		ttLibC_ContainerReadFunc callback,
		void *ptr) {
	// それぞれのコンテナの中身をみる。
	return true;
}

/*
 * close container reader
 */
void ttLibC_ContainerReader_close(ttLibC_ContainerReader **reader) {
	ttLibC_ContainerReader *target = *reader;
	if(target == NULL) {
		return;
	}
	switch(target->type) {
	case containerType_flv:
		ttLibC_FlvReader_close((ttLibC_FlvReader **)reader);
		return;
	case containerType_mp3:
		ttLibC_Mp3Reader_close((ttLibC_Mp3Reader **)reader);
		return;
//	case containerType_mp4:
	case containerType_mpegts:
		ttLibC_MpegtsReader_close((ttLibC_MpegtsReader **)reader);
		return;
//	case containerType_riff:
//	case containerType_wav:
	default:
		ERR_PRINT("unknown container type for reader close.%d", target->type);
		break;
	}
}

/*
 * common work for containerWriter make
 * use inner only.
 * @param container_type target container type.
 * @param writer_size    sizeof object.
 * @param timebase       timebase for writer.
 * @return writer object.
 */
ttLibC_ContainerWriter *ttLibC_ContainerWriter_make(
		ttLibC_Container_Type container_type,
		size_t writer_size,
		uint32_t timebase) {
	if(writer_size == 0) {
		ERR_PRINT("0 is invalid size for malloc.");
		return NULL;
	}
	ttLibC_ContainerWriter *writer = ttLibC_malloc(writer_size);
	if(writer == NULL) {
		ERR_PRINT("failed to allocate memory for writer.");
		return NULL;
	}
	writer->type     = container_type;
	writer->timebase = timebase;
	writer->pts      = 0;
	return writer;
}

bool ttLibC_ContainerWriter_write(
		ttLibC_ContainerWriter *writer,
		ttLibC_Frame *frame,
		ttLibC_ContainerWriteFunc callback,
		void *ptr) {
	return true;
}

/*
 * close container writer
 */
void ttLibC_ContainerWriter_close(ttLibC_ContainerWriter **writer) {
	ttLibC_ContainerWriter *target = *writer;
	if(target == NULL) {
		return;
	}
	switch(target->type) {
	case containerType_flv:
		ttLibC_FlvWriter_close((ttLibC_FlvWriter **)writer);
		return;
	case containerType_mp3:
		ttLibC_Mp3Writer_close((ttLibC_Mp3Writer **)writer);
		return;
//	case containerType_mp4:
	case containerType_mpegts:
		ttLibC_MpegtsWriter_close((ttLibC_MpegtsWriter **)writer);
		return;
//	case containerType_riff:
//	case containerType_wav:
	default:
		ERR_PRINT("unknown container type for writer close.:%d", target->type);
		break;
	}
}
