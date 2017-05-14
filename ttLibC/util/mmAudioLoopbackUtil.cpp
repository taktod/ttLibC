/*
 * mmAudioLoopbackUtil.c
 *
 * Created on: 2017/05/03
 *     Author: taktod
 */

#ifdef __ENABLE_WIN32__

#include "mmAudioLoopbackUtil.h"
#include "msGlobalUtil.h"
#include <ttLibC/allocator.h>
#include <ttLibC/log.h>
#include <ttLibC/frame/audio/pcmS16.h>
#include <ttLibC/util/stlListUtil.h>
#include <ttLibC/container/mkv.h>
#include <string>

#include <locale.h>
#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <avrt.h>
#include <functiondiscoverykeys_devpkey.h>

#pragma comment(lib, "winmm")
#pragma comment(lib, "Avrt")

class CoUninitializeOnExit {
public:
	~CoUninitializeOnExit() {
		ttLibC_MsGlobal_CoUninitialize();
	}
};

class AudioClientStopOnExit {
public:
	AudioClientStopOnExit(IAudioClient *p) : m_p(p) {}
	~AudioClientStopOnExit() {
		HRESULT hr = m_p->Stop();
		if (FAILED(hr)) {
			ERR_PRINT("failed to stop audioClient");
		}
	}
private:
	IAudioClient *m_p;
};

class AvRevertMmThreadCharacteristicsOnExit {
public:
	AvRevertMmThreadCharacteristicsOnExit(HANDLE hTask) : m_hTask(hTask) {}
	~AvRevertMmThreadCharacteristicsOnExit() {
		if (!AvRevertMmThreadCharacteristics(m_hTask)) {
			ERR_PRINT("failed to revert mmThreadCharacteristics");
		}
	}
private:
	HANDLE m_hTask;
};

class PropVariantClearOnExit {
public:
	PropVariantClearOnExit(PROPVARIANT *p) : m_p(p) {}
	~PropVariantClearOnExit() {
		HRESULT hr = PropVariantClear(m_p);
		if (FAILED(hr)) {
			ERR_PRINT("failed to clear propVariant.");
		}
	}
private:
	PROPVARIANT *m_p;
};

class CancelWaitableTimerOnExit {
public:
	CancelWaitableTimerOnExit(HANDLE h) : m_h(h) {}
	~CancelWaitableTimerOnExit() {
		if(!CancelWaitableTimer(m_h)) {
			ERR_PRINT("failed to cancel waitableTimer.");
		}
	}
private:
	HANDLE m_h;
};

/**
 * data of ringBuffer element.
 */
typedef struct ttLibC_Util_MmAudioLoopback_Buffer {
	uint8_t  pcm[655360]; // pcmBuffer could be pcmS16.
	uint32_t length;
} ttLibC_Util_MmAudioLoopback_Buffer;

typedef ttLibC_Util_MmAudioLoopback_Buffer ttLibC_Buffer;

/**
 * MmAudioLoopback_RingBuffer class, auto release on exit.
 */
class MmAudioLoopback_RingBuffer {
public:
	MmAudioLoopback_RingBuffer() {
		refPointer_ = -2; // refPointer will be enable after 2 steps.
		addPointer_ = -1; // addPointer will be enable after 1 steps.
		is_working_ = true;
		prepareNextBuffer();
	}
	~MmAudioLoopback_RingBuffer() {
		is_working_ = false;
		for(int i = 0;i < 16;++ i) {
			releaseBuffer(buffer[i]);
			buffer[i] = NULL;
		}
	}
	void prepareNextBuffer() {
		if(!is_working_) {
			return;
		}
		// alloc new buffer.
		ttLibC_Buffer *newBuffer = (ttLibC_Buffer *)ttLibC_malloc(sizeof(ttLibC_Buffer));
		newBuffer->length = 0;
		// register on ring
		int addPos = (addPointer_ + 1) & 0x0F;
		buffer[addPos] = newBuffer;

		// move to next pointer.
		refPointer_ = addPointer_;
		addPointer_ = addPos;
	}
	/**
	 * ref captured pcm buffer.
	 * @return ttLibC_Buffer object.
	 */
	ttLibC_Buffer *getRefBuffer() {
		if(!is_working_) {
			return NULL;
		}
		if(refPointer_ < 0) {
			// not ready to access.
			return NULL;
		}
		ttLibC_Buffer *res = buffer[refPointer_];
		buffer[refPointer_] = NULL;
		return res;
	}
	/**
	 * ref empty buffer.
	 * @return ttLibC_Buffer object
	 */
	ttLibC_Buffer *getAddBuffer() {
		if(!is_working_) {
			return NULL;
		}
		return buffer[addPointer_];
	}
	/**
	 * release alloced buffer.
	 * alloc in MmAudioLoopback_RingBuffer, so release in MmAudioLoopback_RingBuffer.
	 * @param buffer ttLibC_Buffer object
	 */
	void releaseBuffer(ttLibC_Buffer *buffer) {
		if(buffer == NULL) {
			return;
		}
		ttLibC_free(buffer);
	}
private:
	ttLibC_Buffer *buffer[16]; // element should be number of 2^exp
	int refPointer_;
	int addPointer_;
	bool is_working_;
};

