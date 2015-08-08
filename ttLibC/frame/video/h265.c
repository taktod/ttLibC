/*
 * @file   h265.c
 * @brief  h265 image frame information
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/06
 */

#include "h265.h"

ttLibC_H265 *ttLibC_H265_make(
		ttLibC_H265 *prev_frame,
		ttLibC_H265_Type type,
		uint32_t width,
		uint32_t height,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	return NULL;
}

bool ttLibC_H265_getNalInfo(ttLibC_H265_NalInfo* info, uint8_t *data, size_t data_size) {
	return false;
}

bool ttLibC_H265_getAvccInfo(ttLibC_H265_NalInfo* info, uint8_t *data, size_t data_size) {
	return false;
}

bool ttLibC_H265_isNal(uint8_t *data, size_t data_size) {
	return false;
}

bool ttLibC_H265_isAvcc(uint8_t *data, size_t data_size) {
	return false;
}

bool ttLibC_H265_isKey(uint8_t *data, size_t data_size) {
	return false;
}

uint32_t ttLibC_H265_getWidth(ttLibC_H265 *prev_frame, uint8_t *data, size_t data_size) {
	return 0;
}

uint32_t ttLibC_H265_getHeight(ttLibC_H265 *prev_frame, uint8_t *data, size_t data_size) {
	return 0;
}

ttLibC_H265 *ttLibC_H265_getFrame(
		ttLibC_H265 *prev_frame,
		uint8_t *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	return NULL;
}

void ttLibC_H265_close(ttLibC_H265 **frame) {

}




