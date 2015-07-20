/*
 * @file   ttLibC.c
 * @brief  misc.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/20
 */
static const char *const errors[] = {
		"unknown"
};
static const char *version = "ttLibC"
#if defined(__ENABLE_GPL__)
		" GPLv3 version"
#else
#	if defined(__ENABLE_LGPL__)
		" LGPLv3 version"
#	else
		" 3cause BSD version"
#	endif
#endif
;

const char *ttLibC_getLastError(int error_no) {
	int error_num = sizeof(errors) / sizeof(errors[0]) - 1;
	if(error_no > 0 || error_no < -error_num) {
		return errors[error_num];
	}
	return errors[-error_no];
}

const char *ttLibC_getVersion() {
	return version;
}


