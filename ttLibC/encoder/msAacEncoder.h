/**
 * @file   msAacEncoder.h
 * @brief  windows native aac encoder.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2017/05/11
 */

#ifndef TTLIBC_ENCODER_MSAACENCODER_H_
#define TTLIBC_ENCODER_MSAACENCODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../frame/audio/aac.h"
#include "../frame/audio/pcms16.h"

/**
 * msAacEncoder definition.
 */
typedef struct ttLibC_Encoder_MsAacEncoder {
	uint32_t sample_rate;
	uint32_t channel_num;
	uint32_t bitrate;
} ttLibC_Encoder_MsAacEncoder;

typedef ttLibC_Encoder_MsAacEncoder ttLibC_MsAacEncoder;

/**
 * callback function for msAacEncoder.
 * @param ptr
 * @param audio
 * @return true:success false:error
 */
typedef bool (*ttLibC_MsAacEncodeFunc)(void *ptr, ttLibC_Aac *aac);

ttLibC_MsAacEncoder *ttLibC_MsAacEncoder_make(
	uint32_t sample_rate,
	uint32_t channel_num,
	uint32_t target_bitrate);

bool ttLibC_MsAacEncoder_encode(
	ttLibC_MsAacEncoder *encoder,
	ttLibC_PcmS16 *pcm,
	ttLibC_MsAacEncodeFunc callback,
	void *ptr);

void ttLibC_MsAacEncoder_close(ttLibC_MsAacEncoder **encoder);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_ENCODER_MSAACENCODER_H_ */
