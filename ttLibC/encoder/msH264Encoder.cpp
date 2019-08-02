/**
 * @file   msH264Encoder.cpp
 * @brief  windows native h264 encoder.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2017/05/13
 */
#ifdef __ENABLE_WIN32__

#include "MSH264Encoder.h"
#include "../_log.h"
#include "../allocator.h"
#include "../util/hexUtil.h"

#include <windows.h>
#include <mfapi.h>
#include <Mfidl.h>
#include <shlwapi.h>
#include <Codecapi.h>
#include <comdef.h>
#include <mferror.h>
#include <mftransform.h>
#include <wmcodecdsp.h>
#include <locale.h>
#include <thread>
#include <mutex>
#include <vector>

#include "../util/msGlobalUtil.h"

#pragma comment(lib, "mf")
#pragma comment(lib, "mfuuid")
#pragma comment(lib, "d3d9")
#pragma comment(lib, "dxva2")
#pragma comment(lib, "uuid")
#pragma comment(lib, "wmcodecdspuuid")
#pragma comment(lib, "Shlwapi")

// ringBuffer to supply yuv data.
class MsH264Encoder_RingBuffer {
public:
	MsH264Encoder_RingBuffer(uint32_t width, uint32_t height) {
		size_  = 16;
		getPtr_ = 0;
		setPtr_ = 0;
		samples_ = new IMFSample*[size_];
		uint32_t imageSize;
		MFCalculateImageSize(
			MFVideoFormat_NV12,
			width,
			height,
			&imageSize);
		for (int i = 0; i < size_; ++i) {
			IMFSample* sample;
			IMFMediaBuffer* buffer;
			MFCreateSample(&sample);
			MFCreateMemoryBuffer(imageSize, &buffer);
			sample->AddBuffer(buffer);
			samples_[i] = sample;
		}
	}
	~MsH264Encoder_RingBuffer() {
		for (int i = 0; i < size_; ++i) {
			IMFSample *sample = samples_[i];
			IMFMediaBuffer* buffer;
			sample->GetBufferByIndex(0, &buffer);
			if (buffer != NULL) {
				buffer->Release();
				buffer = NULL;
			}
			sample->Release();
			sample = NULL;
		}
		delete[] samples_;
	}
	// QÆµÄ«o·ÌÆAQÆµÄÇÝÞ
	// ÅAÌf[^ÉÚ®·éÌªÙµ¢í¯©EEE
	// «oµæ€Ì|C^ÆAÇÝÝpÌ|C^ª éKvª éEEEÆ
	IMFSample* refNextSample() {
		mutex_.lock();
		IMFSample* sample = samples_[setPtr_];
		setPtr_ = (setPtr_ + 1) % size_;
		mutex_.unlock();
		return sample;
	}
	IMFSample* popReadySample() {
		mutex_.lock();
		if (isEmpty()) {
			mutex_.unlock();
			return NULL;
		}
		IMFSample* sample = samples_[getPtr_];
		getPtr_ = (getPtr_ + 1) % size_;
		mutex_.unlock();
		return sample;
	}
	bool isEmpty() {
		return getPtr_ == setPtr_;
	}
private:
	IMFSample** samples_;
	std::mutex mutex_;
	int size_;
	int getPtr_;
	int setPtr_;
};

class EncoderEventCallback;

typedef struct ttLibC_Encoder_MsH264Encoder_ {
	ttLibC_MsH264Encoder inherit_super;

	IMFTransform *transform;
	ICodecAPI* codecApi;
	IMFMediaEventGenerator *pGenerator;

	ttLibC_H264 *h264;
	uint32_t width;
	uint32_t height;
	uint32_t bitrate;
	uint32_t fps;

	MsH264Encoder_RingBuffer *ringBuffer;
	EncoderEventCallback *eventCallback;
	bool is_async;
	bool has_sample;
	bool is_first;

	ttLibC_MsH264EncodeFunc callback;
	void *ptr;
	uint32_t id;
} ttLibC_Encoder_MsH264Encoder_;

typedef ttLibC_Encoder_MsH264Encoder_ ttLibC_MsH264Encoder_;

