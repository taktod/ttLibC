/**
 * @file   vtCompressSessionEncoder.c
 * @brief  osx or ios naative video encode.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/04/13
 */

#ifdef __ENABLE_APPLE__

#include "vtCompressSessionEncoder.h"
#include "../ttLibC_predef.h"
#include "../_log.h"
#include "../allocator.h"
#include "../util/hexUtil.h"
#include "../util/dynamicBufferUtil.h"
#include "../frame/video/h264.h"
#include "../frame/video/jpeg.h"

#include <VideoToolbox/VideoToolbox.h>
#include <string.h>

typedef struct ttLibC_Encoder_VtCompressSession_VtEncoder_ {
	ttLibC_VtEncoder inherit_super;

	VTCompressionSessionRef session;
	uint32_t luma_size;
	uint32_t chroma_size;
	CVPixelBufferRef image_buffer;
	uint32_t fps;

	ttLibC_H264 *configData;
	ttLibC_Video *video;

	void *ptr;
	ttLibC_VtEncodeFunc callback;
} ttLibC_Encoder_VtCompressSession_VtEncoder_;

typedef ttLibC_Encoder_VtCompressSession_VtEncoder_ ttLibC_VtEncoder_;

static bool VtEncoder_makeH264Frame(
		ttLibC_H264_Type target_type,
		ttLibC_VtEncoder_ *encoder,
		ttLibC_DynamicBuffer *buffer,
		CMTime pts) {
	ttLibC_H264 *h264 = NULL;
	if(target_type == H264Type_configData) {
		h264 = ttLibC_H264_make(
				encoder->configData,
				target_type,
				encoder->inherit_super.width,
				encoder->inherit_super.height,
				ttLibC_DynamicBuffer_refData(buffer),
				ttLibC_DynamicBuffer_refSize(buffer),
				false,
				pts.value,
				pts.timescale);
		if(h264 == NULL) {
			ERR_PRINT("failed to make configData for h264Frame.");
			ttLibC_DynamicBuffer_close(&buffer);
			return false;
		}
		encoder->configData = h264;
	}
	else {
		h264 = ttLibC_H264_make(
				(ttLibC_H264 *)encoder->video,
				target_type,
				encoder->inherit_super.width,
				encoder->inherit_super.height,
				ttLibC_DynamicBuffer_refData(buffer),
				ttLibC_DynamicBuffer_refSize(buffer),
				true,
				pts.value,
				pts.timescale);
		if(h264 == NULL) {
			ERR_PRINT("failed to make h264 frame.");
			ttLibC_DynamicBuffer_close(&buffer);
			return false;
		}
		encoder->video = (ttLibC_Video *)h264;
	}
	if(encoder->callback != NULL) {
		if(!encoder->callback(encoder->ptr, (ttLibC_Video *)h264)) {
			ttLibC_DynamicBuffer_close(&buffer);
			return false;
		}
	}
	return true;
}

static bool VtEncoder_checkH264EncodedData(
		ttLibC_VtEncoder_ *encoder,
		uint8_t *data,
		size_t data_size,
		CMTime pts) {
	if(data_size == 0) {
		return true;
	}
	uint8_t separator[] = {0x00, 0x00, 0x00, 0x01};

	ttLibC_H264_Type target_type;
	ttLibC_DynamicBuffer *data_buffer = ttLibC_DynamicBuffer_make();
	ttLibC_H264_NalInfo nal_info;
	while(ttLibC_H264_getAvccInfo(&nal_info, data, data_size)) {
		switch(nal_info.nal_unit_type) {
		case H264NalType_supplementalEnhancementInformation:
			break;
		case H264NalType_slice:
			target_type = H264Type_slice;
			ttLibC_DynamicBuffer_append(data_buffer, separator, 4);
			ttLibC_DynamicBuffer_append(data_buffer, data + nal_info.data_pos, nal_info.nal_size - nal_info.data_pos);
			break;
		case H264NalType_sliceIDR:
			target_type = H264Type_sliceIDR;
			ttLibC_DynamicBuffer_append(data_buffer, separator, 4);
			ttLibC_DynamicBuffer_append(data_buffer, data + nal_info.data_pos, nal_info.nal_size - nal_info.data_pos);
			break;
		default:
			LOG_PRINT("unknown nal type is found:%d", nal_info.nal_unit_type);
			break;
		}
		data += nal_info.nal_size;
		data_size -= nal_info.nal_size;
	}
	if(!VtEncoder_makeH264Frame(target_type, encoder, data_buffer, pts)) {
		return false;
	}
	ttLibC_DynamicBuffer_close(&data_buffer);
	return true;
}

