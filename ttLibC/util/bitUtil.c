/*
 * @file   bitUtil.c
 * @brief  bit work support.
 *
 * this code is under 3-Cause BSD license
 *
 * @author taktod
 * @date   2015/08/01
 */

#include "bitUtil.h"

#include <stdlib.h>

#include "../log.h"

typedef struct {
	/** inherit data from ttLibC_BitReader */
	ttLibC_BitReader inherit_super;
	/** reading target position. */
	uint32_t pos;
	/** data */
	uint8_t *data;
	/** data size */
	size_t data_size;
} ttLibC_Util_BitReader_;

typedef ttLibC_Util_BitReader_ ttLibC_BitReader_;

/*
 * make bit reader object.
 * @param data
 * @param data_size
 * @return bit reader object.
 */
ttLibC_BitReader *ttLibC_BitReader_make(void *data, size_t data_size) {
	ttLibC_BitReader_ *reader = (ttLibC_BitReader_ *)malloc(sizeof(ttLibC_BitReader_));
	reader->pos = 0;
	reader->data = data;
	reader->data_size = data_size;
	return (ttLibC_BitReader *)reader;
}

/*
 * get bit from bit reader.
 * @param reader  target bit reader object
 * @param bit_num bit size for read.
 * @return number
 */
uint32_t ttLibC_BitReader_bit(ttLibC_BitReader *reader, uint32_t bit_num) {
	ttLibC_BitReader_ *reader_ = (ttLibC_BitReader_ *)reader;
	uint8_t val = 0;
	uint32_t result = 0;

	uint32_t bit = 0;
	uint32_t mask = 0;
	uint32_t shift = 0;
	do {
		if(reader_->data_size <= 0) {
			ERR_PRINT("no more data buffer.");
			return 0;
		}
		val = *reader_->data;
		bit = 8 - reader_->pos > bit_num ? bit_num : 8 - reader_->pos;
		result <<= bit;
		mask = (1 << bit) - 1;
		shift = 8 - reader_->pos - bit;
		result = result | ((val >> shift) & mask);
		reader_->pos += bit;
		if(reader_->pos == 8) {
			++ reader_->data;
			-- reader_->data_size;
		}
		reader_->pos = reader_->pos & 0x07;
		bit_num -= bit;
	}while(bit_num > 0);
	return result;
}

/*
 * get exp golomb from bit reader.
 * @param reader target bit reader
 * @param sign   true: signed false:unsigned
 * @return value
 */
int32_t ttLibC_BitReader_expGolomb(ttLibC_BitReader *reader, bool sign) {
	ttLibC_BitReader_ *reader_ = (ttLibC_BitReader_ *)reader;
	uint8_t bit = 0;
	uint32_t count = 1;
	uint32_t val = *reader_->data;
	do {
		bit = (1 << (7 - reader_->pos));
		if((val & bit) != 0) {
			break;
		}
		count ++;
		reader_->pos ++;
		if(reader_->pos == 8) {
			++ reader_->data;
			if(reader_->data_size == 0) {
				ERR_PRINT("no more data.");
				return 0;
			}
			-- reader_->data_size;
			val = *reader_->data;
			reader_->pos = 0;
		}
	} while(true);
	if(count > 32) {
		ERR_PRINT("too big exp golumb.");
		return 0;
	}
	val = ttLibC_BitReader_bit(reader, count);
	if(val == 0) {
		return 0;
	}
	if(sign) {
		if((val & 1) != 0) {
			// minus
			return -1 * (val >> 1);
		}
		else {
			return (val >> 1);
		}
	}
	else {
		return ttLibC_BitReader_bit(reader, count) - 1;
	}
}

/*
 * get ebmp value from bit reader.
 * @param reader target bit reader
 * @param is_tag true: read as tag, false:read as number.
 * @return number
 */