static void MsH264Encoder_drainH264(ttLibC_MsH264Encoder_ *encoder) {
	HRESULT hr = S_OK;
	MFT_OUTPUT_DATA_BUFFER outDataBuffer;
	MFT_OUTPUT_STREAM_INFO outputInfo = { 0 };
	DWORD status;
	outDataBuffer.dwStatus = 0;
	outDataBuffer.dwStreamID = 0;
	outDataBuffer.pEvents = 0;
	IMFSample *sample = NULL;
	IMFMediaBuffer *buffer = NULL;
	if(encoder->has_sample) {
		outDataBuffer.pSample = NULL;
	}
	else {
		encoder->transform->GetOutputStreamInfo(0, &outputInfo);
		MFCreateSample(&sample);
		MFCreateMemoryBuffer(outputInfo.cbSize, &buffer);
		sample->AddBuffer(buffer);
		outDataBuffer.pSample = sample;
	}
	ReleaseOnExit roeSample(sample);
	ReleaseOnExit roeBuffer(buffer);
	hr = encoder->transform->ProcessOutput(0, 1, &outDataBuffer, &status);
	ReleaseOnExit roeEvents(outDataBuffer.pEvents);
	if(hr == MF_E_TRANSFORM_NEED_MORE_INPUT) {
		return;
	}
	if(SUCCEEDED(hr)) {
		// ok
		IMFMediaBuffer *pOutBuffer = NULL;
		outDataBuffer.pSample->GetBufferByIndex(0, &pOutBuffer);
		ReleaseOnExit roeOutDataBuffer(pOutBuffer);
		DWORD totalLength = 0;
		outDataBuffer.pSample->GetTotalLength(&totalLength);
		DWORD currentLength = 0;
		pOutBuffer->GetCurrentLength(&currentLength); // doesn't make any difference.
		int64_t samplePts;
		outDataBuffer.pSample->GetSampleTime(&samplePts);
		BYTE *pData;
		pOutBuffer->Lock(&pData, NULL, &totalLength);
		uint8_t *buffer = pData;
		size_t left_size = currentLength;
		do {
			ttLibC_H264 *h264 = ttLibC_H264_getFrame(
				(ttLibC_H264 *)encoder->h264,
				buffer,
				left_size,
				true,
				samplePts,
				10000);
			if(h264 == NULL) {
				puts("error failed to make h264 frame.");
				break;
			}
			// ignore about id.
			encoder->h264 = h264;
			encoder->h264->inherit_super.inherit_super.id = encoder->id;
			if(encoder->callback != NULL) {
				if(!encoder->callback(encoder->ptr, encoder->h264)) {
					puts("errored with callback.");
					break;
				}
			}
			buffer += h264->inherit_super.inherit_super.buffer_size;
			left_size -= h264->inherit_super.inherit_super.buffer_size;
		} while(left_size > 0);
		pOutBuffer->Unlock();
	}
	if(FAILED(hr)) {
		if(hr == MF_E_TRANSFORM_STREAM_CHANGE) {
			// need to update outputType.
			IMFMediaType *outputType;
			hr = MFCreateMediaType(&outputType);
			ReleaseOnExit roeOutputType(outputType);
			if(SUCCEEDED(hr)) {
				outputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
				outputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
//				MFSetAttributeRatio(outputType, MF_MT_FRAME_RATE, 15, 1);
				MFSetAttributeSize(outputType, MF_MT_FRAME_SIZE, encoder->width, encoder->height);
				outputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlaceMode::MFVideoInterlace_Progressive);
				MFSetAttributeRatio(outputType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
				outputType->SetUINT32(MF_MT_AVG_BITRATE, encoder->bitrate);

				hr = encoder->transform->SetOutputType(0, outputType, 0);
			}
			hr = encoder->transform->SetOutputType(0, outputType, 0);
			if(FAILED(hr)) {
				ERR_PRINT("failed to update outputType.");
			}
		}
		else {
			ERR_PRINT("something wrong.%x %x", hr, status);
		}
	}
}

