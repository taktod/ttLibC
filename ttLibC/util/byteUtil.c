/*
 * @file   byteUtil.c
 * @brief  byte work support.
 *
 * this code is under 3-Cause BSD license
 *
 * @author taktod
 * @date   2015/10/17
 * @note need little endian mode?
 */

#include "byteUtil.h"

#include <stdlib.h>
#include <string.h>

#include "../ttLibC_predef.h"
#include "../_log.h"
#include "../allocator.h"
#include "../ttLibC_common.h"

/*
 * ByteReader object
 */
typedef struct {
	/** inherit data from ttLibC_ByteReader */
	ttLibC_ByteReader inherit_super;
	/** reading target position */
	uint32_t pos;
	/** data */
	uint8_t *data;
	/** data_size */
	size_t data_size;
	/** zero count */
	uint8_t zero_count; // for emulation prevention three byte(h264 h265)
} ttLibC_Util_ByteReader_;

typedef ttLibC_Util_ByteReader_ ttLibC_ByteReader_;

/*
 * make ByteReader object.
 * @param data      target data
 * @param data_size target data size
 * @param type      target data type
 * @return byte reader object.
 */
ttLibC_ByteReader TT_VISIBILITY_DEFAULT *ttLibC_ByteReader_make(
		void *data,
		size_t data_size,
		ttLibC_ByteUtil_Type type) {
	ttLibC_ByteReader_ *reader = (ttLibC_ByteReader_ *)ttLibC_malloc(sizeof(ttLibC_ByteReader_));
	if(reader == NULL) {
		return NULL;
	}
	reader->zero_count = 0;
	reader->pos = 0;
	reader->data = (uint8_t *)data;
	reader->data_size = data_size;
	reader->inherit_super.type = type;
	reader->inherit_super.read_size = 0;
	reader->inherit_super.error_number = 0;
	reader->inherit_super.error = Error_noError;
	if(*reader->data == 0) {
		++ reader->zero_count;
	}
	return (ttLibC_ByteReader *)reader;
}

/*
 * get bit from ByteReader
 * @param reader
 * @param bit_num
 * @return value
 */
uint64_t TT_VISIBILITY_DEFAULT ttLibC_ByteReader_bit(
		ttLibC_ByteReader *reader,
		uint32_t bit_num) {
	ttLibC_ByteReader_ *reader_ = (ttLibC_ByteReader_ *)reader;
	if(reader_ == NULL) {
		return 0;
	}
	uint8_t val = 0;
	uint64_t result = 0;

	uint32_t bit = 0;
	uint32_t mask = 0;
	uint32_t shift = 0;

	val = *reader_->data;
	do {
		bit = 8 - reader_->pos > bit_num ? bit_num : 8 - reader_->pos;
		result <<= bit;
		mask = (1 << bit) - 1;
		shift = 8 - reader_->pos - bit;
		result = result | ((val >> shift) & mask);
		reader_->pos += bit;
		if(reader_->pos == 8) {
			++ reader_->data;
			if(reader_->data_size <= 0) {
				LOG_PRINT("no more data buffer.");
				reader_->inherit_super.error_number = 1;
				reader_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_NeedMoreInput);
				return 0;
			}
			++ reader_->inherit_super.read_size;
			-- reader_->data_size;
			val = *reader_->data;
			if(reader_->inherit_super.type == ByteUtilType_h26x) {
				if(val == 0) {
					++ reader_->zero_count;
				}
				else if(val == 3 && reader_->zero_count == 2) {
					++ reader_->data;
					if(reader_->data_size == 0) {
						LOG_PRINT("no more data.");
						reader_->inherit_super.error_number = 1;
						reader_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_NeedMoreInput);
						return 0;
					}
					-- reader_->data_size;
					++ reader_->inherit_super.read_size;
					val = *reader_->data;
					if(val == 0) {
						reader_->zero_count = 1;
					}
					else {
						reader_->zero_count = 0;
					}
				}
				else {
					reader_->zero_count = 0;
				}
			}
		}
		reader_->pos = reader_->pos & 0x07;
		bit_num -= bit;
	} while(bit_num > 0);
	return result;
}

