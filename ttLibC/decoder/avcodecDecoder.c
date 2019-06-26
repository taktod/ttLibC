/*
 * @file   avcodecDecoder.c
 * @brief  decode frame with libavcodec
 *
 * this code is under LGPLv3 license.
 *
 * @author taktod
 * @date   2015/08/02
 */

#ifdef __ENABLE_AVCODEC__

#include "avcodecDecoder.h"

#include <libavcodec/avcodec.h>
#include "../_log.h"
#include "../allocator.h"
#include "../util/hexUtil.h"
#include "../util/dynamicBufferUtil.h"

#include "../frame/video/bgr.h"
#include "../frame/video/h264.h"
#include "../frame/video/theora.h"
#include "../frame/video/yuv420.h"

#include "../frame/audio/aac.h"
#include "../frame/audio/pcms16.h"
#include "../frame/audio/pcmf32.h"
#include "../frame/audio/speex.h"
#include "../frame/audio/vorbis.h"

#ifdef __ENABLE_SWSCALE__
#	include <libswscale/swscale.h>
#endif

/*
 * avcodecDecoder detail definition
 */
typedef struct {
	ttLibC_AvcodecDecoder inherit_super;
	AVCodecContext *dec;
	AVFrame *avframe;
	bool is_opened;

	AVPacket packet;
	ttLibC_Frame *frame;
	ttLibC_Frame *h26x_configData;
	ttLibC_DynamicBuffer *extraDataBuffer;

#ifdef __ENABLE_SWSCALE__
	struct SwsContext *convertCtx;
	enum AVPixelFormat inFormat;
#endif
} ttLibC_Decoder_AvcodecDecoder_;

typedef ttLibC_Decoder_AvcodecDecoder_ ttLibC_AvcodecDecoder_;

/*
 * do audio decode.
 * @param decoder  decoder object
 * @param frame    target frame
 * @param callback callback func
 * @param ptr      user def pointer
 */
