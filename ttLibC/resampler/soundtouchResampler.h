/*
 * soundTouchResampler.h
 *
 *  Created on: 2016/12/19
 *      Author: taktod
 */

#ifndef TTLIBC_RESAMPLER_SOUNDTOUCHRESAMPLER_H_
#define TTLIBC_RESAMPLER_SOUNDTOUCHRESAMPLER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../ttLibC_predef.h"
#include "../frame/audio/pcms16.h"
#include "../frame/audio/pcmf32.h"

typedef struct ttLibC_Resampler_Soundtouch {
	uint32_t version;
	uint32_t sample_rate;
	uint32_t channel_num;
} ttLibC_Resampler_Soundtouch;

typedef ttLibC_Resampler_Soundtouch ttLibC_Soundtouch;

typedef bool (*ttLibC_SoundtouchResampleFunc)(void *ptr, ttLibC_Audio *pcm);

ttLibC_Soundtouch TT_ATTRIBUTE_API *ttLibC_Soundtouch_make(
		uint32_t sample_rate,
		uint32_t channel_num);

void TT_ATTRIBUTE_API ttLibC_Soundtouch_setRate(
		ttLibC_Soundtouch *soundtouch,
		double newRate);
void TT_ATTRIBUTE_API ttLibC_Soundtouch_setTempo(
		ttLibC_Soundtouch *soundtouch,
		double newTempo);
void TT_ATTRIBUTE_API ttLibC_Soundtouch_setRateChange(
		ttLibC_Soundtouch *soundtouch,
		double newRate);
void TT_ATTRIBUTE_API ttLibC_Soundtouch_setTempoChange(
		ttLibC_Soundtouch *soundtouch,
		double newTempo);
void TT_ATTRIBUTE_API ttLibC_Soundtouch_setPitch(
		ttLibC_Soundtouch *soundtouch,
		double newPitch);
void TT_ATTRIBUTE_API ttLibC_Soundtouch_setPitchOctaves(
		ttLibC_Soundtouch *soundtouch,
		double newPitch);
void TT_ATTRIBUTE_API ttLibC_Soundtouch_setPitchSemiTones(
		ttLibC_Soundtouch *soundtouch,
		double newPitch);

bool TT_ATTRIBUTE_API ttLibC_Soundtouch_resample(
		ttLibC_Soundtouch *soundtouch,
		ttLibC_Audio *pcm,
		ttLibC_SoundtouchResampleFunc callback,
		void *ptr);

void TT_ATTRIBUTE_API ttLibC_Soundtouch_close(ttLibC_Soundtouch **soundtouch);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_RESAMPLER_SOUNDTOUCHRESAMPLER_H_ */