/*
 * get exp golomb value from ByteReader
 * @param reader
 * @param sign
 * @return value
 */
int32_t TT_VISIBILITY_DEFAULT ttLibC_ByteReader_expGolomb(
		ttLibC_ByteReader *reader,
		bool sign) {
	ttLibC_ByteReader_ *reader_ = (ttLibC_ByteReader_ *)reader;
	if(reader_ == NULL) {
		return 0;
	}
	uint8_t bit = 0;
	uint32_t count = 1;
	uint32_t val = *reader_->data;
	do {
		bit = (1 << (7 - reader_->pos));
		if((val & bit) != 0) {
			break;
		}
		++ count;
		++ reader_->pos;
		if(reader_->pos == 8) {
			++ reader_->data;
			if(reader_->data_size == 0) {
				LOG_PRINT("no more data.");
				reader_->inherit_super.error_number = 1;
				reader_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_NeedMoreInput);
				return 0;
			}
			++ reader_->inherit_super.read_size;
			-- reader_->data_size;
			val = *reader_->data;
			if(reader_->inherit_super.type == ByteUtilType_h26x) {
				if(val == 0) {
					++ reader_->zero_count;
				}
				else if(val == 3 && reader_->zero_count == 2) {
					++ reader_->data;
					if(reader_->data_size == 0) {
						LOG_PRINT("no more data.");
						reader_->inherit_super.error_number = 1;
						reader_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_NeedMoreInput);
						return 0;
					}
					++ reader_->inherit_super.read_size;
					-- reader_->data_size;
					val = *reader_->data;
					if(val == 0) {
						reader_->zero_count = 1;
					}
					else {
						reader_->zero_count = 0;
					}
				}
				else {
					reader_->zero_count = 0;
				}
			}
			reader_->pos = 0;
		}
	} while(true);
	if(count > 32) {
		ERR_PRINT("too big exp golomb.");
		reader_->inherit_super.error_number = 1;
		reader_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_BrokenInput);
		return 0;
	}
	val = ttLibC_ByteReader_bit(reader, count);
	if(val == 0) {
		return 0;
	}
	if(sign) {
		if((val & 1) != 0) {
			return -1 * (val >> 1);
		}
		else {
			return (val >> 1);
		}
	}
	else {
		return val - 1;
	}
}

/*
 * get ebml value from ByteReader
 * @param reader
 * @param is_tag
 * @return value
 */
