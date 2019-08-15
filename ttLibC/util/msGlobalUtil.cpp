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

static std::wstring MsGlobal_stringToWstring(std::string const& src) {
  std::size_t converted{};
  std::vector<wchar_t> dest(src.size() + 1, L'\0');
  ::mbstowcs_s(&converted, dest.data(), dest.size(), src.data(), -1);
  return std::wstring(dest.begin(), dest.end());
}

static std::string MsGlobal_wstringToUtf8string(std::wstring const& src) {
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

void TT_ATTRIBUTE_API ttLibC_MsGlobal_WriteBitmap(const char *name, ttLibC_Bgr *bgr) {
  if(bgr == nullptr) {
    return;
  }
  switch(bgr->type) {
  case BgrType_bgr:
    {
      // try to write bitmap
      HANDLE file;
      BITMAPFILEHEADER fileHeader;
      BITMAPINFOHEADER fileInfo;
      DWORD write = 0;

      file = CreateFile(
        name,
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

      fileHeader.bfType = 19778;
      fileHeader.bfSize = sizeof(fileHeader.bfOffBits) + sizeof(RGBTRIPLE);
      fileHeader.bfReserved1 = 0;
      fileHeader.bfReserved2 = 0;
      fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

      fileInfo.biSize = sizeof(BITMAPINFOHEADER);
      fileInfo.biWidth = bgr->inherit_super.width;
      fileInfo.biHeight = bgr->inherit_super.height;
      fileInfo.biPlanes = 1;
      fileInfo.biBitCount = 24;
      fileInfo.biCompression = BI_RGB;
      fileInfo.biSizeImage = bgr->inherit_super.width * bgr->inherit_super.height * (24/8);
      fileInfo.biXPelsPerMeter = 2400;
      fileInfo.biYPelsPerMeter = 2400;
      fileInfo.biClrImportant = 0;
      fileInfo.biClrUsed = 0;

      WriteFile(file,&fileHeader,sizeof(fileHeader),&write,NULL);
      WriteFile(file,&fileInfo,sizeof(fileInfo),&write,NULL);
      WriteFile(file, (BYTE *)bgr->data, fileInfo.biSizeImage, &write, NULL);

      CloseHandle(file);
    }
    break;
  case BgrType_bgra:
    {
      // try to write bitmap
      HANDLE file;
      BITMAPFILEHEADER fileHeader;
      BITMAPINFOHEADER fileInfo;
      DWORD write = 0;

      file = CreateFile(
        name,
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

      fileHeader.bfType = 19778;
      fileHeader.bfSize = sizeof(fileHeader.bfOffBits) + sizeof(RGBTRIPLE);
      fileHeader.bfReserved1 = 0;
      fileHeader.bfReserved2 = 0;
      fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

      fileInfo.biSize = sizeof(BITMAPINFOHEADER);
      fileInfo.biWidth = bgr->inherit_super.width;
      fileInfo.biHeight = bgr->inherit_super.height;
      fileInfo.biPlanes = 1;
      fileInfo.biBitCount = 32;
      fileInfo.biCompression = BI_RGB;
      fileInfo.biSizeImage = bgr->inherit_super.width * bgr->inherit_super.height * (32/8);
      fileInfo.biXPelsPerMeter = 2400;
      fileInfo.biYPelsPerMeter = 2400;
      fileInfo.biClrImportant = 0;
      fileInfo.biClrUsed = 0;

      WriteFile(file,&fileHeader,sizeof(fileHeader),&write,NULL);
      WriteFile(file,&fileInfo,sizeof(fileInfo),&write,NULL);
      WriteFile(file, (BYTE *)bgr->data, fileInfo.biSizeImage, &write, NULL);

      CloseHandle(file);
    }
    break;
  default:
    return;
  }
}

int TT_ATTRIBUTE_API ttLibC_MsGlobal_unicodeToUtf8(const wchar_t *unicode, char *utf8, int utf8_length) {
  return WideCharToMultiByte(CP_UTF8, 0, unicode, -1, utf8, utf8_length, nullptr, nullptr);
}
int TT_ATTRIBUTE_API ttLibC_MsGlobal_unicodeToSjis(const wchar_t *unicode, char *sjis, int sjis_length) {
  return WideCharToMultiByte(CP_ACP, 0, unicode, -1, sjis, sjis_length, nullptr, nullptr);
}
int TT_ATTRIBUTE_API ttLibC_MsGlobal_utf8ToUnicode(const char *utf8, wchar_t *unicode, int unicode_length) {
  return MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, utf8, -1, unicode, unicode_length);
}
int TT_ATTRIBUTE_API ttLibC_MsGlobal_sjisToUnicode(const char *sjis, wchar_t *unicode, int unicode_length) {
  return MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, sjis, -1, unicode, unicode_length);
}


#endif
