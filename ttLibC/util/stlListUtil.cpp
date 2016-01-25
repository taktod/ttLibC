/*
 * @file   stlListUtil.cpp
 * @brief  std::list support.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2015/11/30
 */

#include "stlListUtil.h"
#include <list>
#include "../log.h"
#include "../allocator.h"
#include <string.h>

typedef struct {
	ttLibC_StlList inherit_super;
	std::list<void *> *list;
} ttLibC_Util_StlList_;

typedef ttLibC_Util_StlList_ ttLibC_StlList_;

static ttLibC_StlList_ *StlList_make() {
	ttLibC_StlList_ *list = (ttLibC_StlList_ *)ttLibC_malloc(sizeof(ttLibC_StlList_));
	if(list == NULL) {
		return NULL;
	}
	list->list = new std::list<void *>();
	list->inherit_super.size = 0;
	return list;
}

static bool StlList_addFirst(
		ttLibC_StlList_ *list,
		void *add_item) {
	list->list->push_front(add_item);
	list->inherit_super.size = list->list->size();
	return true;
}

static bool StlList_addLast(
		ttLibC_StlList_ *list,
		void *add_item) {
	list->list->push_back(add_item);
	list->inherit_super.size = list->list->size();
	return true;
}

static void *StlList_refFirst(ttLibC_StlList_ *list) {
	return (void *)list->list->front();
}

static void *StlList_refLast(ttLibC_StlList_ *list) {
	return (void *)list->list->back();
}

static bool StlList_remove(
		ttLibC_StlList_ *list,
		void *remove_item) {
	list->list->remove(remove_item);
	list->inherit_super.size = list->list->size();
//	list->list->erase(remove_item);
	return true;
}

static bool StlList_removeAll(ttLibC_StlList_ *list) {
	list->list->clear();
	list->inherit_super.size = list->list->size();
	return true;
}

static bool StlList_forEach(
		ttLibC_StlList_ *list,
		ttLibC_StlListRefFunc callback,
		void *ptr) {
	uint32_t original_size = list->list->size();
	std::list<void *>::iterator iter = list->list->begin();
	while(iter != list->list->end()) {
		void *item = *iter;
		++ iter;
		if(!callback(ptr, item)) {
			return false;
		}
		if(list->list->size() != original_size) {
			// element size is changed in loop.
			break;
		}
	}
	return true;
}

bool StlList_forEachReverse(
		ttLibC_StlList_ *list,
		ttLibC_StlListRefFunc callback,
		void *ptr) {
	uint32_t original_size = list->list->size();
	std::list<void *>::reverse_iterator iter = list->list->rbegin();
	while(iter != list->list->rend()) {
		void *item = *iter;
		++ iter;
		if(!callback(ptr, item)) {
			return false;
		}
		if(list->list->size() != original_size) {
			// element size is changed in loop.
			break;
		}
	}
	return true;

}

static void StlList_close(ttLibC_StlList_ **list) {
	ttLibC_StlList_ *target = *list;
	if(target == NULL) {
		return;
	}
	delete target->list;
	ttLibC_free(target);
	*list = NULL;
}

extern "C" {

ttLibC_StlList *ttLibC_StlList_make() {
	return (ttLibC_StlList *)StlList_make();
}

bool ttLibC_StlList_addFirst(
		ttLibC_StlList *list,
		void *add_item) {
	return StlList_addFirst(
			(ttLibC_StlList_ *)list,
			add_item);
}

bool ttLibC_StlList_addLast(
		ttLibC_StlList *list,
		void *add_item) {
	return StlList_addLast(
			(ttLibC_StlList_ *)list,
			add_item);
}

void *ttLibC_StlList_refFirst(ttLibC_StlList *list) {
	return StlList_refFirst((ttLibC_StlList_ *)list);
}

void *ttLibC_StlList_refLast(ttLibC_StlList *list) {
	return StlList_refLast((ttLibC_StlList_ *)list);
}

bool ttLibC_StlList_remove(
		ttLibC_StlList *list,
		void *remove_item) {
	return StlList_remove(
			(ttLibC_StlList_ *)list,
			remove_item);
}

bool ttLibC_StlList_removeAll(ttLibC_StlList *list) {
	return StlList_removeAll((ttLibC_StlList_ *)list);
}

bool ttLibC_StlList_forEach(
		ttLibC_StlList *list,
		ttLibC_StlListRefFunc callback,
		void *ptr) {
	return StlList_forEach(
			(ttLibC_StlList_ *)list,
			callback,
			ptr);
}

bool ttLibC_StlList_forEachReverse(
		ttLibC_StlList *list,
		ttLibC_StlListRefFunc callback,
		void *ptr) {
	return StlList_forEachReverse(
			(ttLibC_StlList_ *)list,
			callback,
			ptr);
}
void ttLibC_StlList_close(ttLibC_StlList **list) {
	StlList_close((ttLibC_StlList_ **)list);
}

}
