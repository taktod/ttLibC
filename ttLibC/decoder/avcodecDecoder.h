/**
 * @file   avcodecDecoder.h
 * @brief  decode frame with libavcodec
 *
 * this code is under LGPLv3 license.
 *
 * @author taktod
 * @date   2015/08/02
 * @note for some codec, you need to apply gplv3 license.
 */

#ifndef TTLIBC_DECODER_AVCODECDECODER_H_
#define TTLIBC_DECODER_AVCODECDECODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../frame/frame.h"

/**
 * avcodec decoder definition
 */
typedef struct {
	/** input frame type */
	ttLibC_Frame_Type frame_type;
	union {
		struct {
			/** width of decoded frame(for video). */
			uint32_t width;
			/** height of decoded frame(for video). */
			uint32_t height;
		};
		struct {
			/** sample_rate of decoded frame(for audio). */
			uint32_t sample_rate;
			/** channel_num of decoded frame(for audio). */
			uint32_t channel_num;
		};
	};
} ttLibC_Decoder_AvcodecDecoder;

typedef ttLibC_Decoder_AvcodecDecoder ttLibC_AvcodecDecoder;

/**
 * callback function for avcodec decoder.
 * @param ptr   user def value pointer.
 * @param frame decoded frame
 */
typedef bool (* ttLibC_AvcodecDecodeFunc)(void *ptr, ttLibC_Frame *frame);

/**
 * getAVCodecContext for target frameType.
 * @param frame_type target ttLibC_Frame_Type
 */
void *ttLibC_AvcodecDecoder_getAVCodecContext(ttLibC_Frame_Type frame_type);

/**
 * make avcodecDecoder with AVCodecContext
 * @param dec_context target AVCodecContext
 */
ttLibC_AvcodecDecoder *ttLibC_AvcodecDecoder_makeWithAVCodecContext(void *dec_context);

/**
 * make audio decoder
 * @param frame_type  target ttLibC_Frame_Type
 * @param sample_rate target sample_rate
 * @param channel_num target channel_num
 */
ttLibC_AvcodecDecoder *ttLibC_AvcodecAudioDecoder_make(
		ttLibC_Frame_Type frame_type,
		uint32_t sample_rate,
		uint32_t channel_num);

/**
 * make audio decoder
 * @param frame_type     target ttLibC_Frame_Type
 * @param sample_rate    target sample_rate
 * @param channel_num    target channel_num
 * @param extradata      extradata(some codec require these value, like vorbis)
 * @param extradata_size extradata_size
 */
ttLibC_AvcodecDecoder *ttLibC_AvcodecAudioDecoder_make_ex(
		ttLibC_Frame_Type frame_type,
		uint32_t sample_rate,
		uint32_t channel_num,
		void *extradata,
		size_t extradata_size);

/**
 * make video decoder
 * @param frame_type target ttLibC_Frame_Type
 * @param width      target width
 * @param height     target height
 */
ttLibC_AvcodecDecoder *ttLibC_AvcodecVideoDecoder_make(
		ttLibC_Frame_Type frame_type,
		uint32_t width,
		uint32_t height);

/**
 * make video decoder
 * @param frame_type     target ttLibC_Frame_Type
 * @param width          target width
 * @param height         target height
 * @param extradata      extradata(some codec require these value.)
 * @param extradata_size extradata_size
 */
ttLibC_AvcodecDecoder *ttLibC_AvcodecVideoDecoder_make_ex(
		ttLibC_Frame_Type frame_type,
		uint32_t width,
		uint32_t height,
		void *extradata,
		size_t extradata_size);

/**
 * make decoder
 * @param frame_type target ttLibC_Frame_Type
 */
ttLibC_AvcodecDecoder *ttLibC_AvcodecDecoder_make(
		ttLibC_Frame_Type frame_type);

/**
 * decode frame
 * @param decoder  avcodec decoder
 * @param frame    source frame. frame_type should be the same as decoder->frame_type
 * @param callback callback func for avcodec decode.
 * @param ptr      pointer for user def value, which call in callback.
 */
void ttLibC_AvcodecDecoder_decode(
		ttLibC_AvcodecDecoder *decoder,
		ttLibC_Frame *frame,
		ttLibC_AvcodecDecodeFunc callback,
		void *ptr);

/**
 * close avcodec decoder.
 * @param decoder.
 */
void ttLibC_AvcodecDecoder_close(ttLibC_AvcodecDecoder **decoder);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_DECODER_AVCODECDECODER_H_ */
