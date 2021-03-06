/*
 * @file   opencvUtil.c
 * @brief  library to use opencv libraries.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/18
 */

#ifdef __ENABLE_OPENCV__
#include <opencv2/core/version.hpp>

#if (CV_MAJOR_VERSION == 4)

#include "opencvUtil.h"
#include <opencv2/videoio.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

#include "../ttLibC_predef.h"
#include "../_log.h"
#include "../allocator.h"
#include "../ttLibC_common.h"

#ifdef __APPLE__
// for osx, mach_time is better than time.h.
#include <mach/mach_time.h>
#else
// TODO check on linux and windows.
#include <time.h>
#endif

using namespace cv;

typedef struct {
	ttLibC_CvWindow inherit_super;
} ttLibC_Util_OpencvUtil_CvWindow_;

typedef ttLibC_Util_OpencvUtil_CvWindow_ ttLibC_CvWindow_;

ttLibC_CvWindow TT_VISIBILITY_DEFAULT *ttLibC_CvWindow_make(const char *name) {
	ttLibC_CvWindow_ *window = (ttLibC_CvWindow_ *)ttLibC_malloc(sizeof(ttLibC_CvWindow_));
	if(window == NULL) {
		ERR_PRINT("failed to allocate memory for window.");
		return NULL;
	}
	size_t len = strlen(name);
	window->inherit_super.name = (char *)ttLibC_malloc(len + 1);
	if(window->inherit_super.name == NULL) {
		ERR_PRINT("failed to allocate memory for window name.");
		ttLibC_free(window);
		return NULL;
	}
	sprintf(window->inherit_super.name, "%s", name);
	namedWindow(window->inherit_super.name, 0);
	return (ttLibC_CvWindow *)window;
}
ttLibC_CvWindow TT_VISIBILITY_DEFAULT *ttLibC_CvWindow_makeWindow(const char *name) {
	return ttLibC_CvWindow_make(name);
}
void TT_VISIBILITY_DEFAULT ttLibC_CvWindow_showBgr(ttLibC_CvWindow *window, ttLibC_Bgr *bgr) {
	if(window == NULL) {
		return;
	}
	if(bgr == NULL) {
		return;
	}
	uint8_t *data = (uint8_t *)bgr->data;
	switch(bgr->type) {
	case BgrType_bgr:
		{
			Mat frame(bgr->inherit_super.height, bgr->inherit_super.width, CV_8UC3, data, bgr->width_stride);
			imshow(window->name, frame);
		}
		break;
	case BgrType_bgra:
		{
			Mat frame(bgr->inherit_super.height, bgr->inherit_super.width, CV_8UC4, data, bgr->width_stride);
			imshow(window->name, frame);
		}
		break;
	case BgrType_abgr:
		{
			data ++;
			Mat frame(bgr->inherit_super.height, bgr->inherit_super.width, CV_8UC4, data, bgr->width_stride);
			imshow(window->name, frame);
		}
		break;
	case BgrType_rgb:
		{
			Mat frame(bgr->inherit_super.height, bgr->inherit_super.width, CV_8UC3, data, bgr->width_stride);
			Mat bgr_frame;
			cvtColor(frame, bgr_frame, COLOR_BGR2RGB);
			imshow(window->name, bgr_frame);
		}
		break;
	case BgrType_rgba:
		{
			Mat frame(bgr->inherit_super.height, bgr->inherit_super.width, CV_8UC4, data, bgr->width_stride);
			Mat bgr_frame;
			cvtColor(frame, bgr_frame, COLOR_BGR2RGB);
			imshow(window->name, bgr_frame);
		}
		break;
	case BgrType_argb:
		{
			data ++;
			Mat frame(bgr->inherit_super.height, bgr->inherit_super.width, CV_8UC4, data, bgr->width_stride);
			Mat bgr_frame;
			cvtColor(frame, bgr_frame, COLOR_BGR2RGB);
			imshow(window->name, bgr_frame);
		}
		break;
	default:
		break;
	}
}

