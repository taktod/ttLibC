/**
 * @file   vtDecompressSessionDecoder.c
 * @brief  osx or ios native decode.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/04/13
 */

#ifdef __ENABLE_APPLE__

#include "vtDecompressSessionDecoder.h"
#include "../log.h"
#include "../allocator.h"
#include "../util/dynamicBufferUtil.h"
#include "../util/ioUtil.h"
#include "../util/hexUtil.h"
#include "../frame/video/h264.h"
#include "../frame/video/jpeg.h"

#include <string.h>
#include <VideoToolbox/VideoToolbox.h>

typedef struct ttLibC_Decoder_VtDecompressionSession_VtDecoder_ {
	ttLibC_VtDecoder inherit_super;

	CMFormatDescriptionRef desc;
	VTDecompressionSessionRef session;

	ttLibC_Yuv420 *yuv420;

	ttLibC_DynamicBuffer *buffer;

	ttLibC_VtDecodeFunc callback;
	ttLibC_VtDecodeRawFunc raw_callback;
	void *ptr;
} ttLibC_Decoder_VtDecompressionSession_VtDecoder_;

typedef ttLibC_Decoder_VtDecompressionSession_VtDecoder_ ttLibC_VtDecoder_;

static void VtDecoder_decodeCallback(
		void *decompressionOutputRefCon,
		void *sourceFrameRefCon,
		OSStatus status,
		VTDecodeInfoFlags infoFlags,
		CVImageBufferRef imageBuffer,
		CMTime pts,
		CMTime dts) {
	if(status != noErr) {
		ERR_PRINT("error on decode buffer.:%x(%d)", status, status);
		return;
	}
	ttLibC_VtDecoder_ *decoder = (ttLibC_VtDecoder_ *)decompressionOutputRefCon;
	if(decoder->raw_callback != NULL) {
		if(!decoder->raw_callback(decoder->ptr, imageBuffer, &pts, &dts)) {
		}
	}
	if(decoder->callback != NULL) {
		uint32_t width = CVPixelBufferGetWidth(imageBuffer);
		uint32_t height = CVPixelBufferGetHeight(imageBuffer);
		decoder->inherit_super.width = width;
		decoder->inherit_super.height = height;
		CVPixelBufferLockBaseAddress(imageBuffer, 0);
		uint8_t *y_data = (uint8_t *)CVPixelBufferGetBaseAddressOfPlane(imageBuffer, 0);
		uint8_t *uv_data = (uint8_t *)CVPixelBufferGetBaseAddressOfPlane(imageBuffer, 1);

		ttLibC_Yuv420 *y = ttLibC_Yuv420_make(
				decoder->yuv420,
				Yuv420Type_semiPlanar,
				width,
				height,
				NULL,
				0,
				y_data,
				CVPixelBufferGetBytesPerRowOfPlane(imageBuffer, 0),
				uv_data,
				CVPixelBufferGetBytesPerRowOfPlane(imageBuffer, 1),
				uv_data + 1,
				CVPixelBufferGetBytesPerRowOfPlane(imageBuffer, 1),
				true,
				pts.value,
				pts.timescale);
		if(y != NULL) {
			decoder->yuv420 = y;
			if(decoder->callback != NULL) {
				if(!decoder->callback(decoder->ptr, decoder->yuv420)) {
				}
			}
		}
		CVPixelBufferUnlockBaseAddress(imageBuffer, 0);
	}
}

ttLibC_VtDecoder *ttLibC_VtDecoder_make(ttLibC_Frame_Type target_frame_type) {
	ttLibC_VtDecoder_ *decoder = ttLibC_malloc(sizeof(ttLibC_VtDecoder_));
	if(decoder == NULL) {
		return NULL;
	}
	decoder->session = NULL;
	decoder->desc = NULL;
	decoder->inherit_super.frame_type = target_frame_type;
	decoder->yuv420 = NULL;
	decoder->inherit_super.width = 0;
	decoder->inherit_super.height = 0;
	decoder->buffer = ttLibC_DynamicBuffer_make();
	decoder->callback = NULL;
	decoder->ptr = NULL;
	return (ttLibC_VtDecoder *)decoder;
}

