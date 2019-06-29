/*
 * soundtouchResampler.cpp
 *
 *  Created on: 2016/12/19
 *      Author: taktod
 */

#ifdef __ENABLE_SOUNDTOUCH__

#include "soundtouchResampler.h"
#include "../ttLibC_predef.h"
#include "../allocator.h"
#include "../_log.h"
#include "audioResampler.h"

#include <stdio.h>
#include <soundtouch/SoundTouch.h>

using namespace soundtouch;

typedef struct ttLibC_Resampler_Soundtouch_ {
	ttLibC_Soundtouch inherit_super;
	SoundTouch *soundtouch;
	ttLibC_PcmS16 *bases16;
	ttLibC_PcmF32 *basef32;
	ttLibC_PcmS16 *results16;
	ttLibC_PcmF32 *resultf32;
	uint32_t sample_rate;
	uint32_t channel_num;
	ttLibC_Frame_Type target_type;
} ttLibC_Resampler_Soundtouch_;

typedef ttLibC_Resampler_Soundtouch_ ttLibC_Soundtouch_;

extern "C" {

ttLibC_Soundtouch TT_VISIBILITY_DEFAULT *ttLibC_Soundtouch_make(
		uint32_t sample_rate,
		uint32_t channel_num) {
	ttLibC_Soundtouch_ *soundtouch = (ttLibC_Soundtouch_ *)ttLibC_malloc(sizeof(ttLibC_Soundtouch_));
	if(soundtouch == NULL) {
		return NULL;
	}
	soundtouch->bases16 = NULL;
	soundtouch->basef32 = NULL;
	soundtouch->results16 = NULL;
	soundtouch->resultf32 = NULL;
	soundtouch->soundtouch = new SoundTouch();
	soundtouch->soundtouch->setSampleRate(sample_rate);
	soundtouch->soundtouch->setChannels(channel_num);
	soundtouch->sample_rate = sample_rate;
	soundtouch->channel_num = channel_num;
	soundtouch->inherit_super.sample_rate = sample_rate;
	soundtouch->inherit_super.channel_num = channel_num;
	soundtouch->target_type = frameType_pcmS16;
	return (ttLibC_Soundtouch *)soundtouch;
}

void TT_VISIBILITY_DEFAULT ttLibC_Soundtouch_setRate(
		ttLibC_Soundtouch *soundtouch,
		double newRate) {
	ttLibC_Soundtouch_ *soundtouch_ = (ttLibC_Soundtouch_ *)soundtouch;
	if(soundtouch_ == NULL) {
		return;
	}
#if SOUNDTOUCH_VERSION_ID < 10901
	soundtouch_->soundtouch->setRate((float)newRate);
#else
	soundtouch_->soundtouch->setRate(newRate);
#endif
}
void TT_VISIBILITY_DEFAULT ttLibC_Soundtouch_setTempo(
		ttLibC_Soundtouch *soundtouch,
		double newTempo) {
	ttLibC_Soundtouch_ *soundtouch_ = (ttLibC_Soundtouch_ *)soundtouch;
	if(soundtouch_ == NULL) {
		return;
	}
#if SOUNDTOUCH_VERSION_ID < 10901
	soundtouch_->soundtouch->setTempo((float)newTempo);
#else
	soundtouch_->soundtouch->setTempo(newTempo);
#endif
}
void TT_VISIBILITY_DEFAULT ttLibC_Soundtouch_setRateChange(
		ttLibC_Soundtouch *soundtouch,
		double newRate) {
	ttLibC_Soundtouch_ *soundtouch_ = (ttLibC_Soundtouch_ *)soundtouch;
	if(soundtouch_ == NULL) {
		return;
	}
#if SOUNDTOUCH_VERSION_ID < 10901
	soundtouch_->soundtouch->setRateChange((float)newRate);
#else
	soundtouch_->soundtouch->setRateChange(newRate);
#endif
}
void TT_VISIBILITY_DEFAULT ttLibC_Soundtouch_setTempoChange(
		ttLibC_Soundtouch *soundtouch,
		double newTempo) {
	ttLibC_Soundtouch_ *soundtouch_ = (ttLibC_Soundtouch_ *)soundtouch;
	if(soundtouch_ == NULL) {
		return;
	}
#if SOUNDTOUCH_VERSION_ID < 10901
	soundtouch_->soundtouch->setTempoChange((float)newTempo);
#else
	soundtouch_->soundtouch->setTempoChange(newTempo);
#endif
}
void TT_VISIBILITY_DEFAULT ttLibC_Soundtouch_setPitch(
		ttLibC_Soundtouch *soundtouch,
		double newPitch) {
	ttLibC_Soundtouch_ *soundtouch_ = (ttLibC_Soundtouch_ *)soundtouch;
	if(soundtouch_ == NULL) {
		return;
	}
#if SOUNDTOUCH_VERSION_ID < 10901
	soundtouch_->soundtouch->setPitch((float)newPitch);
#else
	soundtouch_->soundtouch->setPitch(newPitch);
#endif
}
void TT_VISIBILITY_DEFAULT ttLibC_Soundtouch_setPitchOctaves(
		ttLibC_Soundtouch *soundtouch,
		double newPitch) {
	ttLibC_Soundtouch_ *soundtouch_ = (ttLibC_Soundtouch_ *)soundtouch;
	if(soundtouch_ == NULL) {
		return;
	}
#if SOUNDTOUCH_VERSION_ID < 10901
	soundtouch_->soundtouch->setPitchOctaves((float)newPitch);
#else
	soundtouch_->soundtouch->setPitchOctaves(newPitch);
#endif
}
void TT_VISIBILITY_DEFAULT ttLibC_Soundtouch_setPitchSemiTones(
		ttLibC_Soundtouch *soundtouch,
		double newPitch) {
	ttLibC_Soundtouch_ *soundtouch_ = (ttLibC_Soundtouch_ *)soundtouch;
	if(soundtouch_ == NULL) {
		return;
	}
#if SOUNDTOUCH_VERSION_ID < 10901
	soundtouch_->soundtouch->setPitchSemiTones((float)newPitch);
#else
	soundtouch_->soundtouch->setPitchSemiTones(newPitch);
#endif
}

bool TT_VISIBILITY_DEFAULT ttLibC_Soundtouch_resample(
		ttLibC_Soundtouch *soundtouch,
		ttLibC_Audio *pcm,
		ttLibC_SoundtouchResampleFunc callback,
		void *ptr) {
	ttLibC_Soundtouch_ *soundtouch_ = (ttLibC_Soundtouch_ *)soundtouch;
	if(soundtouch_ == NULL) {
		ERR_PRINT("soundtouch object is null.");
		return false;
	}
	if(pcm == NULL) {
		// do flush all data.
		soundtouch_->soundtouch->flush();
	}
	else {
		switch(pcm->inherit_super.type) {
		case frameType_pcmS16:
		case frameType_pcmF32:
			break;
		default:
			ERR_PRINT("work with pcm only.");
			return false;
		}
		soundtouch_->target_type = pcm->inherit_super.type;
#ifdef SOUNDTOUCH_INTEGER_SAMPLES
		// soundtouch work with int16.
		// TODO checked this.
		ttLibC_PcmS16 *source = NULL;
		if(pcm->inherit_super.type == frameType_pcmS16) {
			ttLibC_PcmS16 *f = (ttLibC_PcmS16 *)pcm;
			if(f->type == PcmS16Type_littleEndian) {
				source = f;
			}
		}
		if(source == NULL) {
			ttLibC_PcmS16 *s = (ttLibC_PcmS16 *)ttLibC_AudioResampler_convertFormat((ttLibC_Audio *)soundtouch_->bases16, frameType_pcmS16, (uint32_t)PcmS16Type_littleEndian, pcm->channel_num, pcm);
			if(s == NULL) {
				// failed to resample.
				return false;
			}
			soundtouch_->bases16 = s;
			source = s;
		}
		soundtouch_->soundtouch->putSamples((const int16_t *)source->l_data, source->inherit_super.sample_num);
#else
		// soundtouch work with float
		ttLibC_PcmF32 *source = NULL;
		if(pcm->inherit_super.type == frameType_pcmF32) {
			ttLibC_PcmF32 *f = (ttLibC_PcmF32 *)pcm;
			if(f->type == PcmF32Type_interleave) {
				source = f;
			}
		}
		if(source == NULL) {
			ttLibC_PcmF32 *f = (ttLibC_PcmF32 *)ttLibC_AudioResampler_convertFormat((ttLibC_Audio *)soundtouch_->basef32, frameType_pcmF32, (uint32_t)PcmF32Type_interleave, pcm->channel_num, pcm);
			if(f == NULL) {
				// failed to resample.
				return false;
			}
			soundtouch_->basef32 = f;
			source = f;
		}
		soundtouch_->soundtouch->putSamples((const float *)source->l_data, source->inherit_super.sample_num);
#endif
	}
	int nSamples = 0;
#ifdef SOUNDTOUCH_INTEGER_SAMPLES
	do {
		int16_t resultBuf[1024];
		nSamples = soundtouch_->soundtouch->receiveSamples(resultBuf, 1024 / soundtouch_->
				channel_num);
		if(nSamples == 0) {
			// no more data.
			break;
		}
		ttLibC_PcmS16 *s = ttLibC_PcmS16_make(
				soundtouch_->results16,
				PcmS16Type_littleEndian,
				soundtouch_->sample_rate,
				nSamples,
				soundtouch_->channel_num,
				resultBuf,
				soundtouch_->channel_num * nSamples * 2,
				resultBuf,
				soundtouch_->channel_num * nSamples * 2,
				NULL,
				0,
				true,
				0, // not do sanything for pts. do on application.
				soundtouch_->sample_rate);
		if(s == NULL) {
			// failed to make pcm object.
			return false;
		}
		soundtouch_->results16 = s;
		// if origial is pcmf32
		if(soundtouch_->target_type == frameType_pcmF32) {
			// result should be pcmf32.
			ttLibC_PcmF32 *f = ttLibC_AudioResampler_makePcmF32FromPcmS16(
					soundtouch_->resultf32,
					PcmF32Type_interleave,
					soundtouch_->results16);
			if(f == NULL) {
				// failed to make pcmf32 object.
				return false;
			}
			soundtouch_->resultf32 = f;
			soundtouch_->resultf32->inherit_super.inherit_super.id = pcm->inherit_super.id;
			if(callback != NULL) {
				callback(ptr, (ttLibC_Audio *)soundtouch_->resultf32);
			}
		}
		else {
			soundtouch_->results16->inherit_super.inherit_super.id = pcm->inherit_super.id;
			if(callback != NULL) {
				callback(ptr, (ttLibC_Audio *)soundtouch_->results16);
			}
		}
	} while(true);
#else
	do {
		float resultBuf[1024];
		nSamples = soundtouch_->soundtouch->receiveSamples(resultBuf, 1024 / soundtouch_->channel_num);
		if(nSamples == 0) {
			// no more data.
			break;
		}
		ttLibC_PcmF32 *f = ttLibC_PcmF32_make(
				soundtouch_->resultf32,
				PcmF32Type_interleave,
				soundtouch_->sample_rate,
				nSamples,
				soundtouch_->channel_num,
				resultBuf,
				soundtouch_->channel_num * nSamples * 4,
				resultBuf,
				soundtouch_->channel_num * nSamples * 4,
				NULL,
				0,
				true,
				0, // not do sanything for pts. do on application.
				soundtouch_->sample_rate);
		if(f == NULL) {
			// failed to make pcm object.
			return false;
		}
		soundtouch_->resultf32 = f;
		// if original is pcms16
		if(soundtouch_->target_type == frameType_pcmS16) {
			// result should be pcms16
			ttLibC_PcmS16 *s = ttLibC_AudioResampler_makePcmS16FromPcmF32(
					soundtouch_->results16,
					PcmS16Type_littleEndian,
					soundtouch_->resultf32);
			if(s == NULL) {
				// failed to make pcms16 object.
				return false;
			}
			soundtouch_->results16 = s;
			soundtouch_->results16->inherit_super.inherit_super.id = pcm->inherit_super.id;
			if(callback != NULL) {
				callback(ptr, (ttLibC_Audio *)soundtouch_->results16);
			}
		}
		else {
			soundtouch_->resultf32->inherit_super.inherit_super.id = pcm->inherit_super.id;
			if(callback != NULL) {
				callback(ptr, (ttLibC_Audio *)soundtouch_->resultf32);
			}
		}
	} while(true);
#endif
	return true;
}

void TT_VISIBILITY_DEFAULT ttLibC_Soundtouch_close(ttLibC_Soundtouch **soundtouch) {
	ttLibC_Soundtouch_ *target = (ttLibC_Soundtouch_ *)*soundtouch;
	if(target == NULL) {
		return;
	}
	delete target->soundtouch;
	ttLibC_PcmS16_close(&target->bases16);
	ttLibC_PcmS16_close(&target->results16);
	ttLibC_PcmF32_close(&target->basef32);
	ttLibC_PcmF32_close(&target->resultf32);
	ttLibC_free(target);
	*soundtouch = NULL;
}

}

#endif
