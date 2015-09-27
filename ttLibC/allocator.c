/*
 * @file   allocator.c
 * @brief  custom memory control for memory leak.
 *
 * this code is under 3-Cause BSD license
 *
 * @author taktod
 * @date   2015/09/07
 */

#include "log.h"

#include "allocator.h"

#if __DEBUG_FLG__ == 1
#	include "khash.h"

KHASH_MAP_INIT_INT64(ttLibC_Allocator, void *)
static khash_t(ttLibC_Allocator) *ttLibC_Allocator_Table = NULL;

typedef struct {
	size_t alloc_size;
	char alloc_info[256];
} ttLibC_Allocator_Info;
#endif

/*
 * malloc with information.
 * @param size      allocate size
 * @param file_name caller file name
 * @param line      caller line number
 * @param func_name caller func name
 * @return memory pointer
 */
void *ttLibC_Allocator_malloc(size_t size, const char *file_name, int line, const char *func_name) {
	int ret;
	void *ptr = malloc(size);
#if __DEBUG_FLG__ == 1
	if(ptr) {
		if(ttLibC_Allocator_Table != NULL) {
			ttLibC_Allocator_Info *info = malloc(sizeof(ttLibC_Allocator_Info));
			info->alloc_size = size;
			sprintf(info->alloc_info, "%s(%d) %s sz:%lu %x", file_name, line, func_name, size, (int)ptr);
			khiter_t it = kh_put(ttLibC_Allocator, ttLibC_Allocator_Table, (uint64_t)ptr, &ret);
			kh_value(ttLibC_Allocator_Table, it) = info;
		}
	}
#endif
	return ptr;
}

/*
 * free with information
 * @param ptr memory pointer
 */
void ttLibC_Allocator_free(void *ptr) {
	if(ptr) {
#if __DEBUG_FLG__ == 1
		if(ttLibC_Allocator_Table != NULL) {
			khiter_t it = kh_get(ttLibC_Allocator, ttLibC_Allocator_Table, (uint64_t)ptr);
			free(kh_value(ttLibC_Allocator_Table, it));
			kh_del(ttLibC_Allocator, ttLibC_Allocator_Table, it);
		}
#endif
		free(ptr);
	}
}

/*
 * initialize information table.
 * @return true:success false:error(ignore info collecting.)
 */
bool ttLibC_Allocator_init() {
#if __DEBUG_FLG__ == 1
	if(ttLibC_Allocator_Table == NULL) {
		ttLibC_Allocator_Table = kh_init(ttLibC_Allocator);
	}
#endif
	return ttLibC_Allocator_Table != NULL;
}

/*
 * dump current memory information.
 * @return total size of allocate.
 */
size_t ttLibC_Allocator_dump() {
	if(ttLibC_Allocator_Table == NULL) {
		return 0;
	}
#if __DEBUG_FLG__ == 1
	khiter_t it;
	size_t total_size = 0;
	printf("\nAllocator dump:\n");
	for(it = kh_begin(ttLibC_Allocator_Table);it != kh_end(ttLibC_Allocator_Table); ++ it) {
		if(kh_exist(ttLibC_Allocator_Table, it)) {
			ttLibC_Allocator_Info *info = (ttLibC_Allocator_Info *)kh_value(ttLibC_Allocator_Table, it);
			total_size += info->alloc_size;
			puts(info->alloc_info);
		}
	}
	printf("total_size:%lu\n", total_size);
	return total_size;
#endif
}

/*
 * close information table.
 */
void ttLibC_Allocator_close() {
#if __DEBUG_FLG__ == 1
	khiter_t it;
	for(it = kh_begin(ttLibC_Allocator_Table);it != kh_end(ttLibC_Allocator_Table); ++ it) {
		if(kh_exist(ttLibC_Allocator_Table, it)) {
			free(kh_value(ttLibC_Allocator_Table, it));
		}
	}
	kh_clear(ttLibC_Allocator, ttLibC_Allocator_Table);
	kh_destroy(ttLibC_Allocator, ttLibC_Allocator_Table);
	ttLibC_Allocator_Table = NULL;
#endif
}