static void VtEncoder_encodeH264Callback(ttLibC_VtEncoder_ *encoder, CMSampleBufferRef sampleBuffer) {
	CMBlockBufferRef block = CMSampleBufferGetDataBuffer(sampleBuffer);
	CFArrayRef attachments =CMSampleBufferGetSampleAttachmentsArray(sampleBuffer, false);
	CMTime pts = CMSampleBufferGetPresentationTimeStamp(sampleBuffer);
//	CMTime dts = CMSampleBufferGetDecodeTimeStamp(sampleBuffer);
	bool is_keyFrame = false;
	if(attachments != NULL) {
		CFDictionaryRef attachment;
		CFBooleanRef dependsOnOthers;
		attachment = (CFDictionaryRef)CFArrayGetValueAtIndex(attachments, 0);
		dependsOnOthers = (CFBooleanRef)CFDictionaryGetValue(attachment, kCMSampleAttachmentKey_DependsOnOthers);
		is_keyFrame = (dependsOnOthers == kCFBooleanFalse);
	}

	if(is_keyFrame) {
		CMFormatDescriptionRef format = CMSampleBufferGetFormatDescription(sampleBuffer);
		size_t sps_size, pps_size;
		size_t parm_count;
		uint8_t *sps, *pps;

		CMVideoFormatDescriptionGetH264ParameterSetAtIndex(format, 0, (const uint8_t **)&sps, &sps_size, &parm_count, NULL);
		CMVideoFormatDescriptionGetH264ParameterSetAtIndex(format, 1, (const uint8_t **)&pps, &pps_size, &parm_count, NULL);

		ttLibC_DynamicBuffer *data_buffer = ttLibC_DynamicBuffer_make();
		uint8_t separator[] = {0x00, 0x00, 0x00, 0x01};
		ttLibC_DynamicBuffer_append(data_buffer, separator, 4);
		ttLibC_DynamicBuffer_append(data_buffer, sps, sps_size);
		ttLibC_DynamicBuffer_append(data_buffer, separator, 4);
		ttLibC_DynamicBuffer_append(data_buffer, pps, pps_size);
		if(!VtEncoder_makeH264Frame(H264Type_configData, encoder, data_buffer, pts)) {
			ERR_PRINT("failed to make configData frame.");
			return;
		}
		ttLibC_DynamicBuffer_close(&data_buffer);
	}
	uint8_t *bufferData;
	size_t size;
	CMBlockBufferGetDataPointer(block, 0, NULL, &size, (char **)&bufferData);
	VtEncoder_checkH264EncodedData(encoder, bufferData, size, pts);
}

static void VtEncoder_encodeJpegCallback(ttLibC_VtEncoder_ *encoder, CMSampleBufferRef sampleBuffer) {
	CMBlockBufferRef block = CMSampleBufferGetDataBuffer(sampleBuffer);
	CMTime pts = CMSampleBufferGetPresentationTimeStamp(sampleBuffer);
//	CMTime dts = CMSampleBufferGetDecodeTimeStamp(sampleBuffer);

	uint8_t *bufferData;
	size_t size;
	CMBlockBufferGetDataPointer(block, 0, NULL, &size, (char **)&bufferData);
	ttLibC_Jpeg *jpeg = ttLibC_Jpeg_make(
			(ttLibC_Jpeg *)encoder->video,
			encoder->inherit_super.width,
			encoder->inherit_super.height,
			bufferData,
			size,
			true,
			pts.value,
			pts.timescale);
	if(jpeg == NULL) {
		return;
	}
	encoder->video = (ttLibC_Video *)jpeg;
	if(encoder->callback != NULL) {
		if(!encoder->callback(encoder->ptr, (ttLibC_Video *)jpeg)) {
			return;
		}
	}
}

