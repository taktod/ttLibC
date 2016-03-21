/**
 * @file   vtCompressSessionH264Encoder.c
 * @brief  osx or ios native h264 encode.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/02/20
 */

#ifdef __ENABLE_APPLE__

#include "vtCompressSessionH264Encoder.h"
#include "../log.h"
#include "../allocator.h"
#include "../util/hexUtil.h"
#include "../util/dynamicBufferUtil.h"

#include <VideoToolbox/VideoToolbox.h>
#include <string.h>

typedef struct ttLibC_Encoder_VtCompressionSession_VtH264Encoder_ {
	ttLibC_VtH264Encoder inherit_super;

	VTCompressionSessionRef session;
	uint32_t luma_size;
	uint32_t chroma_size;
	CVPixelBufferRef image_buffer;
	uint32_t fps;

	ttLibC_H264 *configData;
	ttLibC_H264 *h264;

	void *ptr;
	ttLibC_VtH264EncodeFunc callback;
} ttLibC_Encoder_VtCompressionSession_VtH264Encoder_;

typedef ttLibC_Encoder_VtCompressionSession_VtH264Encoder_ ttLibC_VtH264Encoder_;

static bool VtH264Encoder_makeH264Frame(
		ttLibC_H264_Type target_type,
		ttLibC_VtH264Encoder_ *encoder,
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
				encoder->h264,
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
		encoder->h264 = h264;
	}
	if(encoder->callback != NULL) {
		if(!encoder->callback(encoder->ptr, h264)) {
			ttLibC_DynamicBuffer_close(&buffer);
			return false;
		}
	}
	return true;
}

static bool VtH264Encoder_checkEncodedData(
		ttLibC_VtH264Encoder_ *encoder,
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
	if(!VtH264Encoder_makeH264Frame(target_type, encoder, data_buffer, pts)) {
		return false;
	}
	ttLibC_DynamicBuffer_close(&data_buffer);
	return true;
}

static void VtH264Encoder_encodeCallback(
		void *outputCallbackRefCon,
		void *sourceFrameRefCon,
		OSStatus status,
		VTEncodeInfoFlags infoFlags,
		CMSampleBufferRef sampleBuffer) {
	ttLibC_VtH264Encoder_ *encoder = (ttLibC_VtH264Encoder_ *)outputCallbackRefCon;
	CMBlockBufferRef block = CMSampleBufferGetDataBuffer(sampleBuffer);
	CFArrayRef attachments =CMSampleBufferGetSampleAttachmentsArray(sampleBuffer, false);
	CMTime pts = CMSampleBufferGetPresentationTimeStamp(sampleBuffer);
	CMTime dts = CMSampleBufferGetDecodeTimeStamp(sampleBuffer);

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
		if(!VtH264Encoder_makeH264Frame(H264Type_configData, encoder, data_buffer, pts)) {
			ERR_PRINT("failed to make configData frame.");
			return;
		}
		ttLibC_DynamicBuffer_close(&data_buffer);
	}
	uint8_t *bufferData;
	size_t size;
	CMBlockBufferGetDataPointer(block, 0, NULL, &size, (char **)&bufferData);
	VtH264Encoder_checkEncodedData(encoder, bufferData, size, pts);
}

/**
 * make vtH264Encoder.
 * @param width
 * @param height
 * @return ttLibCVtH264Encoder object.
 */
ttLibC_VtH264Encoder *ttLibC_VtH264Encoder_make(
		uint32_t width,
		uint32_t height) {
	return ttLibC_VtH264Encoder_make_ex(
			width,
			height,
			15,
			320000,
			true);
}

