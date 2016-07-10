/*
 * @file   hexUtil.h
 * @brief  library deal with hex data.
 *
 * this code is under 3-Cause BSD license.
 *
 * @see    hexUtilTest()
 * @author taktod
 * @date   2015/07/18
 */

#include "hexUtil.h"
#include "../ttLibC_common.h"

#include <ctype.h>
#include <string.h>

static char hex_digits[16] = { \
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', \
		'A', 'B', 'C', 'D', 'E', 'F'};

/*
 * dump memory data.
 * @param ptr            target data
 * @param length         data length
 * @param separator_flag insert space among each byte
 */
void ttLibC_HexUtil_dump(void *ptr, size_t length, bool separator_flag) {
	char *cptr = ptr;
	for(uint32_t i = 0;i < length;++ i) {
		printf("%c", hex_digits[(cptr[i] & 0xF0) >> 4]);
		printf("%c", hex_digits[(cptr[i] & 0x0F)]);
		if(separator_flag) {
			printf(" ");
		}
	}
	puts("");
}

/*
 * make void* buffer according to hex string
 * @param target hex string
 * @param ptr    data to store
 * @param length size of ptr
 * @return size of filled data
 */
uint32_t ttLibC_HexUtil_makeBuffer(const char *target, void *ptr, size_t length) {
	uint64_t counter;
	ttLibC_HexUtil_makeBuffer2(target, ptr, (uint64_t)length, &counter);
	return (uint32_t)counter;
}

/**
 * make void* buffer according to hex string
 * @param target       hex string
 * @param ptr          data to store
 * @param length       size of ptr
 * @param written_size size of written
 * @return ErrorCode
 */
Error_e ttLibC_HexUtil_makeBuffer2(const char *target, void *ptr, size_t length, uint64_t *written_size) {
	size_t target_length = strlen(target);
	uint32_t counter = 0;
	char *cptr = ptr;
	char tmp = 0;
	bool first = true;
	uint32_t i, j;
	Error_e error = Error_noError;
	for(i = 0, j = 0;i < target_length && j < length; ++ i) {
		char buf = toupper(target[i]);
		if(buf == ' ') {
			continue;
		}
		char tmpval = 0;
		if(buf >= '0' && buf <= '9') {
			tmpval = 0 + buf - '0';
		}
		else if(buf >= 'A' && buf <= 'F') {
			tmpval = 10 + buf - 'A';
		}
		if(first) {
			tmp = (tmpval << 4) & 0xF0;
			first = false;
		}
		else {
			tmp = tmp | (tmpval & 0x0F);
			cptr[j] = tmp;
			j ++;
			first = true;
			counter ++;
		}
	}
	*written_size = counter;
	if(!(i < target_length)) {
		error = Error_MemoryShort;
	}
	return ttLibC_updateError(Target_On_Util, error);
}
