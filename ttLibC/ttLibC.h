/**
 * @file   ttLibC.h
 * @brief  misc.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/12
 */

#ifndef TTLIBC_TTLIBC_H_
#define TTLIBC_TTLIBC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "log.h"

/**
 * error code...
 * format ABCXXXXX
 * XXXXX is error code.
 * ABC is the stack information.
 */
typedef enum Error_Target_e {
	Target_On_Unspecified     = 0x00,
	Target_On_ContainerReader = 0x01,
	Target_On_ContainerWriter = 0x02,
	Target_On_Decoder         = 0x03,
	Target_On_Encoder         = 0x04,
	Target_On_VideoFrame      = 0x05,
	Target_On_AudioFrame      = 0x06,
	Target_On_NetClient       = 0x07,
	Target_On_NetServer       = 0x08,
	Target_On_Tetty           = 0x09,
	Target_On_Resampler       = 0x0A,
	Target_On_Util            = 0x0B,
} Error_Target_e;

typedef enum Error_e {
	Error_noError          = 0x00000000,
	Error_MemoryAllocate   = 0x00000001,
	Error_MemoryShort      = 0x00000002,
	Error_LibraryError     = 0x00000003,
	Error_RequireMoreData  = 0x00000004,
	Error_InvalidOperation = 0x00000005,
} Error_e;

/**
 * get error string.
 * @param error_no
 * @deprecated
 */
const char *ttLibC_getLastError(int error_no);

/**
 * print error string on console.
 * @param error           error code
 * @param is_errorMessage true:print on stderr flase:print on stdout
 * @param func
 * @param line
 */
void ttLibC_printLastError(Error_e error, bool is_errorMessage, const char *func, uint32_t line);

#if __DEBUG_FLAG__ == 1
#	define LOG_ERROR(err) ttLibC_printLastError(err, false, __func__, __LINE__)
#else
#	define LOG_ERROR(err)
#endif

#define ERR_ERROR(err) ttLibC_printLastError(err, true, __func__, __LINE__)

/**
 * @return version string for ttLibC
 */
const char *ttLibC_getVersion();

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* TTLIBC_TTLIBC_H_ */
