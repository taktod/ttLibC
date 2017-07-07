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
#include "../frame/audio/audio.h"
#include "../frame/audio/aac.h"
#include "../frame/audio/mp3.h"
#include "../frame/audio/speex.h"
#include "../frame/audio/vorbis.h"

#include "../ttLibC_predef.h"
#include "../_log.h"
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
ttLibC_Container TT_VISIBILITY_HIDDEN *ttLibC_Container_make(
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
	case containerType_webm:
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
bool TT_VISIBILITY_DEFAULT ttLibC_Container_getFrame(
		ttLibC_Container *container,
		ttLibC_getFrameFunc callback,
		void *ptr) {
	switch(container->type) {
	case containerType_flv:
		return ttLibC_Flv_getFrame((ttLibC_Flv *)container, callback, ptr);
	case containerType_mkv:
	case containerType_webm:
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
ttLibC_ContainerReader TT_VISIBILITY_HIDDEN *ttLibC_ContainerReader_make(
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
	memset(reader, 0, reader_size);
	reader->type = container_type;
	return reader;
}

bool TT_VISIBILITY_DEFAULT ttLibC_ContainerReader_read(
		ttLibC_ContainerReader *reader,
		void *data,
		size_t data_size,
		ttLibC_ContainerReadFunc callback,
		void *ptr) {
	(void)reader;
	(void)data;
	(void)data_size;
	(void)callback;
	(void)ptr;
	// TODO make this.
	return true;
}

/*
 * close container reader
 */
void TT_VISIBILITY_DEFAULT ttLibC_ContainerReader_close(ttLibC_ContainerReader **reader) {
	ttLibC_ContainerReader *target = *reader;
	if(target == NULL) {
		return;
	}
	switch(target->type) {
	case containerType_flv:
		ttLibC_FlvReader_close((ttLibC_FlvReader **)reader);
		return;
	case containerType_mkv:
	case containerType_webm:
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
ttLibC_ContainerWriter TT_VISIBILITY_HIDDEN *ttLibC_ContainerWriter_make(
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
	memset(writer, 0, writer_size);
	writer->type     = container_type;
	writer->timebase = timebase;
	writer->pts      = 0;
	writer->mode     = containerWriter_normal;
	return writer;
}

ttLibC_ContainerWriter TT_VISIBILITY_HIDDEN *ttLibC_ContainerWriter_make_(
		ttLibC_Container_Type container_type,
		size_t                writer_size,
		uint32_t              timebase,
		size_t                track_size,
		uint32_t              track_base_id,
		ttLibC_Frame_Type    *target_frame_types,
		uint32_t              types_num,
		uint32_t              unit_duration) {
	ttLibC_ContainerWriter_ *writer = (ttLibC_ContainerWriter_ *)ttLibC_ContainerWriter_make(
			container_type,
			writer_size,
			timebase);
	if(writer == NULL) {
		return NULL;
	}
	if(track_size == 0) {
		writer->track_list = NULL;
	}
	else {
		writer->track_list = ttLibC_StlMap_make();
		for(uint32_t i = 0;i < types_num;++ i) {
			ttLibC_ContainerWriter_WriteTrack *track = ttLibC_ContainerWriteTrack_make(
					track_size,
					i + track_base_id,
					target_frame_types[i]);
			if(track == NULL) {
				ERR_PRINT("failed to make track object.");
			}
			else {
				ttLibC_StlMap_put(writer->track_list, (void *)(long)(i + track_base_id), (void *)track);
			}
		}
	}
	writer->callback          = NULL;
	writer->ptr               = NULL;
	writer->is_first          = true;
	writer->status            = status_init_check;
	writer->current_pts_pos   = 0;
	writer->target_pos        = 0;
	writer->unit_duration = unit_duration;
	return (ttLibC_ContainerWriter *)writer;
}

static bool ContainerWriter_closeTracks(void *ptr, void *key, void *item) {
	(void)ptr;
	(void)key;
	if(item != NULL) {
		ttLibC_ContainerWriter_WriteTrack *track = (ttLibC_ContainerWriter_WriteTrack *)item;
		ttLibC_ContainerWriteTrack_close((ttLibC_ContainerWriter_WriteTrack **)&track);
	}
	return true;
}

void TT_VISIBILITY_HIDDEN ttLibC_ContainerWriter_close_(ttLibC_ContainerWriter_ **writer) {
	ttLibC_ContainerWriter_ *target = (ttLibC_ContainerWriter_ *)*writer;
	if(target == NULL) {
		return;
	}
	// in the case of track have extra memory, not use this function.
	ttLibC_StlMap_forEach(target->track_list, ContainerWriter_closeTracks, NULL);
	ttLibC_StlMap_close(&target->track_list);
	ttLibC_free(target);
	*writer = NULL;
}

bool TT_VISIBILITY_DEFAULT ttLibC_ContainerWriter_write(
		ttLibC_ContainerWriter *writer,
		ttLibC_Frame *frame,
		ttLibC_ContainerWriteFunc callback,
		void *ptr) {
	(void)writer;
	(void)frame;
	(void)callback;
	(void)ptr;
	return true;
}

/*
 * close container writer
 */
void TT_VISIBILITY_DEFAULT ttLibC_ContainerWriter_close(ttLibC_ContainerWriter **writer) {
	ttLibC_ContainerWriter *target = *writer;
	if(target == NULL) {
		return;
	}
	switch(target->type) {
	case containerType_flv:
		ttLibC_FlvWriter_close((ttLibC_FlvWriter **)writer);
		return;
	case containerType_mkv:
	case containerType_webm:
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

ttLibC_ContainerWriter_WriteTrack TT_VISIBILITY_HIDDEN *ttLibC_ContainerWriteTrack_make(
		size_t            track_size,
		uint32_t          track_id,
		ttLibC_Frame_Type frame_type) {
	ttLibC_ContainerWriter_WriteTrack *track = ttLibC_malloc(track_size);
	if(track == NULL) {
		return NULL;
	}
	memset(track, 0, track_size);
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
		uint64_t dts,
		uint32_t timebase) {
	uint64_t original_pts      = frame->pts;
	uint64_t original_dts      = frame->dts;
	uint32_t original_timebase = frame->timebase;
	frame->pts      = pts;
	frame->dts      = dts;
	frame->timebase = timebase;
	bool result     = ttLibC_FrameQueue_queue(track->frame_queue, frame);
	frame->pts      = original_pts;
	frame->dts      = original_dts;
	frame->timebase = original_timebase;
	return result;
}

static bool ContainerWriter_isReadyToStartCallback(void *ptr, void *key, void *item) {
	(void)ptr;
	(void)key;
	if(item == NULL) {
		return false;
	}
	ttLibC_ContainerWriter_WriteTrack *track = (ttLibC_ContainerWriter_WriteTrack *)item;
	switch(track->frame_type) {
	case frameType_h264:
	case frameType_h265:
		{
			return track->h26x_configData != NULL;
		}
	default:
		{
			return track->is_appending;
		}
	}
}

bool TT_VISIBILITY_HIDDEN ttLibC_ContainerWriter_isReadyToStart(ttLibC_ContainerWriter_ *writer) {
	if(ttLibC_StlMap_forEach(writer->track_list, ContainerWriter_isReadyToStartCallback, writer)) {
		return true;
	}
	return false;
}

bool TT_VISIBILITY_HIDDEN ttLibC_ContainerWriter_primaryTrackCheck(void *ptr, ttLibC_Frame *frame) {
	ttLibC_ContainerWriter_ *writer = (ttLibC_ContainerWriter_ *)ptr;
	ttLibC_ContainerWriter_WriteTrack *track = (ttLibC_ContainerWriter_WriteTrack *)ttLibC_StlMap_get(writer->track_list, (void *)(long)frame->id);
	if(ttLibC_Frame_isAudio(frame)) {
		if(frame->dts < writer->unit_duration + writer->current_pts_pos) {
			return true;
		}
		writer->target_pos = frame->pts;
		return false;
	}
	else if(ttLibC_Frame_isVideo(frame)) {
		if(frame->pts != 0 && frame->dts == 0) {
			// dts is 0, frame is not ready.
			return true;
		}
		ttLibC_Video *video = (ttLibC_Video *)frame;
		if(video->type == videoType_key) {
			if((track->use_mode & containerWriter_allKeyFrame_split) != 0) {
				if(writer->current_pts_pos < frame->dts) {
					writer->target_pos = frame->dts;
					return false;
				}
			}
		}
		if(frame->dts < writer->unit_duration + writer->current_pts_pos) {
			return true;
		}
		switch(frame->type) {
		case frameType_h264:
			{
				ttLibC_H264 *h264 = (ttLibC_H264 *)frame;
				switch(h264->frame_type) {
				case H264FrameType_B:
					if((track->use_mode & containerWriter_bFrame_split) != 0) {
					}
					else if((track->use_mode & containerWriter_disposableBFrame_split) != 0 && h264->is_disposable) {
					}
					else {
						return true;
					}
					break;
				case H264FrameType_SP:
					ERR_PRINT("sp frame is not checked.");
					/* no break */
				case H264FrameType_P:
					if((track->use_mode & containerWriter_pFrame_split) != 0) {
					}
					else if((track->use_mode & containerWriter_innerFrame_split) != 0 && frame->dts == frame->pts) {
					}
					else {
						return true;
					}
					break;
				case H264FrameType_SI:
					ERR_PRINT("si frame is not checked.");
					/* no break */
				case H264FrameType_I:
					break;
				default:
					return true;
				}
				writer->target_pos = frame->dts;
				return false;
			}
			break;
		case frameType_h265:
			{
				ttLibC_H265 *h265 = (ttLibC_H265 *)frame;
				switch(h265->frame_type) {
				case H265FrameType_B:
					if((track->use_mode & containerWriter_bFrame_split) != 0) {
					}
					else if((track->use_mode & containerWriter_disposableBFrame_split) != 0 && h265->is_disposable) {
					}
					else {
						return true;
					}
					break;
				case H265FrameType_P:
					if((track->use_mode & containerWriter_pFrame_split) != 0) {
					}
					else if((track->use_mode & containerWriter_innerFrame_split) != 0 && frame->dts == frame->pts) {
					}
					else {
						return true;
					}
					break;
				case H265FrameType_I:
					break;
				default:
					return true;
				}
				writer->target_pos = frame->dts;
				return false;
			}
			break;
		default:
			{
				switch(video->type) {
				default:
				case videoType_info:
					return true;
				case videoType_key:
					break;
				case videoType_inner:
					if((track->use_mode & containerWriter_innerFrame_split) == 0) {
					}
					else if((track->use_mode & containerWriter_pFrame_split) == 0) {
					}
					else if((track->use_mode & containerWriter_disposableBFrame_split) == 0) {
					}
					else if((track->use_mode & containerWriter_bFrame_split) == 0) {
					}
					else {
						return true;
					}
					break;
				}
				writer->target_pos = frame->pts;
				return false;
			}
			break;
		}
	}
	return true;
}

static bool ContainerWriter_isReadyToWriteCallback(void *ptr, void *key, void *item) {
	(void)key;
	if(ptr != NULL && item != NULL) {
		ttLibC_ContainerWriter_ *writer = (ttLibC_ContainerWriter_ *)ptr;
		ttLibC_ContainerWriter_WriteTrack *track = (ttLibC_ContainerWriter_WriteTrack *)item;
		uint64_t pts = track->frame_queue->pts;
		if(writer->inherit_super.timebase != track->frame_queue->timebase) {
			pts = (uint64_t)(1.0 * track->frame_queue->pts * writer->inherit_super.timebase / track->frame_queue->timebase);
		}
		if(writer->target_pos > pts) {
			return false;
		}
		return true;
	}
	return false;
}
bool TT_VISIBILITY_HIDDEN ttLibC_ContainerWriter_isReadyToWrite(ttLibC_ContainerWriter_ *writer) {
	if(ttLibC_StlMap_forEach(writer->track_list, ContainerWriter_isReadyToWriteCallback, writer)) {
		return true;
	}
	return false;
}

int TT_VISIBILITY_HIDDEN ttLibC_ContainerWriter_write_(
		ttLibC_ContainerWriter_ *writer,
		ttLibC_Frame *frame,
		ttLibC_ContainerWriteFunc callback,
		void *ptr) {
	if(writer == NULL) {
		ERR_PRINT("writer is null.");
		return -1;
	}
	if(frame == NULL) {
		return 0;
	}
	ttLibC_ContainerWriter_WriteTrack *track = (ttLibC_ContainerWriter_WriteTrack *)ttLibC_StlMap_get(writer->track_list, (void *)(long)frame->id);
	if(track == NULL) {
		ERR_PRINT("failed to get correspond track. %d", frame->id);
		return -1;
	}
	uint64_t pts = (uint64_t)(1.0 * frame->pts * writer->inherit_super.timebase / frame->timebase);
	uint64_t dts = (uint64_t)(1.0 * frame->dts * writer->inherit_super.timebase / frame->timebase);
	if(frame->timebase == writer->inherit_super.timebase) {
		pts = frame->pts;
		dts = frame->dts;
	}
	track->enable_mode = writer->inherit_super.mode;
	switch(frame->type) {
	case frameType_h264:
		{
			ttLibC_H264 *h264 = (ttLibC_H264 *)frame;
			if(h264->type == H264Type_unknown) {
				return 0;
			}
			if(h264->type == H264Type_configData) {
				ttLibC_H264 *h = ttLibC_H264_clone(
						(ttLibC_H264 *)track->h26x_configData,
						h264);
				if(h == NULL) {
					ERR_PRINT("failed to make clone data.");
					return -1;
				}
				h->inherit_super.inherit_super.pts = 0;
				h->inherit_super.inherit_super.dts = 0;
				h->inherit_super.inherit_super.timebase = writer->inherit_super.timebase;
				track->h26x_configData = (ttLibC_Frame *)h;
				return 0;
			}
			if(h264->frame_type == H264FrameType_B && (writer->inherit_super.mode & containerWriter_enable_dts) == 0) {
				ERR_PRINT("detect h264Bframe in non-dts write mode.");
			}
			if(!track->is_appending && h264->type != H264Type_sliceIDR) {
				return 0;
			}
		}
		break;
	case frameType_h265:
		{
			ttLibC_H265 *h265 = (ttLibC_H265 *)frame;
			if(h265->type == H265Type_unknown) {
				return 0;
			}
			if(h265->type == H265Type_configData) {
				ttLibC_H265 *h = ttLibC_H265_clone(
						(ttLibC_H265 *)track->h26x_configData,
						h265);
				if(h == NULL) {
					ERR_PRINT("failed to make clone data.");
					return -1;
				}
				h->inherit_super.inherit_super.pts = 0;
				h->inherit_super.inherit_super.dts = 0;
				h->inherit_super.inherit_super.timebase = writer->inherit_super.timebase;
				track->h26x_configData = (ttLibC_Frame *)h;
				return 0;
			}
			if(h265->frame_type == H265FrameType_B && (writer->inherit_super.mode & containerWriter_enable_dts) == 0) {
				ERR_PRINT("detect h265Bframe in non-dts write mode.");
			}
			if(!track->is_appending && h265->type != H265Type_sliceIDR) {
				return 0;
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
							return 0;
						}
						++ track->counter;
					}
					break;
				case 1:
					{
						if(theora->type != TheoraType_commentHeaderFrame) {
							return 0;
						}
						++ track->counter;
					}
					break;
				case 2:
					{
						if(theora->type != TheoraType_setupHeaderFrame) {
							return 0;
						}
						++ track->counter;
					}
					break;
				default:
					break;
				}
				if(track->counter < 3) {
					if(!ContainerWriteTrack_appendQueue(track, frame, 0, 0, writer->inherit_super.timebase)) {
						return -1;
					}
					else {
						return 0;
					}
				}
			}
		}
		break;
	case frameType_vp8:
	case frameType_vp9:
		{
			ttLibC_Video *video = (ttLibC_Video *)frame;
			if(!track->is_appending && video->type != videoType_key) {
				return 0;
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
				return 0;
			default:
				ERR_PRINT("unexpected mp3 frame.:%d", mp3->type);
				return 0;
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
						return 0;
					}
				}
				break;
			case SpeexType_comment:
				return 0;
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
						return 0;
					}
					++ track->counter;
					break;
				case 1:
					if(vorbis->type != VorbisType_comment) {
						return 0;
					}
					++ track->counter;
					break;
				case 2:
					if(vorbis->type != VorbisType_setup) {
						return 0;
					}
					++ track->counter;
					break;
				default:
					break;
				}
				if(track->counter < 3) {
					if(!ContainerWriteTrack_appendQueue(track, frame, 0, 0, writer->inherit_super.timebase)){
						return -1;
					}
					else {
						return 0;
					}
				}
			}
		}
		break;
	default:
		break;
	}
	track->is_appending = true;
	if(!ContainerWriteTrack_appendQueue(track, frame, pts, dts, writer->inherit_super.timebase)) {
		return -1;
	}
	if(writer->is_first) {
		writer->current_pts_pos = pts;
		writer->target_pos = pts;
		writer->inherit_super.pts = pts;
		writer->is_first = false;
	}
	writer->callback = callback;
	writer->ptr = ptr;
	return 1;
}