uint64_t TT_VISIBILITY_DEFAULT ttLibC_ByteReader_ebml(
		ttLibC_ByteReader *reader,
		bool is_tag) {
	ttLibC_ByteReader_ *reader_ = (ttLibC_ByteReader_ *)reader;
	if(reader_ == NULL) {
		return 0;
	}
	if(reader_->data_size == 0) {
		LOG_PRINT("no more data.");
		reader_->inherit_super.error_number = 1;
		reader_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_NeedMoreInput);
		return 0;
	}
	if(reader_->pos != 0) {
		ERR_PRINT("ebml value need to begin with full byte");
		reader_->inherit_super.error_number = 1;
		reader_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_BrokenInput);
		return 0;
	}
	uint64_t val = *reader_->data;
	++ reader_->data;
	-- reader_->data_size;
	++ reader_->inherit_super.read_size;
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
			LOG_PRINT("no more data.");
			reader_->data += reader_->data_size;
			reader_->data_size = 0;
			reader_->inherit_super.error_number = 1;
			reader_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_NeedMoreInput);
			return 0;
		}
		val = (val << 8) | (*reader_->data);
		++ reader_->data;
		-- reader_->data_size;
		++ reader_->inherit_super.read_size;
		if(is_tag) {
			return val;
		}
		else {
			return val & 0x3FFF;
		}
	}
	else if(val & 0x20) {
		if(reader_->data_size < 2) {
			LOG_PRINT("no more data.");
			reader_->data += reader_->data_size;
			reader_->data_size = 0;
			reader_->inherit_super.error_number = 1;
			reader_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_NeedMoreInput);
			return 0;
		}
		val = (val << 16) | (*(reader_->data) << 8) | (*(reader_->data + 1));
		reader_->data += 2;
		reader_->data_size -= 2;
		reader_->inherit_super.read_size += 2;
		if(is_tag) {
			return val;
		}
		else {
			return val & 0x1FFFFF;
		}
	}
	else if(val & 0x10) {
		if(reader_->data_size < 3) {
			LOG_PRINT("no more data.");
			reader_->data += reader_->data_size;
			reader_->data_size = 0;
			reader_->inherit_super.error_number = 1;
			reader_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_NeedMoreInput);
			return 0;
		}
		val = (val << 24) | (*(reader_->data) << 16) | (*(reader_->data + 1) << 8) | (*(reader_->data + 2));
		reader_->data += 3;
		reader_->data_size -= 3;
		reader_->inherit_super.read_size += 3;
		if(is_tag) {
			return val;
		}
		else {
			return val & 0x0FFFFFFFL;
		}
	}
	else if(val & 0x08) {
		if(reader_->data_size < 4) {
			LOG_PRINT("no more data.");
			reader_->data += reader_->data_size;
			reader_->data_size = 0;
			reader_->inherit_super.error_number = 1;
			reader_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_NeedMoreInput);
			return 0;
		}
		val = (val << 32) | (*(reader_->data) << 24) | (*(reader_->data + 1) << 16) | (*(reader_->data + 2) << 8) | (*(reader_->data + 3));
		reader_->data += 4;
		reader_->data_size -= 4;
		reader_->inherit_super.read_size += 4;
		if(is_tag) {
			return val;
		}
		else {
			return val & 0x07FFFFFFFFL;
		}
	}
	else if(val & 0x04) {
		if(reader_->data_size < 5) {
			LOG_PRINT("no more data.");
			reader_->data += reader_->data_size;
			reader_->data_size = 0;
			reader_->inherit_super.error_number = 1;
			reader_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_NeedMoreInput);
			return 0;
		}
		val = (val << 8) | (*(reader_->data));
		val = (val << 32) | (*(reader_->data + 1) << 24) | (*(reader_->data + 2) << 16) | (*(reader_->data + 3) << 8) | (*(reader_->data + 4));
		reader_->data += 5;
		reader_->data_size -= 5;
		reader_->inherit_super.read_size += 5;
		if(is_tag) {
			return val;
		}
		else {
			return val & 0x03FFFFFFFFFFL;
		}
	}
	else if(val & 0x02) {
		if(reader_->data_size < 6) {
			LOG_PRINT("no more data.");
			reader_->data += reader_->data_size;
			reader_->data_size = 0;
			reader_->inherit_super.error_number = 1;
			reader_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_NeedMoreInput);
			return 0;
		}
		val = (val << 16) | (*(reader_->data) << 8) | (*(reader_->data + 1));
		val = (val << 32) | (*(reader_->data + 2) << 24) | (*(reader_->data + 3) << 16) | (*(reader_->data + 4) << 8) | (*(reader_->data + 5));
		reader_->data += 6;
		reader_->data_size -= 6;
		reader_->inherit_super.read_size += 6;
		if(is_tag) {
			return val;
		}
		else {
			return val & 0x01FFFFFFFFFFFFL;
		}
	}
	else if(val & 0x01) {
		if(reader_->data_size < 7) {
			LOG_PRINT("no more data.");
			reader_->data += reader_->data_size;
			reader_->data_size = 0;
			reader_->inherit_super.error_number = 1;
			reader_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_NeedMoreInput);
			return 0;
		}
		val = (val << 24) | (*(reader_->data) << 16) | (*(reader_->data + 1) << 8) | (*(reader_->data + 2));
		val = (val << 32) | (*(reader_->data + 3) << 24) | (*(reader_->data + 4) << 16) | (*(reader_->data + 5) << 8) | (*(reader_->data + 6));
		reader_->data += 7;
		reader_->data_size -= 7;
		reader_->inherit_super.read_size += 7;
		if(is_tag) {
			return val;
		}
		else {
			return val & 0x00FFFFFFFFFFFFFFL;
		}
	}
	else {
		ERR_PRINT("invalid ebml value.");
		reader_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_BrokenInput);
	}
	return Error_noError;
}

