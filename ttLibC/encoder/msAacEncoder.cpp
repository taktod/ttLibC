/**
 * @file   msAacEncoder.cpp
 * @brief  windows native aac encoder.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2017/05/11
 */

#ifdef __ENABLE_WIN32__

#include "msAacEncoder.h"
#include <ttLibC/log.h>
#include <ttLibC/allocator.h>
#include <ttLibC/frame/audio/pcms16.h>

#include <windows.h>
#include <mfapi.h>
#include <Mfidl.h>
#include <shlwapi.h>
#include <Codecapi.h>
#include <comdef.h>
#include <mftransform.h>
#include <wmcodecdsp.h>

#include "../util/msGlobalUtil.h"

#pragma comment(lib, "mfplat")
#pragma comment(lib, "mf")
#pragma comment(lib, "mfuuid")
#pragma comment(lib, "d3d9")
#pragma comment(lib, "dxva2")
#pragma comment(lib, "uuid")
#pragma comment(lib, "wmcodecdspuuid")

typedef struct ttLibC_Encoder_MsAacEncoder_ {
	ttLibC_MsAacEncoder inherit_super;
	IMFTransform *transform;
	ttLibC_Aac *aac;
	uint32_t sample_rate;
	uint32_t channel_num;
	uint32_t bitrate;
} ttLibC_Encoder_MsAacEncoder_;

typedef ttLibC_Encoder_MsAacEncoder_ ttLibC_MsAacEncoder_;

ttLibC_MsAacEncoder *ttLibC_MsAacEncoder_make(
		uint32_t sample_rate,
		uint32_t channel_num,
		uint32_t target_bitrate) {
	ttLibC_MsAacEncoder_ *encoder = (ttLibC_MsAacEncoder_ *)ttLibC_malloc(sizeof(ttLibC_MsAacEncoder_));
	if(encoder == NULL) {
		return NULL;
	}
	memset(encoder, 0, sizeof(ttLibC_MsAacEncoder_));
	encoder->sample_rate = sample_rate;
	encoder->channel_num = channel_num;
	encoder->bitrate = target_bitrate;
	encoder->inherit_super.sample_rate = sample_rate;
	encoder->inherit_super.channel_num = channel_num;
	encoder->inherit_super.bitrate = target_bitrate;

	// try to make transform.
	HRESULT hr = S_OK;
	IMFMediaType *inputType;
	IMFMediaType *outputType;
	hr = CoCreateInstance(
		CLSID_AACMFTEncoder,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&encoder->transform));
	if(SUCCEEDED(hr)) {
		hr = MFCreateMediaType(&outputType);
		if(SUCCEEDED(hr)) {
			outputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
			outputType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_AAC);
			outputType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
			outputType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, sample_rate);
			outputType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, channel_num);
			outputType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, target_bitrate / 8);
			encoder->transform->SetOutputType(0, outputType, 0);
		}
	}
	ReleaseOnExit roeOutputType(outputType);
	if(SUCCEEDED(hr)) {
		hr = MFCreateMediaType(&inputType);
		if(SUCCEEDED(hr)) {
			inputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
			inputType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
			inputType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
			inputType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, sample_rate);
			inputType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, channel_num);
			encoder->transform->SetInputType(0, inputType, 0);
		}
	}
	ReleaseOnExit roeInputType(inputType);
	if(SUCCEEDED(hr)) {
		hr = encoder->transform->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, NULL);
	}
	if(SUCCEEDED(hr)) {
		hr = encoder->transform->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, NULL);
	}
	if(SUCCEEDED(hr)) {
		// done ok.
		return (ttLibC_MsAacEncoder *)encoder;
	}
	else {
		ttLibC_MsAacEncoder_close((ttLibC_MsAacEncoder **)&encoder);
		return NULL;
	}
}

