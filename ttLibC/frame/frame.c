/*
 * @file   frame.c
 * @brief  frame information.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/18
 */

#include <stdio.h>

#include "frame.h"
#include "audio/audio.h"
#include "video/video.h"
#include "../_log.h"

/*
 * check frame type is audio frame.
 * @param type
 * @return true:audio false:not audio
 */
bool TT_ATTRIBUTE_API ttLibC_isAudio(ttLibC_Frame_Type type) {
	switch(type) {
	case frameType_aac:
	case frameType_adpcm_ima_wav:
	case frameType_mp3:
	case frameType_nellymoser:
	case frameType_opus:
	case frameType_pcm_alaw:
	case frameType_pcmF32:
	case frameType_pcm_mulaw:
	case frameType_pcmS16:
	case frameType_speex:
	case frameType_vorbis:
		return true;
	default:
		return false;
	}
}

/*
 * check frame type is video frame.
 * @param type
 * @return true:video false:not video
 */
bool TT_ATTRIBUTE_API ttLibC_isVideo(ttLibC_Frame_Type type) {
	switch(type) {
	case frameType_bgr:
	case frameType_flv1:
	case frameType_h264:
	case frameType_h265:
	case frameType_jpeg:
	case frameType_png:
	case frameType_theora:
	case frameType_vp6:
	case frameType_vp8:
	case frameType_vp9:
	case frameType_wmv1:
	case frameType_wmv2:
	case frameType_yuv420:
		return true;
	default:
		return false;
	}
}

/*
 * make clone frame.
 * always make copy buffer on it.
 * @param prev_frame reuse frame object.
 * @param src_frame  source of clone.
 */
ttLibC_Frame TT_ATTRIBUTE_API *ttLibC_Frame_clone(
		ttLibC_Frame *prev_frame,
		ttLibC_Frame *src_frame) {
	if(ttLibC_Frame_isAudio(src_frame)) {
		return (ttLibC_Frame *)ttLibC_Audio_clone(
				(ttLibC_Audio *)prev_frame,
				(ttLibC_Audio *)src_frame);
	}
	if(ttLibC_Frame_isVideo(src_frame)) {
		return (ttLibC_Frame *)ttLibC_Video_clone(
				(ttLibC_Video *)prev_frame,
				(ttLibC_Video *)src_frame);
	}
	ERR_PRINT("unknown frame type:%d", src_frame->type);
	return NULL;
}

/**
 * check frame is audio frame.
 * @param frame
 * @return true:audio false:not audio
 */
bool TT_ATTRIBUTE_API ttLibC_Frame_isAudio(ttLibC_Frame *frame) {
	if(frame == NULL) {
		return false;
	}
	return ttLibC_isAudio(frame->type);
}

/**
 * check frame is video frame.
 * @param frame
 * @return true:video false:not video
 */
bool TT_ATTRIBUTE_API ttLibC_Frame_isVideo(ttLibC_Frame *frame) {
	if(frame == NULL) {
		return false;
	}
	return ttLibC_isVideo(frame->type);
}

/*
 * release frame
 * @param frame
 */
void TT_ATTRIBUTE_API ttLibC_Frame_close(ttLibC_Frame **frame) {
	ttLibC_Frame *target = *frame;
	if(target == NULL) {
		return;
	}
	if(ttLibC_Frame_isAudio(target)) {
		ttLibC_Audio_close((ttLibC_Audio **)frame);
	}
	else if(ttLibC_Frame_isVideo(target)) {
		ttLibC_Video_close((ttLibC_Video **)frame);
	}
	else {
		ERR_PRINT("unknown frame type:%d", target->type);
	}
}
