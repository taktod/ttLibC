/**
 * @file   audioUnitUtil.h
 * @brief  play or record sound by audioUnit in OSX or iOS
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2015/12/22
 */

#ifndef TTLIBC_UTIL_AUDIOUNITUTIL_H_
#define TTLIBC_UTIL_AUDIOUNITUTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../frame/audio/pcms16.h"

/**
 * definition of player type.
 */
typedef enum ttLibC_AuPlayer_Type {
	AuPlayerType_DefaultOutput = 0x00000000
} ttLibC_AuPlayer_Type;

/**
 * definition of audioUnit Player
 */
typedef struct ttLibC_Util_AudioUnit_AuPlayer {
	/** work samplerate */
	uint32_t sample_rate;
	/** work channel track number */
	uint32_t channel_num;
	/** playing pts position. timebase is the same as sample_rate */
	uint64_t pts;
} ttLibC_Util_AudioUnit_AuPlayer;

typedef ttLibC_Util_AudioUnit_AuPlayer ttLibC_AuPlayer;

/**
 * make audio player by audioUnit.
 * @param sample_rate target sample rate
 * @param channel_num target channel num
 * @param type        device type(kAudioUnitSubType)
 * @return ttLibC_AuPlayer object.
 */
ttLibC_AuPlayer *ttLibC_AuPlayer_make(
		uint32_t sample_rate,
		uint32_t channel_num,
		uint32_t type);

/**
 * queue pcm data.
 * @param player target ttLibC_AuPlayer object.
 * @param pcmS16 pcmS16 object for play.
 * @return true:success to put queue. false:error to put queue(queue is full, you need to try again later.)
 */
bool ttLibC_AuPlayer_queue(ttLibC_AuPlayer *player, ttLibC_PcmS16 *pcmS16);

uint64_t ttLibC_AuPlayer_getPts(ttLibC_AuPlayer *player);

uint32_t ttLibC_AuPlayer_getTimebase(ttLibC_AuPlayer *player);

/**
 * close player
 * @param player ttLibC_AuPlayer object.
 */
void ttLibC_AuPlayer_close(ttLibC_AuPlayer **player);


/**
 * definition of auRecorder type.
 */
typedef enum ttLibC_AuRecorder_Type {
	AuRecorderType_DefaultInput = 0x00000000,
} ttLibC_AuRecorder_Type;

/**
 * definition of audioUnit Recorder
 */
typedef struct ttLibC_Util_AudioUnit_AuRecorder {
	/** work sample_rate */
	uint32_t sample_rate;
	/** work channel_num */
	uint32_t channel_num;
	/** current work pts */
	uint64_t pts;
	/** recording flag. */
	bool is_recording;
} ttLibC_Util_AudioUnit_AuRecorder;

typedef ttLibC_Util_AudioUnit_AuRecorder ttLibC_AuRecorder;

/**
 * callback for generated sound
 * @param ptr   user defined pointer.(set on start func.)
 * @param audio generated ttLibC_Audio object.
 * @return true continue to record / false stop to record.
 */
typedef bool (* ttLibC_AuRecorderFunc)(void *ptr, ttLibC_Audio *audio);

/**
 * make audio recorder by audioUnit.
 * @param sample_rate target sample rate
 * @param channel_num target channel num
 * @param type        device type(kAudioUnitSubType)
 * @param device_id   device_id
 * @return ttLibC_AuRecorder object.
 */
ttLibC_AuRecorder *ttLibC_AuRecorder_make(
		uint32_t sample_rate,
		uint32_t channel_num,
		uint32_t type,
		uint32_t device_id);

/**
 * start recorder
 * @param recorder target ttLibC_AuRecorder object.
 * @param callback set the callback.
 * @param ptr      set the data pointer which will passing in callback.
 */
bool ttLibC_AuRecorder_start(
		ttLibC_AuRecorder *recorder,
		ttLibC_AuRecorderFunc callback,
		void *ptr);

/**
 * stop recorder
 * @param recorder target ttLibC_AuRecorder object.
 */
bool ttLibC_AuRecorder_stop(ttLibC_AuRecorder *recorder);

/**
 * close recorder
 * @param recorder
 */
void ttLibC_AuRecorder_close(ttLibC_AuRecorder **recorder);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_UTIL_AUDIOUNITUTIL_H_ */