static bool VtDecoder_decodeH264(
		ttLibC_VtDecoder_ *decoder,
		ttLibC_H264 *h264,
		ttLibC_VtDecodeFunc callback,
		ttLibC_VtDecodeRawFunc raw_callback,
		void *ptr) {
	decoder->callback = callback;
	decoder->raw_callback = raw_callback;
	decoder->ptr = ptr;
	uint8_t *data = (uint8_t *)h264->inherit_super.inherit_super.data;
	size_t data_size = h264->inherit_super.inherit_super.buffer_size;
	OSStatus err = noErr;
	switch(h264->type) {
	case H264Type_configData:
		{
			// support only sps pps 1:1.
			uint8_t *param_sets[2];
			size_t param_sets_size[2];

			ttLibC_H264_NalInfo nal_info;
			// sps
			ttLibC_H264_getNalInfo(&nal_info, data, data_size);
			if(nal_info.nal_unit_type != H264NalType_sequenceParameterSet) {
				ERR_PRINT("1st nal of configData is not sps. unexpected.");
				return false;
			}
			param_sets[0] = data + nal_info.data_pos;
			param_sets_size[0] = nal_info.nal_size - nal_info.data_pos;
			data += nal_info.nal_size;
			data_size -= nal_info.nal_size;
			// pps
			ttLibC_H264_getNalInfo(&nal_info, data, data_size);
			if(nal_info.nal_unit_type != H264NalType_pictureParameterSet) {
				ERR_PRINT("2nd nal of configData is not pps. unexpected.");
				return false;
			}
			param_sets[1] = data + nal_info.data_pos;
			param_sets_size[1] = nal_info.nal_size - nal_info.data_pos;
			data += nal_info.nal_size;
			data_size -= nal_info.nal_size;
			if(data_size != 0) {
				ERR_PRINT("configData hold extra data. unexpected.");
				return false;
			}
			if(decoder->desc != NULL) {
				CFRelease(decoder->desc);
				decoder->desc = NULL;
			}
			err = CMVideoFormatDescriptionCreateFromH264ParameterSets(kCFAllocatorDefault, 2, (const uint8_t *const *)param_sets, param_sets_size, 4, &decoder->desc);
			if(err != noErr) {
				ERR_PRINT("failed to make video format description.%x %d", err, err);
				return false;
			}
			if(decoder->session == NULL) {
				CFMutableDictionaryRef destinationPixelBufferAttributes;
				destinationPixelBufferAttributes = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
				CFNumberRef number;

				int val = kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;
				number = CFNumberCreate(NULL, kCFNumberSInt32Type, &val);
				CFDictionarySetValue(destinationPixelBufferAttributes, kCVPixelBufferPixelFormatTypeKey,number);
				CFRelease(number);

				VTDecompressionOutputCallbackRecord outputCallback;
				outputCallback.decompressionOutputCallback = VtDecoder_decodeCallback;
				outputCallback.decompressionOutputRefCon = decoder;
				err = VTDecompressionSessionCreate(NULL, decoder->desc, NULL, destinationPixelBufferAttributes, &outputCallback, &decoder->session);
				if(err != noErr) {
					ERR_PRINT("failed to make decompress session.:%x %d", err, err);
					return FALSE;
				}
			}
		}
		break;
	case H264Type_slice:
	case H264Type_sliceIDR:
		{
			CMSampleBufferRef sampleBuffer = NULL;
			CMBlockBufferRef blockBuffer = NULL;
			ttLibC_DynamicBuffer_empty(decoder->buffer);
			uint8_t *data = h264->inherit_super.inherit_super.data;
			size_t data_size = h264->inherit_super.inherit_super.buffer_size;
			// make size nal data.
			ttLibC_H264_NalInfo nal_info;
			uint32_t nal_data_size = 0;
			uint32_t nal_data_size_be;
			do {
				ttLibC_H264_getNalInfo(&nal_info, data, data_size);
				nal_data_size = nal_info.nal_size - nal_info.data_pos;
				nal_data_size_be = be_uint32_t(nal_data_size);
				ttLibC_DynamicBuffer_append(decoder->buffer, (uint8_t *)(&nal_data_size_be), 4);
				ttLibC_DynamicBuffer_append(decoder->buffer, data + nal_info.data_pos, nal_data_size);
				data += nal_info.nal_size;
				data_size -= nal_info.nal_size;
			} while(data_size > 0);
			uint8_t *buffer = ttLibC_DynamicBuffer_refData(decoder->buffer);
			size_t buffer_size = ttLibC_DynamicBuffer_refSize(decoder->buffer);
			// make buffer for OSX or iOS
			err = CMBlockBufferCreateWithMemoryBlock(
					NULL,
					buffer,
					buffer_size,
					kCFAllocatorNull,
					NULL,
					0,
					buffer_size,
					0,
					&blockBuffer);
			if(err != noErr) {
				ERR_PRINT("failed to make blockBuffer.%x %d", err, err);
				return false;
			}
			err = CMSampleBufferCreate(
					kCFAllocatorDefault,
					blockBuffer,
					true,
					NULL,
					NULL,
					decoder->desc,
					1,
					0,
					NULL,
					1,
					&buffer_size,
					&sampleBuffer);
			if(err != noErr) {
				ERR_PRINT("failed to make sampleBuffer.%x %d", err, err);
				CFRelease(blockBuffer);
				return false;
			}
			CMSampleBufferSetOutputPresentationTimeStamp(sampleBuffer, CMTimeMake(h264->inherit_super.inherit_super.pts, h264->inherit_super.inherit_super.timebase));
			if(decoder->session) {
				VTDecodeFrameFlags flags = 0;
				VTDecodeInfoFlags flagsOut;
				err = VTDecompressionSessionDecodeFrame(decoder->session, sampleBuffer, flags, NULL, &flagsOut);
				if(err != noErr) {
					ERR_PRINT("failed to decode frame.%x %d", err, err);
					CFRelease(blockBuffer);
					CFRelease(sampleBuffer);
					return false;
				}
			}
			CFRelease(blockBuffer);
			CFRelease(sampleBuffer);
		}
		break;
	default:
		ERR_PRINT("unexpected nal:%d", h264->type);
		break;
	}
	return true;
}

