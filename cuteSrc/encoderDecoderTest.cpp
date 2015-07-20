/**
 * @file   encoderDecoderTest.cpp
 * @brief  encoder, decoder, and resampler test code.
 *
 * this code is under GPLv3 license.
 *
 * @author taktod
 * @date   2015/07/18
 */

#include <cute.h>

#include <ttLibC/log.h>

#ifdef __ENABLE_OPENCV__
#	include <ttLibC/util/opencvUtil.h>
#endif

#include <ttLibC/frame/video/bgr.h>
#include <ttLibC/frame/video/yuv420.h>

#include <ttLibC/resampler/imageResampler.h>

/**
 * how to use imageResampler for bgr data.
 */
static void imageResamplerTest() {
	LOG_PRINT("imageResamplerTest");
#ifdef __ENABLE_OPENCV__
	ttLibC_CvCapture *capture = ttLibC_CvCapture_makeCapture(0, 320, 240);
	ttLibC_CvWindow *window = ttLibC_CvWindow_makeWindow("opencvTest");
	ttLibC_CvWindow *resampled_window = ttLibC_CvWindow_makeWindow("resampled");
	ttLibC_Bgr *bgr = NULL;
	ttLibC_Yuv420 *yuv420 = NULL;
	ttLibC_Bgr *resampled_bgr = NULL;
	while(true) {
		ttLibC_Bgr *b = ttLibC_CvCapture_queryFrame(capture, bgr);
		if(b == NULL) {
			break;
		}
		bgr = b;
		ttLibC_Yuv420 *yuv = ttLibC_ImageResampler_makeYuv420FromBgr(yuv420, Yuv420Type_planar, bgr);
		if(yuv == NULL) {
			break;
		}
		yuv420 = yuv;
		b = ttLibC_ImageResampler_makeBgrFromYuv420(resampled_bgr, BgrType_abgr, yuv420);
		if(b == NULL) {
			break;
		}
		resampled_bgr = b;
		ttLibC_CvWindow_showBgr(window, bgr);
		ttLibC_CvWindow_showBgr(resampled_window, resampled_bgr);
		uint8_t key = ttLibC_CvWindow_waitForKeyInput(10);
		if(key == Keychar_Esc) {
			break;
		}
	}
	ttLibC_Bgr_close(&resampled_bgr);
	ttLibC_Yuv420_close(&yuv420);
	ttLibC_Bgr_close(&bgr);
	ttLibC_CvWindow_close(&resampled_window);
	ttLibC_CvWindow_close(&window);
	ttLibC_CvCapture_close(&capture);
#endif
}

/**
 * define all test for encoder and decoder package.
 * @param s cute::suite obj
 * @return cute::suite obj
 */
cute::suite encoderDecoderTests(cute::suite s) {
	s.clear();
	s.push_back(CUTE(imageResamplerTest));
	return s;
}