typedef struct ttLibC_Util_MmAudioLoopback_ {
	ttLibC_MmAudioLoopback inherit_super;
	char locale[256]; // target locale
	char deviceName[256]; // target deviceName
	bool is_running;
	uint64_t totalSampleNum; // generated pcm sampleNum(to make pts.)
	MmAudioLoopback_RingBuffer *ringBuffer;  // ringBuffer to handle between record thread and node thread.
	HANDLE hThread;
	uint32_t channel_num; // working channel_num
	uint32_t sample_rate; // working sample_rate
} ttLibC_Util_MmAudioLoopback_;

typedef ttLibC_Util_MmAudioLoopback_ ttLibC_MmAudioLoopback_;

/**
 * thread working
 */
static DWORD WINAPI MmAudioLoopback_threadCallback(LPVOID pContext) {
	// share loopback object.
	ttLibC_MmAudioLoopback_ *loopback = (ttLibC_MmAudioLoopback_ *)pContext;
	if(loopback == NULL) {
		return 0;
	}
	MmAudioLoopback_RingBuffer ringBuffer;
	loopback->ringBuffer = &ringBuffer;

	// update locale for this thread.
	if(strcmp(loopback->locale, "") != 0) {
		ttLibC_MsGlobal_setlocale(loopback->locale);
	}
	// initialize com for this thread.
	HRESULT hr = S_OK;
	if(!ttLibC_MsGlobal_CoInitialize(CoInitializeType_normal)) {
		ERR_PRINT("failed to coInitialize");
		return 0;
	}
	CoUninitializeOnExit cuoe;

	IMMDeviceEnumerator *pMMDeviceEnumerator = NULL;
	hr = CoCreateInstance(
		__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
		__uuidof(IMMDeviceEnumerator),
		(void **)&pMMDeviceEnumerator);
	if(FAILED(hr)) {
		ERR_PRINT("failed to get device enumerator");
		return 0;
	}
	ReleaseOnExit roeDeviceEnumerator(pMMDeviceEnumerator);

	IMMDevice *pMMDevice = NULL;
	// here
	if(std::string(loopback->deviceName) != "") {
		IMMDeviceCollection *pMMDeviceCollection = NULL;
		hr = pMMDeviceEnumerator->EnumAudioEndpoints(
			eRender, DEVICE_STATE_ACTIVE, &pMMDeviceCollection
		);
		if(FAILED(hr)) {
			return false;
		}
		ReleaseOnExit roeDeviceCollection(pMMDeviceCollection);

		uint32_t deviceNum = 0;
		hr = pMMDeviceCollection->GetCount(&deviceNum);
		if(FAILED(hr)) {
			return false;
		}
		for(uint32_t i = 0;i < deviceNum;++ i) {
			IMMDevice *pMMDeviceFind = NULL;
			hr = pMMDeviceCollection->Item(i, &pMMDeviceFind);
			if(FAILED(hr)) {
				return false;
			}
			IPropertyStore *pPropertyStore = NULL;
			hr = pMMDeviceFind->OpenPropertyStore(STGM_READ, &pPropertyStore);
			if(FAILED(hr)) {
				if(pMMDeviceFind != NULL) {
					pMMDeviceFind->Release();
					pMMDeviceFind = NULL;
				}
				return false;
			}
			ReleaseOnExit roePropertyStore(pPropertyStore);
			PROPVARIANT pv;
			PropVariantInit(&pv);
			hr = pPropertyStore->GetValue(PKEY_Device_FriendlyName, &pv);
			if(FAILED(hr)) {
				if(pMMDeviceFind != NULL) {
					pMMDeviceFind->Release();
					pMMDeviceFind = NULL;
				}
				return false;
			}
			PropVariantClearOnExit pvcoe(&pv);
			if(pv.vt != VT_LPWSTR) {
				if(pMMDeviceFind != NULL) {
					pMMDeviceFind->Release();
					pMMDeviceFind = NULL;
				}
				continue;
			}
			// change name into wchar -> char with locale
			const char *deviceName = ttLibC_MsGlobal_wcharToUtf8string(pv.pwszVal).c_str();
			if(strcmp(deviceName, loopback->deviceName) == 0) {
				puts("find same device.");
				// use this device
				pMMDevice = pMMDeviceFind;
			}
			else {
				// this device is not useful.
				if(pMMDeviceFind != NULL) {
					pMMDeviceFind->Release();
					pMMDeviceFind = NULL;
				}
			}
		}
	}
	else {
		// get enum to figure out the same name of device.
		hr = pMMDeviceEnumerator->GetDefaultAudioEndpoint(
				eRender,
				eConsole,
				&pMMDevice);
		if(FAILED(hr)) {
			ERR_PRINT("failed to get deviceEndpoint.");
			return 0;
		}
	}
	if(pMMDevice == NULL) {
		puts("no device is detected to work.");
		return 0;
	}
	ReleaseOnExit roeDevice(pMMDevice);

	IAudioClient *pAudioClient = NULL;
	hr = pMMDevice->Activate(
			__uuidof(IAudioClient),
			CLSCTX_ALL, NULL,
			(void **)&pAudioClient);
	if(FAILED(hr)) {
		ERR_PRINT("failed to get audioClient");
		return 0;
	}
	ReleaseOnExit roeAudioClient(pAudioClient);

	// update thread for audio recording mode.
	DWORD nTaskIndex = 0;
	HANDLE hTask = AvSetMmThreadCharacteristics("Audio", &nTaskIndex);
	if(hTask == NULL) {
		ERR_PRINT("failed to set mmthread characteristics.");
		return 0;
	}
	AvRevertMmThreadCharacteristicsOnExit armtcoe(hTask);

	// ref the device working period.
	REFERENCE_TIME hnsDefaultDevicePeriod;
	hr = pAudioClient->GetDevicePeriod(&hnsDefaultDevicePeriod, NULL);
	if(FAILED(hr)) {
		ERR_PRINT("failed to get devicePeriod.");
		return 0;
	}

	WAVEFORMATEX *pwfx = NULL;
	hr = pAudioClient->GetMixFormat(&pwfx);
	if(FAILED(hr)) {
		ERR_PRINT("failed to get device default format.");
		return 0;
	}
	CoTaskMemFreeOnExit ctmfoe(pwfx);

	// update format for pcmS16.
	switch (pwfx->wFormatTag) {
	case WAVE_FORMAT_IEEE_FLOAT:
		pwfx->wFormatTag      = WAVE_FORMAT_PCM;
		pwfx->wBitsPerSample  = 16;
		pwfx->nBlockAlign     = pwfx->nChannels * pwfx->wBitsPerSample / 8;
		pwfx->nAvgBytesPerSec = pwfx->nBlockAlign * pwfx->nSamplesPerSec;
		break;
	case WAVE_FORMAT_EXTENSIBLE:
		{
			PWAVEFORMATEXTENSIBLE pEx = reinterpret_cast<PWAVEFORMATEXTENSIBLE>(pwfx);
			if (IsEqualGUID(KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, pEx->SubFormat)) {
				pEx->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
				pEx->Samples.wValidBitsPerSample = 16;
				pwfx->wBitsPerSample = 16;
				pwfx->nBlockAlign = pwfx->nChannels * pwfx->wBitsPerSample / 8;
				pwfx->nAvgBytesPerSec = pwfx->nBlockAlign * pwfx->nSamplesPerSec;
			} else {
				return 0;
			}
		}
		break;
	default:
		return 0;
	}
	// store in sharing object for reference.
	loopback->channel_num = pwfx->nChannels;
	loopback->sample_rate = pwfx->nSamplesPerSec;

	HANDLE hWakeUp = CreateWaitableTimer(NULL, FALSE, NULL);
	if(hWakeUp == NULL) {
		ERR_PRINT("failed to make waitableTimer.");
		return 0;
	}
	CloseHandleOnExit choeWakeUp(hWakeUp);

	// initialize audioClient (put the information of waveformatex.)
	hr = pAudioClient->Initialize(
			AUDCLNT_SHAREMODE_SHARED,
			AUDCLNT_STREAMFLAGS_LOOPBACK,
			0,
			0,
			pwfx,
			0);
	if(FAILED(hr)) {
		ERR_PRINT("failed to initialize audioClient.");
		return 0;
	}

	IAudioCaptureClient *pAudioCaptureClient = NULL;
	hr = pAudioClient->GetService(
			__uuidof(IAudioCaptureClient),
			(void **)&pAudioCaptureClient);
	if(FAILED(hr)) {
		ERR_PRINT("failed to get audioCaptureClient.");
		return 0;
	}
	ReleaseOnExit roeAudioCaptureClient(pAudioCaptureClient);

	LARGE_INTEGER liFirstFire;
	liFirstFire.QuadPart = -hnsDefaultDevicePeriod / 2;
	LONG lTimerBetweenFires = (LONG)hnsDefaultDevicePeriod / 2 / 10000;
	BOOL bOK = SetWaitableTimer(
			hWakeUp,
			&liFirstFire,
			lTimerBetweenFires,
			NULL,
			NULL,
			FALSE);
	if(!bOK) {
		ERR_PRINT("failed to set timer.");
		return 0;
	}
	CancelWaitableTimerOnExit cancelWakeUp(hWakeUp);

	hr = pAudioClient->Start();
	if(FAILED(hr)) {
		ERR_PRINT("failed to start audioClient to capture.");
		return 0;
	}
	AudioClientStopOnExit stopAudioClient(pAudioClient);

	loopback->is_running = true;

	while(true) {
		bool local_is_running = loopback->is_running;
		if(!local_is_running) {
			break;
		}
		uint32_t nNextPacketSize;
		for(hr = pAudioCaptureClient->GetNextPacketSize(&nNextPacketSize);
				SUCCEEDED(hr) && nNextPacketSize > 0;
				hr = pAudioCaptureClient->GetNextPacketSize(&nNextPacketSize)) {
			BYTE *pData;
			uint32_t nNumFramesToRead;
			DWORD dwFlags;
			hr = pAudioCaptureClient->GetBuffer(
					&pData,
					&nNumFramesToRead,
					&dwFlags,
					NULL,
					NULL);
			if(FAILED(hr)) {
				ERR_PRINT("failed to getBuffer.");
				return 0;
			}
			ttLibC_Buffer *buffer = ringBuffer.getAddBuffer();
			if(buffer != NULL) {
				uint8_t *pcm = buffer->pcm;
				if(buffer->length < 524288) { // if we have too much buffer, we skip to record.
					size_t size = nNumFramesToRead * pwfx->nBlockAlign;
					pcm += buffer->length;
					memcpy(pcm, pData, size);
					buffer->length += size;
				}
				else {
					ERR_PRINT("buffer is full, you need to call queryFrame with shorter period.");
				}
			}
			// release captured pcm buffer, for next work.
			hr = pAudioCaptureClient->ReleaseBuffer(nNumFramesToRead);
			if(FAILED(hr)) {
				ERR_PRINT("failed to releaseBuffer.");
				return 0;
			}
		}
		// wait for timer.
		if(WaitForSingleObject(hWakeUp, INFINITE) != WAIT_OBJECT_0) {
			ERR_PRINT("failed to wait waitableTimer signal.");
			return 0;
		}
	}
	return 0;
}

