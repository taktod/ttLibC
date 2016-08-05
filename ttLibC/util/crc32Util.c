/*
 * @file   crc32Util.c
 * @brief  crc32 support
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/13
 */

#include "crc32Util.h"
#include <stdio.h>
#include <stdlib.h>
#include "../allocator.h"

/**
 * crc32 table make up on the first object.
 */
static uint32_t crc_table[256] = {0};

static const uint32_t POLYNOMINAL = 0x04C11DB7L;

/*
 * make crc32
 * @param initial_data
 * memo:for mpegts initial_data 0xFFFFFFFFL
 * @return crc32 object.
 */
ttLibC_Crc32 *ttLibC_Crc32_make(uint32_t initial_data) {
	ttLibC_Crc32 *crc32 = ttLibC_malloc(sizeof(ttLibC_Crc32));
	if(crc32 == NULL) {
		return NULL;
	}
	crc32->error = Error_noError;
	if(crc_table[1] == 0) {
		// not initialize.
		uint64_t crc = 0;
		for(int i = 0;i < 256; ++ i) {
			crc = i << 24;
			for(int j = 0;j < 8;++ j) {
				crc = (crc << 1) ^ ((crc & 0x80000000L) != 0 ? POLYNOMINAL : 0);
			}
			crc_table[i] = crc & 0xFFFFFFFFL;
		}
	}
	crc32->crc = initial_data;
	return crc32;
}

/*
 * update crc32 with byte data.
 * @param crc32 crc32 object.
 * @param byte  update byte value.
 */
void ttLibC_Crc32_update(ttLibC_Crc32 *crc32, uint8_t byte) {
	if(crc32 == NULL) {
		return;
	}
	crc32->crc = (crc32->crc << 8) ^ crc_table[(int)(((crc32->crc >> 24) ^ byte) & 0xFF)];
}

/*
 * get value
 * @param crc32
 */
uint32_t ttLibC_Crc32_getValue(ttLibC_Crc32 *crc32) {
	if(crc32 == NULL) {
		return 0;
	}
	return crc32->crc & 0xFFFFFFFFL;
}

/*
 * close crc32 object.
 * @param crc32
 */
void ttLibC_Crc32_close(ttLibC_Crc32 **crc32) {
	ttLibC_Crc32 *target = *crc32;
	if(target == NULL) {
		return;
	}
	ttLibC_free(target);
	*crc32 = NULL;
}
