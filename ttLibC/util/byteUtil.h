/**
 * @file   byteUtil.h
 * @brief  byte work support.
 *
 * this code is under 3-Cause BSD license
 *
 * @author taktod
 * @date   2015/10/17
 */

#ifndef TTLIBC_UTIL_BYTEUTIL_H_
#define TTLIBC_UTIL_BYTEUTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

<<<<<<< HEAD
=======
#include "../ttLibC.h"

>>>>>>> 9bbbf00f57e1bb3b2a36a3faa606dbefb135e8a0
/**
 * work type for byteUtil
 */
typedef enum ttLibC_ByteUtil_Type {
	/** normal */
	ByteUtilType_default,
	/** support of emulation prevention three byte for h264 h265 */
	ByteUtilType_h26x,
} ttLibC_ByteUtil_Type;

/**
 * data for ByteReader
 * get data from byte array.
 */
typedef struct ttLibC_Util_ByteReader {
	/** read type. */
	ttLibC_ByteUtil_Type type;
	/** reading_size */
	size_t read_size;
	/** number for reading error */
	bool error_number;
<<<<<<< HEAD
=======
	/** error information */
	Error_e error;
>>>>>>> 9bbbf00f57e1bb3b2a36a3faa606dbefb135e8a0
} ttLibC_Util_ByteReader;

typedef ttLibC_Util_ByteReader ttLibC_ByteReader;

/**
 * make ByteReader object.
 * @param data      target data
 * @param data_size target data size
 * @param type      target data type
 * @return byte reader object.
 */
ttLibC_ByteReader *ttLibC_ByteReader_make(
		void *data,
		size_t data_size,
		ttLibC_ByteUtil_Type type);

/**
 * get bit from ByteReader
 * @param reader
 * @param bit_num
 * @return value
 */
uint64_t ttLibC_ByteReader_bit(
		ttLibC_ByteReader *reader,
		uint32_t bit_num);

/**
 * get exp golomb value from ByteReader
 * @param reader
 * @param sign
 * @return value
 */
int32_t ttLibC_ByteReader_expGolomb(
		ttLibC_ByteReader *reader,
		bool sign);

/**
 * get ebml value from ByteReader
 * @param reader
 * @param is_tag
 * @return value
 */
uint64_t ttLibC_ByteReader_ebml(
		ttLibC_ByteReader *reader,
		bool is_tag);

/**
 * get string data from ByteReader
 * @param reader
 * @param buffer
 * @param buffer_size
 * @param target_size
 * @return read size.
 */
size_t ttLibC_ByteReader_string(
		ttLibC_ByteReader *reader,
		char *buffer,
		size_t buffer_size,
		size_t target_size);

/**
 * skip several size of bit reading.
 * @param reader
 * @param skip_size size in bit num.
 * TODO do later, if need.
 */
/*size_t ttLibC_ByteReader_skipBit(
		ttLibC_ByteReader *reader,
		size_t skip_size);*/

/**
 * skip several size of byte reading.
 * @param reader
 * @param skip_size size in byte num.
 */
size_t ttLibC_ByteReader_skipByte(
		ttLibC_ByteReader *reader,
		size_t skip_size);

/**
 * rewind several size of byte reading.
 * (go backward.)
 * @param reader
 * @param rewind_size
 */
size_t ttLibC_ByteReader_rewindByte(
		ttLibC_ByteReader *reader,
		size_t rewind_size);

/**
 * close ByteReader
 * @param reader
 */
void ttLibC_ByteReader_close(ttLibC_ByteReader **reader);

/**
 * data for ByteConnector
 * make byte array from bit (expGolomb), and ebml
 */
typedef struct ttLibC_Util_ByteConnector {
	ttLibC_ByteUtil_Type type;
	/** writing data size */
	size_t write_size;
	/** connecting error number */
	bool error_number;
<<<<<<< HEAD
=======
	/** error information */
	Error_e error;
>>>>>>> 9bbbf00f57e1bb3b2a36a3faa606dbefb135e8a0
} ttLibC_Util_ByteConnector;

typedef ttLibC_Util_ByteConnector ttLibC_ByteConnector;

/**
 * make byteConnector object.
 * @param data      memory data, result is copyed on this buffer.
 * @param data_size data size
 * @param type      work type(now, default only.)
 * @return ByteConnector object.
 */
ttLibC_ByteConnector *ttLibC_ByteConnector_make(
		void *data,
		size_t data_size,
		ttLibC_ByteUtil_Type type);

/**
 * write bit data into ByteConnector.
 * @param connector
 * @param value
 * @param bit_num
 * @return true:success false:error
 */
bool ttLibC_ByteConnector_bit(
		ttLibC_ByteConnector *connector,
		uint32_t value,
		uint32_t bit_num);

/**
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
		bool sign);
*/

/**
 * write ebml data into ByteConnector
 * @param connector
 * @param value
 * @return true:success false:error
 */
bool ttLibC_ByteConnector_ebml(
		ttLibC_ByteConnector *connector,
		uint64_t value);

/**
<<<<<<< HEAD
=======
 * write ebml data into ByteConnector
 * @param connector
 * @param value
 * @param is_tag
 * @return true:success false:error
 */
bool ttLibC_ByteConnector_ebml2(
		ttLibC_ByteConnector *connector,
		uint64_t value,
		bool is_tag);

/**
>>>>>>> 9bbbf00f57e1bb3b2a36a3faa606dbefb135e8a0
 * write string data into ByteConnector
 * @param connector
 * @param str
 * @param str_size
 * @return true:success false:error
 */
bool ttLibC_ByteConnector_string(
		ttLibC_ByteConnector *connector,
		const char *str,
		size_t str_size);

/**
 * close ByteConnector object
 * @param connector
 */
void ttLibC_ByteConnector_close(ttLibC_ByteConnector **connector);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_UTIL_BYTEUTIL_H_ */
