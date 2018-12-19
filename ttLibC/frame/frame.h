/**
 * @file   frame.h
 * @brief  frame information.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/18
 */

#ifndef TTLIBC_FRAME_FRAME_H_
#define TTLIBC_FRAME_FRAME_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * frame type.
 */
typedef enum ttLibC_Frame_Type {
	/** ttLibC_Bgr video frame */
	frameType_bgr,
	/** ttLibC_Flv1 video frame */
	frameType_flv1,
	/** ttLibC_H264 video frame */
	frameType_h264,
	/** ttLibC_H265 video frame */
	frameType_h265,
	/** ttLibC_Jpeg video frame */
	frameType_jpeg,
	/** ttLibC_Png video frame */
	frameType_png,
	/** ttLibC_Theora video frame */
	frameType_theora,
	/** ttLibC_Vp6 video frame */
	frameType_vp6,
	/** ttLibC_Vp8 video frame */
	frameType_vp8,
	/** ttLibC_Vp9 video frame */
	frameType_vp9,
	/** ttLibC_Wmv1 video frame */
	frameType_wmv1,
	/** ttLibC_Wmv2 video frame */
	frameType_wmv2,
	/** ttLibC_Yuv420 video frame */
	frameType_yuv420,

//	frameType_flashSv, // flash screen video
//	frameType_flashSv2,// flash screen video 2

	/** ttLibC_Aac audio frame */
	frameType_aac,
	/** ttLibC_AdpcmImaWav */
	frameType_adpcm_ima_wav,
	/** ttLibC_Mp3 audio frame */
	frameType_mp3,
	/** ttLibC_Nellymoser */
	frameType_nellymoser,
	/** ttLibC_Opus audio frame */
	frameType_opus,
	/** ttLibC_PcmAlaw */
	frameType_pcm_alaw,
	/** ttLibC_PcmF32 */
	frameType_pcmF32,
	/** ttLibC_PcmMulaw */
	frameType_pcm_mulaw,
	/** ttLibC_PcmS16 audio frame */
	frameType_pcmS16,
	/** ttLibC_Speex audio frame */
	frameType_speex,
	/** ttLibC_Vorbis */
	frameType_vorbis,
//	frameType_adpcm_swf,
//	frameType_ac3,
	frameType_unknown = -1,
} ttLibC_Frame_Type;

/**
 * base definition of frame.
 */
typedef struct ttLibC_Frame {
	/** frame type information */
	ttLibC_Frame_Type type;
	/**
	 * pts information
	 * 1 pts = 1sec / timebase
	 */
	uint64_t pts;
	/**
	 * dts information
	 * use for container work only.
	 */
	uint64_t dts;
	/**
	 * timebase information
	 * 1/timebase is the unit of pts.
	 * for example, 1000 means 1pts = 1mili sec.
	 */
	uint32_t timebase;
	/** data of frame */
	void *data;
	/** data size of frame */
	size_t data_size;
	/**
	 * actual size of frame
	 * data_size can have extra space.
	 */
	size_t buffer_size;
	/**
	 * flag for instanced data or not.
	 * if false, data will be freed on close method.
	 */
	bool is_non_copy;
	uint32_t id;
} ttLibC_Frame;

/**
 * callback of binary passing function.
 * @param ptr       user def pointer object.
 * @param data      data
 * @param data_size data size
 * @return true / false
 */
typedef bool (* ttLibC_FrameBinaryFunc)(void *ptr, void *data, size_t size);

/**
 * make clone frame.
 * always make copy buffer on it.
 * @param prev_frame reuse frame object.
 * @param src_frame  source of clone.
 */
ttLibC_Frame *ttLibC_Frame_clone(
		ttLibC_Frame *prev_frame,
		ttLibC_Frame *src_frame);

/**
 * check frame type is audio frame.
 * @param type
 * @return true:audio false:not audio
 */
bool ttLibC_isAudio(ttLibC_Frame_Type type);

/**
 * check frame type is video frame.
 * @param type
 * @return true:video false:not video
 */
bool ttLibC_isVideo(ttLibC_Frame_Type type);

/**
 * check frame is audio frame.
 * @param frame
 * @return true:audio false:not audio
 */
bool ttLibC_Frame_isAudio(ttLibC_Frame *frame);

/**
 * check frame is video frame.
 * @param frame
 * @return true:video false:not video
 */
bool ttLibC_Frame_isVideo(ttLibC_Frame *frame);

/**
 * close frame
 * @param frame
 */
void ttLibC_Frame_close(ttLibC_Frame **frame);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_FRAME_FRAME_H_ */
