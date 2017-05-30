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
#include <ttLibC/_log.h>
#include <ttLibC/allocator.h>
#include <ttLibC/util/hexUtil.h>

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
	MsH264Encoder_RingBuffer() {
		size_  = 256;
		start_ = 0;
		end_   = 0;

		data_ = std::vector<IMFSample *>(size_);
	}
	IMFSample *popSample() {
		mutex_.lock();
		while(isEmpty()) {
			mutex_.unlock();
			return NULL;
		}
		IMFSample *sample = data_[start_];
		data_[start_] = NULL;
		start_ = (start_ + 1) % size_;
		mutex_.unlock();
		return sample;
	}
	IMFSample *refSample() {
		mutex_.lock();
		while(isEmpty()) {
			mutex_.unlock();
			return NULL;
		}
		IMFSample *sample = data_[start_];
		mutex_.unlock();
		return sample;
	}
	void dropNext() {
		mutex_.lock();
		while(isEmpty()) {
			mutex_.unlock();
			return;
		}
		// try to release.
		IMFSample *sample = data_[start_];
		IMFMediaBuffer *buffer = NULL;
		sample->GetBufferByIndex(0, &buffer);
		if(buffer != NULL) {
			buffer->Release();
			buffer = NULL;
		}
		if(sample != NULL) {
			sample->Release();
			sample = NULL;
		}
		data_[start_] = NULL;
		start_ = (start_ + 1) % size_;
		mutex_.unlock();
	}
	void pushSample(IMFSample *sample) {
		mutex_.lock();
		if(isFull()) {
			start_ = (start_ + 1) & size_;
			ERR_PRINT("Dropping frame from buffer.");
		}
		int newEnd = (end_ + 1) % size_;
		data_[end_] = sample;
		end_ = newEnd;
		mutex_.unlock();
	}
	bool isFull() {
		return ((end_ + 1) % size_) == start_;
	}
	bool isEmpty() {
		return end_ == start_;
	}
private:
	std::vector<IMFSample *>data_;
	std::mutex mutex_;
	int size_, start_, end_;
};

typedef struct ttLibC_Encoder_MsH264Encoder_ {
	ttLibC_MsH264Encoder inherit_super;
	IMFTransform *transform;
	IMFMediaEventGenerator *pGenerator;
	ttLibC_H264 *h264;
	uint32_t width;
	uint32_t height;
	uint32_t bitrate;

	MsH264Encoder_RingBuffer *ringBuffer;
	bool is_async;
	bool has_sample;
	ttLibC_MsH264EncodeFunc callback;
	void *ptr;
} ttLibC_Encoder_MsH264Encoder_;

typedef ttLibC_Encoder_MsH264Encoder_ ttLibC_MsH264Encoder_;

static void drainH264(ttLibC_MsH264Encoder_ *encoder) {
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
//		DWORD currentLength = 0;
//		pOutBuffer->GetCurrentLength(&currentLength); // doesn't make any difference.
		int64_t samplePts;
		outDataBuffer.pSample->GetSampleTime(&samplePts);
		BYTE *pData;
		pOutBuffer->Lock(&pData, NULL, &totalLength);
		if(totalLength > 30) {
			LOG_DUMP(pData, 30, true);
		}
		else {
			LOG_DUMP(pData, totalLength, true);
		}
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
					IMFSample *sample = encoder_->ringBuffer->popSample();
					encoder_->transform->ProcessInput(0, sample, 0); // put NULL, try again.
					if(sample == NULL) {
						// if NULL, sleep and do again later.
						std::this_thread::sleep_for(std::chrono::milliseconds(1000));
					}
					else {
						IMFMediaBuffer *buffer = NULL;
						sample->GetBufferByIndex(0, &buffer);
						if(buffer != NULL) {
							buffer->Release();
							buffer = NULL;
						}
						sample->Release();
						sample = NULL;
					}
				}
				break;
			case METransformHaveOutput:
				{
					// we need to make callback. make later.
					drainH264(encoder_);
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
		encoder_->pGenerator->BeginGetEvent(this, NULL);
		return S_OK;
	}
private:
	ttLibC_MsH264Encoder_ *encoder_;
	long ref_count;
};

/*
bitrate
qp
maxBitrate
rateControl
keyFrameInterval
lowLatency
bufferSize
bFrame count
EntropyEncoding
MinQp
MaxQp

?
*/

bool ttLibC_MsH264Encoder_listEncoders(
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

ttLibC_MsH264Encoder *ttLibC_MsH264Encoder_make(
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
	encoder->h264      = NULL;
	encoder->width     = width;
	encoder->height    = height;
	encoder->bitrate   = bitrate;
	encoder->is_async  = false;

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
//			MFSetAttributeRatio(outputType, MF_MT_FRAME_RATE, 15, 1);
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
//			MFSetAttributeRatio(inputType, MF_MT_FRAME_RATE, 15, 1);
			MFSetAttributeSize(inputType, MF_MT_FRAME_SIZE, width, height);
			inputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlaceMode::MFVideoInterlace_Progressive);
			MFSetAttributeRatio(inputType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
			hr = encoder->transform->SetInputType(0, inputType, 0);
		}
	}
	// make event generator for async works.
	if(SUCCEEDED(hr)) {
		if(encoder->is_async) {
			encoder->ringBuffer = new MsH264Encoder_RingBuffer();
			hr = encoder->transform->QueryInterface(IID_PPV_ARGS(&encoder->pGenerator));
			if(SUCCEEDED(hr)) {
				encoder->pGenerator->BeginGetEvent(new EncoderEventCallback(encoder), NULL);
			}
		}
	}

	// signal begin stream and start of stream.
	if(SUCCEEDED(hr)) {
		hr = encoder->transform->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, NULL);
	}
	if(SUCCEEDED(hr)) {
		hr = encoder->transform->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, NULL);
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

bool ttLibC_MsH264Encoder_encode(
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
	// make IMFSample from yuv.
	uint32_t imageSize;
	MFCalculateImageSize(
			MFVideoFormat_NV12,
			frame->inherit_super.width,
			frame->inherit_super.height,
			&imageSize);
	IMFSample *sample;
	IMFMediaBuffer *buffer;
	MFCreateSample(&sample);
	MFCreateMemoryBuffer(imageSize, &buffer);
	sample->AddBuffer(buffer);

	// we need to copy plane here.

	// timebase = 10000
	uint64_t pts = frame->inherit_super.inherit_super.pts * 10000 / frame->inherit_super.inherit_super.timebase;
//	printf("inputPts:%lld\n", pts);
	sample->SetSampleTime(pts);
	sample->SetSampleDuration(100);
	if(encoder_->is_async) {
		encoder_->callback = callback;
		encoder_->ptr = ptr;
		encoder_->ringBuffer->pushSample(sample);
	}
	else {
		encoder_->transform->ProcessInput(0, sample, 0);
		ReleaseOnExit roeSample(sample);
		ReleaseOnExit roeBuffer(buffer);

		drainH264(encoder_);
	}
	return false;
}

void ttLibC_MsH264Encoder_close(ttLibC_MsH264Encoder **encoder) {
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
	ttLibC_H264_close(&target->h264);
	ttLibC_free(target);
	*encoder = NULL;
}

#endif