static bool AvcodecDecoder_decodeAudio(
		ttLibC_AvcodecDecoder_ *decoder,
		ttLibC_Audio *frame,
		ttLibC_AvcodecDecodeFunc callback,
		void *ptr) {
	switch(frame->inherit_super.type) {
	case frameType_aac:
		{
			ttLibC_Aac *aac = (ttLibC_Aac *)frame;
			if(aac->type == AacType_dsi) {
				return true;
			}
		}
		break;
	case frameType_vorbis:
		{
			ttLibC_Vorbis *vorbis = (ttLibC_Vorbis *)frame;
			switch(vorbis->type) {
			case VorbisType_identification:
				if(decoder->extraDataBuffer == NULL) {
					decoder->extraDataBuffer = ttLibC_DynamicBuffer_make();
				}
				else {
					ttLibC_DynamicBuffer_reset(decoder->extraDataBuffer);
				}
				uint8_t buf[3] = {0x02, frame->inherit_super.buffer_size, 0};
				ttLibC_DynamicBuffer_append(decoder->extraDataBuffer, buf, 3);
				ttLibC_DynamicBuffer_append(decoder->extraDataBuffer, frame->inherit_super.data, frame->inherit_super.buffer_size);
				return true;
			case VorbisType_comment:
				{
					uint8_t *buf = ttLibC_DynamicBuffer_refData(decoder->extraDataBuffer);
					buf[2] = frame->inherit_super.buffer_size;
					ttLibC_DynamicBuffer_append(decoder->extraDataBuffer, frame->inherit_super.data, frame->inherit_super.buffer_size);
				}
				return true;
			case VorbisType_setup:
				ttLibC_DynamicBuffer_append(decoder->extraDataBuffer, frame->inherit_super.data, frame->inherit_super.buffer_size);
				decoder->dec->extradata = ttLibC_DynamicBuffer_refData(decoder->extraDataBuffer);
				decoder->dec->extradata_size = ttLibC_DynamicBuffer_refSize(decoder->extraDataBuffer);
				if(!decoder->is_opened) {
					int result = 0;
					if((result = avcodec_open2(decoder->dec, decoder->dec->codec, NULL)) < 0) {
						ERR_PRINT("failed to open codec.:%d", AVERROR(result));
						av_free(decoder->dec);
						ttLibC_free(decoder);
						return NULL;
					}
					decoder->is_opened = true;
				}
				else {
					ERR_PRINT("avcodec is already opened, therefore failed to set private data.");
				}
				return true;
			default:
				break;
			}
		}
		break;
	case frameType_speex:
		{
			ttLibC_Speex *speex = (ttLibC_Speex *)frame;
			switch(speex->type) {
			case SpeexType_header:
				if(decoder->extraDataBuffer == NULL) {
					decoder->extraDataBuffer = ttLibC_DynamicBuffer_make();
				}
				else {
					ttLibC_DynamicBuffer_reset(decoder->extraDataBuffer);
				}
				ttLibC_DynamicBuffer_append(decoder->extraDataBuffer, frame->inherit_super.data, frame->inherit_super.buffer_size);
				decoder->dec->extradata = ttLibC_DynamicBuffer_refData(decoder->extraDataBuffer);
				decoder->dec->extradata_size = ttLibC_DynamicBuffer_refSize(decoder->extraDataBuffer);
				return true;
			case SpeexType_comment:
				return true;
			default:
				if(!decoder->is_opened) {
					int result = 0;
					if((result = avcodec_open2(decoder->dec, decoder->dec->codec, NULL)) < 0) {
						ERR_PRINT("failed to open codec.:%d", AVERROR(result));
						av_free(decoder->dec);
						ttLibC_free(decoder);
						return NULL;
					}
					decoder->is_opened = true;
				}
				break;
			}
		}
		break;
	default:
		break;
	}
	decoder->packet.data = frame->inherit_super.data;
	decoder->packet.size = frame->inherit_super.buffer_size;
	decoder->packet.pts = frame->inherit_super.pts;
	int got_frame;
	int result = avcodec_decode_audio4(decoder->dec, decoder->avframe, &got_frame, &decoder->packet);
	if(result < 0) {
		ERR_PRINT("failed to decode:%d", result);
		return false;
	}
	if(got_frame != 1) {
		return true;
	}
#ifdef TT_FF_OLD_AVCODEC
	int result = avcodec_send_packet(decoder->dec, &decoder->packet);
	if(result < 0 || result == AVERROR_EOF) {
		ERR_PRINT("failed to decoder:%d", result);
		return false;
	}
	do {
		result = avcodec_receive_frame(decoder->dec, decoder->avframe);
		if(result == AVERROR(EAGAIN)) {
			return true;
		}
		if(result < 0) {
			ERR_PRINT("failed to receive:%d", result);
			return false;
		}
#endif
	// decode complete, now to make frame and call callback.
	decoder->inherit_super.sample_rate = decoder->avframe->sample_rate;
	decoder->inherit_super.channel_num = decoder->avframe->channels;
	switch(decoder->dec->sample_fmt) {
	case AV_SAMPLE_FMT_FLT:
		{
			if(decoder->frame != NULL && decoder->frame->type != frameType_pcmF32) {
				ttLibC_Frame_close(&decoder->frame);
			}
			ttLibC_PcmF32 *pcmf32 = ttLibC_PcmF32_make(
					(ttLibC_PcmF32 *)decoder->frame,
					PcmF32Type_interleave,
					decoder->avframe->sample_rate,
					decoder->avframe->nb_samples,
					decoder->avframe->channels,
					NULL,
					0,
					decoder->avframe->data[0],
					decoder->avframe->nb_samples * 4 * decoder->avframe->channels,
					NULL,
					0,
					true,
#ifndef FF_API_PKT_PTS
					decoder->avframe->pkt_pts,
#else
					decoder->avframe->pts,
#endif
					frame->inherit_super.timebase);
			if(pcmf32 != NULL) {
				decoder->frame = (ttLibC_Frame *)pcmf32;
				if(callback == NULL) {
					return true;
				}
				return callback(ptr, decoder->frame);
			}
		}
		break;
	case AV_SAMPLE_FMT_FLTP:
		{
			if(decoder->frame != NULL && decoder->frame->type != frameType_pcmF32) {
				ttLibC_Frame_close(&decoder->frame);
			}
			ttLibC_PcmF32 *pcmf32 = ttLibC_PcmF32_make(
					(ttLibC_PcmF32 *)decoder->frame,
					PcmF32Type_planar,
					decoder->avframe->sample_rate,
					decoder->avframe->nb_samples,
					decoder->avframe->channels,
					NULL,
					0,
					decoder->avframe->data[0],
					decoder->avframe->nb_samples * 4,
					decoder->avframe->data[1],
					decoder->avframe->nb_samples * 4,
					true,
#ifndef FF_API_PKT_PTS
					decoder->avframe->pkt_pts,
#else
					decoder->avframe->pts,
#endif
					frame->inherit_super.timebase);
			if(pcmf32 != NULL) {
				decoder->frame = (ttLibC_Frame *)pcmf32;
				if(callback == NULL) {
					return true;
				}
				return callback(ptr, decoder->frame);
			}
		}
		break;
	case AV_SAMPLE_FMT_S16:
		{
			if(decoder->frame != NULL && decoder->frame->type != frameType_pcmS16) {
				ttLibC_Frame_close(&decoder->frame);
			}
			ttLibC_PcmS16 *pcms16 = ttLibC_PcmS16_make(
					(ttLibC_PcmS16 *)decoder->frame,
					PcmS16Type_littleEndian,
					decoder->avframe->sample_rate,
					decoder->avframe->nb_samples,
					decoder->avframe->channels,
					NULL,
					0,
					decoder->avframe->data[0],
					decoder->avframe->nb_samples * 2 * decoder->avframe->channels,
					NULL,
					0,
					true,
#ifndef FF_API_PKT_PTS
					decoder->avframe->pkt_pts,
#else
					decoder->avframe->pts,
#endif
					frame->inherit_super.timebase);
			if(pcms16 != NULL) {
				decoder->frame = (ttLibC_Frame *)pcms16;
				if(callback == NULL) {
					return true;
				}
				return callback(ptr, decoder->frame);
			}
		}
		break;
	case AV_SAMPLE_FMT_S16P:
		{
			if(decoder->frame != NULL && decoder->frame->type != frameType_pcmS16) {
				ttLibC_Frame_close(&decoder->frame);
			}
			ttLibC_PcmS16 *pcms16 = ttLibC_PcmS16_make(
					(ttLibC_PcmS16 *)decoder->frame,
					PcmS16Type_littleEndian_planar,
					decoder->avframe->sample_rate,
					decoder->avframe->nb_samples,
					decoder->avframe->channels,
					NULL,
					0,
					decoder->avframe->data[0],
					decoder->avframe->nb_samples * 2,
					decoder->avframe->data[1],
					decoder->avframe->nb_samples * 2,
					true,
#ifndef FF_API_PKT_PTS
					decoder->avframe->pkt_pts,
#else
					decoder->avframe->pts,
#endif
					frame->inherit_super.timebase);
			if(pcms16 != NULL) {
				decoder->frame = (ttLibC_Frame *)pcms16;
				if(callback == NULL) {
					return true;
				}
				return callback(ptr, decoder->frame);
			}
		}
		break;
	default:
		ERR_PRINT("unknown sample format:%d", decoder->dec->sample_fmt);
		return false;
	}
#ifdef TT_FF_OLD_AVCODEC
	} while(true);
#endif
	return false;
}

