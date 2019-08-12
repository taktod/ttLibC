/**
 * @file   msGlobalUtil.cpp
 * @brief  windows util.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2017/05/12
 */

#ifdef __ENABLE_WIN32__

#include "msGlobalUtilCommon.h"

bool TT_ATTRIBUTE_API ttLibC_MsGlobal_CoInitialize(ttLibC_MsGlobal_CoInitializeType type) {
	HRESULT hr = S_OK;
	switch(type) {
	case CoInitializeType_normal:
		hr = CoInitialize(NULL);
		break;
	case CoInitializeType_multiThreaded:
		hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
		break;
	default:
		return false;
	}
	return SUCCEEDED(hr);
}
void TT_ATTRIBUTE_API ttLibC_MsGlobal_CoUninitialize() {
	CoUninitialize();
}

bool TT_ATTRIBUTE_API ttLibC_MsGlobal_MFStartup() {
	HRESULT hr = MFStartup(MF_VERSION);
	return SUCCEEDED(hr);
}
void TT_ATTRIBUTE_API ttLibC_MsGlobal_MFShutdown() {
	MFShutdown();
}

bool TT_ATTRIBUTE_API ttLibC_MsGlobal_setlocale(const char *language) {
	char *current = setlocale(LC_ALL, language);
	return current != NULL;
}

static std::wstring MsGlobal_stringToWstring(std::string const& src)
{
	std::size_t converted{};
	std::vector<wchar_t> dest(src.size(), L'\0');
	::mbstowcs_s(&converted, dest.data(), dest.size(), src.data(), -1);
	return std::wstring(dest.begin(), dest.end());
}

static std::string MsGlobal_wstringToUtf8string(std::wstring const& src)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.to_bytes(src);
}

std::string TT_ATTRIBUTE_API ttLibC_MsGlobal_wcharToUtf8string(const wchar_t *src) {
	char buf[256];
	size_t size;
	wcstombs_s(
			&size,
			buf,
			(wcslen(src) + 1) * sizeof(wchar_t),
			src,
			(wcslen(src) + 1) * sizeof(wchar_t));
	return MsGlobal_wstringToUtf8string(MsGlobal_stringToWstring(std::string(buf)));
}

void TT_ATTRIBUTE_API ttLibC_MsGlobal_sleep(long time) {
	Sleep(time);
}

#endif