static void VtEncoder_encodeCallback(
		void *outputCallbackRefCon,
		void *sourceFrameRefCon,
		OSStatus status,
		VTEncodeInfoFlags infoFlags,
		CMSampleBufferRef sampleBuffer) {
	(void)sourceFrameRefCon;
	(void)status;
	(void)infoFlags;
	ttLibC_VtEncoder_ *encoder = (ttLibC_VtEncoder_ *)outputCallbackRefCon;

	switch(encoder->inherit_super.frame_type) {
	case frameType_jpeg:
		{
			VtEncoder_encodeJpegCallback(encoder, sampleBuffer);
		}
		break;
	case frameType_h264:
		{
			VtEncoder_encodeH264Callback(encoder, sampleBuffer);
		}
		break;
	default:
		return;
	}
}

/**
 * make vtEncoder.
 * @param width
 * @param height
 * @param target_frame_type
 * @return ttLibC_VtEncoder object.
 */
ttLibC_VtEncoder TT_VISIBILITY_DEFAULT *ttLibC_VtEncoder_make(
		uint32_t width,
		uint32_t height,
		ttLibC_Frame_Type target_frame_type) {
	return ttLibC_VtEncoder_make_ex(
			width,
			height,
			15,
			320000,
			true,
			target_frame_type);
}

/**
 * make vtEncoder.
 * @param width
 * @param height
 * @param fps
 * @param bitrate
 * @param is_baseline true:baseline false:main
 * @param target_frame_type
 */
ttLibC_VtEncoder TT_VISIBILITY_DEFAULT *ttLibC_VtEncoder_make_ex(
		uint32_t width,
		uint32_t height,
		uint32_t fps,
		uint32_t bitrate,
		bool is_baseline,
		ttLibC_Frame_Type target_frame_type) {
	CMVideoCodecType codecType;
	switch(target_frame_type) {
//	case frameType_flv1:
//		codecType = kCMVideoCodecType_SorensonVideo;
//		break;
	case frameType_h264:
		codecType = kCMVideoCodecType_H264;
		break;
//	case frameType_h265:
//		codecType = kCMVideoCodecType_HEVC;
//		break;
	case frameType_jpeg:
		codecType = kCMVideoCodecType_JPEG;
		break;
	default:
		return NULL;
	}
	ttLibC_VtEncoder_ *encoder = ttLibC_malloc(sizeof(ttLibC_VtEncoder_));
	if(encoder == NULL) {
		return NULL;
	}
	encoder->session = NULL;
	encoder->luma_size = width * height;
	encoder->chroma_size = encoder->luma_size >> 2;
	encoder->inherit_super.width = width;
	encoder->inherit_super.height = height;
	encoder->inherit_super.frame_type = target_frame_type;
	encoder->image_buffer = NULL;
	encoder->configData = NULL;
	encoder->video = NULL;
	encoder->fps = fps;
	encoder->callback = NULL;
	encoder->ptr = NULL;

	// make compressSession
	OSStatus err = noErr;
	int32_t cvPixelFormatTypeValue = kCVPixelFormatType_420YpCbCr8PlanarFullRange;
	{
		int8_t boolYESValue = 0xFF;

		CFDictionaryRef emptyDict = CFDictionaryCreate(kCFAllocatorDefault, NULL, NULL, 0, NULL, NULL);
		CFNumberRef cvPixelFormatType = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, (const void *)(&(cvPixelFormatTypeValue)));
		CFNumberRef frameW = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, (const void *)(&width));
		CFNumberRef frameH = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, (const void *)(&height));
		CFNumberRef boolYES = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt8Type, (const void *)(&boolYESValue));

		const void *pixelBufferOptionsDictKeys[] = {kCVPixelBufferPixelFormatTypeKey, kCVPixelBufferWidthKey, kCVPixelBufferHeightKey};
		const void *pixelBufferOptionsDictValues[] = {cvPixelFormatType, frameW, frameH};
		CFDictionaryRef pixelBufferOptions = CFDictionaryCreate(kCFAllocatorDefault, pixelBufferOptionsDictKeys, pixelBufferOptionsDictValues, 3, NULL, NULL);

		err = VTCompressionSessionCreate(kCFAllocatorDefault,
				width,
				height,
				codecType,
				NULL,
				pixelBufferOptions,
				NULL,
				&VtEncoder_encodeCallback,
				encoder,
				&encoder->session);
		CFRelease(emptyDict);
		CFRelease(cvPixelFormatType);
		CFRelease(frameW);
		CFRelease(frameH);
		CFRelease(boolYES);
		CFRelease(pixelBufferOptions);
		if(err != noErr) {
			ttLibC_free(encoder);
			return NULL;
		}
	}
	// gop
	if(err == noErr) {
		const int32_t v = fps * 2;
		CFNumberRef ref = CFNumberCreate(NULL, kCFNumberSInt32Type, &v);
		err = VTSessionSetProperty(encoder->session, kVTCompressionPropertyKey_MaxKeyFrameInterval, ref);
		if(err == kVTPropertyNotSupportedErr) {
			err = noErr;
		}
		CFRelease(ref);
	}
	// fps
	if(err == noErr) {
		const int v = fps;
		CFNumberRef ref = CFNumberCreate(NULL, kCFNumberSInt32Type, &v);
		err = VTSessionSetProperty(encoder->session, kVTCompressionPropertyKey_ExpectedFrameRate, ref);
		if(err == kVTPropertyNotSupportedErr) {
			err = noErr;
		}
		CFRelease(ref);
	}
	// baseline
	if(err == noErr) {
		CFBooleanRef allowFrameReordering = is_baseline ? kCFBooleanFalse : kCFBooleanTrue;
		err = VTSessionSetProperty(encoder->session, kVTCompressionPropertyKey_AllowFrameReordering, allowFrameReordering);
		if(err == kVTPropertyNotSupportedErr) {
			err = noErr;
		}
	}
	// bitrate
	if(err == noErr) {
		const int v = bitrate;
		CFNumberRef ref = CFNumberCreate(NULL, kCFNumberSInt32Type, &v);
		err = VTSessionSetProperty(encoder->session, kVTCompressionPropertyKey_AverageBitRate, ref);
		if(err == kVTPropertyNotSupportedErr) {
			err = noErr;
		}
		CFRelease(ref);
	}
	// no delay