/*
 * do video decode.
 * @param decoder  decoder object
 * @param frame    target frame
 * @param callback callback func
 * @param ptr      user def pointer.
 */
static bool AvcodecDecoder_decodeVideo(
		ttLibC_AvcodecDecoder_ *decoder,
		ttLibC_Video *frame,
		ttLibC_AvcodecDecodeFunc callback,
		void *ptr) {
	decoder->packet.data = frame->inherit_super.data;
	decoder->packet.size = frame->inherit_super.buffer_size;
	decoder->packet.pts  = frame->inherit_super.pts;
	switch(frame->inherit_super.type) {
	case frameType_h264:
		{
			ttLibC_H264 *h264 = (ttLibC_H264 *)frame;
			switch(h264->type) {
			case H264Type_configData:
				{
					ttLibC_Frame *f = ttLibC_Frame_clone(decoder->h26x_configData, (ttLibC_Frame *)frame);
					if(f == NULL) {
						ERR_PRINT("failed to make cloned frame.");
						return false;
					}
					decoder->h26x_configData = f;
					return true;
				}
				break;
			case H264Type_sliceIDR:
				{
					if(decoder->h26x_configData == NULL) {
						ERR_PRINT("need h264_configData for decode sliceIDR.");
						return false;
					}
					if(decoder->extraDataBuffer == NULL) {
						decoder->extraDataBuffer = ttLibC_DynamicBuffer_make();
					}
					ttLibC_DynamicBuffer_empty(decoder->extraDataBuffer);
					ttLibC_DynamicBuffer_append(decoder->extraDataBuffer, decoder->h26x_configData->data, decoder->h26x_configData->buffer_size);
					ttLibC_DynamicBuffer_append(decoder->extraDataBuffer, frame->inherit_super.data,      frame->inherit_super.buffer_size);
					decoder->packet.data = ttLibC_DynamicBuffer_refData(decoder->extraDataBuffer);
					decoder->packet.size = ttLibC_DynamicBuffer_refSize(decoder->extraDataBuffer);
				}
				break;
			default:
				break;
			}
		}
		break;
	case frameType_theora:
		{
			ttLibC_Theora *theora = (ttLibC_Theora *)frame;
			switch(theora->type) {
			case TheoraType_identificationHeaderDecodeFrame:
				if(decoder->extraDataBuffer == NULL) {
					decoder->extraDataBuffer = ttLibC_DynamicBuffer_make();
				}
				else {
					ttLibC_DynamicBuffer_reset(decoder->extraDataBuffer);
				}
				uint8_t buf[3] = {0x02, frame->inherit_super.buffer_size, 0};
				ttLibC_DynamicBuffer_append(decoder->extraDataBuffer, buf, 3);
				ttLibC_DynamicBuffer_append(decoder->extraDataBuffer, frame->inherit_super.data, frame->inherit_super.buffer_size);
				return true;
			case TheoraType_commentHeaderFrame:
				{
					uint8_t *buf = ttLibC_DynamicBuffer_refData(decoder->extraDataBuffer);
					buf[2] = frame->inherit_super.buffer_size;
					ttLibC_DynamicBuffer_append(decoder->extraDataBuffer, frame->inherit_super.data, frame->inherit_super.buffer_size);
				}
				return true;
			case TheoraType_setupHeaderFrame:
				ttLibC_DynamicBuffer_append(decoder->extraDataBuffer, frame->inherit_super.data, frame->inherit_super.buffer_size);
				decoder->dec->extradata = ttLibC_DynamicBuffer_refData(decoder->extraDataBuffer);
				decoder->dec->extradata_size = ttLibC_DynamicBuffer_refSize(decoder->extraDataBuffer);
				if(!decoder->is_opened) {
					int result = 0;
					if((result = avcodec_open2(decoder->dec, decoder->dec->codec, NULL)) < 0) {
						ERR_PRINT("failed to open codec.:%d", AVERROR(result));
						av_free(decoder->dec);
						ttLibC_free(decoder);
						return NULL;
					}
					decoder->is_opened = true;
				}
				else {
					ERR_PRINT("avcodec is already opened, therefore failed to set private data.");
				}
				return true;
			default:
				break;
			}
		}
		break;
	case frameType_jpeg:
	case frameType_png:
		{
			decoder->packet.flags = AV_PKT_FLAG_KEY;
		}
		break;
	default:
		break;
	}
	int got_picture;
	int result = avcodec_decode_video2(decoder->dec, decoder->avframe, &got_picture, &decoder->packet);
	if(result < 0) {
		ERR_PRINT("failed to decode:%d", result);
		return false;
	}
	if(got_picture != 1) {
		return true;
	}
#ifdef TT_FF_OLD_AVCODEC
	int result = avcodec_send_packet(decoder->dec, &decoder->packet);
	if(result < 0) {
		ERR_PRINT("failed to decode:%d", result);
		return false;
	}
	if(result != 0) {
		return true;
	}
	do {
		result = avcodec_receive_frame(decoder->dec, decoder->avframe);
		if(result == AVERROR(EAGAIN)) {
			return true;
		}
		if(result < 0) {
			ERR_PRINT("failed to receive:%d", result);
			return false;
		}
#endif
	// decode complete now make frame and call callback.
	decoder->inherit_super.width  = decoder->avframe->width;
	decoder->inherit_super.height = decoder->avframe->height;
	switch(decoder->dec->pix_fmt) {
	case AV_PIX_FMT_YUV420P:
		{
			if(decoder->frame != NULL && decoder->frame->type != frameType_yuv420) {
				ttLibC_Frame_close(&decoder->frame);
			}
			ttLibC_Yuv420 *y = ttLibC_Yuv420_make(
					(ttLibC_Yuv420 *)decoder->frame,
					Yuv420Type_planar,
					decoder->avframe->width,
					decoder->avframe->height,
					NULL,
					0,
					decoder->avframe->data[0], decoder->avframe->linesize[0],
					decoder->avframe->data[1], decoder->avframe->linesize[1],
					decoder->avframe->data[2], decoder->avframe->linesize[2],
					true,
#ifndef FF_API_PKT_PTS
					decoder->avframe->pkt_pts,
#else
					decoder->avframe->pts,
#endif
					frame->inherit_super.timebase);
			if(y != NULL) {
				decoder->frame = (ttLibC_Frame *)y;
				if(callback != NULL) {
					return callback(ptr, decoder->frame);
				}
				else {
					return true;
				}
			}
		}
		break;
	case AV_PIX_FMT_YUV422P:
	case AV_PIX_FMT_YUV444P:
		{
			if(decoder->frame != NULL && decoder->frame->type != frameType_yuv420) {
				ttLibC_Frame_close(&decoder->frame);
			}
			ttLibC_Yuv420 *yuv = ttLibC_Yuv420_makeEmptyFrame2(
					(ttLibC_Yuv420 *)decoder->frame,
					Yuv420Type_planar,
					decoder->avframe->width,
					decoder->avframe->height);
			if(yuv == NULL) {
				ERR_PRINT("failed to make output yuv frame.");
				return false;
			}
			decoder->frame = (ttLibC_Frame *)yuv;
			// make yuv.
			uint8_t *dst_y_data = yuv->y_data;
			uint8_t *dst_u_data = yuv->u_data;
			uint8_t *dst_v_data = yuv->v_data;
			uint8_t *src_y_data = decoder->avframe->data[0];
			uint8_t *src_u_data = decoder->avframe->data[1];
			uint8_t *src_v_data = decoder->avframe->data[2];
			uint32_t src_y_stride = decoder->avframe->linesize[0];
			uint32_t src_u_stride = decoder->avframe->linesize[1];
			uint32_t src_v_stride = decoder->avframe->linesize[2];
			uint32_t half_width  = ((decoder->avframe->width + 1) >> 1);
			uint32_t half_height = ((decoder->avframe->height + 1) >> 1);
			switch(decoder->dec->pix_fmt) {
			case AV_PIX_FMT_YUV422P:
				for(int j = 0;j < decoder->avframe->height;++ j) {
					memcpy(dst_y_data, src_y_data, decoder->avframe->width);
					if((j & 0x01) == 0) {
						memcpy(dst_u_data, src_u_data, half_width);
						memcpy(dst_v_data, src_v_data, half_width);
						dst_u_data += yuv->u_stride;
						dst_v_data += yuv->v_stride;
					}
					dst_y_data += yuv->y_stride;
					src_y_data += src_y_stride;
					src_u_data += src_u_stride;
					src_v_data += src_v_stride;
				}
				break;
			case AV_PIX_FMT_YUV444P:
				for(int j = 0;j < decoder->avframe->height;++ j) {
					memcpy(dst_y_data, src_y_data, decoder->avframe->width);
					if((j & 0x01) == 0) {
						uint8_t *ud = dst_u_data;
						uint8_t *vd = dst_v_data;
						for(int i = 0;i < decoder->avframe->width;++ i) {
							if((i & 0x01) == 0x00) {
								*ud = *(src_u_data + i);
								*vd = *(src_v_data + i);
								ud += yuv->u_step;
								vd += yuv->v_step;
							}
						}
						dst_u_data += yuv->u_stride;
						dst_v_data += yuv->v_stride;
					}
					dst_y_data += yuv->y_stride;
					src_y_data += src_y_stride;
					src_u_data += src_u_stride;
					src_v_data += src_v_stride;
				}
				break;
			default:
				ERR_PRINT("un-reachable.");
				return false;
			}
#ifndef FF_API_PKT_PTS
			yuv->inherit_super.inherit_super.pts = decoder->avframe->pkt_pts;
#else
			yuv->inherit_super.inherit_super.pts = decoder->avframe->pts;
#endif
			yuv->inherit_super.inherit_super.timebase = frame->inherit_super.timebase;
			if(callback != NULL) {
				return callback(ptr, decoder->frame);
			}
			else {
				return true;
			}
		}
		break;
	case AV_PIX_FMT_YUV420P10LE:
	case AV_PIX_FMT_YUVJ420P:
	case AV_PIX_FMT_YUVJ422P:
	case AV_PIX_FMT_YUVJ444P:
		{
#ifdef __ENABLE_SWSCALE__
			if(decoder->inFormat != AV_PIX_FMT_NONE
			&& decoder->inFormat != decoder->dec->pix_fmt) {
				ERR_PRINT("format is changed.");
				return false;
			}
			// use swscale to convert into yuv420.
			ttLibC_Yuv420 *yuv = (ttLibC_Yuv420 *)ttLibC_Yuv420_makeEmptyFrame2(
					(ttLibC_Yuv420 *)decoder->frame,
					Yuv420Type_planar,
					decoder->avframe->width,
					decoder->avframe->height);
			if(yuv == NULL) {
				ERR_PRINT("failed to make empty yuv frame for output.");
				return false;
			}
			decoder->frame = (ttLibC_Frame *)yuv;
			if(decoder->convertCtx == NULL) {
				decoder->convertCtx = sws_getContext(
						decoder->avframe->width,
						decoder->avframe->height,
						decoder->dec->pix_fmt,
						decoder->avframe->width,
						decoder->avframe->height,
						AV_PIX_FMT_YUV420P,
						SWS_FAST_BILINEAR,
						NULL,
						NULL,
						NULL);
				if(decoder->convertCtx == NULL) {
					ERR_PRINT("failed to make swscale convertContext. pixfmt.%d", decoder->dec->pix_fmt);
					return false;
				}
				decoder->inFormat = decoder->dec->pix_fmt;
			}
			uint8_t *dst_data[4];
			uint32_t dst_stride[4];
			dst_data[0] = yuv->y_data;
			dst_data[1] = yuv->u_data;
			dst_data[2] = yuv->v_data;
			dst_data[3] = NULL;
			dst_stride[0] = yuv->y_stride;
			dst_stride[1] = yuv->u_stride;
			dst_stride[2] = yuv->v_stride;
			dst_stride[3] = 0;
			// try to convert.
			sws_scale(
					decoder->convertCtx,
					(const uint8_t *const *)decoder->avframe->data,
					(const int *)decoder->avframe->linesize,
					0,
					decoder->avframe->height,
					(uint8_t *const *)dst_data,
					(const int *)dst_stride);
			// update timestamp.
#ifndef FF_API_PKT_PTS
			decoder->frame->pts = decoder->avframe->pkt_pts,
#else
			decoder->frame->pts = decoder->avframe->pts,
#endif
			decoder->frame->timebase = frame->inherit_super.timebase;
			// done.
			if(callback != NULL) {
				return callback(ptr, decoder->frame);
			}
			else {
				return true;
			}
#else
			ERR_PRINT("unknown pixfmt output.%d", decoder->dec->pix_fmt);
			return false;
#endif
		}
		break;
	case AV_PIX_FMT_RGB24: // rgbrgb...
	case AV_PIX_FMT_BGR24: // bgrbgr...
	case AV_PIX_FMT_ABGR: // abgrabgr...
	case AV_PIX_FMT_BGRA: // bgrabgra...
	case AV_PIX_FMT_ARGB: // argbargb...
	case AV_PIX_FMT_RGBA: // rgbargba...
		{
			if(decoder->frame != NULL && decoder->frame->type != frameType_bgr) {
				ttLibC_Frame_close(&decoder->frame);
			}
			ttLibC_Bgr_Type type = BgrType_bgr;
			switch(decoder->dec->pix_fmt) {
			case AV_PIX_FMT_RGB24: // rgbrgb...
				type = BgrType_rgb;
				break;
			case AV_PIX_FMT_BGR24: // bgrbgr...
				type = BgrType_bgr;
				break;
			case AV_PIX_FMT_ABGR: // abgrabgr...
				type = BgrType_abgr;
				break;
			case AV_PIX_FMT_BGRA: // bgrabgra...
				type = BgrType_bgra;
				break;
			case AV_PIX_FMT_ARGB: // argbargb...
				type = BgrType_argb;
				break;
			case AV_PIX_FMT_RGBA: // rgbargba...
				type = BgrType_rgba;
				break;
			default:
				break;
			}
			ttLibC_Bgr *b = ttLibC_Bgr_make(
					(ttLibC_Bgr *)decoder->frame,
					type,
					decoder->avframe->width,
					decoder->avframe->height,
					decoder->avframe->linesize[0],
					decoder->avframe->data[0],
					decoder->avframe->linesize[0] * decoder->avframe->height,
					true,
#ifndef FF_API_PKT_PTS
					decoder->avframe->pkt_pts,
#else
					decoder->avframe->pts,
#endif
					frame->inherit_super.timebase);
			if(b != NULL) {
				decoder->frame = (ttLibC_Frame *)b;
				if(callback != NULL) {
					return callback(ptr, decoder->frame);
				}
				else {
					return true;
				}
			}
		}
		break;
	case AV_PIX_FMT_PAL8:
		{
			ttLibC_Bgr *bgr = ttLibC_Bgr_makeEmptyFrame2(
					(ttLibC_Bgr *)decoder->frame,
					BgrType_rgba,
					decoder->avframe->width,
					decoder->avframe->height);
			if(bgr == NULL) {
				ERR_PRINT("failed to make bgr frame.");
				return false;
			}
			decoder->frame = (ttLibC_Frame *)bgr;
#ifndef FF_API_PKT_PTS
			decoder->frame->pts = decoder->avframe->pkt_pts,
#else
			decoder->frame->pts = decoder->avframe->pts,
#endif
			decoder->frame->timebase = frame->inherit_super.timebase;

			uint32_t *palette = (uint32_t *)decoder->avframe->data[1];
			uint8_t *src = decoder->avframe->data[0];
			uint8_t *dst = bgr->data;
			for(int i = 0;i < decoder->avframe->height; ++ i) {
				uint8_t *b = src;
				uint8_t *d = dst;

				for(int j = 0;j < decoder->avframe->width;++ j) {
					uint32_t color = palette[*b];
					*d     = (color >> 16) & 0xFF;
					*(d+1) = (color >> 8) & 0xFF;
					*(d+2) = (color) & 0xFF;
					*(d+3) = (color >> 24) & 0xFF;
					d += bgr->unit_size;
					++ b;
				}
				src += decoder->avframe->linesize[0];
				dst += bgr->width_stride;
			}
			if(callback != NULL) {
				return callback(ptr, decoder->frame);
			}
			else {
				return true;
			}
		}
		break;
	case AV_PIX_FMT_NV12:
	case AV_PIX_FMT_NV21:
		ERR_PRINT("not make yet.%d %d", decoder->dec->pix_fmt, AV_PIX_FMT_RGB24);
		return false;
	default:
		ERR_PRINT("unknown pixfmt output.%d", decoder->dec->pix_fmt);
		return false;
	}
#ifdef TT_FF_OLD_AVCODEC
	}while(true);
#endif
	return false;
}