/*
 * get string data from ByteReader
 * @param reader
 * @param buffer
 * @param buffer_size
 * @param target_size
 * @return read size.
 */
size_t TT_VISIBILITY_DEFAULT ttLibC_ByteReader_string(
		ttLibC_ByteReader *reader,
		char *buffer,
		size_t buffer_size,
		size_t target_size) {
	ttLibC_ByteReader_ *reader_ = (ttLibC_ByteReader_ *)reader;
	if(reader_ == NULL) {
		return 0;
	}
	if(buffer_size < target_size) {
		ERR_PRINT("buffer size is too small for reading size.");
		reader_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_NeedMoreOutput);
		return 0;
	}
	if(reader_->data_size < target_size) {
		LOG_PRINT("hold buffer size is smaller than target_size");
		reader_->inherit_super.error_number = 1;
		reader_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_NeedMoreInput);
		return 0;
	}
	memcpy(buffer, reader_->data, target_size);
	buffer[target_size] = '\0';
	reader_->data += target_size;
	reader_->data_size -= target_size;
	reader_->inherit_super.read_size += target_size;
	return target_size;
}

/*
 * skip several size of byte reading.
 * @param reader
 * @param skip_size size in byte num.
 */
size_t TT_VISIBILITY_DEFAULT ttLibC_ByteReader_skipByte(
		ttLibC_ByteReader *reader,
		size_t skip_size) {
	ttLibC_ByteReader_ *reader_ = (ttLibC_ByteReader_ *)reader;
	if(reader_->data_size < skip_size) {
		LOG_PRINT("hold buffer size is smaller than skip_size.");
		reader_->inherit_super.error_number = 1;
		reader_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_NeedMoreInput);
		return 0;
	}
	reader_->data += skip_size;
	reader_->data_size -= skip_size;
	reader_->inherit_super.read_size += skip_size;
	return skip_size;
}

/*
 * rewind several size of byte reading.
 * (go backward.)
 * @param reader
 * @param rewind_size
 */
size_t TT_VISIBILITY_DEFAULT ttLibC_ByteReader_rewindByte(
		ttLibC_ByteReader *reader,
		size_t rewind_size) {
	ttLibC_ByteReader_ *reader_ = (ttLibC_ByteReader_ *)reader;
	// TODO check with global val, bad coder can put fake value on this to destroy program.
	if(reader_->inherit_super.read_size < rewind_size) {
		ERR_PRINT("cannot rewind before original start position.");
		reader_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_TtLibCError);
		return 0;
	}
	reader_->data -= rewind_size;
	reader_->data_size += rewind_size;
	reader_->inherit_super.read_size -= rewind_size;
	return rewind_size;
}

/*
 * close ByteReader
 * @param reader
 */
void TT_VISIBILITY_DEFAULT ttLibC_ByteReader_close(ttLibC_ByteReader **reader) {
	ttLibC_ByteReader_ *target = (ttLibC_ByteReader_ *)*reader;
	if(target == NULL) {
		return;
	}
	ttLibC_free(target);
	*reader = NULL;
}


/*
 * ByteConnector object
 */
typedef struct {
	/** inherit data from ttLibC_ByteConnector */
	ttLibC_ByteConnector inherit_super;
	/** bit position */
	uint32_t pos;
	/** target data */
	uint8_t *data;
	/** target data size */
	size_t data_size;
} ttLibC_Util_ByteConnector_;

typedef ttLibC_Util_ByteConnector_ ttLibC_ByteConnector_;

/*
 * make byteConnector object.
 * @param data      memory data, result is copyed on this buffer.
 * @param data_size data size
 * @param type      work type(now, default only.)
 * @return ByteConnector object.
 */
