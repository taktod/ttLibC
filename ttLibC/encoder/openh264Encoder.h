/**
 * @file   openh264Encoder.h
 * @brief  encode h264 with openh264.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/24
 */

#ifndef TTLIBC_ENCODER_OPENH264ENCODER_H_
#define TTLIBC_ENCODER_OPENH264ENCODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../frame/video/h264.h"
#include "../frame/video/yuv420.h"

/**
 * openh264 encoder type
 */
typedef enum {
	Openh264EncoderType_Baseline, // supported baseline only?
} ttLibC_Openh264Encoder_Type;

/**
 * openh264 enoder definition
 */
typedef struct {
	/** openh264 encoder type */
	ttLibC_Openh264Encoder_Type type;
	/** target width */
	uint32_t width;
	/** target height */
	uint32_t height;
	/** under data is not used.(need to improve.) */
	uint32_t max_framerate;
	uint32_t target_bitrate;
	uint32_t max_bitrate;
	uint32_t max_qval;
	uint32_t min_qval;
} ttLibC_Encoder_Openh264Encoder;

typedef ttLibC_Encoder_Openh264Encoder ttLibC_Openh264Encoder;

/**
 * callback function for openh264 encoder.
 * @param ptr  user def value pointer.
 * @param h264 encoded h264 frame.
 */
typedef bool (* ttLibC_Openh264EncodeFunc)(void *ptr, ttLibC_H264 *h264);

/**
 * make openh264 encoder(baseline only.) (maybe add more params later)
 * @param width  target width
 * @param height target height
 * @return openh264 encoder object.
 */
ttLibC_Openh264Encoder *ttLibC_Openh264Encoder_make(
		uint32_t width,
		uint32_t height);

/**
 * make openh264 encoder(baseline only.)
 * @param width         target width
 * @param height        target height
 * @param max_quantizer max q value
 * @param min_quantizer min q value
 * @param bitrate       target bitrate (bit / sec)
 * @return openh264 encoder object.
 */
ttLibC_Openh264Encoder *ttLibC_Openh264Encoder_make_ex(
		uint32_t width,
		uint32_t height,
		uint32_t max_quantizer,
		uint32_t min_quantizer,
		uint32_t bitrate);

/**
 * setup SEncParamExt data with ttLibC default.
 * @param paramExt structure pointer for SEncParamExt on wels/codec_api.h
 * @param width    target picture width
 * @param height   target picture height
 */
void ttLibC_Openh264Encoder_getDefaultSEncParamExt(
		void *paramExt,
		uint32_t width,
		uint32_t height);

/**
 * make openh264 encoder with SEncParamExt
 * @param paramExt structure pointer for SEncParamExt on wels/codec_api.h
 * @return openh264 encoder object.
 */
ttLibC_Openh264Encoder *ttLibC_Openh264Encoder_makeWithSEncParamExt(void *paramExt);

/**
 * encode frame.
 * @param encoder  openh264 encoder object
 * @param yuv420   source yuv420 data.
 * @param callback callback func for h264 creation.
 * @param ptr      pointer for user def value, which will call in callback.
 */
void ttLibC_Openh264Encoder_encode(
		ttLibC_Openh264Encoder *encoder,
		ttLibC_Yuv420 *yuv420,
		ttLibC_Openh264EncodeFunc callback,
		void *ptr);

/**
 * close openh264 encoder
 * @param encoder
 */
void ttLibC_Openh264Encoder_close(ttLibC_Openh264Encoder **encoder);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_ENCODER_OPENH264ENCODER_H_ */
