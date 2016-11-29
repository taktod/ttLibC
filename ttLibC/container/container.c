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
#include "mkv.h"
#include "mp3.h"
#include "mp4.h"
#include "mpegts.h"

#include "../frame/video/video.h"
#include "../frame/video/h265.h"
#include "../frame/video/h264.h"
#include "../frame/video/theora.h"
#include "../frame/audio/mp3.h"
#include "../frame/audio/speex.h"
#include "../frame/audio/vorbis.h"

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
	case containerType_mkv:
		return ttLibC_Mkv_getFrame((ttLibC_Mkv *)container, callback, ptr);
	case containerType_mp3:
		return ttLibC_Container_Mp3_getFrame((ttLibC_Container_Mp3 *)container, callback, ptr);
	case containerType_mp4:
		return ttLibC_Mp4_getFrame((ttLibC_Mp4 *)container, callback, ptr);
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
	case containerType_mkv:
		ttLibC_MkvReader_close((ttLibC_MkvReader **)reader);
		return;
	case containerType_mp3:
		ttLibC_Mp3Reader_close((ttLibC_Mp3Reader **)reader);
		return;
	case containerType_mp4:
		ttLibC_Mp4Reader_close((ttLibC_Mp4Reader **)reader);
		return;
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
	writer->mode     = containerWriter_normal;
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
	case containerType_mkv:
		ttLibC_MkvWriter_close((ttLibC_MkvWriter **)writer);
		return;
	case containerType_mp3:
		ttLibC_Mp3Writer_close((ttLibC_Mp3Writer **)writer);
		return;
	case containerType_mp4:
		ttLibC_Mp4Writer_close((ttLibC_Mp4Writer **)writer);
		return;
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

ttLibC_ContainerWriter_WriteTrack *ttLibC_ContainerWriteTrack_make(
		size_t            track_size,
		uint32_t          track_id,
		ttLibC_Frame_Type frame_type) {
	ttLibC_ContainerWriter_WriteTrack *track = ttLibC_malloc(track_size);
	track->frame_queue     = ttLibC_FrameQueue_make(track_id, 255);
	track->h26x_configData = NULL;
	track->frame_type      = frame_type;
	track->counter         = 0;
	track->is_appending    = false;
	track->enable_mode     = containerWriter_normal;
	track->use_mode        = containerWriter_normal;
	return track;
}

static bool ContainerWriteTrack_appendQueue(
		ttLibC_ContainerWriter_WriteTrack *track,
		ttLibC_Frame *frame,
		uint64_t pts,
		uint32_t timebase) {
	uint64_t original_pts      = frame->pts;
	uint32_t original_timebase = frame->timebase;
	frame->pts      = pts;
	frame->timebase = timebase;
	bool result     = ttLibC_FrameQueue_queue(track->frame_queue, frame);
	frame->pts      = original_pts;
	frame->timebase = original_timebase;
	return result;
}

