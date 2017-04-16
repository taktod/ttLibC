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
#include "../log.h"
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
//					decoder->avframe->nb_samples * 4,
					decoder->avframe->nb_samples * 4 * decoder->avframe->channels,
//					decoder->avframe->data[1],
//					decoder->avframe->nb_samples * 4,
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
//					decoder->avframe->nb_samples * 2,
					decoder->avframe->nb_samples * 2 * decoder->avframe->channels,
//					decoder->avframe->data[1],
//					decoder->avframe->nb_samples * 2,
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
	case AV_PIX_FMT_YUVJ420P:
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
	case AV_PIX_FMT_YUVJ422P:
	case AV_PIX_FMT_YUV444P:
	case AV_PIX_FMT_YUVJ444P:
		{
			if(decoder->frame != NULL && decoder->frame->type != frameType_yuv420) {
				ttLibC_Frame_close(&decoder->frame);
			}
			uint32_t width  = decoder->avframe->width;
			uint32_t height = decoder->avframe->height;
			uint32_t wh = ((width * height) >> 1);
			uint8_t *data = NULL;
			size_t data_size = wh;
			wh = (wh >> 1);
			bool is_alloc_flag = false;
			if(decoder->frame != NULL && !decoder->frame->is_non_copy) {
				if(decoder->frame->data_size < data_size) {
					// need to realloc
					ttLibC_free(decoder->frame->data);
					decoder->frame->data = NULL;
				}
				else {
					// can reuse prev data.
					data = decoder->frame->data;
					data_size = decoder->frame->data_size;
				}
			}
			if(data == NULL) {
				data = ttLibC_malloc(data_size);
				if(data == NULL) {
					ERR_PRINT("failed to alloc yuv buffer.");
					return false;
				}
				is_alloc_flag = true;
			}
			// make uv.
			uint8_t *u_data = data;
			uint8_t *v_data = data + wh;
			uint32_t u_stride = (width >> 1);
			uint32_t v_stride = (width >> 1);
			uint8_t *src_u_data = decoder->avframe->data[1];
			uint8_t *src_v_data = decoder->avframe->data[2];
			uint32_t src_u_stride = decoder->avframe->linesize[1];
			uint32_t src_v_stride = decoder->avframe->linesize[2];
			switch(decoder->dec->pix_fmt) {
			case AV_PIX_FMT_YUV422P:
			case AV_PIX_FMT_YUVJ422P:
				for(uint32_t j = 0;j < height;++ j) {
					if((j & 0x01) == 1) {
						memcpy(u_data, src_u_data, u_stride);
						memcpy(v_data, src_v_data, v_stride);
						u_data += u_stride;
						v_data += v_stride;
					}
					src_u_data += src_u_stride;
					src_v_data += src_v_stride;
				}
				break;
			case AV_PIX_FMT_YUV444P:
			case AV_PIX_FMT_YUVJ444P:
				for(uint32_t j = 0;j < height;++ j) {
					if((j & 0x01) == 1) {
						for(uint32_t i = 0;i < width;++ i) {
							if((i & 0x01) == 0x00) {
								*u_data = *(src_u_data + i);
								*v_data = *(src_v_data + i);
								++ u_data;
								++ v_data;
							}
						}
					}
					src_u_data += src_u_stride;
					src_v_data += src_v_stride;
				}
				break;
			default:
				ERR_PRINT("un-reachable.");
				return false;
			}
			if(decoder->frame != NULL) {
				decoder->frame->is_non_copy = true;
			}
			u_data = data;
			v_data = data + wh;
			ttLibC_Yuv420 *y = ttLibC_Yuv420_make(
					(ttLibC_Yuv420 *)decoder->frame,
					Yuv420Type_planar,
					width,
					height,
					data,
					data_size,
					decoder->avframe->data[0],
					decoder->avframe->linesize[0],
					u_data,
					u_stride,
					v_data,
					v_stride,
					true,
#ifndef FF_API_PKT_PTS
					decoder->avframe->pkt_pts,
#else
					decoder->avframe->pts,
#endif
					frame->inherit_super.timebase);
			if(y == NULL) {
				if(is_alloc_flag) {
					ttLibC_free(data);
				}
				ERR_PRINT("failed to make yuv420 frame object.");
				return false;
			}
			decoder->frame = (ttLibC_Frame *)y;
			decoder->frame->is_non_copy = false;
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
	case AV_PIX_FMT_BGR24:
	case AV_PIX_FMT_ABGR:
	case AV_PIX_FMT_BGRA:
		ERR_PRINT("not make yet.");
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
void *ttLibC_AvcodecDecoder_getAVCodecContext(ttLibC_Frame_Type frame_type) {
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
ttLibC_AvcodecDecoder *ttLibC_AvcodecDecoder_makeWithAVCodecContext(void *dec_context) {
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
ttLibC_AvcodecDecoder *ttLibC_AvcodecAudioDecoder_make(
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
ttLibC_AvcodecDecoder *ttLibC_AvcodecAudioDecoder_make_ex(
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
ttLibC_AvcodecDecoder *ttLibC_AvcodecVideoDecoder_make(
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
ttLibC_AvcodecDecoder *ttLibC_AvcodecVideoDecoder_make_ex(
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
ttLibC_AvcodecDecoder *ttLibC_AvcodecDecoder_make(
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
bool ttLibC_AvcodecDecoder_decode(
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
void ttLibC_AvcodecDecoder_close(ttLibC_AvcodecDecoder **decoder) {
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
	ttLibC_DynamicBuffer_close(&target->extraDataBuffer);
	ttLibC_Frame_close(&target->frame);
	ttLibC_Frame_close(&target->h26x_configData);
	ttLibC_free(target);
	*decoder = NULL;
}

#endif

