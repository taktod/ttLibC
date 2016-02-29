/**
 * @file   avcodecEncoder.h
 * @brief  encode frame with libavcodec
 *
 * this code is under LGPLv3 license.
 *
 * @author taktod
 * @date   2015/08/02
 * @note for some codec, you need to apply gplv3 license.
 */

#ifndef TTLIBC_ENCODER_AVCODECENCODER_H_
#define TTLIBC_ENCODER_AVCODECENCODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../frame/frame.h"

/**
 * avcodec encoder definition.
 */
typedef struct ttLibC_Encoder_AvcodecEncoder {
	/** output frame type */
	ttLibC_Frame_Type frame_type;
	/** input frame type */
	ttLibC_Frame_Type input_frame_type;
	/** input frame format type. ttLibC_PcmS16_Type, ttLibC_PcmF32_Type, ttLibC_Yuv420_Type ... */
	uint32_t input_format_type;
	union {
		struct {
			/** width for video */
			uint32_t width;
			/** height for video */
			uint32_t height;
		};
		struct {
			/** sample_rate for audio */
			uint32_t sample_rate;
			/** channel_num for audio */
			uint32_t channel_num;
		};
	};
} ttLibC_Encoder_AvcodecEncoder;

typedef ttLibC_Encoder_AvcodecEncoder ttLibC_AvcodecEncoder;

/**
 * callback function for avcodecEncoder
 * @param ptr   user def value pointer.
 * @param frame encoded frame.
 */
typedef bool (* ttLibC_AvcodecEncodeFunc)(void *ptr, ttLibC_Frame *frame);

/**
 * get AVCodecContext for target frameType.
 * @param frame_type target ttLibC_Frame_Type
 */
void *ttLibC_AvcodecEncoder_getAVCodecContext(ttLibC_Frame_Type frame_type);

/**
 * make avcodecEncoder with AVCodecContext
 * @param enc_context target AVCodecContext
 */
ttLibC_AvcodecEncoder *ttLibC_AvcodecEncoder_makeWithAVCodecContext(void *enc_context);

/**
 * make audio encoder
 * @param frame_type  target ttLibC_Frame_Type
 * @param sample_rate target sample_rate
 * @param channel_num target channel_num
 */
ttLibC_AvcodecEncoder *ttLibC_AvcodecAudioEncoder_make(
		ttLibC_Frame_Type frame_type,
		uint32_t sample_rate,
		uint32_t channel_num);

/**
 * make audio encoder
 * @param frame_type  target ttLibC_Frame_Type
 * @param sample_rate target sample_rate
 * @param channel_num target channel_num
 * @param bitrate     target bitrate
 */
ttLibC_AvcodecEncoder *ttLibC_AvcodecAudioEncoder_make_ex(
		ttLibC_Frame_Type frame_type,
		uint32_t sample_rate,
		uint32_t channel_num,
		uint32_t bitrate);

/**
 * make video encoder
 * @param frame_type target ttLibC_Frame_Type
 * @param width      width
 * @param height     height
 */
ttLibC_AvcodecEncoder *ttLibC_AvcodecVideoEncoder_make(
		ttLibC_Frame_Type frame_type,
		uint32_t width,
		uint32_t height);

/**
 * make video encoder
 * @param frame_type target ttLibC_Frame_Type
 * @param width      width
 * @param height     height
 * @param quality    q-value
 * @param bitrate    target bitrate in bit/sec
 * @param timebase   time base
 */
ttLibC_AvcodecEncoder *ttLibC_AvcodecVideoEncoder_make_ex(
		ttLibC_Frame_Type frame_type,
		uint32_t width,
		uint32_t height,
		uint32_t quality,
		uint32_t bitrate,
		uint32_t timebase);

/**
 * encode frame
 * @param encoder  avcodec encoder object
 * @param frame    source frame. frame_type and frame format type is the same as encoder->input_frame_type and encoder->input_format_type.
 * @param callback callback func for avcodec encode.
 * @param ptr      pointer for user def value, which will call in callback.
 */
void ttLibC_AvcodecEncoder_encode(
		ttLibC_AvcodecEncoder *encoder,
		ttLibC_Frame *frame,
		ttLibC_AvcodecEncodeFunc callback,
		void *ptr);

/**
 * ref the avcodec extra data.
 * @param encoder avcodec encoder object.
 */
void *ttLibC_AvcodecEncoder_getExtraData(ttLibC_AvcodecEncoder *encoder);

/**
 * ref the avcodec extra data size.
 * @param encoder avcodec encoder object.
 */
size_t ttLibC_AvcodecEncoder_getExtraDataSize(ttLibC_AvcodecEncoder *encoder);

/**
 * close avcodec encoder.
 * @param encoder
 */
void ttLibC_AvcodecEncoder_close(ttLibC_AvcodecEncoder **encoder);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_ENCODER_AVCODECENCODER_H_ */
