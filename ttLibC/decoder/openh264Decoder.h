/**
 * @file   openh264Decoder.h
 * @brief  decode h264 with openh264.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/26
 */

#ifndef TTLIBC_DECODER_OPENH264DECODER_H_
#define TTLIBC_DECODER_OPENH264DECODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../frame/video/h264.h"
#include "../frame/video/yuv420.h"

/**
 * openh264 decoder definition.
 */
typedef struct {
	uint32_t width;
	uint32_t height;
} ttLibC_Decoder_Openh264Decoder;

typedef ttLibC_Decoder_Openh264Decoder ttLibC_Openh264Decoder;

/**
 * callback funtion for openh264 decoder.
 * @param ptr    user def value pointer.
 * @param yuv420 decoded yuv420 frame.
 */
typedef bool (* ttLibC_Openh264DecodeFunc)(void *ptr, ttLibC_Yuv420 *yuv420);

/**
 * make openh264 decoder (maybe add more params later.)
 */
ttLibC_Openh264Decoder *ttLibC_Openh264Decoder_make();

/**
 * decode frame.
 * @param decoder  openh264 decoder object.
 * @param h264     source h264 data.
 * @param callback callback func for h264 decode.
 * @param ptr      pointer for user def value, which will call in callback.
 */
void ttLibC_Openh264Decoder_decode(
		ttLibC_Openh264Decoder *decoder,
		ttLibC_H264 *h264,
		ttLibC_Openh264DecodeFunc callback,
		void *ptr);

/**
 * close openh264 decoder
 * @param decoder
 */
void ttLibC_Openh264Decoder_close(ttLibC_Openh264Decoder **decoder);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_DECODER_OPENH264DECODER_H_ */
