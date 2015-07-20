/**
 * @file   beepUtil.h
 * @brief  library for beep sound.(pcms16 only.)
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/20
 */

#ifndef TTLIBC_UTIL_BEEPUTIL_H_
#define TTLIBC_UTIL_BEEPUTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../frame/audio/pcms16.h"

/**
 * data for beepGenerator
 */
typedef struct {
	/** target pcms16 type. */
	ttLibC_PcmS16_Type type;
	/** target_Hz 440 for A sound. */
	uint32_t target_Hz;
	/** sample_rate */
	uint32_t sample_rate;
	/** channel_num 1:monoral 2:stereo */
	uint32_t channel_num;
	/** beep position. */
	uint64_t pos;
} ttLibC_Util_BeepUtil_BeepGenerator;

typedef ttLibC_Util_BeepUtil_BeepGenerator ttLibC_BeepGenerator;

/**
 * make beep generator
 * @param type        pcms16 type information.
 * @param target_Hz   target Hz 440 = "A" sound.
 * @param sample_rate sample_rate for generated beep.
 * @param channel_num channel_num for generated beep. 1:monoral 2:stereo
 */
ttLibC_BeepGenerator *ttLibC_BeepGenerator_make(
		ttLibC_PcmS16_Type type,
		uint32_t target_Hz,
		uint32_t sample_rate,
		uint32_t channel_num);

/**
 * make beep sound.
 * @param generator  generator object.
 * @param prev_frame reuse frame
 * @param mili_sec   length of pcmframe in mili sec.
 */
ttLibC_PcmS16 *ttLibC_BeepGenerator_makeBeepByMiliSec(
		ttLibC_BeepGenerator *generator,
		ttLibC_PcmS16 *prev_frame,
		uint32_t mili_sec);

/**
 * make beep sound
 * @param generator  generator object.
 * @param prev_frame reuse frame.
 * @param sample_num target sample num.
 */
ttLibC_PcmS16 *ttLibC_BeepGenerator_makeBeepBySampleNum(
		ttLibC_BeepGenerator *generator,
		ttLibC_PcmS16 *prev_frame,
		uint32_t sample_num);

/**
 * close the generator object.
 */
void ttLibC_BeepGenerator_close(ttLibC_BeepGenerator **generator);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_UTIL_BEEPUTIL_H_ */
