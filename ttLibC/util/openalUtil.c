/*
 * @file   openalUtil.c
 * @brief  openal library support.
 *
 * now work with pcms16 only. it is also possible to work with pcms8.
 *
 * this code is under GPLv3 license.
 *
 * @author taktod
 * @date   2015/07/20
 */

#ifdef __ENABLE_OPENAL__

#include "openalUtil.h"
#include "../log.h"
#include <time.h>

#include <stdlib.h>
#include <AL/al.h>
#include <AL/alc.h>

typedef struct {
	ttLibC_AlDevice inherit_super;
	ALCdevice *device;
	ALCcontext *context;
	ALuint source;
	ALuint *buffers;
} ttLibC_Util_OpenalUtil_AlDevice_;

typedef ttLibC_Util_OpenalUtil_AlDevice_ ttLibC_AlDevice_;

/*
 * make openal play device.
 * @param buffer_num number for queue buffers.
 */
ttLibC_AlDevice *ttLibC_AlDevice_make(uint32_t buffer_num) {
	ttLibC_AlDevice_ *device = malloc(sizeof(ttLibC_AlDevice_));
	if(device == NULL) {
		ERR_PRINT("failed to allocate memory for alDevice.");
		return NULL;
	}
	device->device = alcOpenDevice(NULL);
	if(device->device == NULL) {
		ERR_PRINT("failed to open ALCdevice.");
		free(device);
		return NULL;
	}
	device->context = alcCreateContext(device->device, NULL);
	if(device->context == NULL) {
		ERR_PRINT("failed to create ALCcontext.");
		alcCloseDevice(device->device);
		free(device);
		return NULL;
	}
	alcMakeContextCurrent(device->context);
	alGenSources(1, &device->source);

	device->inherit_super.buffer_num = buffer_num;
	device->buffers = malloc(sizeof(ALuint) * device->inherit_super.buffer_num);
	if(device->buffers == NULL) {
		ERR_PRINT("failed to allocate memory for buffers");
		alDeleteSources(1, &device->source);
		alcMakeContextCurrent(NULL);
		alcDestroyContext(device->context);
		alcCloseDevice(device->device);
		free(device);
		return NULL;
	}
	for(uint32_t i = 0;i < device->inherit_super.buffer_num;++ i) {
		device->buffers[i] = 0;
	}
	return (ttLibC_AlDevice *)device;
}

/*
 * queue pcm data.
 * @param device openalDevice object.
 * @param pcms16 pcms16 object.
 */
bool ttLibC_AlDevice_queue(ttLibC_AlDevice *device, ttLibC_PcmS16 *pcms16) {
	if(device == NULL) {
		return false;
	}
	if(pcms16 == NULL) {
		return false;
	}
	switch(pcms16->type) {
	case PcmS16Type_bigEndian:
	case PcmS16Type_bigEndian_planar:
		ERR_PRINT("big endian is not support by openal.");
		return false;
	case PcmS16Type_littleEndian:
		break;
	case PcmS16Type_littleEndian_planar:
		if(pcms16->inherit_super.channel_num != 1) {
			ERR_PRINT("planar data is supported only monoral.");
			return false;
		}
		break;
	default:
		ERR_PRINT("unknown pcms16Type.%d", pcms16->type);
		return false;
	}
	ttLibC_AlDevice_ *device_ = (ttLibC_AlDevice_ *)device;
	ALenum format = AL_FORMAT_MONO16;
	switch(pcms16->inherit_super.channel_num) {
	case 1: // monoral
		format = AL_FORMAT_MONO16;
		break;
	case 2: // stereo
		format = AL_FORMAT_STEREO16;
		break;
	default:
		ERR_PRINT("only monoral or stereo for openal.");
		return false;
	}
	int num;
	ALuint buffer = 0;
	alGetSourcei(device_->source, AL_BUFFERS_PROCESSED, &num);
	if(num != -1) {
		// reuse played buffer.
		alSourceUnqueueBuffers(device_->source, 1, &buffer);
	}
	else {
		// establish new buffer.
		alGetSourcei(device_->source, AL_BUFFERS_QUEUED, &num);
		if(num == device_->inherit_super.buffer_num) {
			ERR_PRINT("buffer is already fulled. cannot queue anymore.");
			return false;
		}
	}
	if(buffer == 0) {
		alGenBuffers(1, &buffer);
		for(uint32_t i = 0;i < device_->inherit_super.buffer_num;++ i) {
			if(device_->buffers[i] == 0) {
				device_->buffers[i] = buffer;
				break;
			}
		}
	}
	alBufferData(buffer, format, pcms16->inherit_super.inherit_super.data, pcms16->inherit_super.inherit_super.buffer_size, pcms16->inherit_super.sample_rate);
	alSourceQueueBuffers(device_->source, 1, &buffer);
	return true;
}