/*
 * getAVCodecContext for target frameType.
 * @param frame_type target ttLibC_Frame_Type
 */
void TT_ATTRIBUTE_API *ttLibC_AvcodecDecoder_getAVCodecContext(ttLibC_Frame_Type frame_type) {
	avcodec_register_all();
	AVCodec *codec = NULL;
	switch(frame_type) {
	case frameType_aac:
		codec = avcodec_find_decoder(AV_CODEC_ID_AAC);
		break;
	case frameType_adpcm_ima_wav:
		codec = avcodec_find_decoder(AV_CODEC_ID_ADPCM_IMA_WAV);
		break;
	case frameType_flv1:
		codec = avcodec_find_decoder(AV_CODEC_ID_FLV1);
		break;
	case frameType_h264:
		codec = avcodec_find_decoder(AV_CODEC_ID_H264);
		break;
#if defined(AV_CODEC_ID_HEVC) || defined(AV_CODEC_ID_H265)
	case frameType_h265:
		codec = avcodec_find_decoder(AV_CODEC_ID_HEVC);
		break;
#endif
	case frameType_jpeg:
		codec = avcodec_find_decoder(AV_CODEC_ID_MJPEG);
		break;
	case frameType_mp3:
		codec = avcodec_find_decoder(AV_CODEC_ID_MP3);
		break;
	case frameType_nellymoser:
		codec = avcodec_find_decoder(AV_CODEC_ID_NELLYMOSER);
		break;
	case frameType_png:
		codec = avcodec_find_decoder(AV_CODEC_ID_PNG);
		break;
	case frameType_opus:
		codec = avcodec_find_decoder(AV_CODEC_ID_OPUS);
		break;
	case frameType_pcm_alaw:
		codec = avcodec_find_decoder(AV_CODEC_ID_PCM_ALAW);
		break;
	case frameType_pcm_mulaw:
		codec = avcodec_find_decoder(AV_CODEC_ID_PCM_MULAW);
		break;
	case frameType_speex:
		codec = avcodec_find_decoder(AV_CODEC_ID_SPEEX);
		break;
	case frameType_theora:
		codec = avcodec_find_decoder(AV_CODEC_ID_THEORA);
		break;
	case frameType_vorbis:
		codec = avcodec_find_decoder(AV_CODEC_ID_VORBIS);
		break;
	case frameType_vp6:
		codec = avcodec_find_decoder(AV_CODEC_ID_VP6F);
		break;
	case frameType_vp8:
		codec = avcodec_find_decoder(AV_CODEC_ID_VP8);
		break;
	case frameType_vp9:
//		codec = avcodec_find_decoder(AV_CODEC_ID_VP9);
		codec = avcodec_find_decoder_by_name("libvpx-vp9");
		break;
	case frameType_wmv1:
		codec = avcodec_find_decoder(AV_CODEC_ID_WMV1);
		break;
	case frameType_wmv2:
		codec = avcodec_find_decoder(AV_CODEC_ID_WMV2);
		break;
	default:
		ERR_PRINT("unsupported frame type:%d", frame_type);
		return NULL;
	}
	if(codec == NULL) {
		ERR_PRINT("failed to get codec object for frame_type:%d", frame_type);
		return NULL;
	}
	AVCodecContext *dec = avcodec_alloc_context3(codec);
	if(dec == NULL) {
		ERR_PRINT("failed to alloc decode context.");
		return NULL;
	}
	return dec;
}

