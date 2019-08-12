#ifdef __ENABLE_WIN32__

#include "msVideoCapturerUtil.h"
#include "msGlobalUtilCommon.h"
#include "../allocator.h"
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <Dbt.h>
#include <shlwapi.h>
#include <iostream>
#include <cstring>
#include <functional>

#pragma comment(lib, "mfplat")
#pragma comment(lib, "mf")
#pragma comment(lib, "mfreadwrite")
#pragma comment(lib, "mfuuid")
#pragma comment(lib, "shlwapi")

using namespace std;


// make class and use it.
class TTMsVideoCapturer : public IMFSourceReaderCallback {
public:
	STDMETHODIMP QueryInterface(REFIID iid, void** ppv) {
		static const QITAB qit[] = {
			QITABENT(TTMsVideoCapturer, IMFSourceReaderCallback),
			{0},
		};
		return QISearch(this, qit, iid, ppv);
	}
	STDMETHODIMP_(ULONG) AddRef() {
		return InterlockedIncrement(&mRefCount);
	}
	STDMETHODIMP_(ULONG) Release() {
		ULONG uCount = InterlockedDecrement(&mRefCount);
		if (uCount == 0) {
			delete this;
		}
		return uCount;
	}
	STDMETHODIMP OnReadSample(
			HRESULT hrStatus,
			DWORD dwStreamIndex,
			DWORD dwStreamFlags,
			LONGLONG llTimestamp,
			IMFSample *pSample) {
		puts("data is read!!!");
		captureCount --;
		return S_OK;
	}
	STDMETHODIMP OnEvent(DWORD, IMFMediaEvent*) {
		puts("event get");
		return S_OK;
	}
	STDMETHODIMP OnFlush(DWORD) {
		puts("on flush get.");
		return S_OK;
	}

