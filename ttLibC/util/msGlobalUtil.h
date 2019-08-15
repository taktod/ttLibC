/**
 * @file   msGlobalUtil.h
 * @brief  windows util.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2019/08/12
 */

#ifndef TTLIBC_UTIL_MSGLOBALUTIL_H_
#define TTLIBC_UTIL_MSGLOBALUTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../ttLibC_predef.h"
#include <stdbool.h>

#include "../frame/video/bgr.h"

typedef enum ttLibC_MsGlobal_CoInitializeType {
  CoInitializeType_normal,
  CoInitializeType_multiThreaded
} ttLibC_MsGlobal_CoInitializeType;

bool TT_ATTRIBUTE_API ttLibC_MsGlobal_CoInitialize(ttLibC_MsGlobal_CoInitializeType type);
void TT_ATTRIBUTE_API ttLibC_MsGlobal_CoUninitialize();

bool TT_ATTRIBUTE_API ttLibC_MsGlobal_MFStartup();
void TT_ATTRIBUTE_API ttLibC_MsGlobal_MFShutdown();

bool TT_ATTRIBUTE_API ttLibC_MsGlobal_setlocale(const char *language);

int TT_ATTRIBUTE_API ttLibC_MsGlobal_unicodeToUtf8(const wchar_t *unicode, char *utf8, int utf8_length);
int TT_ATTRIBUTE_API ttLibC_MsGlobal_unicodeToSjis(const wchar_t *unicode, char *sjis, int sjis_length);
int TT_ATTRIBUTE_API ttLibC_MsGlobal_utf8ToUnicode(const char *utf8, wchar_t *unicode, int unicode_length);
int TT_ATTRIBUTE_API ttLibC_MsGlobal_sjisToUnicode(const char *sjis, wchar_t *unicode, int unicode_length);

void TT_ATTRIBUTE_API ttLibC_MsGlobal_sleep(long time);

void TT_ATTRIBUTE_API ttLibC_MsGlobal_WriteBitmap(const char *name, ttLibC_Bgr *bgr);

#ifdef __cplusplus
}
#endif

#endif