/*
 * make avcodecDecoder with AVCodecContext
 * @param dec_context target AVCodecContext
 */
ttLibC_AvcodecDecoder TT_ATTRIBUTE_API *ttLibC_AvcodecDecoder_makeWithAVCodecContext(void *dec_context) {
	if(dec_context == NULL) {
		return NULL;
	}
	AVCodecContext *dec = (AVCodecContext *)dec_context;
	ttLibC_Frame_Type frame_type = frameType_aac;
	switch(dec->codec_id) {
	case AV_CODEC_ID_AAC:
		frame_type = frameType_aac;
		break;
	case AV_CODEC_ID_ADPCM_IMA_WAV:
		frame_type = frameType_adpcm_ima_wav;
		if(dec->bits_per_coded_sample == 0) {
			// force 4bit for default.
			dec->bits_per_coded_sample = 4;
		}
		break;
	case AV_CODEC_ID_FLV1:
		frame_type = frameType_flv1;
		break;
	case AV_CODEC_ID_H264:
		frame_type = frameType_h264;
		break;
#if defined(AV_CODEC_ID_HEVC) || defined(AV_CODEC_ID_H265)
	case AV_CODEC_ID_HEVC:
		frame_type = frameType_h265;
		break;
#endif
	case AV_CODEC_ID_MJPEG:
		frame_type = frameType_jpeg;
		break;
	case AV_CODEC_ID_MP3:
		frame_type = frameType_mp3;
		break;
	case AV_CODEC_ID_NELLYMOSER:
		frame_type = frameType_nellymoser;
		break;
	case AV_CODEC_ID_OPUS:
		frame_type = frameType_opus;
		break;
	case AV_CODEC_ID_PCM_ALAW:
		frame_type = frameType_pcm_alaw;
		break;
	case AV_CODEC_ID_PCM_MULAW:
		frame_type = frameType_pcm_mulaw;
		break;
	case AV_CODEC_ID_PNG:
		frame_type = frameType_png;
		break;
	case AV_CODEC_ID_SPEEX:
		frame_type = frameType_speex;
		break;
	case AV_CODEC_ID_THEORA:
		frame_type = frameType_theora;
		break;
	case AV_CODEC_ID_VORBIS:
		frame_type = frameType_vorbis;
		break;
	case AV_CODEC_ID_VP6:
	case AV_CODEC_ID_VP6A:
	case AV_CODEC_ID_VP6F:
		frame_type = frameType_vp6;
		break;
	case AV_CODEC_ID_VP8:
		frame_type = frameType_vp8;
		break;
	case AV_CODEC_ID_VP9:
		frame_type = frameType_vp9;
		break;
	case AV_CODEC_ID_WMV1:
		frame_type = frameType_wmv1;
		break;
	case AV_CODEC_ID_WMV2:
		frame_type = frameType_wmv2;
		break;
	default:
		ERR_PRINT("unsupported frame type:%d", frame_type);
		av_free(dec);
		return NULL;
	}
	ttLibC_AvcodecDecoder_ *decoder = ttLibC_malloc(sizeof(ttLibC_AvcodecDecoder_));
	if(decoder == NULL) {
		ERR_PRINT("failed to allocate decoder object.");
		av_free(dec);
		return NULL;
	}
#ifdef __ENABLE_SWSCALE__
	decoder->convertCtx = NULL;
	decoder->inFormat = AV_PIX_FMT_NONE;
#endif
	decoder->dec = dec;
	decoder->is_opened = false;
	if(frame_type != frameType_theora
	&& frame_type != frameType_vorbis
	&& frame_type != frameType_speex) {
		int result = 0;
		if((result = avcodec_open2(decoder->dec, decoder->dec->codec, NULL)) < 0) {
			ERR_PRINT("failed to open codec.:%d", AVERROR(result));
			av_free(decoder->dec);
			ttLibC_free(decoder);
			return NULL;
		}
		decoder->is_opened = true;
	}
	decoder->avframe = av_frame_alloc();
	if(decoder->avframe == NULL) {
		ERR_PRINT("failed to alloc avframe.");
		avcodec_close(decoder->dec);
		av_free(decoder->dec);
		ttLibC_free(decoder);
		return NULL;
	}
	decoder->inherit_super.frame_type = frame_type;
	switch(dec->codec->type) {
	case AVMEDIA_TYPE_AUDIO:
		switch(dec->sample_fmt) {
		case AV_SAMPLE_FMT_FLTP:
		case AV_SAMPLE_FMT_S16:
		case AV_SAMPLE_FMT_S16P:
		case AV_SAMPLE_FMT_FLT:
			break;
		case AV_SAMPLE_FMT_NONE:
			// happen in vorbis decode.
			if(frame_type != frameType_vorbis
			&& frame_type != frameType_speex) {
				LOG_PRINT("sampleFormat is unknown now. maybe decide later?");
			}
			break;
		default:
			ERR_PRINT("unsupport sample_fmt type:%d", dec->sample_fmt);
			av_free(decoder->avframe);
			avcodec_close(decoder->dec);
			av_free(decoder->dec);
			ttLibC_free(decoder);
			return NULL;
		}
		decoder->inherit_super.sample_rate = 0;
		decoder->inherit_super.channel_num = 0;
		break;
	case AVMEDIA_TYPE_VIDEO:
/*		switch(dec->pix_fmt) {
		case AV_PIX_FMT_YUV420P:
			break;
		case AV_PIX_FMT_NV12:
		case AV_PIX_FMT_NV21:
		case AV_PIX_FMT_ABGR:
		case AV_PIX_FMT_BGRA:
		case AV_PIX_FMT_BGR24:
			LOG_PRINT("target picture data is not implemented. do later.");
			break;
		default:
			ERR_PRINT("unsupport pix_fmt type:%d", dec->pix_fmt);
			av_free(decoder->avframe);
			avcodec_close(decoder->dec);
			av_free(decoder->dec);
			ttLibC_free(decoder);
			return NULL;
		}*/
		decoder->inherit_super.width  = 0;
		decoder->inherit_super.height = 0;
		break;
	default:
		ERR_PRINT("unknown media type:%d", dec->codec->type);
		av_free(decoder->avframe);
		avcodec_close(decoder->dec);
		av_free(decoder->dec);
		ttLibC_free(decoder);
		return NULL;
	}
	av_init_packet(&decoder->packet);
	decoder->frame = NULL;
	decoder->h26x_configData = NULL;
	decoder->extraDataBuffer = NULL;
	return (ttLibC_AvcodecDecoder *)decoder;
}