static bool VtDecoder_decodeJpeg(
		ttLibC_VtDecoder_ *decoder,
		ttLibC_Jpeg *jpeg,
		ttLibC_VtDecodeFunc callback,
		ttLibC_VtDecodeRawFunc raw_callback,
		void *ptr) {
	decoder->callback = callback;
	decoder->raw_callback = raw_callback;
	decoder->ptr = ptr;
	OSStatus err = noErr;
	if(decoder->session == NULL) {
		err = CMVideoFormatDescriptionCreate(
				kCFAllocatorDefault,
				kCMVideoCodecType_JPEG,
				jpeg->inherit_super.width,
				jpeg->inherit_super.height,
				NULL,
				&decoder->desc);
		CFMutableDictionaryRef destinationPixelBufferAttributes;
		destinationPixelBufferAttributes = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		CFNumberRef number;

		int val = kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;
		number = CFNumberCreate(NULL, kCFNumberSInt32Type, &val);
		CFDictionarySetValue(destinationPixelBufferAttributes, kCVPixelBufferPixelFormatTypeKey,number);
		CFRelease(number);

		VTDecompressionOutputCallbackRecord outputCallback;
		outputCallback.decompressionOutputCallback = VtDecoder_decodeCallback;
		outputCallback.decompressionOutputRefCon = decoder;
		err = VTDecompressionSessionCreate(NULL, decoder->desc, NULL, destinationPixelBufferAttributes, &outputCallback, &decoder->session);
		if(err != noErr) {
			ERR_PRINT("failed to make decompress session.:%x %d", err, err);
			return FALSE;
		}
	}
	CMSampleBufferRef sampleBuffer = NULL;
	CMBlockBufferRef blockBuffer = NULL;
	uint8_t *buffer = jpeg->inherit_super.inherit_super.data;
	size_t buffer_size = jpeg->inherit_super.inherit_super.buffer_size;
	err = CMBlockBufferCreateWithMemoryBlock(
			NULL,
			buffer,
			buffer_size,
			kCFAllocatorNull,
			NULL,
			0,
			buffer_size,
			0,
			&blockBuffer);
	if(err != noErr) {
		ERR_PRINT("failed to make blockBuffer.%x %d", err, err);
		return false;
	}
	err = CMSampleBufferCreate(
			kCFAllocatorDefault,
			blockBuffer,
			true,
			NULL,
			NULL,
			decoder->desc,
			1,
			0,
			NULL,
			1,
			&buffer_size,
			&sampleBuffer);
	if(err != noErr) {
		ERR_PRINT("failed to make sampleBuffer.%x %d", err, err);
		CFRelease(blockBuffer);
		return false;
	}
	CMSampleBufferSetOutputPresentationTimeStamp(sampleBuffer, CMTimeMake(jpeg->inherit_super.inherit_super.pts, jpeg->inherit_super.inherit_super.timebase));
	if(decoder->session) {
		VTDecodeFrameFlags flags = 0;
		VTDecodeInfoFlags flagsOut;
		err = VTDecompressionSessionDecodeFrame(decoder->session, sampleBuffer, flags, NULL, &flagsOut);
		if(err != noErr) {
			ERR_PRINT("failed to decode frame.%x %d", err, err);
			CFRelease(blockBuffer);
			CFRelease(sampleBuffer);
			return false;
		}
	}
	CFRelease(blockBuffer);
	CFRelease(sampleBuffer);
	return true;
}

