/**
 * @file   msGlobalUtil.h
 * @brief  windows util.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2017/05/12
 */

#ifndef TTLIBC_UTIL_MSGLOBALUTIL_H_
#define TTLIBC_UTIL_MSGLOBALUTIL_H_

#ifdef __cplusplus

#include <windows.h>
#include <comdef.h>
#include <mfapi.h>
#include <locale.h>
#include <string>
#include <vector>
#include <codecvt>

#pragma comment(lib, "mfplat")

// for auto closing.
class CloseHandleOnExit {
public:
	CloseHandleOnExit(HANDLE h) : m_h(h) {}
	~CloseHandleOnExit() {
		if(!CloseHandle(m_h)) {
			puts("close handle failed.");
		}
	}
private:
	HANDLE m_h;
};

class ReleaseOnExit {
public:
	ReleaseOnExit(IUnknown *p) : m_p(p) {}
	~ReleaseOnExit() {
		if(m_p != NULL) {
			m_p->Release();
			m_p = NULL;
		}
	}
private:
	IUnknown *m_p;
};

class CoTaskMemFreeOnExit {
public:
	CoTaskMemFreeOnExit(PVOID p) : m_p(p) {}
	~CoTaskMemFreeOnExit() {
		CoTaskMemFree(m_p);
	}
private:
	PVOID m_p;
};

// utils for windows.
std::string ttLibC_MsGlobal_wcharToUtf8string(const wchar_t *src);

extern "C" {
#endif

#include <stdbool.h>

typedef enum ttLibC_MsGlobal_CoInitializeType {
	CoInitializeType_normal,
	CoInitializeType_multiThreaded
} ttLibC_MsGlobal_CoInitializeType;

bool ttLibC_MsGlobal_CoInitialize(ttLibC_MsGlobal_CoInitializeType type);
void ttLibC_MsGlobal_CoUninitialize();

bool ttLibC_MsGlobal_MFStartup();
void ttLibC_MsGlobal_MFShutdown();

bool ttLibC_MsGlobal_setlocale(const char *language);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_UTIL_MSGLOBALUTIL_H_ */