/*
 * make audio decoder
 * @param frame_type  target ttLibC_Frame_Type
 * @param sample_rate target sample_rate
 * @param channel_num target channel_num
 */
ttLibC_AvcodecDecoder TT_ATTRIBUTE_API *ttLibC_AvcodecAudioDecoder_make(
		ttLibC_Frame_Type frame_type,
		uint32_t sample_rate,
		uint32_t channel_num) {
	return ttLibC_AvcodecAudioDecoder_make_ex(
			frame_type,
			sample_rate,
			channel_num,
			NULL,
			0);
}

/*
 * make audio decoder
 * @param frame_type     target ttLibC_Frame_Type
 * @param sample_rate    target sample_rate
 * @param channel_num    target channel_num
 * @param extradata      extradata(some codec require these value, like vorbis)
 * @param extradata_size extradata_size
 */
ttLibC_AvcodecDecoder TT_ATTRIBUTE_API *ttLibC_AvcodecAudioDecoder_make_ex(
		ttLibC_Frame_Type frame_type,
		uint32_t sample_rate,
		uint32_t channel_num,
		void *extradata,
		size_t extradata_size) {
	AVCodecContext *dec = (AVCodecContext *)ttLibC_AvcodecDecoder_getAVCodecContext(frame_type);
	if(dec == NULL) {
		return NULL;
	}
	dec->sample_rate = sample_rate;
	dec->channels = channel_num;
	dec->channel_layout = av_get_default_channel_layout(channel_num);
	dec->extradata = extradata;
	dec->extradata_size = (int)extradata_size;
	return ttLibC_AvcodecDecoder_makeWithAVCodecContext(dec);
}