ttLibC_ByteConnector TT_VISIBILITY_DEFAULT *ttLibC_ByteConnector_make(
		void *data,
		size_t data_size,
		ttLibC_ByteUtil_Type type) {
	ttLibC_ByteConnector_ *connector = (ttLibC_ByteConnector_ *)ttLibC_malloc(sizeof(ttLibC_ByteConnector_));
	if(connector == NULL) {
		return NULL;
	}
	connector->pos = 0;
	connector->data = data;
	connector->data_size = data_size;
	connector->inherit_super.type = type;
	connector->inherit_super.write_size = 0;
	connector->inherit_super.error_number = 0;
	connector->inherit_super.error = Error_noError;
	return (ttLibC_ByteConnector *)connector;
}

/*
 * write bit data into ByteConnector.
 * @param connector
 * @param value
 * @param bit_num
 * @return true:success false:error
 */
bool TT_VISIBILITY_DEFAULT ttLibC_ByteConnector_bit(
		ttLibC_ByteConnector *connector,
		uint32_t value,
		uint32_t bit_num) {
	ttLibC_ByteConnector_ *connector_ = (ttLibC_ByteConnector_ *)connector;
	if(connector_ == NULL) {
		return false;
	}
	do {
		if(connector_->pos == 0) {
			if(connector_->data_size == 0) {
				ERR_PRINT("buffer is fulled. no way to add more data.");
				connector_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_NeedMoreOutput);
				return false;
			}
			(*connector_->data) = 0;
			++ connector_->inherit_super.write_size;
		}
		uint32_t bit_mask = (1 << (8 - connector_->pos)) - 1;
		if(8 - connector_->pos < bit_num) {
			// right shift
			uint32_t shift_count = bit_num - 8 + connector_->pos;
			(*connector_->data) |= bit_mask & (value >> shift_count);
			// update bit_num;
			bit_num = bit_num - 8 + connector_->pos;
			connector_->pos = 0; // filled up byte data. so next data start with pos = 0;
			++ connector_->data;
			-- connector_->data_size;
		}
		else {
			// left shift
			uint32_t shift_count = 8 - connector_->pos - bit_num;
			(*connector_->data) |= bit_mask & (value << shift_count);
			connector_->pos = (connector_->pos + bit_num) & 0x07;
			if(connector_->pos == 0) {
				++ connector_->data;
				-- connector_->data_size;
			}
			break;
		}
	} while(true);
	return true;
}

/*
 * write expGolomb data into ByteConnector
 * @param connector
 * @param value
 * @param sign
 * @return true:success false:error
 */
/*
bool ttLibC_ByteConnector_expGolomb(
		ttLibC_ByteConnector *connector,
		int32_t value,
		bool sign) {

}
*/

/*
 * write ebml data into ByteConnector
 * @param connector
 * @param value
 * @return true:success false:error
 */
bool TT_VISIBILITY_DEFAULT ttLibC_ByteConnector_ebml(
		ttLibC_ByteConnector *connector,
		uint64_t value) {
	return ttLibC_ByteConnector_ebml2(connector, value, false);
}

/**
 * write ebml data into ByteConnector
 * @param connector
 * @param value
 * @param is_tag
 * @return true:success false:error
 */
