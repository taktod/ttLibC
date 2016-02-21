/**
 * @file   vtDecompressSessionH264Decoder.h
 * @brief  osx or ios native h264 decode.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/02/20
 */

#ifndef TTLIBC_DECODER_VTDECOMPRESSSESSIONH264DECODER_H_
#define TTLIBC_DECODER_VTDECOMPRESSSESSIONH264DECODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../frame/video/h264.h"
#include "../frame/video/yuv420.h"

typedef struct ttLibC_Decoder_VtDecompressionSession_VtH264Decoder {
	uint32_t width;
	uint32_t height;
} ttLibC_Decoder_VtDecompressionSession_VtH264Decoder;

typedef ttLibC_Decoder_VtDecompressionSession_VtH264Decoder ttLibC_VtH264Decoder;

typedef bool (* ttLibC_VtH264DecodeFunc)(void *ptr, ttLibC_Yuv420 *yuv420);

ttLibC_VtH264Decoder *ttLibC_VtH264Decoder_make();

bool ttLibC_VtH264Decoder_decode(
		ttLibC_VtH264Decoder *decoder,
		ttLibC_H264 *h264,
		ttLibC_VtH264DecodeFunc callback,
		void *ptr);

void ttLibC_VtH264Decoder_close(ttLibC_VtH264Decoder **decoder);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_DECODER_VTDECOMPRESSSESSIONH264DECODER_H_ */