/*
 * make video decoder
 * @param frame_type target ttLibC_Frame_Type
 * @param width      target width
 * @param height     target height
 */
ttLibC_AvcodecDecoder TT_ATTRIBUTE_API *ttLibC_AvcodecVideoDecoder_make(
		ttLibC_Frame_Type frame_type,
		uint32_t width,
		uint32_t height) {
	return ttLibC_AvcodecVideoDecoder_make_ex(
			frame_type,
			width,
			height,
			NULL,
			0);
}

/*
 * make video decoder
 * @param frame_type     target ttLibC_Frame_Type
 * @param width          target width
 * @param height         target height
 * @param extradata      extradata(some codec require these value.)
 * @param extradata_size extradata_size
 */
ttLibC_AvcodecDecoder TT_ATTRIBUTE_API *ttLibC_AvcodecVideoDecoder_make_ex(
		ttLibC_Frame_Type frame_type,
		uint32_t width,
		uint32_t height,
		void *extradata,
		size_t extradata_size) {
	AVCodecContext *dec = (AVCodecContext *)ttLibC_AvcodecDecoder_getAVCodecContext(frame_type);
	if(dec == NULL) {
		return NULL;
	}
	dec->width = width;
	dec->height = height;
	dec->extradata = extradata;
	dec->extradata_size = (int)extradata_size;
	return ttLibC_AvcodecDecoder_makeWithAVCodecContext(dec);
}

