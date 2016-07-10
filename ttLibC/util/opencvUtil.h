/**
 * @file   opencvUtil.h
 * @brief  library to use opencv libraries.
 *
 * this code is under 3-Cause BSD license.
 *
 * @see    opencvUtilTest
 * @author taktod
 * @date   2015/07/18
 */

#ifndef TTLIBC_UTIL_OPENCVUTIL_H_
#define TTLIBC_UTIL_OPENCVUTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../frame/video/bgr.h"
#include "../ttLibC.h"

/**
 * significant keychar value.
 */
typedef enum ttLibC_Keychar {
	Keychar_Esc = '\x1b'
} ttLibC_Keychar;

/**
 * data for opencv window object.
 */
typedef struct ttLibC_Util_OpencvUtil_CvWindow {
	/** name for window */
	char *name;
	Error_e error;
} ttLibC_Util_OpencvUtil_CvWindow;

typedef ttLibC_Util_OpencvUtil_CvWindow ttLibC_CvWindow;

/**
 * make cvWindow
 * @param name window name
 * @return ttLibC_CvWindow object
 */
ttLibC_CvWindow *ttLibC_CvWindow_make(const char *name);
/** @deprected */
ttLibC_CvWindow *ttLibC_CvWindow_makeWindow(const char *name);

/**
 * show bgr image.
 * @param window target window
 * @param bgr    target bgr image.
 */
void ttLibC_CvWindow_showBgr(ttLibC_CvWindow *window, ttLibC_Bgr *bgr);

/**
 * close cvWindow
 * @param window target window
 */
void ttLibC_CvWindow_close(ttLibC_CvWindow **window);

/**
 * wait for key Input.
 * @param delay waiting interval in mili sec.
 * @return pressed key code.
 */
uint8_t ttLibC_CvWindow_waitForKeyInput(int delay);

/**
 * data for opencv capture object.
 */
typedef struct ttLibC_Util_OpencvUtil_CvCapture {
	/** camera num */
	uint32_t camera_num;
	/** capture width */
	uint32_t width;
	/** capture height */
	uint32_t height;
	Error_e error;
} ttLibC_Util_OpencvUtil_CvCapture;

typedef ttLibC_Util_OpencvUtil_CvCapture ttLibC_CvCapture;

/**
 * make capture
 * @param camera_num num of camera.
 * @param width      capture width
 * @param height     capture height
 * @return ttLibC_CvCapture object.
 */
ttLibC_CvCapture *ttLibC_CvCapture_make(
		uint32_t camera_num,
		uint32_t width,
		uint32_t height);
/** @deprected */
ttLibC_CvCapture *ttLibC_CvCapture_makeCapture(
		uint32_t camera_num,
		uint32_t width,
		uint32_t height);

/**
 * capture the frame from device.
 * @param capture    CvCapture object.
 * @param prev_frame reuse bgr frame.
 */
ttLibC_Bgr *ttLibC_CvCapture_queryFrame(ttLibC_CvCapture *capture, ttLibC_Bgr *prev_frame);

/**
 * close cvCapture
 * @param capture target capture.
 */
void ttLibC_CvCapture_close(ttLibC_CvCapture **capture);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_UTIL_OPENCVUTIL_H_ */