bool TT_VISIBILITY_DEFAULT ttLibC_ByteConnector_ebml2(
		ttLibC_ByteConnector *connector,
		uint64_t value,
		bool is_tag) {
	ttLibC_ByteConnector_ *connector_ = (ttLibC_ByteConnector_ *)connector;
	if(connector_ == NULL) {
		return false;
	}
	if(connector_->pos != 0) {
		ERR_PRINT("ebml writing must start with complete byte.");
		connector_->inherit_super.error_number = 1;
		connector_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_BrokenInput);
		return false;
	}
	if(!is_tag) {
		if(value < 0x80) {
			if(connector_->data_size < 1) {
				connector_->inherit_super.error_number = 1;
				connector_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_NeedMoreOutput);
				return false;
			}
			connector_->data[0] = (0x80 | value);
			++ connector_->data;
			-- connector_->data_size;
			++ connector_->inherit_super.write_size;
			return true;
		}
		if(value < 0x4000) {
			if(connector_->data_size < 2) {
				connector_->inherit_super.error_number = 1;
				connector_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_NeedMoreOutput);
				return false;
			}
			connector_->data[0] = (0x40 | ((value >> 8) & 0xFF));
			connector_->data[1] = (value & 0xFF);
			connector_->data += 2;
			connector_->data_size -= 2;
			connector_->inherit_super.write_size += 2;
			return true;
		}
		if(value < 0x200000) {
			if(connector_->data_size < 3) {
				connector_->inherit_super.error_number = 1;
				connector_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_NeedMoreOutput);
				return false;
			}
			connector_->data[0] = (0x20 | ((value >> 16) & 0xFF));
			connector_->data[1] = ((value >> 8) & 0xFF);
			connector_->data[2] = (value & 0xFF);
			connector_->data += 3;
			connector_->data_size -= 3;
			connector_->inherit_super.write_size += 3;
			return true;
		}
		if(value < 0x10000000L) {
			if(connector_->data_size < 4) {
				connector_->inherit_super.error_number = 1;
				connector_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_NeedMoreOutput);
				return false;
			}
			connector_->data[0] = (0x10 | ((value >> 24) & 0xFF));
			connector_->data[1] = ((value >> 16) & 0xFF);
			connector_->data[2] = ((value >> 8) & 0xFF);
			connector_->data[3] = (value & 0xFF);
			connector_->data += 4;
			connector_->data_size -= 4;
			connector_->inherit_super.write_size += 4;
			return true;
		}
		if(value < 0x0800000000L) {
			if(connector_->data_size < 5) {
				connector_->inherit_super.error_number = 1;
				connector_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_NeedMoreOutput);
				return false;
			}
			connector_->data[0] = (0x08 | ((value >> 32) & 0xFF));
			connector_->data[1] = ((value >> 24) & 0xFF);
			connector_->data[2] = ((value >> 16) & 0xFF);
			connector_->data[3] = ((value >> 8) & 0xFF);
			connector_->data[4] = (value & 0xFF);
			connector_->data += 5;
			connector_->data_size -= 5;
			connector_->inherit_super.write_size += 5;
			return true;
		}
		if(value < 0x040000000000L) {
			if(connector_->data_size < 6) {
				connector_->inherit_super.error_number = 1;
				connector_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_NeedMoreOutput);
				return false;
			}
			connector_->data[0] = (0x04 | ((value >> 40) & 0xFF));
			connector_->data[1] = ((value >> 32) & 0xFF);
			connector_->data[2] = ((value >> 24) & 0xFF);
			connector_->data[3] = ((value >> 16) & 0xFF);
			connector_->data[4] = ((value >> 8) & 0xFF);
			connector_->data[5] = (value & 0xFF);
			connector_->data += 6;
			connector_->data_size -= 6;
			connector_->inherit_super.write_size += 6;
			return true;
		}
		if(value < 0x02000000000000L) {
			if(connector_->data_size < 7) {
				connector_->inherit_super.error_number = 1;
				connector_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_NeedMoreOutput);
				return false;
			}
			connector_->data[0] = (0x02 | ((value >> 48) & 0xFF));
			connector_->data[1] = ((value >> 40) & 0xFF);
			connector_->data[2] = ((value >> 32) & 0xFF);
			connector_->data[3] = ((value >> 24) & 0xFF);
			connector_->data[4] = ((value >> 16) & 0xFF);
			connector_->data[5] = ((value >> 8) & 0xFF);
			connector_->data[6] = (value & 0xFF);
			connector_->data += 7;
			connector_->data_size -= 7;
			connector_->inherit_super.write_size += 7;
			return true;
		}
		if(value < 0x0100000000000000L) {
			if(connector_->data_size < 8) {
				connector_->inherit_super.error_number = 1;
				connector_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_NeedMoreOutput);
				return false;
			}
			connector_->data[0] = 0x01;
			connector_->data[1] = ((value >> 48) & 0xFF);
			connector_->data[2] = ((value >> 40) & 0xFF);
			connector_->data[3] = ((value >> 32) & 0xFF);
			connector_->data[4] = ((value >> 24) & 0xFF);
			connector_->data[5] = ((value >> 16) & 0xFF);
			connector_->data[6] = ((value >> 8) & 0xFF);
			connector_->data[7] = (value & 0xFF);
			connector_->data += 8;
			connector_->data_size -= 8;
			connector_->inherit_super.write_size += 8;
			return true;
		}
	}
	else {
		if(value < 0xFF) {
			if(connector_->data_size < 1) {
				connector_->inherit_super.error_number = 1;
				connector_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_NeedMoreOutput);
				return false;
			}
			connector_->data[0] = (value & 0xFF);
			++ connector_->data;
			-- connector_->data_size;
			++ connector_->inherit_super.write_size;
			return true;
		}
		else if(value < 0xFFFF) {
			if(connector_->data_size < 2) {
				connector_->inherit_super.error_number = 1;
				connector_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_NeedMoreOutput);
				return false;
			}
			if((value & 0x8000) != 0) {
				connector_->inherit_super.error_number = 2;
				connector_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_BrokenInput);
				return false;
			}
			else {
				connector_->data[0] = ((value >> 8) & 0xFF);
				connector_->data[1] = (value & 0xFF);
				connector_->data += 2;
				connector_->data_size -= 2;
				connector_->inherit_super.write_size += 2;
			}
		}
		else if(value < 0xFFFFFF) {
			if(connector_->data_size < 3) {
				connector_->inherit_super.error_number = 1;
				connector_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_NeedMoreOutput);
				return false;
			}
			if((value & 0xC00000) != 0) {
				connector_->inherit_super.error_number = 2;
				connector_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_BrokenInput);
				return false;
			}
			else {
				connector_->data[0] = ((value >> 16) & 0xFF);
				connector_->data[1] = ((value >> 8) & 0xFF);
				connector_->data[2] = (value & 0xFF);
				connector_->data += 3;
				connector_->data_size -= 3;
				connector_->inherit_super.write_size += 3;
			}
		}
		else if(value < 0xFFFFFFFF) {
			if(connector_->data_size < 4) {
				connector_->inherit_super.error_number = 1;
				connector_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_NeedMoreOutput);
				return false;
			}
			if((value & 0xE0000000) != 0) {
				connector_->inherit_super.error_number = 2;
				connector_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_BrokenInput);
				return false;
			}
			else {
				connector_->data[0] = ((value >> 24) & 0xFF);
				connector_->data[1] = ((value >> 16) & 0xFF);
				connector_->data[2] = ((value >> 8) & 0xFF);
				connector_->data[3] = (value & 0xFF);
				connector_->data += 4;
				connector_->data_size -= 4;
				connector_->inherit_super.write_size += 4;
			}
		}
		return true;
	}
	connector_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_TtLibCError);
	return false;
}
/*
 * write string data into ByteConnector
 * @param connector
 * @param str
 * @param str_size
 * @return true:success false:error
 */