/*
 * make decoder
 * @param frame_type target ttLibC_Frame_Type
 */
ttLibC_AvcodecDecoder TT_ATTRIBUTE_API *ttLibC_AvcodecDecoder_make(
		ttLibC_Frame_Type frame_type) {
	AVCodecContext *dec = (AVCodecContext *)ttLibC_AvcodecDecoder_getAVCodecContext(frame_type);
	if(dec == NULL) {
		return NULL;
	}
	return ttLibC_AvcodecDecoder_makeWithAVCodecContext(dec);
}

/*
 * decode frame
 * @param decoder  avcodec decoder
 * @param frame    source frame. frame_type should be the same as decoder->frame_type
 * @param callback callback func for avcodec decode.
 * @param ptr      pointer for user def value, which call in callback.
 */
bool TT_ATTRIBUTE_API ttLibC_AvcodecDecoder_decode(
		ttLibC_AvcodecDecoder *decoder,
		ttLibC_Frame *frame,
		ttLibC_AvcodecDecodeFunc callback,
		void *ptr) {
	if(decoder == NULL) {
		return false;
	}
	if(frame == NULL) {
		return true;
	}
	ttLibC_AvcodecDecoder_ *decoder_ = (ttLibC_AvcodecDecoder_ *)decoder;
	if(frame->type != decoder_->inherit_super.frame_type) {
		ERR_PRINT("input frame is different from target frame.:%d %d",
				frame->type, decoder_->inherit_super.frame_type);
		return false;
	}
	if(decoder_->avframe == NULL) {
		ERR_PRINT("avframe is not initialized yet.");
		return false;
	}
	switch(decoder_->dec->codec->type) {
	case AVMEDIA_TYPE_AUDIO:
		return AvcodecDecoder_decodeAudio(decoder_, (ttLibC_Audio *)frame, callback, ptr);
	case AVMEDIA_TYPE_VIDEO:
		return AvcodecDecoder_decodeVideo(decoder_, (ttLibC_Video *)frame, callback, ptr);
	default:
		ERR_PRINT("unknown media type for codec:%d", decoder_->dec->codec->type);
		return false;
	}
}

/*
 * close avcodec decoder.
 * @param decoder.
 */
void TT_ATTRIBUTE_API ttLibC_AvcodecDecoder_close(ttLibC_AvcodecDecoder **decoder) {
	ttLibC_AvcodecDecoder_ *target = (ttLibC_AvcodecDecoder_*)*decoder;
	if(target == NULL) {
		return;
	}
	if(target->avframe != NULL) {
		av_free(target->avframe);
	}
#ifndef FF_API_AVPACKET_OLD_API
	av_free_packet(&target->packet);
#else
	av_packet_unref(&target->packet);
#endif
	if(target->dec != NULL) {
		avcodec_close(target->dec);
		av_free(target->dec);
	}
#ifdef __ENABLE_SWSCALE__
	if(target->convertCtx != NULL) {
		sws_freeContext(target->convertCtx);
		target->convertCtx = NULL;
	}
#endif
	ttLibC_DynamicBuffer_close(&target->extraDataBuffer);
	ttLibC_Frame_close(&target->frame);
	ttLibC_Frame_close(&target->h26x_configData);
	ttLibC_free(target);
	*decoder = NULL;
}

#endif