class EncoderEventCallback : public IMFAsyncCallback {
public:
	EncoderEventCallback(ttLibC_MsH264Encoder_ *encoder) : encoder_(encoder) {
	}
	virtual ~EncoderEventCallback() {
	}
	STDMETHODIMP QueryInterface(REFIID _riid, void ** pp_v) {
		static const QITAB _qit[] =
		{
			QITABENT(EncoderEventCallback, IMFAsyncCallback), {0}
		};
		return QISearch(this, _qit, _riid, pp_v);
	}
	STDMETHODIMP_(ULONG) AddRef() {
		return InterlockedIncrement(&ref_count);
	}
	STDMETHODIMP_(ULONG) Release() {
		long result = InterlockedDecrement(&ref_count);
		if(result == 0) {
			delete this;
		}

		return result;
	}
	STDMETHODIMP GetParameters(DWORD* p_flags, DWORD* p_queue) {
		return E_NOTIMPL;
	}
	STDMETHODIMP Invoke(IMFAsyncResult* pAsyncResult) {
		IMFMediaEvent *pMediaEvent = NULL;
		MediaEventType evType = MEUnknown;
		HRESULT hr = S_OK;
		IMFSample *pSample = NULL;
		int res = -1;

		encoder_->pGenerator->EndGetEvent(pAsyncResult, &pMediaEvent);
		pMediaEvent->GetType(&evType);
		pMediaEvent->GetStatus(&hr);
		switch(evType) {
			case METransformNeedInput:
				{
					IMFSample *sample = encoder_->ringBuffer->popReadySample();
					if(sample == NULL) {
						// if NULL, sleep and do again later.
						std::this_thread::sleep_for(std::chrono::milliseconds(1000));
					}
					else {
						encoder_->transform->ProcessInput(0, sample, 0); // put NULL, try again.
					}
				}
				break;
			case METransformHaveOutput:
				{
				// we need to make callback. make later.
					MsH264Encoder_drainH264(encoder_);
				}
				break;
			case MF_E_TRANSFORM_STREAM_CHANGE:
				ERR_PRINT("stream changed.");
				break;
			default:
				ERR_PRINT("unknown");
				break;
		}
		pMediaEvent->Release();
		if(!encoder_->ringBuffer->isEmpty()) {
			encoder_->pGenerator->BeginGetEvent(this, NULL);
		}
		return S_OK;
	}
private:
	ttLibC_MsH264Encoder_ *encoder_;
	long ref_count; // ±êÍú»µÄÍ¢¯È¢çµ¢B
};

bool TT_ATTRIBUTE_API ttLibC_MsH264Encoder_listEncoders(
		ttLibC_MsH264EncodeNameFunc callback,
		void *ptr) {
	HRESULT hr = S_OK;

	uint32_t count = 0;
	IMFActivate **ppActivate = NULL;
	MFT_REGISTER_TYPE_INFO info = {0};
	info.guidMajorType = MFMediaType_Video;
	info.guidSubtype = MFVideoFormat_H264;

	hr = MFTEnumEx(
			MFT_CATEGORY_VIDEO_ENCODER,
			MFT_ENUM_FLAG_SYNCMFT | MFT_ENUM_FLAG_LOCALMFT | MFT_ENUM_FLAG_SORTANDFILTER | MFT_ENUM_FLAG_ASYNCMFT | MFT_ENUM_FLAG_HARDWARE,
			NULL,
			&info,
			&ppActivate,
			&count);
	CoTaskMemFreeOnExit ctmfoeActivate(ppActivate);
	if(SUCCEEDED(hr)) {
		for(uint32_t i = 0;SUCCEEDED(hr) && i < count; ++ i) {
			LPWSTR pszName = NULL;
			uint32_t nameLength;
			hr = ppActivate[i]->GetAllocatedString(MFT_FRIENDLY_NAME_Attribute, &pszName, &nameLength);
			CoTaskMemFreeOnExit ctmfoePszName(pszName);
			if(SUCCEEDED(hr)) {
				if(callback != NULL) {
					// do callback
					if(!callback(ptr, ttLibC_MsGlobal_wcharToUtf8string(pszName).c_str())) {
						hr = E_ABORT;
					}
				}
			}
		}
	}
	return SUCCEEDED(hr);
}