void TT_VISIBILITY_DEFAULT ttLibC_CvWindow_close(ttLibC_CvWindow **window) {
	if(*window == NULL) {
		return;
	}
	ttLibC_CvWindow_ *window_ = (ttLibC_CvWindow_ *)(*window);
	if(window_->inherit_super.name != NULL) {
		ttLibC_free(window_->inherit_super.name);
		window_->inherit_super.name = NULL;
	}
	ttLibC_free(window_);
	*window = NULL;
}

uint8_t TT_VISIBILITY_DEFAULT ttLibC_CvWindow_waitForKeyInput(int delay) {
	return waitKey(delay);
}

typedef struct {
	ttLibC_CvCapture inherit_super;
	VideoCapture *cap;
	Mat *frame;
#ifdef __APPLE__
	uint64_t startTime;
#else
	clock_t startTime;
#endif
} ttLibC_Util_OpencvUtil_CvCapture_;

typedef ttLibC_Util_OpencvUtil_CvCapture_ ttLibC_CvCapture_;

ttLibC_CvCapture TT_VISIBILITY_DEFAULT *ttLibC_CvCapture_make(
		uint32_t camera_num,
		uint32_t width,
		uint32_t height) {
	ttLibC_CvCapture_ *capture = (ttLibC_CvCapture_ *)ttLibC_malloc(sizeof(ttLibC_CvCapture_));
	if(capture == NULL) {
		ERR_PRINT("failed to allocate memory for cvCapture.");
		return NULL;
	}
	capture->cap = new VideoCapture();
	capture->cap->open(camera_num);
	if(!capture->cap->isOpened()) {
		ERR_PRINT("failed to open capture.");
		ttLibC_free(capture);
		return NULL;
	}
	capture->cap->set(CAP_PROP_FRAME_WIDTH, width);
	capture->cap->set(CAP_PROP_FRAME_HEIGHT, height);
	capture->frame = new Mat(height, width, CV_8UC3);
	capture->startTime = 0;
	capture->inherit_super.width = width;
	capture->inherit_super.height = height;
	capture->inherit_super.camera_num = camera_num;
	capture->inherit_super.error = Error_noError;
	return (ttLibC_CvCapture *)capture;
}

ttLibC_CvCapture TT_VISIBILITY_DEFAULT *ttLibC_CvCapture_makeCapture(
		uint32_t camera_num,
		uint32_t width,
		uint32_t height) {
	return ttLibC_CvCapture_make(camera_num, width, height);
}

ttLibC_Bgr TT_VISIBILITY_DEFAULT *ttLibC_CvCapture_queryFrame(ttLibC_CvCapture *capture, ttLibC_Bgr *prev_frame) {
	if(capture == NULL) {
		return NULL;
	}
	ttLibC_CvCapture_ *capture_ = (ttLibC_CvCapture_ *)capture;
	*capture_->cap >> *capture_->frame;
	if(capture_->frame->empty()) {
		return NULL;
	}
	uint8_t *ptr = capture_->frame->ptr();
	uint32_t timebase = 1000;
#ifdef __APPLE__
	uint64_t time = mach_absolute_time();
	if(capture_->startTime == 0) {
		capture_->startTime = time;
	}
	uint64_t pts = (time - capture_->startTime) / 1000000;
#else
	clock_t time = clock();
	if(capture_->startTime == 0) {
		capture_->startTime = time;
	}
	uint64_t pts = (time - capture_->startTime) * timebase / CLOCKS_PER_SEC;
#endif
	if(capture_->frame->cols >= capture->width && capture_->frame->rows >= capture->height) {
		return ttLibC_Bgr_make(prev_frame, BgrType_bgr, capture->width, capture->height, capture_->frame->step1(), ptr, capture_->frame->total() * 3, true, pts, timebase);
	}
	else {
		return ttLibC_Bgr_make(prev_frame, BgrType_bgr, capture_->frame->cols, capture_->frame->rows, capture_->frame->step1(), ptr, capture_->frame->total() * 3, true, pts, timebase);
	}
}

