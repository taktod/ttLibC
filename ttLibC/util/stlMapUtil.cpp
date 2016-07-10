/*
 * @file   stlMapUtil.cpp
 * @brief  std::map support.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2015/12/18
 */

#include "stlMapUtil.h"

#include <map>
#include "../allocator.h"
#include "../log.h"
#include <cstdint>

typedef struct {
	ttLibC_StlMap inherit_super;
	std::map<void *, void *> *map;
} ttLibC_Util_StlMap_;

typedef ttLibC_Util_StlMap_ ttLibC_StlMap_;

static ttLibC_StlMap_ *StlMap_make() {
	ttLibC_StlMap_ *map = (ttLibC_StlMap_ *)ttLibC_malloc(sizeof(ttLibC_StlMap_));
	if(map == NULL) {
		return NULL;
	}
	map->map = new std::map<void *, void *>();
	if(map->map == NULL) {
		ttLibC_free(map);
		return NULL;
	}
	map->inherit_super.size = 0;
	return map;
}

static bool StlMap_put(
		ttLibC_StlMap_ *map,
		void *key,
		void *item) {
	if(map == NULL) {
		return false;
	}
	map->map->insert(std::pair<void *, void *>(key, item));
	map->inherit_super.size = map->map->size();
	return true;
}

static void *StlMap_get(
		ttLibC_StlMap_ *map,
		void *key) {
	if(map == NULL) {
		return NULL;
	}
	std::map<void *, void *>::iterator iter = map->map->find(key);
	if(iter == map->map->end()) {
		return NULL;
	}
	return iter->second;
}

static bool StlMap_remove(
		ttLibC_StlMap_ *map,
		void *key) {
	if(map == NULL) {
		return false;
	}
	map->map->erase(key);
	map->inherit_super.size = map->map->size();
	return true;
}

static bool StlMap_removeAll(ttLibC_StlMap_ *map) {
	if(map == NULL) {
		return false;
	}
	map->map->clear();
	map->inherit_super.size = map->map->size();
	return true;
}

static bool StlMap_forEach(
		ttLibC_StlMap_ *map,
		ttLibC_StlMapRefFunc callback,
		void *ptr) {
	if(map == NULL) {
		return false;
	}
	uint32_t original_size = map->map->size();
	std::map<void *, void *>::iterator iter = map->map->begin();
	while(iter != map->map->end()) {
		void *key = iter->first;
		void *item = iter->second;
		++ iter;
		if(!callback(ptr, key, item)) {
			return false;
		}
		if(map->map->size() != original_size) {
			// element size is changed in loop.
			break;
		}
	}
	return true;
}

static void StlMap_close(ttLibC_StlMap_ **map) {
	ttLibC_StlMap_ *target = *map;
	if(target == NULL) {
		return;
	}
	delete target->map;
	ttLibC_free(target);
	*map = NULL;
}

extern "C" {

ttLibC_StlMap *ttLibC_StlMap_make() {
	return (ttLibC_StlMap *)StlMap_make();
}

bool ttLibC_StlMap_put(
		ttLibC_StlMap *map,
		void *key,
		void *item) {
	ttLibC_StlMap_remove(map, key);
	return StlMap_put(
			(ttLibC_StlMap_ *)map,
			key,
			item);
}

void *ttLibC_StlMap_get(
		ttLibC_StlMap *map,
		void *key) {
	return StlMap_get(
			(ttLibC_StlMap_ *)map,
			key);
}

bool ttLibC_StlMap_remove(
		ttLibC_StlMap *map,
		void *key) {
	return StlMap_remove(
			(ttLibC_StlMap_ *)map,
			key);
}

bool ttLibC_StlMap_removeAll(ttLibC_StlMap *map) {
	return StlMap_removeAll((ttLibC_StlMap_ *)map);
}

bool ttLibC_StlMap_forEach(
		ttLibC_StlMap *map,
		ttLibC_StlMapRefFunc callback,
		void *ptr) {
	return StlMap_forEach(
			(ttLibC_StlMap_ *)map,
			callback,
			ptr);
}

void ttLibC_StlMap_close(ttLibC_StlMap **map) {
	StlMap_close((ttLibC_StlMap_ **)map);
}

}