ttLibC_MsH264Encoder TT_ATTRIBUTE_API *ttLibC_MsH264Encoder_make(
		const char *target,
		uint32_t width,
		uint32_t height,
		uint32_t bitrate) {
	ttLibC_MsH264Encoder_ *encoder = (ttLibC_MsH264Encoder_ *)ttLibC_malloc(sizeof(ttLibC_MsH264Encoder_));
	if(encoder == NULL) {
		return NULL;
	}
	memset(encoder, 0, sizeof(ttLibC_MsH264Encoder_));
	encoder->transform = NULL;
	encoder->h264 = NULL;
	encoder->width = width;
	encoder->height = height;
	encoder->bitrate = bitrate;
	encoder->fps = 15;
	encoder->is_async  = false;
	encoder->is_first = true;

	HRESULT hr = S_OK;
	// need to put inherit information.

	// use MFTEnumEx find encoder.
	{
		uint32_t count = 0;
		IMFActivate **ppActivate = NULL;
		MFT_REGISTER_TYPE_INFO info = {0};

		// search for h264Encoder.
		info.guidMajorType = MFMediaType_Video;
		info.guidSubtype = MFVideoFormat_H264;
		hr = MFTEnumEx(
				MFT_CATEGORY_VIDEO_ENCODER,
				MFT_ENUM_FLAG_SYNCMFT | MFT_ENUM_FLAG_LOCALMFT | MFT_ENUM_FLAG_SORTANDFILTER | MFT_ENUM_FLAG_ASYNCMFT | MFT_ENUM_FLAG_HARDWARE,
				NULL,
				&info,
				&ppActivate,
				&count);
		CoTaskMemFreeOnExit ctmfoeActivate(ppActivate);
		if(SUCCEEDED(hr)) {
			for(uint32_t i = 0;SUCCEEDED(hr) && i < count; ++ i) {
				LPWSTR pszName = NULL;
				uint32_t nameLength;
				hr = ppActivate[i]->GetAllocatedString(MFT_FRIENDLY_NAME_Attribute, &pszName, &nameLength);
				CoTaskMemFreeOnExit ctmfoePszName(pszName);
				if(SUCCEEDED(hr)) {
					if(strcmp(ttLibC_MsGlobal_wcharToUtf8string(pszName).c_str(), target) == 0) {
						// found.
						hr = ppActivate[0]->ActivateObject(IID_PPV_ARGS(&encoder->transform));
						break;
					}
				}
			}
		}
		if(encoder->transform == NULL) {
			hr = E_ABORT;
		}
	}

	// check async or not.
	if(SUCCEEDED(hr)) {
		IMFAttributes *pAttributes;
		uint32_t isAsync = 0;
		hr = encoder->transform->GetAttributes(&pAttributes);
		ReleaseOnExit roeAttributes(pAttributes);
		if(SUCCEEDED(hr)) {
			hr = pAttributes->GetUINT32(MF_TRANSFORM_ASYNC, &isAsync);
		}
		encoder->is_async = (isAsync == 1);
		if(encoder->is_async && SUCCEEDED(hr)) {
			hr = pAttributes->SetUINT32(MF_TRANSFORM_ASYNC_UNLOCK, TRUE);
		}
	}

	// set outputType
	if(SUCCEEDED(hr)) {
		IMFMediaType *outputType;
		hr = MFCreateMediaType(&outputType);
		ReleaseOnExit roeOutputType(outputType);
		if(SUCCEEDED(hr)) {
			outputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
			outputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
			MFSetAttributeRatio(outputType, MF_MT_FRAME_RATE, 15, 1);
			MFSetAttributeSize(outputType, MF_MT_FRAME_SIZE, width, height);
			outputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlaceMode::MFVideoInterlace_Progressive);
			MFSetAttributeRatio(outputType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
			outputType->SetUINT32(MF_MT_AVG_BITRATE, bitrate);
			hr = encoder->transform->SetOutputType(0, outputType, 0);
		}
	}
	// set inputType
	if(SUCCEEDED(hr)) {
		IMFMediaType *inputType;
		hr = MFCreateMediaType(&inputType);
		ReleaseOnExit roeInputType(inputType);
		if(SUCCEEDED(hr)) {
			inputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
			inputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12);
			MFSetAttributeRatio(inputType, MF_MT_FRAME_RATE, 15, 1);
			MFSetAttributeSize(inputType, MF_MT_FRAME_SIZE, width, height);
			inputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlaceMode::MFVideoInterlace_Progressive);
			MFSetAttributeRatio(inputType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
			hr = encoder->transform->SetInputType(0, inputType, 0);
		}
	}
	// make event generator for async works.
	if(SUCCEEDED(hr)) {
		if(encoder->is_async) {
			encoder->ringBuffer = new MsH264Encoder_RingBuffer(width, height);
			hr = encoder->transform->QueryInterface(IID_PPV_ARGS(&encoder->pGenerator));
			if(SUCCEEDED(hr)) {
				encoder->eventCallback = new EncoderEventCallback(encoder);
				encoder->pGenerator->BeginGetEvent(encoder->eventCallback, NULL);
			}
		}
	}
	// get codecAPI for setting more detail.
	if (SUCCEEDED(hr)) {
		hr = encoder->transform->QueryInterface(&encoder->codecApi);
		// bitrate
		{
			VARIANT v;
			v.vt = VT_UI4;
			v.ulVal = (uint32_t)640000;
			encoder->codecApi->SetValue(&CODECAPI_AVEncCommonMeanBitRate, &v);
		}
	}

	// signal begin stream and start of stream.
	if(SUCCEEDED(hr)) {
		hr = encoder->transform->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, NULL);
	}
	if(SUCCEEDED(hr)) {
		if(!encoder->is_async) {
			hr = encoder->transform->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, NULL);
		}
	}
	if(SUCCEEDED(hr)) {
		MFT_OUTPUT_STREAM_INFO streamInfo = {0};
		hr = encoder->transform->GetOutputStreamInfo(0, &streamInfo);
		if(SUCCEEDED(hr)) {
			encoder->has_sample = (streamInfo.dwFlags &
							(MFT_OUTPUT_STREAM_PROVIDES_SAMPLES |
							 MFT_OUTPUT_STREAM_CAN_PROVIDE_SAMPLES));
		}
	}
	if(SUCCEEDED(hr)) {
		return (ttLibC_MsH264Encoder *)encoder;
	}
	else {
		ttLibC_MsH264Encoder_close((ttLibC_MsH264Encoder **)&encoder);
		return NULL;
	}
}