/**
 * ref the all device on this computer.
 * we will use these name for initialize in future.
 */
bool ttLibC_MmAudioLoopback_getDeviceNames(
		ttLibC_MmAudioLoopbackDeviceNameFunc callback,
		void *ptr) {
	// user must setup locale and coInitialize
	bool result = true;
	HRESULT hr = S_OK;
	IMMDeviceEnumerator *pMMDeviceEnumerator = NULL;
	hr = CoCreateInstance(
		__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
		__uuidof(IMMDeviceEnumerator),
		(void **)&pMMDeviceEnumerator
	);
	if(FAILED(hr)) {
		return false;
	}
	ReleaseOnExit roeDeviceEnumerator(pMMDeviceEnumerator);
	IMMDeviceCollection *pMMDeviceCollection = NULL;
	hr = pMMDeviceEnumerator->EnumAudioEndpoints(
		eRender, DEVICE_STATE_ACTIVE, &pMMDeviceCollection
	);
	if(FAILED(hr)) {
		return false;
	}
	ReleaseOnExit roeDeviceCollection(pMMDeviceCollection);

	uint32_t deviceNum = 0;
	hr = pMMDeviceCollection->GetCount(&deviceNum);
	if(FAILED(hr)) {
		return false;
	}
	for(uint32_t i = 0;i < deviceNum;++ i) {
		IMMDevice *pMMDevice = NULL;
		hr = pMMDeviceCollection->Item(i, &pMMDevice);
		if(FAILED(hr)) {
			return false;
		}
		ReleaseOnExit roeDevice(pMMDevice);
		IPropertyStore *pPropertyStore = NULL;
		hr = pMMDevice->OpenPropertyStore(STGM_READ, &pPropertyStore);
		if(FAILED(hr)) {
			return false;
		}
		ReleaseOnExit roePropertyStore(pPropertyStore);
		PROPVARIANT pv;
		PropVariantInit(&pv);
		hr = pPropertyStore->GetValue(PKEY_Device_FriendlyName, &pv);
		if(FAILED(hr)) {
			return false;
		}
		PropVariantClearOnExit pvcoe(&pv);
		if(pv.vt != VT_LPWSTR) {
			continue;
		}
		// change name into wchar -> char with locale
		const char *deviceName = ttLibC_MsGlobal_wcharToUtf8string(pv.pwszVal).c_str();
		if(callback != NULL) {
			if(!callback(ptr, deviceName)) {
				return false;
			}
		}
	}
	return true;
}