void TT_VISIBILITY_DEFAULT ttLibC_CvCapture_close(ttLibC_CvCapture **capture) {
	if(*capture == NULL) {
		return;
	}
	ttLibC_CvCapture_ *capture_ = (ttLibC_CvCapture_ *)*capture;
	delete capture_->cap;
	delete capture_->frame;
	ttLibC_free(capture_);
	*capture = NULL;
}

#else

#include "opencvUtil.h"

#include <opencv2/opencv_modules.hpp>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "../ttLibC_predef.h"
#include "../_log.h"
#include "../allocator.h"
#include "../ttLibC_common.h"

#ifdef __APPLE__
// for osx, mach_time is better than time.h.
#include <mach/mach_time.h>
#else
// TODO check on linux and windows.
#include <time.h>
#endif

typedef struct {
	ttLibC_CvWindow inherit_super;
	IplImage *image;
} ttLibC_Util_OpencvUtil_CvWindow_;

typedef ttLibC_Util_OpencvUtil_CvWindow_ ttLibC_CvWindow_;

/*
 * make cvWindow
 * @param name window name
 * @return ttLibC_CvWindow object
 */
ttLibC_CvWindow TT_VISIBILITY_DEFAULT *ttLibC_CvWindow_make(const char *name) {
	ttLibC_CvWindow_ *window = (ttLibC_CvWindow_ *)ttLibC_malloc(sizeof(ttLibC_CvWindow_));
	if(window == NULL) {
		ERR_PRINT("failed to allocate memory for window.");
		return NULL;
	}
	size_t len = strlen(name);
	window->inherit_super.name = (char *)ttLibC_malloc(len + 1);
	if(window->inherit_super.name == NULL) {
		ttLibC_free(window);
		return NULL;
	}
	sprintf(window->inherit_super.name, "%s", name);
	cvNamedWindow(window->inherit_super.name, CV_WINDOW_NORMAL);
	window->image = NULL;
	window->inherit_super.error = Error_noError;
	return (ttLibC_CvWindow* )window;
}
ttLibC_CvWindow TT_VISIBILITY_DEFAULT *ttLibC_CvWindow_makeWindow(const char *name) {
	return ttLibC_CvWindow_make(name);
}

/*
 * show bgr image.
 * @param window target window
 * @param bgr    target bgr image.
 */
