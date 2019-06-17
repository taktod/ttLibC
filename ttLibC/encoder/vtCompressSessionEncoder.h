/**
 * @file   vtCompressSessionEncoder.h
 * @brief  osx or ios naative video encode.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/04/13
 */

#ifndef TTLIBC_ENCODER_VTCOMPRESSSESSIONENCODER_H_
#define TTLIBC_ENCODER_VTCOMPRESSSESSIONENCODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../ttLibC_predef.h"
#include "../frame/video/video.h"
#include "../frame/video/yuv420.h"

/**
 * structure for VtEncoder
 */
typedef struct ttLibC_Encoder_VtConpressionSession_VtEncoder {
	/** target width */
	uint32_t width;
	/** target height */
	uint32_t height;
	/** target frame type */
	ttLibC_Frame_Type frame_type;
} ttLibC_Encoder_VtConpressionSession_VtEncoder;

typedef ttLibC_Encoder_VtConpressionSession_VtEncoder ttLibC_VtEncoder;

/**
 * callback function for vtEncoder
 * @param ptr  user def value pointer.
 * @param video encoded video frame.
 */
typedef bool (* ttLibC_VtEncodeFunc)(void *ptr, ttLibC_Video *video);

/**
 * make vtEncoder.
 * @param width
 * @param height
 * @param target_frame_type current support frameType_jpeg and frameType_h264.
 * @return ttLibC_VtEncoder object.
 */
ttLibC_VtEncoder TT_ATTRIBUTE_API *ttLibC_VtEncoder_make(
		uint32_t width,
		uint32_t height,
		ttLibC_Frame_Type target_frame_type);

/**
 * make vtEncoder.
 * @param width
 * @param height
 * @param fps
 * @param bitrate
 * @param is_baseline true:baseline false:main
 * @param target_frame_type
 */
ttLibC_VtEncoder TT_ATTRIBUTE_API *ttLibC_VtEncoder_make_ex(
		uint32_t width,
		uint32_t height,
		uint32_t fps,
		uint32_t bitrate,
		bool is_baseline,
		ttLibC_Frame_Type target_frame_type);

/**
 * encode frame
 * @param encoder
 * @param yuv420
 * @param callback
 * @param ptr
 * @return true:success to continue / false:error
 */
bool TT_ATTRIBUTE_API ttLibC_VtEncoder_encode(
		ttLibC_VtEncoder *encoder,
		ttLibC_Yuv420 *yuv420,
		ttLibC_VtEncodeFunc callback,
		void *ptr);

/**
 * close encoder
 * @param encoder
 */
void TT_ATTRIBUTE_API ttLibC_VtEncoder_close(ttLibC_VtEncoder **encoder);

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* TTLIBC_ENCODER_VTCOMPRESSSESSIONENCODER_H_ */