bool TT_VISIBILITY_DEFAULT ttLibC_ByteConnector_string(
		ttLibC_ByteConnector *connector,
		const char *str,
		size_t str_size) {
	ttLibC_ByteConnector_ *connector_ = (ttLibC_ByteConnector_ *)connector;
	if(connector_ == NULL) {
		return false;
	}
	if(connector_->pos != 0) {
		ERR_PRINT("string writing must start with complete byte.");
		connector_->inherit_super.error_number = 1;
		connector_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_BrokenInput);
		return false;
	}
	if(connector_->data_size < str_size) {
		ERR_PRINT("buffer doesn't have enough size for string write.");
		connector_->inherit_super.error_number = 1;
		connector_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_NeedMoreOutput);
		return false;
	}
	memcpy(connector_->data, str, str_size);
	connector_->data += str_size;
	connector_->data_size -= str_size;
	connector_->inherit_super.write_size += str_size;
	return true;
}

/*
 * close ByteConnector object
 * @param connector
 */
void TT_VISIBILITY_DEFAULT ttLibC_ByteConnector_close(ttLibC_ByteConnector **connector) {
	ttLibC_ByteConnector_ *target = (ttLibC_ByteConnector_ *)*connector;
	if(target == NULL) {
		return;
	}
	ttLibC_free(target);
	*connector = NULL;
}
