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

#include "../ttLibC_predef.h"
#include "../frame/video/h264.h"
#include "../frame/video/yuv420.h"

/**
 * openh264 encoder type
 */
typedef enum ttLibC_Openh264Encoder_Type {
	Openh264EncoderType_Baseline, // supported baseline only?
} ttLibC_Openh264Encoder_Type;

/**
 * openh264 ratecontrol type
 */
typedef enum ttLibC_Openh264Encoder_RCType {
	Openh264EncoderRCType_QualityMode = 0,
	Openh264EncoderRCType_BitrateMode = 1,
	Openh264EncoderRCType_BufferbasedMode = 2,
	Openh264EncoderRCType_TimestampMode = 3,
	Openh264EncoderRCType_BitrateModePostSkip = 4,
	Openh264EncoderRCType_OffMode = -1,
} ttLibC_Openh264Encoder_RCType;

/**
 * openh264 enoder definition
 */
typedef struct ttLibC_Encoder_Openh264Encoder {
	/** openh264 encoder type */
	ttLibC_Openh264Encoder_Type type;
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
	int32_t  idr_interval;
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
 * @return ttLibC_Openh264Encoder object.
 */
ttLibC_Openh264Encoder TT_ATTRIBUTE_API *ttLibC_Openh264Encoder_make(
		uint32_t width,
		uint32_t height);

/**
 * make openh264 encoder(baseline only.)
 * @param width         target width
 * @param height        target height
 * @param max_quantizer max q value
 * @param min_quantizer min q value
 * @param bitrate       target bitrate (bit / sec)
 * @return ttLibC_Openh264Encoder object.
 */
ttLibC_Openh264Encoder TT_ATTRIBUTE_API *ttLibC_Openh264Encoder_make_ex(
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
void TT_ATTRIBUTE_API ttLibC_Openh264Encoder_getDefaultSEncParamExt(
		void *paramExt,
		uint32_t width,
		uint32_t height);

/**
 * parse openh264 param with c string.
 * @param paramExt SEncParamExt structure object.
 * @param name     target element name, should be same as structure element.
 * @param value    support int, double, enum name with  c string.
 */
bool TT_ATTRIBUTE_API ttLibC_Openh264Encoder_paramParse(void *paramExt, const char *name, const char *value);

/**
 * parse openh264 spatialLayer info with c string.
 * @param paramExt SEncParamExt structure object.
 * @param target   spatialLayer index should be less than 4
 * @param name     target element name, should be same as layer element.
 * @param value    support int, double, enum name with  c string.
 */
bool TT_ATTRIBUTE_API ttLibC_Openh264Encoder_spatialParamParse(void *paramExt, uint32_t target, const char *name, const char *value);

/**
 * make openh264 encoder with SEncParamExt
 * @param paramExt structure pointer for SEncParamExt on wels/codec_api.h
 * @return ttLibC_Openh264Encoder object.
 */
ttLibC_Openh264Encoder TT_ATTRIBUTE_API *ttLibC_Openh264Encoder_makeWithSEncParamExt(void *paramExt);

/**
 * encode frame.
 * @param encoder  openh264 encoder object
 * @param yuv420   source yuv420 data.
 * @param callback callback func for h264 creation.
 * @param ptr      pointer for user def value, which will call in callback.
 */
bool TT_ATTRIBUTE_API ttLibC_Openh264Encoder_encode(
		ttLibC_Openh264Encoder *encoder,
		ttLibC_Yuv420 *yuv420,
		ttLibC_Openh264EncodeFunc callback,
		void *ptr);

/**
 * ref liopenh264 native encoder object.
 * @param encoder openh264 encoder object.
 * @return ISVCEncoder pointer.
 */
void TT_ATTRIBUTE_API *ttLibC_Openh264Encoder_refNativeEncoder(ttLibC_Openh264Encoder *encoder);

/**
 * set idr interval
 * @param encoder
 * @param interval -1 means no keyFrame.(first picture is only one keyFrame.)
 * @return true / false
 */
bool TT_ATTRIBUTE_API ttLibC_Openh264Encoder_setIDRInterval(
		ttLibC_Openh264Encoder *encoder,
		int32_t interval);

/**
 * update RateControl mode for openh264.
 * @param encoder
 * @param rcType
 * @return true:success false:error.
 */
bool TT_ATTRIBUTE_API ttLibC_Openh264Encoder_setRCMode(
		ttLibC_Openh264Encoder *encoder,
		ttLibC_Openh264Encoder_RCType rcType);

/**
 * force next encode picture will be key frame(sliceIDR).
 * @param encoder
 * @return true:success false:error.
 */
bool TT_ATTRIBUTE_API ttLibC_Openh264Encoder_forceNextKeyFrame(ttLibC_Openh264Encoder *encoder);

/**
 * try to reduce the nal header size. use 3byte.
 * @param encoder
 * @param reduce_mode_flag
 * @return true:success false:error
 */
bool TT_ATTRIBUTE_API ttLibC_Openh264Encoder_setReduceMode(
		ttLibC_Openh264Encoder *encoder,
		bool reduce_mode_flag);

/**
 * close openh264 encoder
 * @param encoder
 */
void TT_ATTRIBUTE_API ttLibC_Openh264Encoder_close(ttLibC_Openh264Encoder **encoder);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_ENCODER_OPENH264ENCODER_H_ */