bool TT_VISIBILITY_HIDDEN ttLibC_ContainerWriteTrack_appendQueue(
		ttLibC_ContainerWriter_WriteTrack *track,
		ttLibC_Frame                      *frame,
		uint32_t                           timebase,
		ttLibC_ContainerWriter_Mode        enable_mode) {
	if(track == NULL) {
		ERR_PRINT("failed to get correspond track. %d", frame->id);
		return false;
	}
	uint64_t pts = (uint64_t)(1.0 * frame->pts * timebase / frame->timebase);
	uint64_t dts = (uint64_t)(1.0 * frame->dts * timebase / frame->timebase);
	if(frame->timebase == timebase) {
		pts = frame->pts;
		dts = frame->dts;
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
					return ContainerWriteTrack_appendQueue(track, frame, 0, 0, timebase);
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
					return ContainerWriteTrack_appendQueue(track, frame, 0, 0, timebase);
				}
			}
		}
		break;
	default:
		break;
	}
	track->is_appending = true;
	if(!ContainerWriteTrack_appendQueue(track, frame, pts, dts, timebase)) {
		return false;
	}
	return true;
}

void TT_VISIBILITY_HIDDEN ttLibC_ContainerWriteTrack_close(ttLibC_ContainerWriter_WriteTrack **track) {
	ttLibC_ContainerWriter_WriteTrack *target = (ttLibC_ContainerWriter_WriteTrack *)*track;
	if(target == NULL) {
		return;
	}
	ttLibC_FrameQueue_close(&target->frame_queue);
	ttLibC_Frame_close(&target->h26x_configData);
	ttLibC_free(target);
	*track = NULL;
}
