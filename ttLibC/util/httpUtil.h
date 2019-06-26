/**
 * @file   httpUtil.h
 * @brief  library for http access.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/30
 */

#ifndef TTLIBC_UTIL_HTTPUTIL_H_
#define TTLIBC_UTIL_HTTPUTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../ttLibC_predef.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * data for httpClient
 */
typedef struct ttLibC_Util_HttpUtil_HttpClient {
	/** ETag value. */
	char ETag[256];
	/** size of download. */
	size_t content_length;
	/** size of target file. */
	size_t file_length;
	/** wait interval for each read access. */
	uint32_t wait_interval; // mili sec.
	/** hold buffer size for download. */
	size_t buffer_size;
	/** reply status code */
	uint32_t status_code;
} ttLibC_Util_HttpUtil_HttpClient;

typedef ttLibC_Util_HttpUtil_HttpClient ttLibC_HttpClient;

/**
 * callback function for http client.
 * @param ptr       user def value pointer.
 * @param client    httpClient object.
 * @param data      downloaded data.
 * @param data_size downloaded data_size.
 */
typedef bool (* ttLibC_HttpClientFunc)(void *ptr, ttLibC_HttpClient *client, void *data, size_t data_size);

/**
 * make http client
 * @param buffer_size   download buffer size.
 * @param wait_interval interval for each download.
 * @return http client object.
 */
ttLibC_HttpClient TT_ATTRIBUTE_API *ttLibC_HttpClient_make(
		size_t buffer_size,
		uint32_t wait_interval);

/**
 * get method download.
 * @param client         http client object
 * @param target_address address for download.
 * @param is_binary      true:read as binary false:read as string
 * @param callback       callback for download data.
 * @param ptr            user def data pointer.
 */
void TT_ATTRIBUTE_API ttLibC_HttpClient_get(
		ttLibC_HttpClient *client,
		const char *target_address,
		bool is_binary,
		ttLibC_HttpClientFunc callback,
		void *ptr);

/**
 * get method download.
 * @param client         http client object
 * @param target_address address for download.
 * @param range_start    begin point for download. if 0, ignore
 * @param range_length   download size for download. if 0, ignore
 * @param is_binary      true:read as binary false:read as string
 * @param callback       callback for download data.
 * @param ptr            user def data pointer.
 */
void TT_ATTRIBUTE_API ttLibC_HttpClient_getRange(
		ttLibC_HttpClient *client,
		const char *target_address,
		size_t range_start,
		size_t range_length,
		bool is_binary,
		ttLibC_HttpClientFunc callback,
		void *ptr);

/**
 * close http client
 * @param client
 */
void TT_ATTRIBUTE_API ttLibC_HttpClient_close(ttLibC_HttpClient **client);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_UTIL_HTTPUTIL_H_ */