bool ttLibC_MsAacEncoder_encode(
		ttLibC_MsAacEncoder *encoder,
		ttLibC_PcmS16 *pcm,
		ttLibC_MsAacEncodeFunc callback,
		void *ptr) {
	HRESULT hr = S_OK;
	if(pcm == NULL) {
		return true;
	}
	ttLibC_MsAacEncoder_ *encoder_ = (ttLibC_MsAacEncoder_ *)encoder;
	if(encoder_ == NULL) {
		return false;
	}
	if(encoder_->aac == NULL) {
		// need to make dsi information.
		uint8_t dsiBuffer[8];
		size_t dsiSize = ttLibC_Aac_getDsiInfo(
			AacObject_Low,
			encoder_->sample_rate,
			encoder_->channel_num,
			(void *)dsiBuffer,
			8);
		if(dsiSize == 0) {
			return false;
		}
		encoder_->aac = ttLibC_Aac_getFrame(
			NULL,
			dsiBuffer,
			dsiSize,
			true,
			0,
			10000);
		if(encoder_->aac == NULL) {
			return false;
		}
		encoder_->aac->inherit_super.inherit_super.id = pcm->inherit_super.inherit_super.id;
		if(callback != NULL) {
			if(!callback(ptr, encoder_->aac)) {
				return false;
			}
		}
	}
	{
		// do input
		IMFSample *sample = NULL;
		IMFMediaBuffer *buffer = NULL;
		BYTE *bufferData;
		// timebase should be 100 nsec.
		uint64_t pts = pcm->inherit_super.inherit_super.pts * 10000 / pcm->inherit_super.inherit_super.timebase;
		uint32_t duration = pcm->inherit_super.sample_num * 10000 / pcm->inherit_super.sample_rate;
		// CreateEmptySample
		hr = MFCreateSample(&sample);
		ReleaseOnExit roeSample(sample);
		if(SUCCEEDED(hr)) {
			hr = MFCreateMemoryBuffer(pcm->inherit_super.inherit_super.buffer_size, &buffer);
		}
		ReleaseOnExit rowBuffer(buffer);
		if(SUCCEEDED(hr)) {
			hr = sample->AddBuffer(buffer);
		}
		// try to write pcm buffer.
		if(SUCCEEDED(hr)) {
			hr = buffer->Lock(&bufferData, NULL, NULL);
			if(SUCCEEDED(hr)) {
				memcpy(bufferData, pcm->l_data, pcm->inherit_super.inherit_super.buffer_size);
				hr = buffer->Unlock();
			}
			if(SUCCEEDED(hr)) {
				hr = buffer->SetCurrentLength(pcm->inherit_super.inherit_super.buffer_size);
			}
		}
		// try to set pts
		if(SUCCEEDED(hr)) {
			hr = sample->SetSampleTime(pts);
		}
		// try to set duration
		if(SUCCEEDED(hr)) {
			hr = sample->SetSampleDuration(duration);
		}
		if(SUCCEEDED(hr)) {
			hr = encoder_->transform->ProcessInput(0, sample, 0);
		}
		// done.
		if(FAILED(hr)) {
			return false;
		}
	}
  while(true)
	{
		// try to get output aac frame.
		DWORD outputFlags, outputStatus;
		MFT_OUTPUT_STREAM_INFO outputInfo = {0};

		MFT_OUTPUT_DATA_BUFFER output = {0};
		IMFSample *sample = NULL;
		IMFMediaBuffer *buffer = NULL;
		BYTE *bufferData;
		DWORD bufferLength;
		int64_t samplePts;
		if(SUCCEEDED(hr)) {
			hr = encoder_->transform->GetOutputStatus(&outputFlags);
			if(SUCCEEDED(hr)) {
				if(outputFlags != MFT_OUTPUT_STATUS_SAMPLE_READY) {
					// no more data. try next time.
					return true;
				}
			}
			else {
				ERR_PRINT("failed to get output status:%x", hr);
				return false;
			}
		}
		if(SUCCEEDED(hr)) {
			hr = encoder_->transform->GetOutputStreamInfo(0, &outputInfo);
		}
		// try to make enough size of output buffer.
		if(SUCCEEDED(hr)) {
			hr = MFCreateSample(&sample);
		}
		ReleaseOnExit roeSample(sample);
		if(SUCCEEDED(hr)) {
			hr = MFCreateMemoryBuffer(outputInfo.cbSize, &buffer);
		}
		ReleaseOnExit rowBuffer(buffer);
		if(SUCCEEDED(hr)) {
			hr = sample->AddBuffer(buffer);
		}
		output.pSample = sample;
		if(SUCCEEDED(hr)) {
			hr = encoder_->transform->ProcessOutput(0, 1, &output, &outputStatus);
		}
/*		if(hr == MF_E_TRANSFORM_NEED_MORE_INPUT) {
			return true;
		}*/
		if(SUCCEEDED(hr)) {
			hr = sample->GetSampleTime(&samplePts);
		}
		if(SUCCEEDED(hr)) {
			hr = buffer->Lock(&bufferData, NULL, &bufferLength);
			if(SUCCEEDED(hr)) {
				// try to make and do callback.
				ttLibC_Aac *aac = ttLibC_Aac_getFrame(
					encoder_->aac,
					bufferData,
					bufferLength,
					true,
					samplePts,
					10000);
				if(aac == NULL) {
					buffer->Unlock();
					return false;
				}
				aac->inherit_super.inherit_super.id = pcm->inherit_super.inherit_super.id;
				encoder_->aac = aac;
				if(callback != NULL) {
					if(!callback(ptr, encoder_->aac)) {
						hr = -1;
					}
				}
				buffer->Unlock();
			}
		}
		if(FAILED(hr)) {
			return false;
		}
	}
	return SUCCEEDED(hr);
}

void ttLibC_MsAacEncoder_close(ttLibC_MsAacEncoder **encoder) {
	ttLibC_MsAacEncoder_ *target = (ttLibC_MsAacEncoder_ *)*encoder;
	if(target == NULL) {
		return;
	}
	if(target->transform != NULL) {
		target->transform->Release();
		target->transform = NULL;
	}
	ttLibC_Aac_close(&target->aac);
	ttLibC_free(target);
	*encoder = NULL;
}

#endif