//	if(err == noErr) {
//		const int v = 99;
//		CFNumberRef ref = CFNumberCreate(NULL, kCFNumberSInt32Type, &v);
//		err = VTSessionSetProperty(encoder->session, kVTCompressionPropertyKey_MaxFrameDelayCount, NULL);
//		CFRelease(ref);
//	}
	// realtime
	if(err == noErr) {
		err = VTSessionSetProperty(encoder->session, kVTCompressionPropertyKey_RealTime, kCFBooleanTrue);
		if(err == kVTPropertyNotSupportedErr) {
			err = noErr;
		}
	}
	// profile level
	if(err == noErr) {
		CFStringRef profileLevel = is_baseline ? kVTProfileLevel_H264_Baseline_AutoLevel : kVTProfileLevel_H264_Main_AutoLevel;
		err = VTSessionSetProperty(encoder->session, kVTCompressionPropertyKey_ProfileLevel, profileLevel);
		if(err == kVTPropertyNotSupportedErr) {
			err = noErr;
		}
	}	// entropy
	if(err == noErr) {
		if(is_baseline) {
			err = VTSessionSetProperty(encoder->session, kVTCompressionPropertyKey_H264EntropyMode, kVTH264EntropyMode_CAVLC);
		}
		else {
			err = VTSessionSetProperty(encoder->session, kVTCompressionPropertyKey_H264EntropyMode, kVTH264EntropyMode_CABAC);
		}
		if(err == kVTPropertyNotSupportedErr) {
			err = noErr;
		}
	}
	// ready!
	if(err == noErr) {
		VTCompressionSessionPrepareToEncodeFrames(encoder->session);
	}
	else {
		VTCompressionSessionInvalidate(encoder->session);
		CFRelease(encoder->session);
		ttLibC_free(encoder);
		return NULL;
	}
	// make image_buffer
	CVPixelBufferCreate(
			kCFAllocatorDefault,
			width,
			height,
			cvPixelFormatTypeValue,
			NULL,
			&encoder->image_buffer);
	if(encoder->image_buffer == NULL) {
		VTCompressionSessionInvalidate(encoder->session);
		CFRelease(encoder->session);
		ttLibC_free(encoder);
		return NULL;
	}
	return (ttLibC_VtEncoder *)encoder;
}