// h264EncoderðparamÌlÉµœªÁÄú»³¹éB
ttLibC_MsH264Encoder TT_ATTRIBUTE_API *ttLibC_MsH264Encoder_makeWithParam(
	const char *target,
	ttLibC_MsH264Encoder_param *param) {
	ttLibC_MsH264Encoder_ *encoder = (ttLibC_MsH264Encoder_ *)ttLibC_malloc(sizeof(ttLibC_MsH264Encoder_));
	if(encoder == NULL) {
		return NULL;
	}
	memset(encoder, 0, sizeof(ttLibC_MsH264Encoder_));
	encoder->transform = NULL;
	encoder->h264      = NULL;
	encoder->width     = param->width;
	encoder->height    = param->height;
	encoder->bitrate   = param->bitrate;
	encoder->fps       = param->fps;
	encoder->is_async  = false;
	encoder->is_first  = true;
	HRESULT hr = S_OK;

	// ÜžÍencoderð©Â¯éB
	if(SUCCEEDED(hr)) {
		uint32_t count = 0;
		IMFActivate **ppActivate = NULL;
		MFT_REGISTER_TYPE_INFO info = {0};

		info.guidMajorType = MFMediaType_Video;
		info.guidSubtype = MFVideoFormat_H264;
		hr = MFTEnumEx(
				MFT_CATEGORY_VIDEO_ENCODER,
				MFT_ENUM_FLAG_SYNCMFT | MFT_ENUM_FLAG_LOCALMFT | MFT_ENUM_FLAG_SORTANDFILTER | MFT_ENUM_FLAG_ASYNCMFT | MFT_ENUM_FLAG_HARDWARE,
				NULL,
				&info,
				&ppActivate,
				&count);
		CoTaskMemFreeOnExit ctmfoeActivate(ppActivate);
		if(SUCCEEDED(hr)) {
			for(uint32_t i = 0;SUCCEEDED(hr) && i < count; ++ i) {
				LPWSTR pszName = NULL;
				uint32_t nameLength;
				hr = ppActivate[i]->GetAllocatedString(MFT_FRIENDLY_NAME_Attribute, &pszName, &nameLength);
				CoTaskMemFreeOnExit ctmfoePszName(pszName);
				if(SUCCEEDED(hr)) {
					if(strcmp(ttLibC_MsGlobal_wcharToUtf8string(pszName).c_str(), target) == 0) {
						// found.
						hr = ppActivate[0]->ActivateObject(IID_PPV_ARGS(&encoder->transform));
						break;
					}
				}
			}
		}
		if(encoder->transform == NULL) {
			hr = E_ABORT;
		}
	}
	// Éasync®ìÅ é©»èµÄš­
	if(SUCCEEDED(hr)) {
		IMFAttributes *pAttributes;
		uint32_t isAsync = 0;
		hr = encoder->transform->GetAttributes(&pAttributes);
		ReleaseOnExit roeAttributes(pAttributes);
		if(SUCCEEDED(hr)) {
			hr = pAttributes->GetUINT32(MF_TRANSFORM_ASYNC, &isAsync);
		}
		encoder->is_async = (isAsync == 1);
		if(encoder->is_async && SUCCEEDED(hr)) {
			hr = pAttributes->SetUINT32(MF_TRANSFORM_ASYNC_UNLOCK, TRUE);
		}
	}
	// oÍf[^ðÝè
	if(SUCCEEDED(hr)) {
		IMFMediaType *outputType;
		hr = MFCreateMediaType(&outputType);
		ReleaseOnExit roeOutputType(outputType);
		if(SUCCEEDED(hr)) {
			outputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
			outputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
			MFSetAttributeRatio(outputType, MF_MT_FRAME_RATE, param->fps, 1);
			MFSetAttributeSize(outputType, MF_MT_FRAME_SIZE, param->width, param->height);
			outputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlaceMode::MFVideoInterlace_Progressive);
			MFSetAttributeRatio(outputType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
			outputType->SetUINT32(MF_MT_AVG_BITRATE, param->bitrate);

			// Level
			outputType->SetUINT32(MF_MT_MPEG2_LEVEL, param->level);
			// Profile
			switch(param->profile) {
			default:
			case MsH264EncoderProfile_Base:
				outputType->SetUINT32(MF_MT_MPEG2_PROFILE, eAVEncH264VProfile_Base);
				break;
			case MsH264EncoderProfile_Main:
				outputType->SetUINT32(MF_MT_MPEG2_PROFILE, eAVEncH264VProfile_Main);
				break;
			case MsH264EncoderProfile_High:
				outputType->SetUINT32(MF_MT_MPEG2_PROFILE, eAVEncH264VProfile_High);
				break;
			}

			hr = encoder->transform->SetOutputType(0, outputType, 0);
		}
	}
	// üÍf[^ðÝè Æè Šžyuv420SemiPlanarÅ
	if(SUCCEEDED(hr)) {
		IMFMediaType *inputType;
		hr = MFCreateMediaType(&inputType);
		ReleaseOnExit roeInputType(inputType);
		if(SUCCEEDED(hr)) {
			inputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
			inputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12);
			MFSetAttributeRatio(inputType, MF_MT_FRAME_RATE, param->fps, 1);
			MFSetAttributeSize(inputType, MF_MT_FRAME_SIZE, param->width, param->height);
			inputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlaceMode::MFVideoInterlace_Progressive);
			MFSetAttributeRatio(inputType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
			hr = encoder->transform->SetInputType(0, inputType, 0);
		}
	}
	// eventGeneratorõ
	if(SUCCEEDED(hr)) {
		if(encoder->is_async) {
			encoder->ringBuffer = new MsH264Encoder_RingBuffer(param->width, param->height);
			hr = encoder->transform->QueryInterface(IID_PPV_ARGS(&encoder->pGenerator));
			if(SUCCEEDED(hr)) {
				encoder->eventCallback = new EncoderEventCallback(encoder);
				encoder->pGenerator->BeginGetEvent(encoder->eventCallback, NULL);
			}
		}
	}
	// detail setting.
	if(SUCCEEDED(hr)) {
		hr = encoder->transform->QueryInterface(&encoder->codecApi);
	}
	// bitrate
	if(SUCCEEDED(hr)) {
		VARIANT v;
		v.vt = VT_UI4;
		v.ulVal = (uint32_t)param->bitrate;
		hr = encoder->codecApi->SetValue(&CODECAPI_AVEncCommonMeanBitRate, &v);
	}
	// maxBitrate
	if(SUCCEEDED(hr)) {
		VARIANT v;
		v.vt = VT_UI4;
		v.ullVal = (uint32_t)param->maxBitrate;
		hr = encoder->codecApi->SetValue(&CODECAPI_AVEncCommonMaxBitRate, &v);
	}
	// minQp
	if(SUCCEEDED(hr)) {
		VARIANT v;
		v.vt = VT_UI4;
		v.ullVal = (uint32_t)param->minQp;
		hr = encoder->codecApi->SetValue(&CODECAPI_AVEncVideoMinQP, &v);
	}
	// maxQp
	if(SUCCEEDED(hr)) {
		VARIANT v;
		v.vt = VT_UI4;
		v.ullVal = (uint32_t)param->maxQp;
		hr = encoder->codecApi->SetValue(&CODECAPI_AVEncVideoMaxQP, &v);
	}
	// rateType
	if(SUCCEEDED(hr)) {
		VARIANT v;
		v.vt = VT_UI4;
		switch(param->rateType) {
		case MsH264EncoderRateType_CBR:
		default:
			v.ullVal = (uint32_t)eAVEncCommonRateControlMode_CBR;
			break;
		case MsH264EncoderRateType_ConstraintVBR:
			v.ullVal = (uint32_t)eAVEncCommonRateControlMode_PeakConstrainedVBR;
			break;
		case MsH264EncoderRateType_VBR:
			v.ullVal = (uint32_t)eAVEncCommonRateControlMode_UnconstrainedVBR;
			break;
		case MsH264EncoderRateType_CQP:
			v.ullVal = (uint32_t)eAVEncCommonRateControlMode_Quality;
			break;
		}
		hr = encoder->codecApi->SetValue(&CODECAPI_AVEncCommonRateControlMode, &v);
	}
	// GOP
	if(SUCCEEDED(hr)) {
		VARIANT v;
		v.vt = VT_UI4;
		v.ullVal = (uint32_t)param->GOP;
		hr = encoder->codecApi->SetValue(&CODECAPI_AVEncMPVGOPSize, &v);
	}
	// lowLatency
	if(SUCCEEDED(hr)) {
		VARIANT v;
		v.vt = VT_BOOL;
		v.ullVal = param->useLowLatency ? VARIANT_TRUE : VARIANT_FALSE;
		hr = encoder->codecApi->SetValue(&CODECAPI_AVEncCommonMaxBitRate, &v);
	}
	// bufferSize
	if(SUCCEEDED(hr)) {
		VARIANT v;
		v.vt = VT_UI4;
		v.ullVal = (uint32_t)param->bufferSize;
		hr = encoder->codecApi->SetValue(&CODECAPI_AVEncCommonBufferSize, &v);
	}
	// bFrameCount
	if(SUCCEEDED(hr)) {
		VARIANT v;
		v.vt = VT_UI4;
		v.ullVal = (uint32_t)param->bFrameCount;
		/*hr = */encoder->codecApi->SetValue(&CODECAPI_AVEncMPVDefaultBPictureCount, &v);
	}
	// EntropyEncoding
	if(SUCCEEDED(hr)) {
		VARIANT v;
		v.vt = VT_BOOL;
		v.ullVal = param->useCabac ? VARIANT_TRUE : VARIANT_FALSE;
		hr = encoder->codecApi->SetValue(&CODECAPI_AVEncH264CABACEnable, &v);
	}
	// signal begin stream and start of stream.
	if(SUCCEEDED(hr)) {
		hr = encoder->transform->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, NULL);
	}
	if(SUCCEEDED(hr)) {
		if(!encoder->is_async) {
			hr = encoder->transform->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, NULL);
		}
	}
	if(SUCCEEDED(hr)) {
		return (ttLibC_MsH264Encoder *)encoder;
	}
	else {
		puts("failed to initialize h264Encoder");
		ttLibC_MsH264Encoder_close((ttLibC_MsH264Encoder **)&encoder);
		return NULL;
	}
}


