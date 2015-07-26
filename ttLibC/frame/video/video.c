/*
 * @file   video.c
 * @brief  video frame information.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/18
 */

#include "video.h"
#include "bgr.h"
#include "yuv420.h"
#include "h264.h"
#include "../../log.h"

/*
 * close frame
 * @param frame
 */
void ttLibC_Video_close(ttLibC_Video **frame) {
	ttLibC_Video *target = *frame;
	if(target == NULL) {
		return;
	}
	switch(target->inherit_super.type) {
	case frameType_bgr:
		ttLibC_Bgr_close((ttLibC_Bgr **)frame);
		break;
	case frameType_yuv420:
		ttLibC_Yuv420_close((ttLibC_Yuv420 **)frame);
		break;
	case frameType_h264:
		ttLibC_H264_close((ttLibC_H264 **)frame);
		break;
	default:
		ERR_PRINT("unknown type:%d", target->inherit_super.type);
		break;
	}
}