/*
 * ref the queue count.
 * @param device openalDevice object.
 */
uint32_t ttLibC_AlDevice_getQueueCount(ttLibC_AlDevice *device) {
	if(device == NULL) {
		return 0;
	}
	ttLibC_AlDevice_ *device_ = (ttLibC_AlDevice_ *)device;
	int num, processed;
	alGetSourcei(device_->source, AL_BUFFERS_QUEUED, &num);
	alGetSourcei(device_->source, AL_BUFFERS_PROCESSED, &processed);
	return num - processed;
}

/*
 * start playing and wait for certain duration.
 * @param device
 * @param milisec wait interval in mili sec, if -1, wait until use all buffer.
 */
void ttLibC_AlDevice_proceed(ttLibC_AlDevice *device, int32_t milisec) {
	if(device == NULL) {
		return;
	}
	ttLibC_AlDevice_ *device_ = (ttLibC_AlDevice_ *)device;
	int num, processed;
	int state;
	alGetSourcei(device_->source, AL_SOURCE_STATE, &state);
	if(state != AL_PLAYING) {
		alSourcePlay(device_->source);
	}
	if(milisec < 0) {
		// need to wait until all play.
		alGetSourcei(device_->source, AL_BUFFERS_QUEUED, &num);
		while(num > 0) {
			alGetSourcei(device_->source, AL_BUFFERS_PROCESSED, &processed);
			if(processed > 0) {
				while(processed --) {
					ALuint buf;
					alSourceUnqueueBuffers(device_->source, 1, &buf);
				}
			}
			else {
				// wait for 0.1 sec.
				struct timespec ts;
				ts.tv_sec = 0;
				ts.tv_nsec = 100000000;
				nanosleep(&ts, NULL);
			}
			alGetSourcei(device_->source, AL_BUFFERS_QUEUED, &num);
		}
		return;
	}
	alGetSourcei(device_->source, AL_BUFFERS_PROCESSED, &processed);
	if(processed > 0) {
		while(processed --) {
			ALuint buf;
			alSourceUnqueueBuffers(device_->source, 1, &buf);
		}
	}
	alGetSourcei(device_->source, AL_BUFFERS_QUEUED, &num);
	if(num != 0 && milisec > 0) {
		struct timespec ts;
		ts.tv_sec = (milisec / 1000);
		ts.tv_nsec = (milisec % 1000) * 1000000;
		nanosleep(&ts, NULL);
	}
}

/*
 * close the device.
 * @param device.
 */
void ttLibC_AlDevice_close(ttLibC_AlDevice **device) {
	if(*device == NULL) {
		return;
	}
	ttLibC_AlDevice_ *target = (ttLibC_AlDevice_ *)(*device);
	alSourceStop(target->source);
	alDeleteSources(1, &target->source);
	for(uint32_t i = 0;i < target->inherit_super.buffer_num;++ i) {
		if(target->buffers[i] != 0) {
			alDeleteBuffers(1, &target->buffers[i]);
			target->buffers[i] = 0;
		}
	}
	free(target->buffers);
	alcMakeContextCurrent(NULL);
	alcDestroyContext(target->context);
	alcCloseDevice(target->device);
	free(target);
	*device = NULL;
}

#endif /* __ENABLE_OPENAL__ */
