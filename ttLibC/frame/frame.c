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
