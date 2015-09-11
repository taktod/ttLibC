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

#include "../frame/video/bgr.h"
#include "../frame/video/yuv420.h"

#include "../frame/audio/pcms16.h"
#include "../frame/audio/pcmf32.h"

/*
 * avcodecDecoder detail definition
 */
typedef struct {
	ttLibC_AvcodecDecoder inherit_super;
	AVCodecContext *dec;
	AVFrame *avframe;

	AVPacket packet;
	ttLibC_Frame *frame;
} ttLibC_Decoder_AvcodecDecoder_;

typedef ttLibC_Decoder_AvcodecDecoder_ ttLibC_AvcodecDecoder_;

/*
 * do audio decode.
 * @param decoder  decoder object
 * @param frame    target frame
 * @param callback callback func
 * @param ptr      user def pointer
 */
static void AvcodecDecoder_decodeAudio(
		ttLibC_AvcodecDecoder_ *decoder,
		ttLibC_Audio *frame,
		ttLibC_AvcodecDecodeFunc callback,
		void *ptr) {
	decoder->packet.data = frame->inherit_super.data;
	decoder->packet.size = frame->inherit_super.buffer_size;
	decoder->packet.pts = frame->inherit_super.pts;
	int got_frame;
	int result = avcodec_decode_audio4(decoder->dec, decoder->avframe, &got_frame, &decoder->packet);
	if(result < 0) {
		ERR_PRINT("failed to decode:%d", result);
		return;
	}
	if(got_frame != 1) {
		return;
	}
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
					decoder->avframe->nb_samples * 4,
					decoder->avframe->data[1],
					decoder->avframe->nb_samples * 4,
					true,
					frame->inherit_super.pts,
					frame->inherit_super.timebase);
			if(pcmf32 != NULL) {
				decoder->frame = (ttLibC_Frame *)pcmf32;
				callback(ptr, decoder->frame);
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
					frame->inherit_super.pts,
					frame->inherit_super.timebase);
			if(pcmf32 != NULL) {
				decoder->frame = (ttLibC_Frame *)pcmf32;
				callback(ptr, decoder->frame);
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
					decoder->avframe->nb_samples * 2,
					decoder->avframe->data[1],
					decoder->avframe->nb_samples * 2,
					true,
					frame->inherit_super.pts,
					frame->inherit_super.timebase);
			if(pcms16 != NULL) {
				decoder->frame = (ttLibC_Frame *)pcms16;
				callback(ptr, decoder->frame);
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
					frame->inherit_super.pts,
					frame->inherit_super.timebase);
			if(pcms16 != NULL) {
				decoder->frame = (ttLibC_Frame *)pcms16;
				callback(ptr, decoder->frame);
			}
		}
		break;
	default:
		ERR_PRINT("unknown sample format:%d", decoder->dec->sample_fmt);
		return;
	}
}

/*
 * do video decode.
 * @param decoder  decoder object
 * @param frame    target frame
 * @param callback callback func
 * @param ptr      user def pointer.
 */
static void AvcodecDecoder_decodeVideo(
		ttLibC_AvcodecDecoder_ *decoder,
		ttLibC_Video *frame,
		ttLibC_AvcodecDecodeFunc callback,
		void *ptr) {
	decoder->packet.data = frame->inherit_super.data;
	decoder->packet.size = frame->inherit_super.buffer_size;
	decoder->packet.pts  = frame->inherit_super.pts;
	int got_picture;
	int result = avcodec_decode_video2(decoder->dec, decoder->avframe, &got_picture, &decoder->packet);
	if(result < 0) {
		ERR_PRINT("failed to decode:%d", result);
		return;
	}
	if(got_picture != 1) {
		return;
	}
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
					decoder->avframe->pts,
					frame->inherit_super.timebase);
			if(y != NULL) {
				decoder->frame = (ttLibC_Frame *)y;
				callback(ptr, decoder->frame);
			}
		}
		break;
	case AV_PIX_FMT_NV12:
	case AV_PIX_FMT_NV21:
	case AV_PIX_FMT_BGR24:
	case AV_PIX_FMT_ABGR:
	case AV_PIX_FMT_BGRA:
		ERR_PRINT("not make yet.");
		return;
	default:
		ERR_PRINT("unknown pixfmt output.");
		return;
	}
}

/*
 * getAVCodecContext for target frameType.
 * @param frame_type target ttLibC_Frame_Type
 */
void *ttLibC_AvcodecDecoder_getAVCodecContext(ttLibC_Frame_Type frame_type) {
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
	case frameType_h265:
		codec = avcodec_find_decoder(AV_CODEC_ID_HEVC);
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
		codec = avcodec_find_decoder(AV_CODEC_ID_VP6);
		break;
	case frameType_vp8:
		codec = avcodec_find_decoder(AV_CODEC_ID_VP8);
		break;
	case frameType_vp9:
		codec = avcodec_find_decoder(AV_CODEC_ID_VP9);
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
	case AV_CODEC_ID_HEVC:
		frame_type = frameType_h265;
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
	int result = 0;
	if((result = avcodec_open2(decoder->dec, decoder->dec->codec, NULL)) < 0) {
		ERR_PRINT("failed to open codec.:%d", AVERROR(result));
		av_free(decoder->dec);
		ttLibC_free(decoder);
		return NULL;
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
		default:
			ERR_PRINT("unsupport sample_fmt type:%d", dec->pix_fmt);
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
		switch(dec->pix_fmt) {
		case AV_PIX_FMT_YUV420P:
			break;
		case AV_PIX_FMT_NV12:
		case AV_PIX_FMT_NV21:
		case AV_PIX_FMT_ABGR:
		case AV_PIX_FMT_BGRA:
		case AV_PIX_FMT_BGR24:
			LOG_PRINT("該当ピクチャフォーマットは動作未確認です。");
			break;
		default:
			ERR_PRINT("unsupport pix_fmt type:%d", dec->pix_fmt);
			av_free(decoder->avframe);
			avcodec_close(decoder->dec);
			av_free(decoder->dec);
			ttLibC_free(decoder);
			return NULL;
		}
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
void ttLibC_AvcodecDecoder_decode(
		ttLibC_AvcodecDecoder *decoder,
		ttLibC_Frame *frame,
		ttLibC_AvcodecDecodeFunc callback,
		void *ptr) {
	if(decoder == NULL) {
		return;
	}
	if(frame == NULL) {
		return;
	}
	ttLibC_AvcodecDecoder_ *decoder_ = (ttLibC_AvcodecDecoder_ *)decoder;
	if(frame->type != decoder_->inherit_super.frame_type) {
		ERR_PRINT("input frame is different from target frame.:%d %d",
				frame->type, decoder_->inherit_super.frame_type);
		return;
	}
	if(decoder_->avframe == NULL) {
		ERR_PRINT("avframe is not initialized yet.");
		return;
	}
	switch(decoder_->dec->codec->type) {
	case AVMEDIA_TYPE_AUDIO:
		AvcodecDecoder_decodeAudio(decoder_, (ttLibC_Audio *)frame, callback, ptr);
		return;
	case AVMEDIA_TYPE_VIDEO:
		AvcodecDecoder_decodeVideo(decoder_, (ttLibC_Video *)frame, callback, ptr);
		return;
	default:
		ERR_PRINT("unknown media type for codec:%d", decoder_->dec->codec->type);
		return;
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
	av_free_packet(&target->packet);
	if(target->dec != NULL) {
		avcodec_close(target->dec);
		av_free(target->dec);
	}
	ttLibC_Frame_close(&target->frame);
	ttLibC_free(target);
	*decoder = NULL;
}

#endif