uint64_t ttLibC_BitReader_ebml(ttLibC_BitReader *reader, bool is_tag) {
	ttLibC_BitReader_ *reader_ = (ttLibC_BitReader_ *)reader;
	if(reader_->data_size == 0) {
		ERR_PRINT("no more data.");
		return 0;
	}
	if(reader_->pos != 0) {
		ERR_PRINT("ebml value need to begin with full byte.");
		return 0;
	}
	uint64_t val = *reader_->data;
	++ reader_->data;
	-- reader_->data_size;
	if(val & 0x80) {
		if(is_tag) {
			return val;
		}
		else {
			return val & 0x7F;
		}
	}
	else if(val & 0x40) {
		if(reader_->data_size < 1) {
			ERR_PRINT("no more data.");
			reader_->data += reader_->data_size;
			reader_->data_size = 0;
			return 0;
		}
		val = (val << 8) | (*reader_->data);
		++ reader_->data;
		-- reader_->data_size;
		if(is_tag) {
			return val;
		}
		else {
			return val & 0x3FFF;
		}
	}
	else if(val & 0x20) {
		if(reader_->data_size < 2) {
			ERR_PRINT("no more data.");
			reader_->data += reader_->data_size;
			reader_->data_size = 0;
			return 0;
		}
		val = (val << 16) | (*(reader_->data) << 8) | (*(reader_->data + 1));
		reader_->data += 2;
		reader_->data_size -= 2;
		if(is_tag) {
			return val;
		}
		else {
			return val & 0x1FFFFF;
		}
	}
	else if(val & 0x10) {
		if(reader_->data_size < 3) {
			ERR_PRINT("no more data.");
			reader_->data += reader_->data_size;
			reader_->data_size = 0;
			return 0;
		}
		val = (val << 24) | (*(reader_->data) << 16) | (*(reader_->data + 1) << 8) | (*(reader_->data + 2));
		reader_->data += 3;
		reader_->data_size -= 3;
		if(is_tag) {
			return val;
		}
		else {
			return val & 0x0FFFFFFF;
		}
	}
	else if(val & 0x08) {
		if(reader_->data_size < 4) {
			ERR_PRINT("no more data.");
			reader_->data += reader_->data_size;
			reader_->data_size = 0;
			return 0;
		}
		val = (val << 32) | (*(reader_->data) << 24) | (*(reader_->data + 1) << 16) | (*(reader_->data + 2) << 8) | (*(reader_->data + 3));
		reader_->data += 4;
		reader_->data_size -= 4;
		if(is_tag) {
			return val;
		}
		else {
			return val & 0x07FFFFFFFF;
		}
	}
	else if(val & 0x04) {
		if(reader_->data_size < 5) {
			ERR_PRINT("no more data.");
			reader_->data += reader_->data_size;
			reader_->data_size = 0;
			return 0;
		}
		val = (val << 8) | (*(reader_->data));
		val = (val << 32) | (*(reader_->data + 1) << 24) | (*(reader_->data + 2) << 16) | (*(reader_->data + 3) << 8) | (*(reader_->data + 4));
		reader_->data += 5;
		reader_->data_size -= 5;
		if(is_tag) {
			return val;
		}
		else {
			return val & 0x03FFFFFFFFFF;
		}
	}
	else if(val & 0x02) {
		if(reader_->data_size < 6) {
			ERR_PRINT("no more data.");
			reader_->data += reader_->data_size;
			reader_->data_size = 0;
			return 0;
		}
		val = (val << 16) | (*(reader_->data) << 8) | (*(reader_->data + 1));
		val = (val << 32) | (*(reader_->data + 2) << 24) | (*(reader_->data + 3) << 16) | (*(reader_->data + 4) << 8) | (*(reader_->data + 5));
		reader_->data += 6;
		reader_->data_size -= 6;
		if(is_tag) {
			return val;
		}
		else {
			return val & 0x01FFFFFFFFFFFF;
		}
	}
	else if(val & 0x01) {
		if(reader_->data_size < 7) {
			ERR_PRINT("no more data.");
			reader_->data += reader_->data_size;
			reader_->data_size = 0;
			return 0;
		}
		val = (val << 24) | (*(reader_->data) << 16) | (*(reader_->data + 1) << 8) | (*(reader_->data + 2));
		val = (val << 32) | (*(reader_->data + 3) << 24) | (*(reader_->data + 4) << 16) | (*(reader_->data + 5) << 8) | (*(reader_->data + 6));
		reader_->data += 7;
		reader_->data_size -= 7;
		if(is_tag) {
			return val;
		}
		else {
			return val & 0x00FFFFFFFFFFFFFF;
		}
	}
	else {
		ERR_PRINT("invalid ebml value.");
	}
	return 0;
}

/*
 * close bit reader.
 * @param reader
 */
void ttLibC_BitReader_close(ttLibC_BitReader **reader) {
	ttLibC_BitReader_ *target = (ttLibC_BitReader_ *)*reader;
	if(target == NULL) {
		return;
	}
	free(target);
	*reader = NULL;
}