	static bool getDevices(function<bool(IMFActivate *device)> callback) {
		IMFAttributes *pConfig = nullptr;
		IMFActivate** ppDevices = nullptr;
		CoTaskMemFreeOnExit ctmfoe(ppDevices);
		uint32_t count = 0;

		HRESULT hr = MFCreateAttributes(&pConfig, 1);
		if(SUCCEEDED(hr)) {
			hr = pConfig->SetGUID(
				MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
				MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
		}
		if(SUCCEEDED(hr)) {
			hr = MFEnumDeviceSources(pConfig, &ppDevices, &count);
		}
		bool result = true;
		if(SUCCEEDED(hr)) {
			for(uint32_t i = 0;i < count;++ i) {
				if(result) {
					result = callback(ppDevices[i]);
				}
				ReleaseOnExit roe(ppDevices[i]);
			}
		}
		return true;
	}
	TTMsVideoCapturer(string target, uint32_t width, uint32_t height) {
		isInitialized = false;
		pReader = nullptr;
		bool result = getDevices([&](auto device) {
			WCHAR* pszName = nullptr;
			CoTaskMemFreeOnExit ctmfoe(pszName);
			uint32_t nameLength;
			HRESULT hr = device->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &pszName, &nameLength);
			if (SUCCEEDED(hr)) {
				if (strcmp(target.c_str(), ttLibC_MsGlobal_wcharToUtf8string(pszName).c_str()) == 0) {
					IMFMediaSource* pSource = nullptr;
					ReleaseOnExit roeSource(pSource);

					hr = device->ActivateObject(__uuidof(IMFMediaSource), (void **)&pSource);

					// now try to make reader to get image.
					IMFAttributes* pAttribute = nullptr;
					ReleaseOnExit roeAttribute(pAttribute);
					if (SUCCEEDED(hr)) {
						hr = MFCreateAttributes(&pAttribute, 2);
					}
					if (SUCCEEDED(hr)) {
						hr = pAttribute->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, this);
					}
					if (SUCCEEDED(hr)) {
						hr = MFCreateSourceReaderFromMediaSource(
							pSource, pAttribute, &pReader);
					}
					// configure capturer
					IMFMediaType* pType = nullptr;
					ReleaseOnExit roeType(pType);
					if (SUCCEEDED(hr)) {
						hr = pReader->GetNativeMediaType(
						(DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
							0,
							&pType);
					}
					if(SUCCEEDED(hr)) {
						hr = MFSetAttributeSize(pType, MF_MT_FRAME_SIZE, width, height);
					}
					if(SUCCEEDED(hr)) {
						hr = pReader->GetCurrentMediaType(
								(DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
								&pType);
					}
					if(SUCCEEDED(hr)) {
						isInitialized = true;
					}
				}
			}
			return true;
		});
	}
	~TTMsVideoCapturer() {
		if (pReader != nullptr) {
			while(captureCount > 0) {
				Sleep(1);
			}
			pReader->Release();
			pReader = nullptr;
		}
	}
	bool requestFrame(function<bool(ttLibC_Video *video)> callback) {
		pCallback = callback;
		if(pReader != nullptr) {
			HRESULT hr = pReader->ReadSample(
				(DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
				0,
				NULL,
				NULL,
				NULL,
				NULL);
			if(SUCCEEDED(hr)) {
				captureCount ++;
				return true;
			}
		}
		return false;
	}
	bool isInitialized;
private:
	long mRefCount;
	long captureCount = 0;
	IMFSourceReader* pReader;
	function<bool(ttLibC_Video *video)> pCallback;
};

typedef struct ttLibC_Util_MsVideoCapturer_ {
	ttLibC_MsVideoCapturer inherit_super;
	TTMsVideoCapturer *capturer;
} ttLibC_Util_MsVideoCapturer_;

typedef ttLibC_Util_MsVideoCapturer_ ttLibC_MsVideoCapturer_;

bool TT_ATTRIBUTE_API ttLibC_MsVideoCapturer_getDeviceNames(ttLibC_MsVideoCapturerNameFunc callback, void *ptr) {
	return TTMsVideoCapturer::getDevices([&](auto device){
		WCHAR *pszName = nullptr;
		CoTaskMemFreeOnExit ctmfoe(pszName);
		uint32_t nameLength;
		HRESULT hr = device->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &pszName, &nameLength);
		if(SUCCEEDED(hr)) {
				return callback(ptr, ttLibC_MsGlobal_wcharToUtf8string(pszName).c_str());
		}
		return true;
	});
}

ttLibC_MsVideoCapturer TT_ATTRIBUTE_API *ttLibC_MsVideoCapturer_make(
	const char *target, uint32_t width, uint32_t height) {
	ttLibC_MsVideoCapturer_ *capturer = (ttLibC_MsVideoCapturer_ *)ttLibC_malloc(sizeof(ttLibC_MsVideoCapturer_));
	if(capturer == nullptr) {
		return nullptr;
	}
	capturer->capturer = new (std::nothrow)TTMsVideoCapturer(target, width, height);
	if (capturer == nullptr) {
		return nullptr;
	}
	if (!capturer->capturer->isInitialized) {
		delete capturer;
		ttLibC_free(capturer);
		return nullptr;
	}
	return (ttLibC_MsVideoCapturer *)capturer;
}

bool TT_ATTRIBUTE_API ttLibC_MsVideoCapturer_requestFrame(
	ttLibC_MsVideoCapturer *capturer,
	ttLibC_MsVideoCapturerFrameFunc callback,
	void *ptr) {
	if(capturer == nullptr) {
			return false;
	}
	ttLibC_MsVideoCapturer_ *capturer_ = (ttLibC_MsVideoCapturer_ *)capturer;
	return capturer_->capturer->requestFrame([&](ttLibC_Video *video) {
		return true;
	});
}

void TT_ATTRIBUTE_API ttLibC_MsVideoCapturer_close(ttLibC_MsVideoCapturer **capturer) {
	ttLibC_MsVideoCapturer_ *target = (ttLibC_MsVideoCapturer_ *)*capturer;
	if (target != nullptr) {
		if(target->capturer != nullptr) {
			delete target->capturer;
		}
		ttLibC_free(target);
		*capturer = nullptr;
	}
}

#endif