bool TT_ATTRIBUTE_API ttLibC_MsH264Encoder_encode(
		ttLibC_MsH264Encoder *encoder,
		ttLibC_Yuv420 *frame,
		ttLibC_MsH264EncodeFunc callback,
		void *ptr) {
	ttLibC_MsH264Encoder_ *encoder_ = (ttLibC_MsH264Encoder_ *)encoder;
	if(encoder_ == NULL) {
		return false;
	}
	if(frame == NULL) {
		return true;
	}
	encoder_->id = frame->inherit_super.inherit_super.id;
	encoder_->callback = callback;
	encoder_->ptr = ptr;
	if(encoder_->is_async) {
		IMFSample *sample = encoder_->ringBuffer->refNextSample();
		IMFMediaBuffer* buffer;
		sample->GetBufferByIndex(0, &buffer);
		if (buffer == NULL) {
			return false;
		}
		BYTE* bufferData;
		HRESULT hr = buffer->Lock(&bufferData, NULL, NULL);
		if (SUCCEEDED(hr)) {
			memcpy(bufferData, frame->inherit_super.inherit_super.data, frame->inherit_super.inherit_super.buffer_size);
			hr = buffer->Unlock();
		}
		if (SUCCEEDED(hr)) {
			hr = buffer->SetCurrentLength((DWORD)frame->inherit_super.inherit_super.buffer_size);
		}
		// timebase = 10000
		uint64_t pts = frame->inherit_super.inherit_super.pts * 10000 / frame->inherit_super.inherit_super.timebase;
		sample->SetSampleTime(pts);
		sample->SetSampleDuration(100);
		if(encoder_->is_first) {
			hr = encoder_->transform->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, NULL);
			encoder_->is_first = false;
		}
		else {
			encoder_->pGenerator->BeginGetEvent(encoder_->eventCallback, NULL);
		}
		return SUCCEEDED(hr);
	}
	else {
		// make IMFSample from yuv.
		uint32_t imageSize;
		MFCalculateImageSize(
			MFVideoFormat_NV12,
			frame->inherit_super.width,
			frame->inherit_super.height,
			&imageSize);
		IMFSample* sample;
		IMFMediaBuffer* buffer;
		MFCreateSample(&sample);
		MFCreateMemoryBuffer(imageSize, &buffer);
		sample->AddBuffer(buffer);

		// we need to copy plane here.
		BYTE* bufferData;
		HRESULT hr = buffer->Lock(&bufferData, NULL, NULL);
		if (SUCCEEDED(hr)) {
			memcpy(bufferData, frame->inherit_super.inherit_super.data, frame->inherit_super.inherit_super.buffer_size);
			hr = buffer->Unlock();
		}
		if (SUCCEEDED(hr)) {
			hr = buffer->SetCurrentLength((DWORD)frame->inherit_super.inherit_super.buffer_size);
		}
		// timebase = 10000
		uint64_t pts = frame->inherit_super.inherit_super.pts * 10000 / frame->inherit_super.inherit_super.timebase;
		sample->SetSampleTime(pts);
		sample->SetSampleDuration(100);

		encoder_->transform->ProcessInput(0, sample, 0);
		ReleaseOnExit roeSample(sample);
		ReleaseOnExit roeBuffer(buffer);

		MsH264Encoder_drainH264(encoder_);
		return SUCCEEDED(hr);
	}
}

void TT_ATTRIBUTE_API ttLibC_MsH264Encoder_close(ttLibC_MsH264Encoder **encoder) {
	ttLibC_MsH264Encoder_ *target = (ttLibC_MsH264Encoder_ *)*encoder;
	if(target == NULL) {
		return;
	}
	if(target->pGenerator != NULL) {
		target->pGenerator->Release();
		target->pGenerator = NULL;
	}
	if(target->transform != NULL) {
		target->transform->Release();
		target->transform = NULL;
	}
	if(target->ringBuffer != NULL) {
		delete target->ringBuffer;
		target->ringBuffer = NULL;
	}
	if(target->eventCallback != NULL) {
		delete target->eventCallback;
		target->eventCallback = NULL;
	}
	ttLibC_H264_close(&target->h264);
	ttLibC_free(target);
	*encoder = NULL;
}

#endif
