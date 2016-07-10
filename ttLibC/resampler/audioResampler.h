/**
 * @file   audioResampler.h
 * @brief  resample pcm data.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/21
 */

#ifndef TTLIBC_RESAMPLER_AUDIORESAMPLER_H_
#define TTLIBC_RESAMPLER_AUDIORESAMPLER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../frame/audio/pcms16.h"
#include "../frame/audio/pcmf32.h"

/**
 * convert ttLibC_Audio frame
 * @param prev_frame  reuse frame
 * @param frame_type  target frame
 * @param type        target frame type
 * @param channel_num target frame channel 1:monoral 2:stereo
 * @param src_frame   source frame
 */
ttLibC_Audio *ttLibC_AudioResampler_convertFormat(
		ttLibC_Audio *prev_frame,
		ttLibC_Frame_Type frame_type,
		uint32_t type,
		uint32_t channel_num,
		ttLibC_Audio *src_frame);

/**
 * make ttLibC_PcmF32 from ttLibC_PcmS16.
 * @param prev_frame reuse frame.
 * @param type       pcmf32 type.
 * @param src_frame  src pcms16 frame.
 */
ttLibC_PcmF32 *ttLibC_AudioResampler_makePcmF32FromPcmS16(
		ttLibC_PcmF32 *prev_frame,
		ttLibC_PcmF32_Type type,
		ttLibC_PcmS16 *src_frame);

/**
 * make ttLibC_PcmS16 from ttLibC_PcmF32.
 * @param prev_frame reuse frame.
 * @param type       pcms16 type.
 * @param src_frame  src pcmf32 frame.
 */
ttLibC_PcmS16 *ttLibC_AudioResampler_makePcmS16FromPcmF32(
		ttLibC_PcmS16 *prev_frame,
		ttLibC_PcmS16_Type type,
		ttLibC_PcmF32 *src_frame);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_RESAMPLER_AUDIORESAMPLER_H_ */