void TT_VISIBILITY_DEFAULT ttLibC_CvWindow_showBgr(ttLibC_CvWindow *window, ttLibC_Bgr *bgr) {
	if(window == NULL) {
		return;
	}
	if(bgr == NULL) {
		return;
	}
	ttLibC_CvWindow_ *window_ = (ttLibC_CvWindow_ *)window;
	if(window_->image != NULL
	&& (window_->image->width != (int32_t)bgr->inherit_super.width
		|| window_->image->height != (int32_t)bgr->inherit_super.height)) {
		// if width or height is changed, need to remake.
		cvReleaseImage(&window_->image);
		window_->image = NULL;
	}
	if(window_->image == NULL) {
		window_->image = cvCreateImage(cvSize(bgr->inherit_super.width, bgr->inherit_super.height), 8, 3);
		if(window_->image == NULL) {
			window_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_NativeError);
			return;
		}
	}
	uint8_t *image_data = (uint8_t *)window_->image->imageData;
	uint32_t image_stride_diff = window_->image->widthStep - 3 * window_->image->width;
	uint8_t *src_data = bgr->data;
	uint8_t *src_b_data;
	uint8_t *src_g_data;
	uint8_t *src_r_data;
	uint32_t src_step = 3;
	uint32_t src_width_stride_diff;
	switch(bgr->type) {
	default:
	case BgrType_bgr:
		src_step = 3;
		src_width_stride_diff = bgr->width_stride - src_step * bgr->inherit_super.width;
		src_b_data = src_data;
		src_g_data = src_data + 1;
		src_r_data = src_data + 2;
		break;
	case BgrType_bgra:
		src_step = 4;
		src_width_stride_diff = bgr->width_stride - src_step * bgr->inherit_super.width;
		src_b_data = src_data;
		src_g_data = src_data + 1;
		src_r_data = src_data + 2;
		break;
	case BgrType_abgr:
		src_step = 4;
		src_width_stride_diff = bgr->width_stride - src_step * bgr->inherit_super.width;
		src_b_data = src_data + 1;
		src_g_data = src_data + 2;
		src_r_data = src_data + 3;
		break;
	case BgrType_rgb:
		src_step = 3;
		src_width_stride_diff = bgr->width_stride - src_step * bgr->inherit_super.width;
		src_b_data = src_data + 2;
		src_g_data = src_data + 1;
		src_r_data = src_data;
		break;
	case BgrType_rgba:
		src_step = 4;
		src_width_stride_diff = bgr->width_stride - src_step * bgr->inherit_super.width;
		src_b_data = src_data + 2;
		src_g_data = src_data + 1;
		src_r_data = src_data;
		break;
	case BgrType_argb:
		src_step = 4;
		src_width_stride_diff = bgr->width_stride - src_step * bgr->inherit_super.width;
		src_b_data = src_data + 3;
		src_g_data = src_data + 2;
		src_r_data = src_data + 1;
		break;
	}
	for(uint32_t i = 0;i < bgr->inherit_super.height;++ i) {
		for(uint32_t j = 0;j < bgr->inherit_super.width;++ j) {
			(*image_data) = (*src_b_data);
			++ image_data;
			(*image_data) = (*src_g_data);
			++ image_data;
			(*image_data) = (*src_r_data);
			++ image_data;
			src_b_data += src_step;
			src_g_data += src_step;
			src_r_data += src_step;
		}
		if(src_width_stride_diff > 0) {
			src_b_data += src_width_stride_diff;
			src_g_data += src_width_stride_diff;
			src_r_data += src_width_stride_diff;
		}
		if(image_stride_diff > 0) {
			image_data += image_stride_diff;
		}
	}
	cvShowImage(window_->inherit_super.name, window_->image);
}

/*
 * close cvWindow
 * @param window target window
 */
void TT_VISIBILITY_DEFAULT ttLibC_CvWindow_close(ttLibC_CvWindow **window) {
	if(*window == NULL) {
		return;
	}
	ttLibC_CvWindow_ *window_ = (ttLibC_CvWindow_ *)(*window);
	if(window_->inherit_super.name != NULL) {
		ttLibC_free(window_->inherit_super.name);
		window_->inherit_super.name = NULL;
	}
	if(window_->image != NULL) {
		cvReleaseImage(&window_->image);
		window_->image = NULL;
	}
	ttLibC_free(window_);
	*window = NULL;
}

/**
 * wait for key Input.
 * @param delay waiting interval in mili sec.
 * @return pressed key code.
 */
uint8_t TT_VISIBILITY_DEFAULT ttLibC_CvWindow_waitForKeyInput(int delay) {
	return cvWaitKey(delay);
}


typedef struct {
	ttLibC_CvCapture inherit_super;
#if (CV_MAJOR_VERSION == 2) || ((CV_MAJOR_VERSION >= 3) && defined(HAVE_OPENCV_VIDEOIO))
	CvCapture *capture;
#endif
#ifdef __APPLE__
	uint64_t startTime;
#else
	clock_t startTime;
#endif
} ttLibC_Util_OpencvUtil_CvCapture_;

typedef ttLibC_Util_OpencvUtil_CvCapture_ ttLibC_CvCapture_;

/**
 * make capture
 * @param camera_num num of camera.
 * @param width      capture width
 * @param height     capture height
 * @return ttLibC_CvCapture object.
 */
