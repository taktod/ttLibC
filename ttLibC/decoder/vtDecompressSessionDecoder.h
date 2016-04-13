/**
 * @file   vtDecompressSessionDecoder.h
 * @brief  osx or ios native decode.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/04/13
 */

#ifndef TTLIBC_DECODER_VTDECOMPRESSSESSIONDECODER_H_
#define TTLIBC_DECODER_VTDECOMPRESSSESSIONDECODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../frame/video/video.h"
#include "../frame/video/yuv420.h"

/**
 * structure for vtDecoder
 */
typedef struct ttLibC_Decoder_VtDecompressionSession_VtDecoder {
	/** target width */
	uint32_t width;
	/** target height */
	uint32_t height;
	/** target frame type */
	ttLibC_Frame_Type frame_type;
} ttLibC_Decoder_VtDecompressionSession_VtDecoder;

typedef ttLibC_Decoder_VtDecompressionSession_VtDecoder ttLibC_VtDecoder;

/**
 * callback for decode
 * @param ptr
 * @param yuv420
 * @return true / false
 */
typedef bool (* ttLibC_VtDecodeFunc)(void *ptr, ttLibC_Yuv420 *yuv420);

/**
 * callback for access raw imageBuffer
 * @param ptr
 * @param cvImageBuffer
 * @param cmtimePts
 * @param cmtimeDts
 * @return true / false
 */
typedef bool (* ttLibC_VtDecodeRawFunc)(void *ptr, void *cvImageBuffer, void *cmtimePts, void *cmtimeDts);

/**
 * make vtDecoder
 * @param target_frame_type currently support frameType_jpeg and frameType_h264
 * @return vtDecoder object.
 */
ttLibC_VtDecoder *ttLibC_VtDecoder_make(ttLibC_Frame_Type target_frame_type);

/**
 * decode frame
 * @param decoder
 * @param video
 * @param callback
 * @param ptr
 * @return true / false
 */
bool ttLibC_VtDecoder_decode(
		ttLibC_VtDecoder *decoder,
		ttLibC_Video *video,
		ttLibC_VtDecodeFunc callback,
		void *ptr);

/**
 * decode frame callback for cvImageBuffer
 * @param decoder
 * @param video
 * @param callback
 * @param ptr
 * @return true / false
 */
bool ttLibC_VtDecoder_rawDecode(
		ttLibC_VtDecoder *decoder,
		ttLibC_Video *video,
		ttLibC_VtDecodeRawFunc callback,
		void *ptr);

/**
 * close decoder
 * @param decoder
 */
void ttLibC_VtDecoder_close(ttLibC_VtDecoder **decoder);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_DECODER_VTDECOMPRESSSESSIONDECODER_H_ */
