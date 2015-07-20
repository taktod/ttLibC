/*
 * @file   audio.c
 * @brief  audio frame information.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/19
 */

#include "audio.h"
#include "pcms16.h"
#include "../../log.h"

/*
 * close frame
 * @param frame
 */
void ttLibC_Audio_close(ttLibC_Audio **frame) {
	ttLibC_Audio *target = *frame;
	if(target == NULL) {
		return;
	}
	switch(target->inherit_super.type) {
	case frameType_pcmS16:
		ttLibC_PcmS16_close((ttLibC_PcmS16 **)frame);
		break;
	default:
		ERR_PRINT("unknown type:%d", target->inherit_super.type);
		break;
	}
}