bool ttLibC_ContainerWriteTrack_appendQueue(
		ttLibC_ContainerWriter_WriteTrack *track,
		ttLibC_Frame                      *frame,
		uint32_t                           timebase,
		ttLibC_ContainerWriter_Mode        enable_mode) {
	if(track == NULL) {
		ERR_PRINT("failed to get correspond track. %d", frame->id);
		return false;
	}
	uint64_t pts = (uint64_t)(1.0 * frame->pts * timebase / frame->timebase);
	if(frame->timebase == timebase) {
		pts = frame->pts;
	}
	track->enable_mode = enable_mode;
	switch(frame->type) {
	case frameType_h264:
		{
			ttLibC_H264 *h264 = (ttLibC_H264 *)frame;
			if(h264->type == H264Type_unknown) {
				return true;
			}
			if(h264->type == H264Type_configData) {
				ttLibC_H264 *h = ttLibC_H264_clone(
						(ttLibC_H264 *)track->h26x_configData,
						h264);
				if(h == NULL) {
					ERR_PRINT("failed to make clone data.");
					return false;
				}
				h->inherit_super.inherit_super.pts = 0;
				h->inherit_super.inherit_super.timebase = timebase;
				track->h26x_configData = (ttLibC_Frame *)h;
				return true;
			}
			if(!track->is_appending && h264->type != H264Type_sliceIDR) {
				return true;
			}
		}
		break;
	case frameType_h265:
		{
			ttLibC_H265 *h265 = (ttLibC_H265 *)frame;
			if(h265->type == H265Type_unknown) {
				return true;
			}
			if(h265->type == H265Type_configData) {
				ttLibC_H265 *h = ttLibC_H265_clone(
						(ttLibC_H265 *)track->h26x_configData,
						h265);
				if(h == NULL) {
					ERR_PRINT("failed to make clone data.");
					return false;
				}
				h->inherit_super.inherit_super.pts = 0;
				h->inherit_super.inherit_super.timebase = timebase;
				track->h26x_configData = (ttLibC_Frame *)h;
				return true;
			}
			if(!track->is_appending && h265->type != H265Type_sliceIDR) {
				return true;
			}
		}
		break;
	case frameType_theora:
		{
			ttLibC_Theora *theora = (ttLibC_Theora *)frame;
			if(!track->is_appending) {
				switch(track->counter) {
				case 0:
					{
						if(theora->type != TheoraType_identificationHeaderDecodeFrame) {
							return true;
						}
						++ track->counter;
					}
					break;
				case 1:
					{
						if(theora->type != TheoraType_commentHeaderFrame) {
							return true;
						}
						++ track->counter;
					}
					break;
				case 2:
					{
						if(theora->type != TheoraType_setupHeaderFrame) {
							return true;
						}
						++ track->counter;
					}
					break;
				default:
					break;
				}
				if(track->counter < 3) {
					return ContainerWriteTrack_appendQueue(track, frame, 0, timebase);
				}
			}
		}
		break;
	case frameType_vp8:
	case frameType_vp9:
		{
			ttLibC_Video *video = (ttLibC_Video *)frame;
			if(!track->is_appending && video->type != videoType_key) {
				return true;
			}
		}
		break;
	case frameType_mp3:
		{
			ttLibC_Mp3 *mp3 = (ttLibC_Mp3 *)frame;
			switch(mp3->type) {
			case Mp3Type_frame:
				break;
			case Mp3Type_id3:
			case Mp3Type_tag:
				return true;
			default:
				ERR_PRINT("unexpected mp3 frame.:%d", mp3->type);
				return true;
			}
		}
		break;
	case frameType_speex:
		{
			ttLibC_Speex *speex = (ttLibC_Speex *)frame;
			switch(speex->type) {
			case SpeexType_header:
				{
					if(track->is_appending) {
						ERR_PRINT("find speex header frame after track generating.");
						return true;
					}
				}
				break;
			case SpeexType_comment:
				return true;
			default:
				break;
			}
		}
		break;
	case frameType_vorbis:
		{
			ttLibC_Vorbis *vorbis = (ttLibC_Vorbis *)frame;
			if(!track->is_appending) {
				switch(track->counter) {
				case 0:
					if(vorbis->type != VorbisType_identification) {
						return true;
					}
					++ track->counter;
					break;
				case 1:
					if(vorbis->type != VorbisType_comment) {
						return true;
					}
					++ track->counter;
					break;
				case 2:
					if(vorbis->type != VorbisType_setup) {
						return true;
					}
					++ track->counter;
					break;
				default:
					break;
				}
				if(track->counter < 3) {
					return ContainerWriteTrack_appendQueue(track, frame, 0, timebase);
				}
			}
		}
		break;
	default:
		break;
	}
	track->is_appending = true;
	if(!ContainerWriteTrack_appendQueue(track, frame, pts, timebase)) {
		return false;
	}
	return true;
	return true;
}

void ttLibC_ContainerWriteTrack_close(ttLibC_ContainerWriter_WriteTrack **track) {
	ttLibC_ContainerWriter_WriteTrack *target = (ttLibC_ContainerWriter_WriteTrack *)*track;
	if(target == NULL) {
		return;
	}
	ttLibC_FrameQueue_close(&target->frame_queue);
	ttLibC_Frame_close(&target->h26x_configData);
	ttLibC_free(target);
	*track = NULL;
}


