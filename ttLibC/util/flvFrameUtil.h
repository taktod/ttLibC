/**
 * @file   flvFrameUtil.h
 * @brief  help util for analyze flv binary.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/23
 */

#ifndef TTLIBC_UTIL_FLVFRAMEUTIL_H_
#define TTLIBC_UTIL_FLVFRAMEUTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../frame/frame.h"
#include "../frame/video/video.h"
#include "../frame/audio/audio.h"
#include "../container/container.h"
#include "dynamicBufferUtil.h"

typedef enum {
	FlvAudioCodec_pcmBigEndian    = 0x00,
	FlvAudioCodec_swfAdpcm        = 0x01,
	FlvAudioCodec_mp3             = 0x02,
	FlvAudioCodec_pcmLittleEndian = 0x03,
	FlvAudioCodec_nellymoser16    = 0x04,
	FlvAudioCodec_nellymoser8     = 0x05,
	FlvAudioCodec_nellymoser      = 0x06,
	FlvAudioCodec_pcmAlaw         = 0x07,
	FlvAudioCodec_pcmMulaw        = 0x08,
	FlvAudioCodec_reserved        = 0x09,
	FlvAudioCodec_aac             = 0x0A,
	FlvAudioCodec_speex           = 0x0B,
	FlvAudioCodec_unknown         = 0x0C,
	FlvAudioCodec_undefined       = 0x0D,
	FlvAudioCodec_mp38            = 0x0E,
	FlvAudioCodec_deviceSpecific  = 0x0F,
} FlvFrameManager_AudioCodec;

typedef enum {
	FlvVideoCodec_jpeg          = 0x01,
	FlvVideoCodec_flv1          = 0x02,
	FlvVideoCodec_screenVideo   = 0x03,
	FlvVideoCodec_on2Vp6        = 0x04,
	FlvVideoCodec_on2Vp6Alpha   = 0x05,
	FlvVideoCodec_screenVideoV2 = 0x06,
	FlvVideoCodec_avc           = 0x07,
} FlvFrameManager_VideoCodec;

/**
 * structure for flvFrameManager
 */
typedef struct ttLibC_Util_FlvFrameManager {
	ttLibC_Frame_Type audio_type;
	ttLibC_Frame_Type video_type;
} ttLibC_Util_FlvFrameManager;

typedef ttLibC_Util_FlvFrameManager ttLibC_FlvFrameManager;

/**
 * make manager
 * @return ttLibC_FlvFrameManager object.
 */
ttLibC_FlvFrameManager *ttLibC_FlvFrameManager_make();

/**
 * read video binary data.
 * @param manager   manager.
 * @param data      target binary data
 * @param data_size target binary data size
 * @param pts       pts(timebase is 1000 fixed.)
 * @param callback  callback
 * @param ptr       userdef data pointer.
 * @return ture / false
 * TODO need error code?
 */
bool ttLibC_FlvFrameManager_readVideoBinary(
		ttLibC_FlvFrameManager *manager,
		void *data,
		size_t data_size,
		uint64_t pts,
		ttLibC_getFrameFunc callback,
		void *ptr);

/**
 * read audio binary data.
 * @param manager   manager
 * @param data      target binary data
 * @param data_size target binary data size
 * @param pts       pts(timebase is 1000 fixed.)
 * @param callback  callback
 * @param ptr       userdef data pointer.
 * @return ture / false
 */
bool ttLibC_FlvFrameManager_readAudioBinary(
		ttLibC_FlvFrameManager *manager,
		void *data,
		size_t data_size,
		uint64_t pts,
		ttLibC_getFrameFunc callback,
		void *ptr);

/**
 * special get binary data for aac dsi information.
 * @param frame  target frame(aac)
 * @param buffer buffer to append.
 * @return true:success false:error
 */
bool ttLibC_FlvFrameManager_getAacDsiData(
		ttLibC_Frame *frame,
		ttLibC_DynamicBuffer *buffer);

/**
 * get binary data for frame.
 * @param frame  target frame
 * @param buffer buffer to append.
 * @return true:success false:error
 */
bool ttLibC_FlvFrameManager_getData(
		ttLibC_Frame *frame,
		ttLibC_DynamicBuffer *buffer);

/**
 * close manager
 * @param manager
 */
void ttLibC_FlvFrameManager_close(ttLibC_FlvFrameManager **manager);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_UTIL_FLVFRAMEUTIL_H_ */