bool TT_VISIBILITY_DEFAULT ttLibC_VtEncoder_encode(
		ttLibC_VtEncoder *encoder,
		ttLibC_Yuv420 *yuv420,
		ttLibC_VtEncodeFunc callback,
		void *ptr) {
	ttLibC_VtEncoder_ *encoder_ = (ttLibC_VtEncoder_ *)encoder;
	if(encoder_ == NULL) {
		return false;
	}
	if(yuv420 == NULL) {
		return true;
	}
	switch(yuv420->type) {
	case Yuv420Type_planar:
	case Yvu420Type_planar:
		break;
	case Yuv420Type_semiPlanar:
	case Yvu420Type_semiPlanar:
		ERR_PRINT("only support planar.");
		return false;
	}
	encoder_->callback = callback;
	encoder_->ptr = ptr;
	// setup pts
	CMTime pts = CMTimeMake(yuv420->inherit_super.inherit_super.pts, yuv420->inherit_super.inherit_super.timebase);
	CMTime dur = CMTimeMake(1, encoder_->fps);
	VTEncodeInfoFlags flags;
	// update image_buffer
	CVPixelBufferLockBaseAddress(encoder_->image_buffer, 0);
	void *y_data = CVPixelBufferGetBaseAddressOfPlane(encoder_->image_buffer, 0);
	uint32_t y_stride = CVPixelBufferGetBytesPerRowOfPlane(encoder_->image_buffer, 0);
	if(yuv420->y_stride != y_stride) {
		uint8_t *y_buf = y_data;
		uint8_t *y_org_buf = yuv420->y_data;
		for(uint32_t i = 0;i < yuv420->inherit_super.height;++ i) {
			memcpy(y_buf, y_org_buf, yuv420->inherit_super.width);
			y_buf += y_stride;
			y_org_buf += yuv420->y_stride;
		}
	}
	else {
		memcpy(y_data, yuv420->y_data, encoder_->luma_size);
	}
	void *u_data = CVPixelBufferGetBaseAddressOfPlane(encoder_->image_buffer, 1);
	uint32_t u_stride = CVPixelBufferGetBytesPerRowOfPlane(encoder_->image_buffer, 1);
	uint32_t chroma_width = (yuv420->inherit_super.width >> 1);
	uint32_t chroma_height = (yuv420->inherit_super.height >> 1);
	if(yuv420->u_stride != u_stride) {
		uint8_t *u_buf = u_data;
		uint8_t *u_org_buf = yuv420->u_data;
		for(uint32_t i = 0;i < chroma_height;++ i) {
			memcpy(u_buf, u_org_buf, chroma_width);
			u_buf += u_stride;
			u_org_buf += yuv420->u_stride;
		}
	}
	else {
		memcpy(u_data, yuv420->u_data, encoder_->chroma_size);
	}
	void *v_data = CVPixelBufferGetBaseAddressOfPlane(encoder_->image_buffer, 2);
	uint32_t v_stride = CVPixelBufferGetBytesPerRowOfPlane(encoder_->image_buffer, 2);
	if(yuv420->v_stride != v_stride) {
		uint8_t *v_buf = v_data;
		uint8_t *v_org_buf = yuv420->v_data;
		for(uint32_t i = 0;i < chroma_height;++ i) {
			memcpy(v_buf, v_org_buf, chroma_width);
			v_buf += v_stride;
			v_org_buf += yuv420->v_stride;
		}
	}
	else {
		memcpy(v_data, yuv420->v_data, encoder_->chroma_size);
	}
	CVPixelBufferUnlockBaseAddress(encoder_->image_buffer, 0);
	// if we want to force keyframe, do something here...
	VTCompressionSessionEncodeFrame(encoder_->session, encoder_->image_buffer, pts, dur, NULL, NULL, &flags);
	return true;
}

void TT_VISIBILITY_DEFAULT ttLibC_VtEncoder_close(ttLibC_VtEncoder **encoder) {
	ttLibC_VtEncoder_ *target = (ttLibC_VtEncoder_ *)*encoder;
	if(target == NULL) {
		return;
	}
	if(target->image_buffer != NULL) {
		CFRelease(target->image_buffer);
		target->image_buffer = NULL;
	}
	if(target->session != NULL) {
		VTCompressionSessionInvalidate(target->session);
		CFRelease(target->session);
		target->session = NULL;
	}
	ttLibC_H264_close(&target->configData);
	ttLibC_Video_close(&target->video);
	ttLibC_free(target);
	*encoder = NULL;
}

#endif