ttLibC_VtH264Encoder *ttLibC_VtH264Encoder_make_ex(
		uint32_t width,
		uint32_t height,
		uint32_t fps,
		uint32_t bitrate,
		bool is_baseline) {
	ttLibC_VtH264Encoder_ *encoder = ttLibC_malloc(sizeof(ttLibC_VtH264Encoder_));
	if(encoder == NULL) {
		return NULL;
	}
	encoder->session = NULL;
	encoder->luma_size = width * height;
	encoder->chroma_size = encoder->luma_size / 4;
	encoder->inherit_super.width = width;
	encoder->inherit_super.height = height;
	encoder->image_buffer = NULL;
	encoder->configData = NULL;
	encoder->h264 = NULL;
	encoder->fps = fps;
	encoder->callback = NULL;
	encoder->ptr = NULL;
	// make h264 compressSession
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
				kCMVideoCodecType_H264,
				NULL,
				pixelBufferOptions,
				NULL,
				&VtH264Encoder_encodeCallback,
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
		CFRelease(ref);
	}
	// fps
	if(err == noErr) {
		const int v = fps;
		CFNumberRef ref = CFNumberCreate(NULL, kCFNumberSInt32Type, &v);
		err = VTSessionSetProperty(encoder->session, kVTCompressionPropertyKey_ExpectedFrameRate, ref);
		CFRelease(ref);
	}
	// baseline
	if(err == noErr) {
		CFBooleanRef allowFrameReordering = is_baseline ? kCFBooleanFalse : kCFBooleanTrue;
		err = VTSessionSetProperty(encoder->session, kVTCompressionPropertyKey_AllowFrameReordering, allowFrameReordering);
	}
	// bitrate
	if(err == noErr) {
		const int v = bitrate;
		CFNumberRef ref = CFNumberCreate(NULL, kCFNumberSInt32Type, &v);
		err = VTSessionSetProperty(encoder->session, kVTCompressionPropertyKey_AverageBitRate, ref);
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
	}
	// profile level
	if(err == noErr) {
		CFStringRef profileLevel = is_baseline ? kVTProfileLevel_H264_Baseline_AutoLevel : kVTProfileLevel_H264_Main_AutoLevel;
		err = VTSessionSetProperty(encoder->session, kVTCompressionPropertyKey_ProfileLevel, profileLevel);
	}
	// entropy
	if(err == noErr) {
		if(is_baseline) {
			err = VTSessionSetProperty(encoder->session, kVTCompressionPropertyKey_H264EntropyMode, kVTH264EntropyMode_CAVLC);
		}
		else {
			err = VTSessionSetProperty(encoder->session, kVTCompressionPropertyKey_H264EntropyMode, kVTH264EntropyMode_CABAC);
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
	return (ttLibC_VtH264Encoder *)encoder;
}

bool ttLibC_VtH264Encoder_encode(
		ttLibC_VtH264Encoder *encoder,
		ttLibC_Yuv420 *yuv420,
		ttLibC_VtH264EncodeFunc callback,
		void *ptr) {
	ttLibC_VtH264Encoder_ *encoder_ = (ttLibC_VtH264Encoder_ *)encoder;
	encoder_->callback = callback;
	encoder_->ptr = ptr;
	// setup pts
	CMTime pts = CMTimeMake(yuv420->inherit_super.inherit_super.pts, yuv420->inherit_super.inherit_super.timebase);
	CMTime dur = CMTimeMake(1, encoder_->fps);
	VTEncodeInfoFlags flags;

	// update image_buffer
	CVPixelBufferLockBaseAddress(encoder_->image_buffer, 0);
	void *y_data = CVPixelBufferGetBaseAddressOfPlane(encoder_->image_buffer, 0);
	memcpy(y_data, yuv420->y_data, encoder_->luma_size);
	void *u_data = CVPixelBufferGetBaseAddressOfPlane(encoder_->image_buffer, 1);
	memcpy(u_data, yuv420->u_data, encoder_->chroma_size);
	void *v_data = CVPixelBufferGetBaseAddressOfPlane(encoder_->image_buffer, 2);
	memcpy(v_data, yuv420->v_data, encoder_->chroma_size);
	CVPixelBufferUnlockBaseAddress(encoder_->image_buffer, 0);
	// if we want to force keyframe, do something here...
	VTCompressionSessionEncodeFrame(encoder_->session, encoder_->image_buffer, pts, dur, NULL, NULL, &flags);
	return true;
}

void ttLibC_VtH264Encoder_close(ttLibC_VtH264Encoder **encoder) {
	ttLibC_VtH264Encoder_ *target = (ttLibC_VtH264Encoder_ *)*encoder;
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
	ttLibC_H264_close(&target->h264);
	ttLibC_free(target);
	*encoder = NULL;
}

#endif