/**
 * make audioLoopback
 */
ttLibC_MmAudioLoopback *ttLibC_MmAudioLoopback_make(
		const char *locale,
		const char *deviceName) {
	ttLibC_MmAudioLoopback_ *loopback = (ttLibC_MmAudioLoopback_ *)ttLibC_malloc(sizeof(ttLibC_MmAudioLoopback_));
	if(loopback == NULL) {
		return NULL;
	}
	memset(loopback, 0, sizeof(ttLibC_MmAudioLoopback_));
	// copy locale
	if(locale == NULL){
		strcpy(loopback->locale, "");
	}
	else {
		strcpy(loopback->locale, locale);
	}
	// copy device name
	if(deviceName == NULL) {
		strcpy(loopback->deviceName, "");
	}
	else {
		strcpy(loopback->deviceName, deviceName);
	}
	loopback->is_running = false;
	// ringBuffer is set in thread.
	// sample_rate
	// channel_num
	loopback->hThread = CreateThread(
			NULL, 0,
			MmAudioLoopback_threadCallback,
			loopback,
			0,
			NULL);
	return (ttLibC_MmAudioLoopback *)loopback;
}

/**
 * get capture frame as ttLibC_PcmS16
 */
bool ttLibC_MmAudioLoopback_queryFrame(
		ttLibC_MmAudioLoopback *device,
		ttLibC_MmAudioLoopbackFrameFunc callback,
		void *ptr) {
	ttLibC_MmAudioLoopback_ *loopback = (ttLibC_MmAudioLoopback_ *)device;
	if(loopback == NULL) {
		return false;
	}
	if(loopback->is_running) {
		ttLibC_Buffer *buffer = loopback->ringBuffer->getRefBuffer();
		loopback->ringBuffer->prepareNextBuffer();
		if(buffer != NULL) {
			// convert from buffer to pcmS16
			uint8_t *pcm = buffer->pcm;
			uint8_t *r_data = NULL;
			size_t r_size = 0;
			uint32_t sample_num = (buffer->length >> 1);
			if(loopback->channel_num == 2) {
				r_data = pcm + 1;
				r_size = buffer->length;
				sample_num = (buffer->length >> 2);
			}
			ttLibC_PcmS16 *p = ttLibC_PcmS16_make(
				NULL,
				PcmS16Type_littleEndian,
				loopback->sample_rate,
				sample_num,
				loopback->channel_num,
				pcm,
				buffer->length,
				pcm,
				buffer->length,
				r_data,
				r_size,
				true,
				loopback->totalSampleNum,
				loopback->sample_rate);
			loopback->totalSampleNum += sample_num;
			if(p != NULL) {
				if(callback != NULL) {
					callback(ptr, p);
				}
				ttLibC_PcmS16_close(&p);
			}
			loopback->ringBuffer->releaseBuffer(buffer);
		}
	}
	return true;
}

/**
 * close object.
 */
void ttLibC_MmAudioLoopback_close(ttLibC_MmAudioLoopback **device) {
	ttLibC_MmAudioLoopback_ *target = (ttLibC_MmAudioLoopback_ *)*device;
	if(target == NULL) {
		return;
	}
	if(target->is_running) {
		target->is_running = false;
	}
	if(target->hThread != NULL) {
		CloseHandle(target->hThread);
		target->hThread = NULL;
	}
	ttLibC_free(target);
	*device = NULL;
}

#endif
