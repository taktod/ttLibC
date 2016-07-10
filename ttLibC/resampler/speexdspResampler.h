/**
 * @file   speexdspResampler.h
 * @brief  resampler by speexdsp.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/26
 */

#ifndef TTLIBC_RESAMPLER_SPEEXDSPRESAMPLER_H_
#define TTLIBC_RESAMPLER_SPEEXDSPRESAMPLER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../frame/audio/pcms16.h"
#include "../ttLibC.h"

/**
 * speexdsp resampler definition.
 */
typedef struct ttLibC_Resampler_SpeexdspResampler {
	uint32_t channel_num;
	uint32_t input_sample_rate;
	uint32_t output_sample_rate;
	/** speexdsp resample quality 0:low quality - 10:high quality */
	uint32_t quality;
	Error_e error;
} ttLibC_Resampler_SpeexdspResampler;

typedef ttLibC_Resampler_SpeexdspResampler ttLibC_SpeexdspResampler;

/**
 * make speexdsp resampler.
 * @param channel_num        target channel num
 * @param input_sample_rate  input sample rate
 * @param output_sample_rate output sample rate
 * @param quality            quality for speexdsp resampler. 0:low quality - 10:max quality.
 * @return resampler object.
 */
ttLibC_SpeexdspResampler *ttLibC_SpeexdspResampler_make(uint32_t channel_num, uint32_t input_sample_rate, uint32_t output_sample_rate, uint32_t quality);

/**
 * sample_rate resample.
 * @param resampler  resampler object.
 * @param prev_frame reuse frame.
 * @param src_pcms16 source pcms16 data.
 * @return resampled pcms16 data.
 */
ttLibC_PcmS16 *ttLibC_SpeexdspResampler_resample(ttLibC_SpeexdspResampler *resampler, ttLibC_PcmS16 *prev_frame, ttLibC_PcmS16 *src_pcms16);

/**
 * close resampler.
 * @param resampler
 */
void ttLibC_SpeexdspResampler_close(ttLibC_SpeexdspResampler **resampler);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_RESAMPLER_SPEEXDSPRESAMPLER_H_ */
