/**
 * @file   theoraEncoder.h
 * @brief  encode theora with libtheora.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/04/04
 */

#ifndef TTLIBC_ENCODER_THEORAENCODER_H_
#define TTLIBC_ENCODER_THEORAENCODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../frame/video/theora.h"
#include "../frame/video/yuv420.h"

/**
 * theora encoder definition
 */
typedef struct ttLibC_Encoder_TheoraEncoder {
	/** target width */
	uint32_t width;
	/** target height */
	uint32_t height;
} ttLibC_Encoder_TheoraEncoder;

typedef ttLibC_Encoder_TheoraEncoder ttLibC_TheoraEncoder;

/**
 * callback function for theora encoder.
 * @param ptr    user def value pointer.
 * @param theora encoded theora frame.
 */
typedef bool (* ttLibC_TheoraEncodeFunc)(void *ptr, ttLibC_Theora *theora);

/**
 * make theora encoder.
 * @param width
 * @param height
 * @return theoraEncoder object.
 */
ttLibC_TheoraEncoder *ttLibC_TheoraEncoder_make(
		uint32_t width,
		uint32_t height);

/**
 * make theora encoder with special values.
 * @param width
 * @param height
 * @param quality 0-63
 * @param bitrate in bit/sec
 * @param key_frame_interval 1 - 31
 * @return theoraEncoder object.
 */
ttLibC_TheoraEncoder *ttLibC_TheoraEncoder_make_ex(
		uint32_t width,
		uint32_t height,
		uint32_t quality,
		uint32_t bitrate,
		uint32_t key_frame_interval);

/**
 * make theora encoder with th_info
 * @param ti theora info object.
 * @return theoraEncoder object.
 */
ttLibC_TheoraEncoder *ttLibC_TheoraEncoder_makeWithInfo(void *ti);

/**
 * ref theora context.
 * @param encoder theoraEncoder object.
 * @return pointer for theora context.(th_enc_ctx)
 */
void *ttLibC_TheoraEncoder_refNativeEncodeContext(ttLibC_TheoraEncoder *encoder);

/**
 * encode frame.
 * @param encoder
 * @param yuv420
 * @param callback
 * @param ptr
 * @return true/success false/error
 */
bool ttLibC_TheoraEncoder_encode(
		ttLibC_TheoraEncoder *encoder,
		ttLibC_Yuv420 *yuv420,
		ttLibC_TheoraEncodeFunc callback,
		void *ptr);

/**
 * close theoraEncoder
 * @param encoder
 */
void ttLibC_TheoraEncoder_close(ttLibC_TheoraEncoder **encoder);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_ENCODER_THEORAENCODER_H_ */
