/*
 * @file   ttLibC.c
 * @brief  misc.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/20
 */

#include "ttLibC_predef.h"
#include "ttLibC_common.h"
#include "log.h"

static const char *const errors[] = {
		"unknown"
};

static const char *const errorMessage[] = {
		"no error",
		"failed to memory allocate.",
		"need more output buffer.",
		"native library error.",
		"need more input data.",
		"ttLibC internal error.",
		"input data is broken.",
		"missing required params.",
		"input data is invalid data type."
};

static const char *const errorTarget[] = {
		"",
		"    at ContainerReader",
		"    at ContainerWriter",
		"    at Decoder",
		"    at Enoder",
		"    at VideoFrame",
		"    at AudioFrame",
		"    at NetClient",
		"    at NetServer",
		"    at Tetty",
		"    at Resampler",
		"    at Util",
};

static const char *version = "ttLibC"
#if defined(__ENABLE_GPL__)
		" GPLv3 version"
#else
#	if defined(__ENABLE_LGPL__)
		" LGPLv3 version"
#	else
		" 3-Cause BSD version"
#	endif
#endif
;

Error_e TT_VISIBILITY_HIDDEN ttLibC_updateError(Error_Target_e target, Error_e error) {
	if(error == Error_noError) {
		return error;
	}
	if((error & 0xF0000000) == 0) {
		return (target << 28) | error;
	}
	else if((error & 0x0F000000) == 0) {
		return (target << 24) | error;
	}
	else if((error & 0x00F00000) == 0) {
		return (target << 20) | error;
	}
	else {
		return error;
	}
}

void TT_VISIBILITY_DEFAULT ttLibC_printLastError(Error_e error, bool is_errorMessage, const char *func, uint32_t line) {
	if((error & 0x000FFFFF) == Error_noError) {
		return;
	}
	FILE *target = stdout;
	if(is_errorMessage) {
		target = stderr;
	}
	fprintf(target, "[error]%s(%d) %s\n", func, line, errorMessage[(error & 0x000FFFFF)]);
	if((error & 0xF0000000) != 0x00) {
		fprintf(target, "%s\n", errorTarget[((error >> 28) & 0x0F)]);
	}
	else {
		return;
	}
	if((error & 0x0F000000) != 0x00) {
		fprintf(target, "%s\n", errorTarget[((error >> 24) & 0x0F)]);
	}
	else {
		return;
	}
	if((error & 0x00F00000) != 0x00) {
		fprintf(target, "%s\n", errorTarget[((error >> 20) & 0x0F)]);
	}
	return;
}

const char TT_VISIBILITY_DEFAULT *ttLibC_getLastError(int error_no) {
	int error_num = sizeof(errors) / sizeof(errors[0]) - 1;
	if(error_no > 0 || error_no < -error_num) {
		return errors[error_num];
	}
	return errors[-error_no];
}

const char TT_VISIBILITY_DEFAULT *ttLibC_getVersion() {
	return version;
}
