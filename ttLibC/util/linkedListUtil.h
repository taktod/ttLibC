/**
 * @file   linkedListUtil.h
 * @brief  linkedList data structure support.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/11/26
 */

#ifndef TTLIBC_UTIL_LINKEDLISTUTIL_H_
#define TTLIBC_UTIL_LINKEDLISTUTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdio.h>

/**
 * definition of linked list object.
 */
typedef struct {
	/** element size */
	size_t size;
} ttLibC_Util_LinkedList;

typedef ttLibC_Util_LinkedList ttLibC_LinkedList;

/**
 * callback function for forEach func.
 * @param ptr       user def data pointer.
 * @param item      holding object.
 * @param item_size object size.
 */
typedef bool (* ttLibC_LinkedListRefFunc)(void *ptr, void *item, size_t item_size);

/**
 * make linkedList object.
 * @return linkedList object.
 */
ttLibC_LinkedList *ttLibC_LinkedList_make();

/**
 * add the item object on the top.
 * @param list          linkedList object.
 * @param add_item      target add item.
 * @param add_item_size target add item size.
 * @param is_non_copy   if true:hold the data pointer. false: allocate and copy data.
 * @return true:success false:error
 */
bool ttLibC_LinkedList_addFirst(
		ttLibC_LinkedList *list,
		void *add_item,
		size_t add_item_size,
		bool is_non_copy);

/**
 * add the item object on the last.
 * @param list          linkedList object.
 * @param add_item      target add item.
 * @param add_item_size target add item size.
 * @param is_non_copy   if true:hold the data pointer. false: allocate and copy data.
 * @return true:success false:error
 */
bool ttLibC_LinkedList_addLast(
		ttLibC_LinkedList *list,
		void *add_item,
		size_t add_item_size,
		bool is_non_copy);

/*
 以下の関数については、itemがどのデータであるか、区別するのが難しいため
 primitive型の場合はpointerで確認できない。
 is_non_copyであるかどうかでも判定がしにくい。
 あとでどうしても必要になったら実装しようと思う。
 あ・・でもtettyの動作で特定のクライアントがcloseしたとき・・・という動作がはいったら・・・微妙になるか・・・
 やっぱり必要っぽいな・・・
bool ttLibC_LinkedList_addAfter(
		ttLibC_LinkedList *list,
		void *item,
		void *add_item,
		size_t add_item_size,
		bool is_non_copy);
bool ttLibC_LinkedList_addBefore(
		ttLibC_LinkedList *list,
		void *item,
		void *add_item,
		size_t add_item_size,
		bool is_non_copy);
*/

// この関数はforEachの中でのみ有効なものとする。(forEachで応答するitemのaddressを頼りに削除しようとすると定義する。)
bool ttLibC_LinkedList_remove(
		ttLibC_LinkedList *list,
		void *remove_item);
bool ttLibC_LinkedList_removeAll(ttLibC_LinkedList *list);
// 関数で見つけたものを応答する形にしたい。
bool ttLibC_LinkedList_forEach(
		ttLibC_LinkedList *list,
		ttLibC_LinkedListRefFunc callback,
		void *ptr);

void ttLibC_LinkedList_close(ttLibC_LinkedList **list);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_UTIL_LINKEDLISTUTIL_H_ */
