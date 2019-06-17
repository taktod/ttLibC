/**
 * @file   stlMapUtil.h
 * @brief  std::map support.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2015/12/18
 */

#ifndef TTLIBC_UTIL_STLMAPUTIL_H_
#define TTLIBC_UTIL_STLMAPUTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../ttLibC_predef.h"
#include <stdio.h>
#include <stdbool.h>

/**
 * definition of stlMap object.
 */
typedef struct ttLibC_Util_StlMap {
	/** element size */
	size_t size;
} ttLibC_Util_StlMap;

typedef ttLibC_Util_StlMap ttLibC_StlMap;

typedef bool (* ttLibC_StlMapRefFunc)(void *ptr, void *key, void *item);

ttLibC_StlMap TT_ATTRIBUTE_API *ttLibC_StlMap_make();

bool TT_ATTRIBUTE_API ttLibC_StlMap_put(
		ttLibC_StlMap *map,
		void *key,
		void *item);

void TT_ATTRIBUTE_API *ttLibC_StlMap_get(
		ttLibC_StlMap *map,
		void *key);

bool TT_ATTRIBUTE_API ttLibC_StlMap_remove(
		ttLibC_StlMap *map,
		void *key);

bool TT_ATTRIBUTE_API ttLibC_StlMap_removeAll(ttLibC_StlMap *map);

bool TT_ATTRIBUTE_API ttLibC_StlMap_forEach(
		ttLibC_StlMap *map,
		ttLibC_StlMapRefFunc callback,
		void *ptr);

void TT_ATTRIBUTE_API ttLibC_StlMap_close(ttLibC_StlMap **map);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_UTIL_STLMAPUTIL_H_ */
