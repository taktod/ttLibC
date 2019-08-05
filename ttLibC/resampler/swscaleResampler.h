/**
 * @file   swscaleResampler.h
 * @brief  
 * @author taktod
 * @date   2017/04/28
 */

#ifndef TTLIBC_RESAMPLER_SWSCALERESAMPLER_H_
#define TTLIBC_RESAMPLER_SWSCALERESAMPLER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../ttLibC_predef.h"
#include "../frame/video/yuv420.h"
#include "../frame/video/bgr.h"
#include "../frame/frame.h"

typedef enum ttLibC_SwscaleResampler_Mode {
	SwscaleResampler_FastBiLinear,
	SwscaleResampler_Bilinear,
	SwscaleResampler_Bicubic,
	SwscaleResampler_X,
	SwscaleResampler_Point,
	SwscaleResampler_Area,
	SwscaleResampler_Bicublin,
	SwscaleResampler_Gauss,
	SwscaleResampler_Sinc,
	SwscaleResampler_Lanczos,
	SwscaleResampler_Spline
} ttLibC_SwscaleResampler_Mode;

typedef struct ttLibC_Resampler_SwscaleResampler {
	/** target width */
	uint32_t width;
	/** target height */
	uint32_t height;
} ttLibC_Resampler_SwscaleResampler;

typedef ttLibC_Resampler_SwscaleResampler ttLibC_SwscaleResampler;

ttLibC_SwscaleResampler TT_ATTRIBUTE_API *ttLibC_SwscaleResampler_make(
		ttLibC_Frame_Type            input_frame_type,
		uint32_t                     input_sub_type,
		uint32_t                     input_width,
		uint32_t                     input_height,
		ttLibC_Frame_Type            output_frame_type,
		uint32_t                     output_sub_type,
		uint32_t                     output_width,
		uint32_t                     output_height,
		ttLibC_SwscaleResampler_Mode scale_mode);

bool TT_ATTRIBUTE_API ttLibC_SwscaleResampler_resample(
		ttLibC_SwscaleResampler *resampler,
		ttLibC_Video *dest_frame,
		ttLibC_Video *src_frame);

void TT_ATTRIBUTE_API ttLibC_SwscaleResampler_close(ttLibC_SwscaleResampler **resampler);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_RESAMPLER_SWSCALERESAMPLER_H_ */