static bool VtDecoder_decode(
		ttLibC_VtDecoder *decoder,
		ttLibC_Video *video,
		ttLibC_VtDecodeFunc callback,
		ttLibC_VtDecodeRawFunc raw_callback,
		void *ptr) {
	if(decoder == NULL) {
		return false;
	}
	if(video == NULL) {
		return true;
	}
	ttLibC_VtDecoder_ *decoder_ = (ttLibC_VtDecoder_ *)decoder;
	switch(decoder_->inherit_super.frame_type) {
	case frameType_h264:
		return VtDecoder_decodeH264(decoder_,
				(ttLibC_H264 *)video,
				callback,
				raw_callback,
				ptr);
	case frameType_jpeg:
		return VtDecoder_decodeJpeg(decoder_,
				(ttLibC_Jpeg *)video,
				callback,
				raw_callback,
				ptr);
	default:
		return true;
	}
	return true;
}

bool ttLibC_VtDecoder_decode(
		ttLibC_VtDecoder *decoder,
		ttLibC_Video *video,
		ttLibC_VtDecodeFunc callback,
		void *ptr) {
	return VtDecoder_decode(decoder,
			video,
			callback,
			NULL,
			ptr);
}

bool ttLibC_VtDecoder_rawDecode(
		ttLibC_VtDecoder *decoder,
		ttLibC_Video *video,
		ttLibC_VtDecodeRawFunc callback,
		void *ptr) {
	return VtDecoder_decode(
			decoder,
			video,
			NULL,
			callback,
			ptr);
}

void ttLibC_VtDecoder_close(ttLibC_VtDecoder **decoder) {
	ttLibC_VtDecoder_ *target = (ttLibC_VtDecoder_ *)*decoder;
	if(target == NULL) {
		return;
	}
	if(target->session != NULL)
	{
		VTDecompressionSessionInvalidate(target->session);
		CFRelease(target->session);
		target->session = NULL;
	}
	if(target->desc != NULL) {
		CFRelease(target->desc);
		target->desc = NULL;
	}
	ttLibC_DynamicBuffer_close(&target->buffer);
	ttLibC_Yuv420_close(&target->yuv420);
	ttLibC_free(target);
	*decoder = NULL;
}

#endif