ttLibC_CvCapture TT_VISIBILITY_DEFAULT *ttLibC_CvCapture_make(
		uint32_t camera_num,
		uint32_t width,
		uint32_t height) {
#if (CV_MAJOR_VERSION == 2) || ((CV_MAJOR_VERSION >= 3) && defined(HAVE_OPENCV_VIDEOIO))
	ttLibC_CvCapture_ *capture = (ttLibC_CvCapture_ *)ttLibC_malloc(sizeof(ttLibC_CvCapture_));
	if(capture == NULL) {
		ERR_PRINT("failed to allocate memory for cvCapture.");
		return NULL;
	}
	capture->capture = cvCreateCameraCapture(camera_num);
	if(capture->capture == NULL) {
		ERR_PRINT("failed to open camera device.");
		ttLibC_free(capture);
		return NULL;
	}
	cvSetCaptureProperty(capture->capture, CV_CAP_PROP_FRAME_WIDTH, width);
	cvSetCaptureProperty(capture->capture, CV_CAP_PROP_FRAME_HEIGHT, height);
	capture->startTime = 0;
	capture->inherit_super.width      = width;
	capture->inherit_super.height     = height;
	capture->inherit_super.camera_num = camera_num;
	capture->inherit_super.error      = Error_noError;
	return (ttLibC_CvCapture *)capture;
#else
	return NULL;
#endif
}

ttLibC_CvCapture TT_VISIBILITY_DEFAULT *ttLibC_CvCapture_makeCapture(
		uint32_t camera_num,
		uint32_t width,
		uint32_t height) {
	return ttLibC_CvCapture_make(camera_num, width, height);
}

/**
 * capture the frame from device.
 * @param capture    CvCapture object.
 * @param prev_frame reuse bgr frame.
 */
ttLibC_Bgr TT_VISIBILITY_DEFAULT *ttLibC_CvCapture_queryFrame(ttLibC_CvCapture *capture, ttLibC_Bgr *prev_frame) {
#if (CV_MAJOR_VERSION == 2) || ((CV_MAJOR_VERSION >= 3) && defined(HAVE_OPENCV_VIDEOIO))
	if(capture == NULL) {
		return NULL;
	}
	ttLibC_CvCapture_ *capture_ = (ttLibC_CvCapture_*)capture;
	IplImage *frame = cvQueryFrame(capture_->capture);
	if(frame == NULL) {
		capture_->inherit_super.error = ttLibC_updateError(Target_On_Util, Error_NativeError);
		return NULL;
	}

	uint32_t timebase = 1000;
#ifdef __APPLE__
	uint64_t time = mach_absolute_time();
	if(capture_->startTime == 0) {
		capture_->startTime = time;
	}
	uint64_t pts = (time - capture_->startTime) / 1000000;
#else
	clock_t time = clock();
	if(capture_->startTime == 0) {
		capture_->startTime = time;
	}
	uint64_t pts = (time - capture_->startTime) * timebase / CLOCKS_PER_SEC;
#endif
	if(frame->width >= capture->width && frame->height >= capture->height) {
		return ttLibC_Bgr_make(prev_frame, BgrType_bgr, capture->width, capture->height, frame->widthStep, frame->imageData, frame->imageSize, true, pts, timebase);
	}
	else {
		return ttLibC_Bgr_make(prev_frame, BgrType_bgr, frame->width, frame->height, frame->widthStep, frame->imageData, frame->imageSize, true, pts, timebase);
	}
#else
	return NULL;
#endif
}

/*
 * close cvCapture
 * @param capture target capture.
 */
void TT_VISIBILITY_DEFAULT ttLibC_CvCapture_close(ttLibC_CvCapture **capture) {
#if (CV_MAJOR_VERSION == 2) || ((CV_MAJOR_VERSION >= 3) && defined(HAVE_OPENCV_VIDEOIO))
	if(*capture == NULL) {
		return;
	}
	ttLibC_CvCapture_ *capture_ = (ttLibC_CvCapture_ *)*capture;
	if(capture_->capture != NULL) {
		cvReleaseCapture(&capture_->capture);
		capture_->capture = NULL;
	}
	ttLibC_free(capture_);
	*capture = NULL;
#endif
}

#endif
#endif /* __ENABLE_OPENCV__ */
