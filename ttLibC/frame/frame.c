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
#include "../log.h"

/*
 * make clone frame.
 * always make copy buffer on it.
 * @param prev_frame reuse frame object.
 * @param src_frame  source of clone.
 */
ttLibC_Frame *ttLibC_Frame_clone(
		ttLibC_Frame *prev_frame,
		ttLibC_Frame *src_frame) {
	if(src_frame == NULL) {
		return NULL;
	}
	switch(src_frame->type) {
	case frameType_bgr:
	case frameType_flv1:
	case frameType_h264:
	case frameType_h265:
	case frameType_jpeg:
	case frameType_theora:
	case frameType_vp6:
	case frameType_vp8:
	case frameType_vp9:
	case frameType_wmv1:
	case frameType_wmv2:
	case frameType_yuv420:
		return (ttLibC_Frame *)ttLibC_Video_clone(
				(ttLibC_Video *)prev_frame,
				(ttLibC_Video *)src_frame);
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
		return (ttLibC_Frame *)ttLibC_Audio_clone(
				(ttLibC_Audio *)prev_frame,
				(ttLibC_Audio *)src_frame);
	default:
		ERR_PRINT("unknown frame type:%d", src_frame->type);
		break;
	}
	return NULL;
}

/**
 * check frame is audio frame.
 * @param frame
 * @return true:audio false:not audio
 */
bool ttLibC_Frame_isAudio(ttLibC_Frame *frame) {
	if(frame == NULL) {
		return false;
	}
	switch(frame->type) {
	case frameType_bgr:
	case frameType_flv1:
	case frameType_h264:
	case frameType_h265:
	case frameType_jpeg:
	case frameType_theora:
	case frameType_vp6:
	case frameType_vp8:
	case frameType_vp9:
	case frameType_wmv1:
	case frameType_wmv2:
	case frameType_yuv420:
		return false;
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
		ERR_PRINT("unknown frame type:%d", frame->type);
		break;
	}
	return false;
}

/**
 * check frame is video frame.
 * @param frame
 * @return true:video false:not video
 */
bool ttLibC_Frame_isVideo(ttLibC_Frame *frame) {
	if(frame == NULL) {
		return false;
	}
	switch(frame->type) {
	case frameType_bgr:
	case frameType_flv1:
	case frameType_h264:
	case frameType_h265:
	case frameType_jpeg:
	case frameType_theora:
	case frameType_vp6:
	case frameType_vp8:
	case frameType_vp9:
	case frameType_wmv1:
	case frameType_wmv2:
	case frameType_yuv420:
		return true;
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
		return false;
	default:
		ERR_PRINT("unknown frame type:%d", frame->type);
		break;
	}
	return false;
}

/*
 * release frame
 * @param frame
 */
void ttLibC_Frame_close(ttLibC_Frame **frame) {
	ttLibC_Frame *target = *frame;
	if(target == NULL) {
		return;
	}
	switch(target->type) {
	case frameType_bgr:
	case frameType_flv1:
	case frameType_h264:
	case frameType_h265:
	case frameType_jpeg:
	case frameType_theora:
	case frameType_vp6:
	case frameType_vp8:
	case frameType_vp9:
	case frameType_wmv1:
	case frameType_wmv2:
	case frameType_yuv420:
		ttLibC_Video_close((ttLibC_Video **)frame);
		break;
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
		ttLibC_Audio_close((ttLibC_Audio **)frame);
		break;
	default:
		ERR_PRINT("unknown frame type:%d", target->type);
		break;
	}
}
