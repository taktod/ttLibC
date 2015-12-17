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

extern "C" {

typedef struct {
	ttLibC_StlMap inherit_super;
	std::map<void *, void *> *map;
} ttLibC_Util_StlMap_;

typedef ttLibC_Util_StlMap_ ttLibC_StlMap_;

ttLibC_StlMap *ttLibC_StlMap_make() {
	ttLibC_StlMap_ *map = (ttLibC_StlMap_ *)ttLibC_malloc(sizeof(ttLibC_StlMap_));
	if(map == NULL) {
		return NULL;
	}
	map->map = new std::map<void *, void *>();
	map->inherit_super.size = 0;
	return (ttLibC_StlMap *)map;
}

bool ttLibC_StlMap_put(
		ttLibC_StlMap *map,
		void *key,
		void *item) {
	ttLibC_StlMap_ *map_ = (ttLibC_StlMap_ *)map;
	map_->map->insert(std::pair<void *, void *>(key, item));
	map_->inherit_super.size = map_->map->size();
	return true;
}

bool ttLibC_StlMap_remove(
		ttLibC_StlMap *map,
		void *key) {
	ttLibC_StlMap_ *map_ = (ttLibC_StlMap_ *)map;
	map_->map->erase(key);
	map_->inherit_super.size = map_->map->size();
	return true;
}

bool ttLibC_StlMap_removeAll(ttLibC_StlMap *map) {
	ttLibC_StlMap_ *map_ = (ttLibC_StlMap_ *)map;
	map_->map->clear();
	map_->inherit_super.size = map_->map->size();
	return true;
}

bool ttLibC_StlMap_forEach(
		ttLibC_StlMap *map,
		ttLibC_StlMapRefFunc callback,
		void *ptr) {
	ttLibC_StlMap_ *map_ = (ttLibC_StlMap_ *)map;
	std::map<void *, void *>::iterator iter = map_->map->begin();
	while(iter != map_->map->end()) {
		void *key = iter->first;
		void *item = iter->second;
		++ iter;
		if(!callback(ptr, key, item)) {
			return false;
		}
	}
	return true;
}

void ttLibC_StlMap_close(ttLibC_StlMap **map) {
	ttLibC_StlMap_ *target = (ttLibC_StlMap_ *)*map;
	if(target == NULL) {
		return;
	}
	delete target->map;
	ttLibC_free(target);
	*map = NULL;
}

}
