/**
 * @file   stlListUtil.h
 * @brief  
 * @author taktod
 * @date   2015/11/30
 * こっちの動作は、オブジェクトの管理とかは、元のプログラム側でやってもらう感じに調整しておく。
 * なのでオブジェクトのコピーとか、オブジェクトの解放とかは実施しない。
 */

#ifndef TTLIBC_UTIL_STLLISTUTIL_H_
#define TTLIBC_UTIL_STLLISTUTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdbool.h>

/**
 * definition of stlList object.
 */
typedef struct {
	/** element size */
	size_t size;
} ttLibC_Util_StlList;

typedef ttLibC_Util_StlList ttLibC_StlList;

typedef bool (* ttLibC_StlListRefFunc)(void *ptr, void *item);

ttLibC_StlList *ttLibC_StlList_make();

bool ttLibC_StlList_addFirst(
		ttLibC_StlList *list,
		void *add_item);

bool ttLibC_StlList_addLast(
		ttLibC_StlList *list,
		void *add_item);
bool ttLibC_StlList_remove(
		ttLibC_StlList *list,
		void *remote_item);
bool ttLibC_StlList_removeAll(ttLibC_StlList *list);
bool ttLibC_StlList_forEach(
		ttLibC_StlList *list,
		ttLibC_StlListRefFunc callback,
		void *ptr);
bool ttLibC_StlList_forEachReverse(
		ttLibC_StlList *list,
		ttLibC_StlListRefFunc callback,
		void *ptr);
void ttLibC_StlList_close(ttLibC_StlList **list);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_UTIL_STLLISTUTIL_H_ */
