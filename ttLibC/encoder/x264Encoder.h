/**
 * @file   x264Encoder.h
 * @brief  encode h264 with x264
 *
 * this code is under GPLv3 license
 *
 * @author taktod
 * @date   2016/02/18
 */

#ifndef TTLIBC_ENCODER_X264ENCODER_H_
#define TTLIBC_ENCODER_X264ENCODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../frame/video/h264.h"
#include "../frame/video/yuv420.h"

/**
 * x264 encoder definition
 */
typedef struct ttLibC_Encoder_X264Encoder {
	/** target width */
	uint32_t width;
	/** target height */
	uint32_t height;

	/** not used.(need to improve.) */
	uint32_t max_framerate;
	uint32_t target_bitrate;
	uint32_t max_bitrate;
	uint32_t max_qval;
	uint32_t min_qval;
	int32_t idr_interval;
} ttLibC_Encoder_X264Encoder;

typedef ttLibC_Encoder_X264Encoder ttLibC_X264Encoder;

/**
 * callback function for x264 encoder.
 * @param ptr  user def value pointer.
 * @param h264 encoded h264 frame.
 */
typedef bool (* ttLibC_X264EncodeFunc)(void *ptr, ttLibC_H264 *h264);

/**
 * make x264 encoder.
 * @param width  target width
 * @param height target height
 * @return ttLibC_X264Encoder object.
 */
ttLibC_X264Encoder *ttLibC_X264Encoder_make(
		uint32_t width,
		uint32_t height);

/**
 * make x264 encoder.
 * @param width
 * @param height
 * @param max_quantizer
 * @param min_quantizer
 * @param bitrate
 * @return ttLibC_X264Encoder object.
 */
ttLibC_X264Encoder *ttLibC_X264Encoder_make_ex(
		uint32_t width,
		uint32_t height,
		uint32_t max_quantizer,
		uint32_t min_quantizer,
		uint32_t bitrate);

/**
 * setup x264_param_t data with ttLibC default.
 * @param param_t structure pointer for x264_param_t on x264.h
 * @param width   target width
 * @param height  target height
 * @return true:success false:on error.
 */
bool ttLibC_X264Encoder_getDefaultX264ParamT(
		void *param_t,
		uint32_t width,
		uint32_t height);

/**
 * setup x264_param_t data with ttLibC default.
 * @param param_t structure pointer for x264_param_t on x264.h
 * @param width
 * @param height
 * @param preset
 * @param tune
 * @return true:success false:on error.
 */
bool ttLibC_X264Encoder_getDefaultX264ParamTWithPresetTune(
		void *param_t,
		uint32_t width,
		uint32_t height,
		const char *preset,
		const char *tune);

/**
 * make x264 encoder with x264_param_t
 * @param param_t structure pointer for x264_param_t on x264.h
 * @return ttLibC_X264Encoder object.
 */
ttLibC_X264Encoder *ttLibC_X264Encoder_makeWithX264ParamT(void *param_t);

/**
 * encode frame.
 * @param encoder  x264 encoder object.
 * @param yuv420   source yuv420 data.
 * @param callback callback func for h264 creation.
 * @param ptr      pointer for user def value, which will call in callback.
 */
bool ttLibC_X264Encoder_encode(
		ttLibC_X264Encoder *encoder,
		ttLibC_Yuv420 *yuv420,
		ttLibC_X264EncodeFunc callback,
		void *ptr);

/**
 * close x264 encoder
 * @param encoder
 */
void ttLibC_X264Encoder_close(ttLibC_X264Encoder **encoder);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_ENCODER_X264ENCODER_H_ */
