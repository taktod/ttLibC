/*
 * @file   avcodecEncoder.c
 * @brief  encode frame with libavcodec
 *
 * this code is under LGPLv3 license.
 *
 * @author taktod
 * @date   2015/08/02
 */

#ifdef __ENABLE_AVCODEC__

#include "avcodecEncoder.h"

#include <libavcodec/avcodec.h>
#include "../log.h"
#include "../allocator.h"
#include "../util/hexUtil.h"
#include "../frame/video/yuv420.h"
#include "../frame/video/bgr.h"
#include "../frame/audio/pcms16.h"
#include "../frame/audio/pcmf32.h"

#include "../frame/audio/aac.h"
#include "../frame/audio/adpcmImaWav.h"
#include "../frame/audio/mp3.h"
#include "../frame/audio/nellymoser.h"
#include "../frame/audio/opus.h"
#include "../frame/audio/pcmAlaw.h"
#include "../frame/audio/pcmMulaw.h"
#include "../frame/audio/speex.h"
#include "../frame/audio/vorbis.h"

#include "../frame/video/flv1.h"
#include "../frame/video/h264.h"
#include "../frame/video/h265.h"
#include "../frame/video/theora.h"
#include "../frame/video/vp6.h"
#include "../frame/video/vp8.h"
#include "../frame/video/vp9.h"
#include "../frame/video/wmv1.h"
#include "../frame/video/wmv2.h"

/*
 * avcodecEncoder detail definition
 */
typedef struct {
	ttLibC_AvcodecEncoder inherit_super;
	AVCodecContext *enc;
	AVFrame *avframe;

	uint8_t *pcm_buffer;
	size_t pcm_buffer_next_pos;
	uint64_t passed_sample_num;
	uint32_t frame_size;

	AVPacket packet;
	ttLibC_H264 *configData; // for h264 configData.
	ttLibC_Frame *frame; // reuse frame for output.
} ttLibC_Encoder_AvcodecEncoder_;

typedef ttLibC_Encoder_AvcodecEncoder_ ttLibC_AvcodecEncoder_;

/*
 * do audio encode.
 * @param encoder     encoder object
 * @param l_data      left channel pcm
 * @param l_data_size left channel size
 * @param r_data      right channel pcm (planar only)
 * @param r_data_size right channel size (planar only)
 * @param sample_num  sample_num
 * @param callback    callback func
 * @param ptr         user def pointer
 */
static bool AvcodecEncoder_encode_AudioDetail(
		ttLibC_AvcodecEncoder_ *encoder,
		uint8_t *l_data,
		uint32_t l_data_size,
		uint8_t *r_data,
		uint32_t r_data_size,
		uint32_t sample_num,
		ttLibC_AvcodecEncodeFunc callback,
		void *ptr) {
	encoder->avframe->pts = encoder->passed_sample_num;
	encoder->avframe->data[0] = l_data;
	encoder->avframe->linesize[0] = l_data_size;
	encoder->avframe->data[1] = r_data;
	encoder->avframe->linesize[1] = r_data_size;
	encoder->avframe->nb_samples = sample_num;
	encoder->packet.data = NULL;
	encoder->packet.size = 0;
	encoder->passed_sample_num += sample_num;
//	LOG_PRINT("sample_num:%d, frame_size:%d", sample_num, encoder->frame_size);
	int got_output;
	int result = avcodec_encode_audio2(encoder->enc, &encoder->packet, encoder->avframe, &got_output);
	if(result != 0) {
		ERR_PRINT("failed to encode audio.:%d", result);
		return false;
	}
//	LOG_PRINT("got_packet:%d", got_output);
	if(got_output != 1) {
		return true;
	}
	// now data is ready, make frame and call callback.
	if(encoder->frame != NULL && encoder->frame->type != encoder->inherit_super.frame_type) {
		ttLibC_Frame_close(&encoder->frame);
	}
	switch(encoder->inherit_super.frame_type) {
	case frameType_aac:
		{
			// we can get as adts buffer.
			ttLibC_Aac *aac = ttLibC_Aac_make(
					(ttLibC_Aac *)encoder->frame,
					AacType_adts,
					encoder->inherit_super.sample_rate,
					encoder->frame_size,
					encoder->inherit_super.channel_num,
					encoder->packet.data,
					encoder->packet.size,
					true,
					encoder->packet.pts,
					encoder->inherit_super.sample_rate,
					0);
			if(aac != NULL) {
				encoder->frame = (ttLibC_Frame *)aac;
				if(callback == NULL) {
					return true;
				}
				return callback(ptr, (ttLibC_Frame *)aac);
			}
		}
		break;
	case frameType_adpcm_ima_wav:
		{
			ttLibC_AdpcmImaWav *adpcm_ima_wav = ttLibC_AdpcmImaWav_make(
					(ttLibC_AdpcmImaWav *)encoder->frame,
					encoder->inherit_super.sample_rate,
					encoder->frame_size,
					encoder->inherit_super.channel_num,
					encoder->packet.data,
					encoder->packet.size,
					true,
					encoder->packet.pts,
					encoder->inherit_super.sample_rate);
			if(adpcm_ima_wav != NULL) {
				encoder->frame = (ttLibC_Frame *)adpcm_ima_wav;
				if(callback == NULL) {
					return true;
				}
				return callback(ptr, (ttLibC_Frame *)adpcm_ima_wav);
			}
		}
		break;
	case frameType_mp3:
		{
			// on the beginning, there is some noise?
			// it is better to use mp3lame encoder.
			ttLibC_Mp3 *mp3 = ttLibC_Mp3_make(
					(ttLibC_Mp3 *)encoder->frame,
					Mp3Type_frame,
					encoder->inherit_super.sample_rate,
					encoder->frame_size,
					encoder->inherit_super.channel_num,
					encoder->packet.data,
					encoder->packet.size,
					true,
					encoder->packet.pts,
					encoder->inherit_super.sample_rate);
			if(mp3 != NULL) {
				encoder->frame = (ttLibC_Frame *)mp3;
				if(callback == NULL) {
					return true;
				}
				return callback(ptr, (ttLibC_Frame *)mp3);
			}
		}
		break;
	case frameType_nellymoser:
		{
			// monoral only.
			ttLibC_Nellymoser *nellymoser = ttLibC_Nellymoser_make(
					(ttLibC_Nellymoser*)encoder->frame,
					encoder->inherit_super.sample_rate,
					encoder->frame_size,
					encoder->inherit_super.channel_num,
					encoder->packet.data,
					encoder->packet.size,
					true,
					encoder->packet.pts,
					encoder->inherit_super.sample_rate);
			if(nellymoser != NULL) {
				encoder->frame = (ttLibC_Frame *)nellymoser;
				if(callback == NULL) {
					return true;
				}
				return callback(ptr, (ttLibC_Frame *)nellymoser);
			}
		}
		break;
	case frameType_opus:
		{
			// TODO not check yet.
			LOG_PRINT("not check yet.");
			ttLibC_Opus *opus = ttLibC_Opus_make(
					(ttLibC_Opus *)encoder->frame,
					OpusType_frame,
					encoder->inherit_super.sample_rate,
					encoder->frame_size,
					encoder->inherit_super.channel_num,
					encoder->packet.data,
					encoder->packet.size,
					true,
					encoder->packet.pts,
					encoder->inherit_super.sample_rate);
			if(opus != NULL) {
				encoder->frame = (ttLibC_Frame *)opus;
				if(callback == NULL) {
					return true;
				}
				return callback(ptr, (ttLibC_Frame *)opus);
			}
		}
		break;
	case frameType_pcm_alaw:
		{
			ttLibC_PcmAlaw *pcm_alaw = ttLibC_PcmAlaw_make(
					(ttLibC_PcmAlaw *)encoder->frame,
					encoder->inherit_super.sample_rate,
					encoder->frame_size,
					encoder->inherit_super.channel_num,
					encoder->packet.data,
					encoder->packet.size,
					true,
					encoder->packet.pts,
					encoder->inherit_super.sample_rate);
			if(pcm_alaw != NULL) {
				encoder->frame = (ttLibC_Frame *)pcm_alaw;
				if(callback == NULL) {
					return true;
				}
				return callback(ptr, (ttLibC_Frame *)pcm_alaw);
			}
		}
		break;
	case frameType_pcm_mulaw:
		{
			ttLibC_PcmMulaw *pcm_mulaw = ttLibC_PcmMulaw_make(
					(ttLibC_PcmMulaw *)encoder->frame,
					encoder->inherit_super.sample_rate,
					encoder->frame_size,
					encoder->inherit_super.channel_num,
					encoder->packet.data,
					encoder->packet.size,
					true,
					encoder->packet.pts,
					encoder->inherit_super.sample_rate);
			if(pcm_mulaw != NULL) {
				encoder->frame = (ttLibC_Frame *)pcm_mulaw;
				if(callback == NULL) {
					return true;
				}
				return callback(ptr, (ttLibC_Frame *)pcm_mulaw);
			}
		}
		break;
	case frameType_speex:
		{
			// TODO not check yet.
			LOG_PRINT("not check yet.");
			ttLibC_Speex *speex = ttLibC_Speex_make(
					(ttLibC_Speex *)encoder->frame,
					SpeexType_frame,
					encoder->inherit_super.sample_rate,
					encoder->frame_size,
					encoder->inherit_super.channel_num,
					encoder->packet.data,
					encoder->packet.size,
					true,
					encoder->packet.pts,
					encoder->inherit_super.sample_rate);
			if(speex != NULL) {
				encoder->frame = (ttLibC_Frame *)speex;
				if(callback == NULL) {
					return true;
				}
				return callback(ptr, (ttLibC_Frame *)speex);
			}
		}
		break;
	case frameType_vorbis:
		{
			ttLibC_Vorbis *vorbis = ttLibC_Vorbis_make(
					(ttLibC_Vorbis *)encoder->frame,
					VorbisType_frame,
					encoder->inherit_super.sample_rate,
					encoder->frame_size,
					encoder->inherit_super.channel_num,
					encoder->packet.data,
					encoder->packet.size,
					true,
					encoder->packet.pts,
					encoder->inherit_super.sample_rate);
			if(vorbis != NULL) {
				encoder->frame = (ttLibC_Frame *)vorbis;
				if(callback == NULL) {
					return true;
				}
				return callback(ptr, (ttLibC_Frame *)vorbis);
			}
		}
		break;
	default:
		return false;
	}
	return false;
}

/*
 * make encode chunk for pcmf32
 * @note make sample_num of chunk which avcodec asks.
 * @param encoder  encoder object
 * @param pcmf32   source pcmf32 object(AV_SAMPLE_FMT_FLT or FLTP)
 * @param callback callback func.
 * @param ptr      user def data pointer.
 */
static bool AvcodecEncoder_encode_PcmF32(
		ttLibC_AvcodecEncoder_ *encoder,
		ttLibC_PcmF32 *pcmf32,
		ttLibC_AvcodecEncodeFunc callback,
		void *ptr) {
	if(encoder == NULL) {
		return false;
	}
	if(pcmf32 == NULL) {
		return true;
	}
	if(encoder->inherit_super.input_format_type != pcmf32->type) {
		ERR_PRINT("not support by avcodec.");
		return false;
	}
	uint8_t *l_data = NULL;
	uint8_t *r_data = NULL;
	uint32_t l_data_size = 0;
	uint32_t r_data_size = 0;
	uint32_t step = 0;
	switch(pcmf32->type) {
	default:
	case PcmF32Type_interleave:
		l_data = pcmf32->inherit_super.inherit_super.data;
		if(pcmf32->inherit_super.channel_num == 2) {
			step = 8;
		}
		else {
			step = 4;
		}
		break;
	case PcmF32Type_planar:
		l_data = pcmf32->inherit_super.inherit_super.data;
		if(pcmf32->inherit_super.channel_num == 2) {
			r_data = l_data + pcmf32->inherit_super.sample_num * 4;
			r_data_size = encoder->frame_size * 4;
		}
		step = 4;
		break;
	}
	l_data_size = encoder->frame_size * step;
	uint32_t left_size = pcmf32->inherit_super.inherit_super.buffer_size;
	if(r_data != NULL) {
		// buffer_size / 2 = one channel size.
		left_size >>= 1;
	}
	if(encoder->pcm_buffer_next_pos != 0) {
		if(left_size < encoder->frame_size * step - encoder->pcm_buffer_next_pos) {
			memcpy(encoder->pcm_buffer + encoder->pcm_buffer_next_pos, l_data, left_size);
			if(r_data != NULL) {
				memcpy(encoder->pcm_buffer + encoder->frame_size * step + encoder->pcm_buffer_next_pos, r_data, left_size);
			}
			encoder->pcm_buffer_next_pos += left_size;
			return true;
		}
		memcpy(encoder->pcm_buffer + encoder->pcm_buffer_next_pos, l_data, encoder->frame_size * step - encoder->pcm_buffer_next_pos);
		l_data += encoder->frame_size * step - encoder->pcm_buffer_next_pos;
		if(r_data != NULL) {
			memcpy(encoder->pcm_buffer + encoder->frame_size * step + encoder->pcm_buffer_next_pos, r_data, encoder->frame_size * step - encoder->pcm_buffer_next_pos);
			r_data += encoder->frame_size * step - encoder->pcm_buffer_next_pos;
		}
		left_size -= encoder->frame_size * step - encoder->pcm_buffer_next_pos;
		uint8_t *tmp_l_data = encoder->pcm_buffer;
		uint8_t *tmp_r_data = NULL;
		if(r_data != NULL) {
			tmp_r_data = encoder->pcm_buffer + encoder->frame_size * step;
		}
		if(!AvcodecEncoder_encode_AudioDetail(
				encoder,
				tmp_l_data,
				l_data_size,
				tmp_r_data,
				r_data_size,
				encoder->frame_size,
				callback,
				ptr)) {
			return false;
		}
		encoder->pcm_buffer_next_pos = 0;
	}
	do {
		if(left_size < encoder->frame_size * step) {
			if(left_size != 0) {
				memcpy(encoder->pcm_buffer, l_data, left_size);
				if(r_data != NULL) {
					memcpy(encoder->pcm_buffer + encoder->frame_size * step, r_data, left_size);
				}
				encoder->pcm_buffer_next_pos = left_size;
			}
			break;
		}
		if(!AvcodecEncoder_encode_AudioDetail(
				encoder,
				l_data,
				l_data_size,
				r_data,
				r_data_size,
				encoder->frame_size,
				callback,
				ptr)) {
			return false;
		}
		l_data += encoder->frame_size * step;
		if(r_data != NULL) {
			r_data += encoder->frame_size * step;
		}
		left_size -= encoder->frame_size * step;
	} while(true);
	return true;
}

/*
 * make encode chunk for pcms16
 * @note make sample_num of chunk which avcodec asks.
 * @param encoder  encoder object
 * @param pcmf32   source pcms16 object(AV_SAMPLE_FMT_S16 or S16P)
 * @param callback callback func.
 * @param ptr      user def data pointer.
 */
static bool AvcodecEncoder_encode_PcmS16(
		ttLibC_AvcodecEncoder_ *encoder,
		ttLibC_PcmS16 *pcms16,
		ttLibC_AvcodecEncodeFunc callback,
		void *ptr) {
	if(encoder == NULL) {
		return false;
	}
	if(pcms16 == NULL) {
		return true;
	}
	if(encoder->inherit_super.input_format_type != pcms16->type) {
		ERR_PRINT("not support by avcodec.");
		return false;
	}
	uint8_t *l_data = NULL;
	uint8_t *r_data = NULL;
	uint32_t l_data_size = 0;
	uint32_t r_data_size = 0;
	uint32_t step = 0;
	switch(pcms16->type) {
	case PcmS16Type_bigEndian:
	default:
	case PcmS16Type_littleEndian:
		l_data = pcms16->inherit_super.inherit_super.data;
		if(pcms16->inherit_super.channel_num == 2) {
			step = 4;
		}
		else {
			step = 2;
		}
		break;
	case PcmS16Type_bigEndian_planar:
	case PcmS16Type_littleEndian_planar:
		l_data = pcms16->inherit_super.inherit_super.data;
		if(pcms16->inherit_super.channel_num == 2) {
			r_data = l_data + pcms16->inherit_super.sample_num * 2;
			r_data_size = encoder->frame_size * 2;
		}
		step = 2;
		break;
	}
	l_data_size = encoder->frame_size * step;
	uint32_t left_size = pcms16->inherit_super.inherit_super.buffer_size;
	if(r_data != NULL) {
		left_size >>= 1;
	}
	if(encoder->pcm_buffer_next_pos != 0) {
		if(left_size < encoder->frame_size * step - encoder->pcm_buffer_next_pos) {
			memcpy(encoder->pcm_buffer + encoder->pcm_buffer_next_pos, l_data, left_size);
			if(r_data != NULL) {
				memcpy(encoder->pcm_buffer + encoder->frame_size * step + encoder->pcm_buffer_next_pos, r_data, left_size);
			}
			encoder->pcm_buffer_next_pos += left_size;
			return true;
		}
		memcpy(encoder->pcm_buffer + encoder->pcm_buffer_next_pos, l_data, encoder->frame_size * step - encoder->pcm_buffer_next_pos);
		l_data += encoder->frame_size * step - encoder->pcm_buffer_next_pos;
		if(r_data != NULL) {
			memcpy(encoder->pcm_buffer + encoder->frame_size * step + encoder->pcm_buffer_next_pos, r_data, encoder->frame_size * step - encoder->pcm_buffer_next_pos);
			r_data += encoder->frame_size * step - encoder->pcm_buffer_next_pos;
		}
		left_size -= encoder->frame_size * step - encoder->pcm_buffer_next_pos;
		uint8_t *tmp_l_data = encoder->pcm_buffer;
		uint8_t *tmp_r_data = NULL;
		if(r_data != NULL) {
			tmp_r_data = encoder->pcm_buffer + encoder->frame_size * step;
		}
		if(!AvcodecEncoder_encode_AudioDetail(
				encoder,
				tmp_l_data,
				l_data_size,
				tmp_r_data,
				r_data_size,
				encoder->frame_size,
				callback,
				ptr)) {
			return false;
		}
		encoder->pcm_buffer_next_pos = 0;
	}
	do {
		if(left_size < encoder->frame_size * step) {
			if(left_size != 0) {
				memcpy(encoder->pcm_buffer, l_data, left_size);
				if(r_data != NULL) {
					memcpy(encoder->pcm_buffer + encoder->frame_size * step, r_data, left_size);
				}
				encoder->pcm_buffer_next_pos = left_size;
			}
			break;
		}
		if(!AvcodecEncoder_encode_AudioDetail(
				encoder,
				l_data,
				l_data_size,
				r_data,
				r_data_size,
				encoder->frame_size,
				callback,
				ptr)) {
			return false;
		}
		l_data += encoder->frame_size * step;
		if(r_data != NULL) {
			r_data += encoder->frame_size * step;
		}
		left_size -= encoder->frame_size * step;
	} while(true);
	return true;
}

/*
 * make encode chunk for bgr
 * TODO make this later.(maybe need to FLASH sv 1 or 2)
 * @param encoder  encoder object
 * @param bgr      bgr input data.(AV_PIX_FMT_BGR24, ABGR, or BGRA)
 * @param callback callback func.
 * @param ptr      user def data pointer.
 */
static bool AvcodecEncoder_encode_Bgr(
		ttLibC_AvcodecEncoder_ *encoder,
		ttLibC_Bgr *bgr,
		ttLibC_AvcodecEncodeFunc callback,
		void *ptr) {
	if(encoder == NULL) {
		return false;
	}
	if(bgr == NULL) {
		return true;
	}
	puts("encode bgr is not implemented. do later.");
	return false;
}

/*
 * make encode chunk for yuv420
 * @param encoder  encoder object
 * @param yuv420   bgr input data.(AV_PIX_FMT_YUV420P, NV12, or NV21)
 * @param callback callback func.
 * @param ptr      user def data pointer.
 */
static bool AvcodecEncoder_encode_Yuv420(
		ttLibC_AvcodecEncoder_ *encoder,
		ttLibC_Yuv420 *yuv420,
		ttLibC_AvcodecEncodeFunc callback,
		void *ptr) {
	if(encoder == NULL) {
		return false;
	}
	if(yuv420 == NULL) {
		return true;
	}
	if(encoder->inherit_super.input_format_type != yuv420->type) {
		ERR_PRINT("not support by avcodec.");
		return false;
	}
	encoder->avframe->pts = yuv420->inherit_super.inherit_super.pts;
	switch(yuv420->type) {
	case Yuv420Type_planar:
		encoder->avframe->data[0] = yuv420->y_data;
		encoder->avframe->data[1] = yuv420->u_data;
		encoder->avframe->data[2] = yuv420->v_data;
		break;
	case Yuv420Type_semiPlanar:
		ERR_PRINT("not check yet1.");
		encoder->avframe->data[0] = yuv420->y_data;
		encoder->avframe->data[1] = yuv420->u_data;
		break;
	case Yvu420Type_semiPlanar:
		ERR_PRINT("not check yet2.");
		encoder->avframe->data[0] = yuv420->y_data;
		encoder->avframe->data[1] = yuv420->v_data;
		break;
	default:
	case Yvu420Type_planar: // not supported by ffmpeg.
		return false;
	}
	encoder->packet.data = NULL;
	encoder->packet.size = 0;
	int got_output;
	int result = avcodec_encode_video2(encoder->enc, &encoder->packet, encoder->avframe, &got_output);
	if(result != 0) {
		ERR_PRINT("failed to encode.:%d", result);
		return false;
	}
	if(got_output != 1) {
		// encode is ok. however, output is none. this can be.
		// just skip and wait for convert.
//		ERR_PRINT("got output is not 1, failed to encode.:%d", got_output);
		return true;
	}
	// now check the output packet according to frame type.
	if(encoder->frame != NULL && encoder->frame->type != encoder->inherit_super.frame_type) {
		ttLibC_Frame_close(&encoder->frame);
	}
	switch(encoder->inherit_super.frame_type) {
	case frameType_flv1:
		{
			ttLibC_Flv1_Type flv1_type = Flv1Type_inner;
			if(encoder->packet.flags & AV_PKT_FLAG_KEY) {
				flv1_type = Flv1Type_intra;
			}
			ttLibC_Flv1 *flv1 = ttLibC_Flv1_make(
					(ttLibC_Flv1 *)encoder->frame,
					flv1_type,
					encoder->inherit_super.width,
					encoder->inherit_super.height,
					encoder->packet.data,
					encoder->packet.size,
					true,
					yuv420->inherit_super.inherit_super.pts,
					yuv420->inherit_super.inherit_super.timebase);
			if(flv1 != NULL) {
				encoder->frame = (ttLibC_Frame *)flv1;
				if(callback == NULL) {
					return true;
				}
				return callback(ptr, (ttLibC_Frame *)flv1);
			}
		}
		break;
	case frameType_h264:
		// TODO do later.
		// check nal and divide into configData, slice, sliceIDR...
		LOG_PRINT("pkt_size:%d got_output:%d", encoder->packet.size, got_output);
		if(encoder->packet.size > 10) {
			ttLibC_HexUtil_dump(encoder->packet.data, 10, true);
		}
		puts("h264 is not implemented");
		/*	if(encoder_->avframe != NULL) {
				// video
				ttLibC_Yuv420 *yuv420 = (ttLibC_Yuv420 *)frame;
				encoder_->avframe->data[0] = yuv420->y_data;
				encoder_->avframe->data[1] = yuv420->u_data;
				encoder_->avframe->data[2] = yuv420->v_data;

				encoder_->packet.data = NULL;
				encoder_->packet.size = 0;
				int got_output;
				int result = avcodec_encode_video2(encoder_->enc, &encoder_->packet, encoder_->avframe, &got_output);
				if(result != 0) {
					ERR_PRINT("failed to encode.");
					return;
				}
				LOG_PRINT("got output:%d", got_output);
				switch(encoder_->inherit_super.frame_type) {
				case frameType_h264:
					{
						if(encoder_->packet.size == 0) {
							// nodata.
							return;
						}
						// need to analyze h264 nal.
						if((encoder_->packet.flags & AV_PKT_FLAG_KEY) != 0) {
							// for keyframe need to get sps pps as configData.
							// sliceIDR as sliceIDR frame
							// supplemental information -> ignore?
							ttLibC_H264_NalInfo info;
							uint8_t *start_ptr = encoder_->packet.data;
							uint8_t *data = start_ptr;
							size_t data_size = encoder_->packet.size;
							if(ttLibC_H264_isAvcc(data, data_size)) {
								ERR_PRINT("avcc is not expected.");
								return;
							}
							else if(ttLibC_H264_isNal(data, data_size)) {
								do {
									if(!ttLibC_H264_getNalInfo(&info, data, data_size)) {
										ERR_PRINT("failed to get nal information.");
										return;
									}
									data += info.nal_size;
									data_size -= info.nal_size;
									switch(info.nal_unit_type) {
									case H264NalType_sequenceParameterSet:
										break;
									case H264NalType_pictureParameterSet:
										{
											// expect to have sps -> pps order.
											ttLibC_H264 *h264 = ttLibC_H264_make(encoder_->configData, H264Type_configData, encoder_->inherit_super.width, encoder_->inherit_super.height, start_ptr, data - start_ptr, true, 0, 1000);
											if(h264 == NULL) {
												ERR_PRINT("failed to make h264 config frame.");
												return;
											}
											encoder_->configData = h264;
											callback(ptr, (ttLibC_Frame *)encoder_->configData);
											start_ptr = data;
										}
										break;
									case H264NalType_supplementalEnhancementInformation:
		//								LOG_PRINT("supplementalEnhancementInformationをみつけた");
										start_ptr = data;
										break;
									case H264NalType_sliceIDR:
										break;
									default:
										ERR_PRINT("find nal which I don't expect.:%d", info.nal_unit_type);
										return;
									}
								} while(data_size > 0);
								if(info.nal_unit_type == H264NalType_sliceIDR) {
									ttLibC_H264 *h264 = ttLibC_H264_make((ttLibC_H264 *)encoder_->frame, H264Type_sliceIDR, encoder_->inherit_super.width, encoder_->inherit_super.height, start_ptr, data - start_ptr, true, 0, 1000);
									if(h264 == NULL) {
										ERR_PRINT("failed to make h264 sliceIDR.");
										return;
									}
									encoder_->frame = (ttLibC_Frame *)h264;
									callback(ptr, encoder_->frame);
								}
							}
							else {
								ERR_PRINT("unknown packet type.");
								return;
							}
						}
						else {
							ttLibC_H264_NalInfo info;
							uint8_t *start_ptr = encoder_->packet.data;
							uint8_t *data = start_ptr;
							size_t data_size = encoder_->packet.size;
							if(ttLibC_H264_isAvcc(data, data_size)) {
								ERR_PRINT("avcc is not expected.");
								return;
							}
							else if(ttLibC_H264_isNal(data, data_size)) {
								do {
									if(!ttLibC_H264_getNalInfo(&info, data, data_size)) {
										ERR_PRINT("failed to get nal information.");
										return;
									}
									data += info.nal_size;
									data_size -= info.nal_size;
									switch(info.nal_unit_type) {
									case H264NalType_slice:
										// find nal.
										break;
									default:
										ERR_PRINT("find nal which I don't expect.:%d", info.nal_unit_type);
										return;
									}
								} while(data_size > 0);
								if(info.nal_unit_type == H264NalType_slice) {
									ttLibC_H264 *h264 = ttLibC_H264_make((ttLibC_H264 *)encoder_->frame, H264Type_slice, encoder_->inherit_super.width, encoder_->inherit_super.height, start_ptr, data - start_ptr, true, 0, 1000);
									if(h264 == NULL) {
										ERR_PRINT("failed to make h264 slice.");
										return;
									}
									encoder_->frame = (ttLibC_Frame *)h264;
									callback(ptr, encoder_->frame);
								}
							}
							else {
								ERR_PRINT("unknown packet type.");
								return;
							}
						}
					}
					break;
				default:
					ERR_PRINT("unexpected frames.");
					break;
				}
			}*/
		break;
	case frameType_h265:
		// does not work for convert.
		// TODO do later
		puts("h265 is not implemented.");
		break;
	case frameType_theora:
		// unsupported on my pc.
		// TODO do later
		puts("theora is not implemented");
		break;
	case frameType_vp6:
		// libavcodec is not support for encode.
		puts("vp6 is not implemented");
		break;
	case frameType_vp8:
		{
			ttLibC_Video_Type video_type = videoType_inner;
			if(encoder->packet.flags & AV_PKT_FLAG_KEY) {
				video_type = videoType_key;
			}
			ttLibC_Vp8 *vp8 = ttLibC_Vp8_make(
					(ttLibC_Vp8 *)encoder->frame,
					video_type,
					encoder->inherit_super.width,
					encoder->inherit_super.height,
					encoder->packet.data,
					encoder->packet.size,
					true,
					yuv420->inherit_super.inherit_super.pts,
					yuv420->inherit_super.inherit_super.timebase);
			if(vp8 != NULL) {
				encoder->frame = (ttLibC_Frame *)vp8;
				if(callback == NULL) {
					return true;
				}
				return callback(ptr, (ttLibC_Frame *)vp8);
			}
		}
		break;
	case frameType_vp9:
		// get some error.
		// apply VP8E_SET_NOISE_SENSITIVITY for vp9, and struggle with error.
		// TODO check later.
		puts("vp9 is not implemented.");
		break;
	case frameType_wmv1:
		{
			ttLibC_Video_Type video_type = videoType_inner;
			if(encoder->packet.flags & AV_PKT_FLAG_KEY) {
				video_type = videoType_key;
			}
			ttLibC_Wmv1 *wmv1 = ttLibC_Wmv1_make(
					(ttLibC_Wmv1 *)encoder->frame,
					video_type,
					encoder->inherit_super.width,
					encoder->inherit_super.height,
					encoder->packet.data,
					encoder->packet.size,
					true,
					yuv420->inherit_super.inherit_super.pts,
					yuv420->inherit_super.inherit_super.timebase);
			if(wmv1 != NULL) {
				encoder->frame = (ttLibC_Frame *)wmv1;
				if(callback == NULL) {
					return true;
				}
				return callback(ptr, (ttLibC_Frame *)wmv1);
			}
		}
		break;
	case frameType_wmv2:
		{
			ttLibC_Video_Type video_type = videoType_inner;
			if(encoder->packet.flags & AV_PKT_FLAG_KEY) {
				video_type = videoType_key;
			}
			ttLibC_Wmv2 *wmv2 = ttLibC_Wmv2_make(
					(ttLibC_Wmv2 *)encoder->frame,
					video_type,
					encoder->inherit_super.width,
					encoder->inherit_super.height,
					encoder->packet.data,
					encoder->packet.size,
					true,
					yuv420->inherit_super.inherit_super.pts,
					yuv420->inherit_super.inherit_super.timebase);
			if(wmv2 != NULL) {
				encoder->frame = (ttLibC_Frame *)wmv2;
				if(callback == NULL) {
					return true;
				}
				return callback(ptr, (ttLibC_Frame *)wmv2);
			}
		}
		break;
	default:
		// something wrong.
		return false;
	}
	return false;
}

/*
 * get AVCodecContext for target frameType.
 * @param frame_type target ttLibC_Frame_Type
 */
void *ttLibC_AvcodecEncoder_getAVCodecContext(ttLibC_Frame_Type frame_type) {
	AVCodec *codec = NULL;
	switch(frame_type) {
	case frameType_aac:
		codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
		break;
	case frameType_adpcm_ima_wav:
		codec = avcodec_find_encoder(AV_CODEC_ID_ADPCM_IMA_WAV);
		break;
	case frameType_flv1:
		codec = avcodec_find_encoder(AV_CODEC_ID_FLV1);
		break;
	case frameType_h264:
		codec = avcodec_find_encoder(AV_CODEC_ID_H264);
		break;
#ifdef AV_CODEC_ID_HEVC
	case frameType_h265:
		codec = avcodec_find_encoder(AV_CODEC_ID_HEVC);
		break;
#endif
	case frameType_mp3:
		codec = avcodec_find_encoder(AV_CODEC_ID_MP3);
		break;
	case frameType_nellymoser:
		codec = avcodec_find_encoder(AV_CODEC_ID_NELLYMOSER);
		break;
	case frameType_opus:
		codec = avcodec_find_encoder(AV_CODEC_ID_OPUS);
		break;
	case frameType_pcm_alaw:
		codec = avcodec_find_encoder(AV_CODEC_ID_PCM_ALAW);
		break;
	case frameType_pcm_mulaw:
		codec = avcodec_find_encoder(AV_CODEC_ID_PCM_MULAW);
		break;
	case frameType_speex:
		codec = avcodec_find_encoder(AV_CODEC_ID_SPEEX);
		break;
	case frameType_theora:
		codec = avcodec_find_encoder(AV_CODEC_ID_THEORA);
		break;
	case frameType_vorbis:
		codec = avcodec_find_encoder(AV_CODEC_ID_VORBIS);
		break;
	case frameType_vp6:
//		codec = avcodec_find_encoder(AV_CODEC_ID_VP6); // return 0;
		ERR_PRINT("vp6 encoder is not supported by avcodec.");
		return NULL;
	case frameType_vp8:
		codec = avcodec_find_encoder(AV_CODEC_ID_VP8);
		break;
	case frameType_vp9:
		codec = avcodec_find_encoder(AV_CODEC_ID_VP9);
		break;
	case frameType_wmv1:
		codec = avcodec_find_encoder(AV_CODEC_ID_WMV1);
		break;
	case frameType_wmv2:
		codec = avcodec_find_encoder(AV_CODEC_ID_WMV2);
		break;
	default:
		ERR_PRINT("unsupported frame type:%d", frame_type);
		return NULL;
	}
	if(codec == NULL) {
		ERR_PRINT("failed to get codec object for frame_type:%d", frame_type);
		return NULL;
	}
	AVCodecContext *enc = avcodec_alloc_context3(codec);
	if(enc == NULL) {
		ERR_PRINT("failed to alloc encode context.");
		return NULL;
	}
	return enc;
}

/*
 * make avcodecEncoder with AVCodecContext
 * @param enc_context target AVCodecContext
 */
ttLibC_AvcodecEncoder *ttLibC_AvcodecEncoder_makeWithAVCodecContext(void *enc_context) {
	if(enc_context == NULL) {
		return NULL;
	}
	AVCodecContext *enc = (AVCodecContext *)enc_context;
	ttLibC_Frame_Type frame_type = frameType_aac;
	switch(enc->codec_id) {
	case AV_CODEC_ID_AAC:
		frame_type = frameType_aac;
		break;
	case AV_CODEC_ID_ADPCM_IMA_WAV:
		frame_type = frameType_adpcm_ima_wav;
		break;
	case AV_CODEC_ID_FLV1:
		frame_type = frameType_flv1;
		break;
	case AV_CODEC_ID_H264:
		frame_type = frameType_h264;
		break;
#ifdef AV_CODEC_ID_HEVC
	case AV_CODEC_ID_HEVC:
		frame_type = frameType_h265;
		break;
#endif
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
		av_free(enc);
		return NULL;
	}
	ttLibC_AvcodecEncoder_ *encoder = ttLibC_malloc(sizeof(ttLibC_AvcodecEncoder_));
	if(encoder == NULL) {
		ERR_PRINT("failed to allocate encoder object.");
		av_free(enc);
		return NULL;
	}
	encoder->enc = enc;
	if(avcodec_open2(encoder->enc, encoder->enc->codec, NULL) < 0) {
		ERR_PRINT("failed to open codec.");
		av_free(encoder->enc);
		ttLibC_free(encoder);
		return NULL;
	}
	encoder->avframe = av_frame_alloc();
	if(encoder->avframe == NULL) {
		ERR_PRINT("failed to alloc avframe.");
		avcodec_close(encoder->enc);
		av_free(encoder->enc);
		ttLibC_free(encoder);
		return NULL;
	}
	encoder->pcm_buffer = NULL;
	encoder->pcm_buffer_next_pos = 0;
	encoder->passed_sample_num = 0;
	encoder->frame_size = 0;
	// then make sample.
	switch(enc->codec->type) {
	case AVMEDIA_TYPE_AUDIO:
		encoder->frame_size = encoder->enc->frame_size;
		if(encoder->frame_size == 0) {
			// force frame_size 200 for now. or we will get stack with infinite loop.
			// this could be happen for pcm_alaw and pcm_mulaw
			encoder->frame_size = 200;
		}
		encoder->avframe->channels         = encoder->enc->channels;
		encoder->avframe->sample_rate      = encoder->enc->sample_rate;
		encoder->avframe->format           = encoder->enc->sample_fmt;
		encoder->avframe->channel_layout   = encoder->enc->channel_layout;
		encoder->inherit_super.sample_rate = encoder->enc->sample_rate;
		encoder->inherit_super.channel_num = encoder->enc->channels;
		// now need to allocate pcm_buffer.
		size_t pcm_buffer_size = 0;
		switch(encoder->enc->sample_fmt) {
		case AV_SAMPLE_FMT_DBL:
		case AV_SAMPLE_FMT_DBLP:
		case AV_SAMPLE_FMT_NB: // ?
		case AV_SAMPLE_FMT_NONE:
		case AV_SAMPLE_FMT_S32:
		case AV_SAMPLE_FMT_S32P:
		case AV_SAMPLE_FMT_U8:
		case AV_SAMPLE_FMT_U8P:
			ERR_PRINT("unsupported audio format.:%d", encoder->enc->sample_fmt);
			av_free(encoder->avframe);
			avcodec_close(encoder->enc);
			av_free(encoder->enc);
			ttLibC_free(encoder);
			return NULL;
			// support with ttLibC_PcmF32
		case AV_SAMPLE_FMT_FLT:
			pcm_buffer_size = encoder->frame_size * encoder->enc->channels * 4;
			encoder->pcm_buffer = ttLibC_malloc(pcm_buffer_size);
			encoder->inherit_super.input_frame_type  = frameType_pcmF32;
			encoder->inherit_super.input_format_type = PcmF32Type_interleave;
			break;
		case AV_SAMPLE_FMT_FLTP:
			pcm_buffer_size = encoder->frame_size * encoder->enc->channels * 4;
			encoder->pcm_buffer = ttLibC_malloc(pcm_buffer_size);
			encoder->inherit_super.input_frame_type  = frameType_pcmF32;
			encoder->inherit_super.input_format_type = PcmF32Type_planar;
			break;
			// support with ttLibC_PcmS16
		case AV_SAMPLE_FMT_S16:
			// endian shoud be according to cpu.
			pcm_buffer_size = encoder->frame_size * encoder->enc->channels * 2;
			encoder->pcm_buffer = ttLibC_malloc(pcm_buffer_size);
			encoder->inherit_super.input_frame_type = frameType_pcmS16;
			encoder->inherit_super.input_format_type = PcmS16Type_littleEndian;
			break;
		case AV_SAMPLE_FMT_S16P:
			// endian shoud be according to cpu.
			pcm_buffer_size = encoder->frame_size * encoder->enc->channels * 2;
			encoder->pcm_buffer = ttLibC_malloc(pcm_buffer_size);
			encoder->inherit_super.input_frame_type  = frameType_pcmS16;
			encoder->inherit_super.input_format_type = PcmS16Type_littleEndian_planar;
			break;
		}
		break;
	case AVMEDIA_TYPE_VIDEO:
		encoder->avframe->format      = encoder->enc->pix_fmt;
		encoder->avframe->width       = encoder->enc->width;
		encoder->avframe->height      = encoder->enc->height;
		encoder->inherit_super.width  = encoder->enc->width;
		encoder->inherit_super.height = encoder->enc->height;
		switch(encoder->enc->pix_fmt) {
			// support with ttLibC_Yuv420
		case AV_PIX_FMT_YUV420P: // yuv420 planar　yyyy.... uuuu.... vvvv....
			encoder->inherit_super.input_frame_type = frameType_yuv420;
			encoder->inherit_super.input_format_type = Yuv420Type_planar;
			encoder->avframe->linesize[0] = encoder->enc->width;
			encoder->avframe->linesize[1] = (encoder->enc->width >> 1);
			encoder->avframe->linesize[2] = (encoder->enc->width >> 1);
			break;
		case AV_PIX_FMT_NV12: // y plane + uv plane yyyy.... uvuvuvuv....
			encoder->inherit_super.input_frame_type = frameType_yuv420;
			encoder->inherit_super.input_format_type = Yuv420Type_semiPlanar;
			encoder->avframe->linesize[0] = encoder->enc->width;
			encoder->avframe->linesize[1] = encoder->enc->width;
			break;
		case AV_PIX_FMT_NV21: // y plane + vu plane yyyy.... vuvuvuvu....
			encoder->inherit_super.input_frame_type = frameType_yuv420;
			encoder->inherit_super.input_format_type = Yvu420Type_semiPlanar;
			encoder->avframe->linesize[0] = encoder->enc->width;
			encoder->avframe->linesize[1] = encoder->enc->width;
			break;

			// support with ttLibC_Bgr
		case AV_PIX_FMT_ABGR: // abgrabgrabgr....
			encoder->inherit_super.input_frame_type = frameType_bgr;
			encoder->inherit_super.input_format_type = BgrType_abgr;
			encoder->avframe->linesize[0] = (encoder->enc->width << 2);
			break;
		case AV_PIX_FMT_BGRA: // bgrabgrabgra....
			encoder->inherit_super.input_frame_type = frameType_bgr;
			encoder->inherit_super.input_format_type = BgrType_bgra;
			encoder->avframe->linesize[0] = (encoder->enc->width << 2);
			break;
		case AV_PIX_FMT_BGR24: // bgrbgrbgr....
			encoder->inherit_super.input_frame_type = frameType_bgr;
			encoder->inherit_super.input_format_type = BgrType_bgr;
			encoder->avframe->linesize[0] = (encoder->enc->width << 1) + encoder->enc->width;
			break;
//		case AV_PIX_FMT_ARGB:
//		case AV_PIX_FMT_RGBA:
		default:
			ERR_PRINT("unsupported picture format.:%d", encoder->enc->pix_fmt);
			av_free(encoder->avframe);
			avcodec_close(encoder->enc);
			av_free(encoder->enc);
			ttLibC_free(encoder);
			return NULL;
		}
		break;
	default:
		ERR_PRINT("unknown media type.%d", enc->codec->type);
		av_free(encoder->avframe);
		avcodec_close(encoder->enc);
		av_free(encoder->enc);
		ttLibC_free(encoder);
		return NULL;
	}
	av_init_packet(&encoder->packet);

	encoder->frame = NULL;
	encoder->configData = NULL;
	encoder->inherit_super.frame_type = frame_type;
	return (ttLibC_AvcodecEncoder *)encoder;
}

/*
 * make audio encoder
 * @param frame_type  target ttLibC_Frame_Type
 * @param sample_rate target sample_rate
 * @param channel_num target channel_num
 */
ttLibC_AvcodecEncoder *ttLibC_AvcodecAudioEncoder_make(
		ttLibC_Frame_Type frame_type,
		uint32_t sample_rate,
		uint32_t channel_num) {
	return ttLibC_AvcodecAudioEncoder_make_ex(
			frame_type,
			sample_rate,
			channel_num,
			96000); // set 96kbps for default
}

/*
 * make audio encoder
 * @param frame_type  target ttLibC_Frame_Type
 * @param sample_rate target sample_rate
 * @param channel_num target channel_num
 * @param bitrate     target bitrate
 */
ttLibC_AvcodecEncoder *ttLibC_AvcodecAudioEncoder_make_ex(
		ttLibC_Frame_Type frame_type,
		uint32_t sample_rate,
		uint32_t channel_num,
		uint32_t bitrate) {
	avcodec_register_all();
	AVCodecContext *enc = ttLibC_AvcodecEncoder_getAVCodecContext(frame_type);
	if(enc == NULL) {
		return NULL;
	}
	enc->bit_rate = bitrate;
	enc->sample_rate = sample_rate;
	enc->time_base = (AVRational){1, sample_rate};
	enc->channels = channel_num;
	enc->channel_layout = av_get_default_channel_layout(channel_num);
	enc->sample_fmt = AV_SAMPLE_FMT_NONE;
	const enum AVSampleFormat *formats = enc->codec->sample_fmts;
	while(*formats != -1) {
		if(*formats == AV_SAMPLE_FMT_S16) {
			enc->sample_fmt = AV_SAMPLE_FMT_S16;
			break;
		}
		else if(*formats == AV_SAMPLE_FMT_S16P) {
			enc->sample_fmt = AV_SAMPLE_FMT_S16P;
			break;
		}
		else if(*formats == AV_SAMPLE_FMT_FLT) {
			enc->sample_fmt = AV_SAMPLE_FMT_FLT;
			break;
		}
		else if(*formats == AV_SAMPLE_FMT_FLTP) {
			enc->sample_fmt = AV_SAMPLE_FMT_FLTP;
			break;
		}
		formats ++;
	}
	return ttLibC_AvcodecEncoder_makeWithAVCodecContext(enc);
}

/*
 * make video encoder
 * @param frame_type target ttLibC_Frame_Type
 * @param width      width
 * @param height     height
 */
ttLibC_AvcodecEncoder *ttLibC_AvcodecVideoEncoder_make(
		ttLibC_Frame_Type frame_type,
		uint32_t width,
		uint32_t height) {
	return ttLibC_AvcodecVideoEncoder_make_ex(
			frame_type,
			width,
			height,
			0,
			150000,
			1000);
}

/*
 * make video encoder
 * @param frame_type target ttLibC_Frame_Type
 * @param width      width
 * @param height     height
 * @param quality    q-value
 * @param bitrate    target bitrate in bit/sec
 * @param timebase   time base
 */
ttLibC_AvcodecEncoder *ttLibC_AvcodecVideoEncoder_make_ex(
		ttLibC_Frame_Type frame_type,
		uint32_t width,
		uint32_t height,
		uint32_t quality,
		uint32_t bitrate,
		uint32_t timebase) {
	avcodec_register_all();
	AVCodecContext *enc = ttLibC_AvcodecEncoder_getAVCodecContext(frame_type);
	if(enc == NULL) {
		return NULL;
	}
	enc->bit_rate = bitrate;
	enc->width = width;
	enc->height = height;
	enc->global_quality = quality;
	// unused for encoder.
//	enc->framerate = (AVRational){framerate, 1};
	enc->time_base = (AVRational){1, timebase};
	enc->gop_size = 10;
	enc->max_b_frames = 0;
	enc->pix_fmt = AV_PIX_FMT_NONE;
	const enum AVPixelFormat *formats = enc->codec->pix_fmts;
	while(*formats != -1) {
		if(*formats == AV_PIX_FMT_YUV420P) {
			enc->pix_fmt = AV_PIX_FMT_YUV420P;
			break;
		}
		else if(*formats == AV_PIX_FMT_BGR24){
			enc->pix_fmt = AV_PIX_FMT_BGR24;
			break;
		}
		else if(*formats == AV_PIX_FMT_BGRA){
			enc->pix_fmt = AV_PIX_FMT_BGRA;
			break;
		}
		else if(*formats == AV_PIX_FMT_ABGR){
			enc->pix_fmt = AV_PIX_FMT_ABGR;
			break;
		}
		else if(*formats == AV_PIX_FMT_NV12) {
			enc->pix_fmt = AV_PIX_FMT_NV12;
			break;
		}
		else if(*formats == AV_PIX_FMT_NV21) {
			enc->pix_fmt = AV_PIX_FMT_NV21;
			break;
		}
		formats ++;
	}
	return ttLibC_AvcodecEncoder_makeWithAVCodecContext(enc);
}

/*
 * encode frame
 * @param encoder  avcodec encoder object
 * @param frame    source frame. frame_type and frame format type is the same as encoder->input_frame_type and encoder->input_format_type.
 * @param callback callback func for avcodec encode.
 * @param ptr      pointer for user def value, which will call in callback.
 */
bool ttLibC_AvcodecEncoder_encode(
		ttLibC_AvcodecEncoder *encoder,
		ttLibC_Frame *frame,
		ttLibC_AvcodecEncodeFunc callback,
		void *ptr) {
	if(encoder == NULL) {
		return false;
	}
	if(frame == NULL) {
		return true;
	}
	ttLibC_AvcodecEncoder_ *encoder_ = (ttLibC_AvcodecEncoder_ *)encoder;
	// check the target data.
	if(frame->type != encoder_->inherit_super.input_frame_type) {
		ERR_PRINT("input frame is not supported by avcodec.");
		return false;
	}
	if(encoder_->avframe == NULL) {
		ERR_PRINT("avframe is not initialized yet.");
		return false;
	}
	switch(frame->type) {
	case frameType_pcmF32:
		return AvcodecEncoder_encode_PcmF32(encoder_, (ttLibC_PcmF32 *)frame, callback, ptr);
	case frameType_pcmS16:
		return AvcodecEncoder_encode_PcmS16(encoder_, (ttLibC_PcmS16 *)frame, callback, ptr);
	case frameType_bgr:
		return AvcodecEncoder_encode_Bgr(encoder_, (ttLibC_Bgr *)frame, callback, ptr);
	case frameType_yuv420:
		return AvcodecEncoder_encode_Yuv420(encoder_, (ttLibC_Yuv420 *)frame, callback, ptr);
	default:
		ERR_PRINT("unknown frame type for avcodec input:%d", frame->type);
		return false;
	}
}

/*
 * ref the avcodec extra data.
 * @param encoder avcodec encoder object.
 */
void *ttLibC_AvcodecEncoder_getExtraData(ttLibC_AvcodecEncoder *encoder) {
	ttLibC_AvcodecEncoder_ *encoder_ = (ttLibC_AvcodecEncoder_ *)encoder;
	if(encoder_ == NULL) {
		return NULL;
	}
	return encoder_->enc->extradata;
}

/*
 * ref the avcodec extra data size.
 * @param encoder avcodec encoder object.
 */
size_t ttLibC_AvcodecEncoder_getExtraDataSize(ttLibC_AvcodecEncoder *encoder) {
	ttLibC_AvcodecEncoder_ *encoder_ = (ttLibC_AvcodecEncoder_ *)encoder;
	if(encoder_ == NULL) {
		return 0;
	}
	return encoder_->enc->extradata_size;
}

/*
 * close avcodec encoder.
 * @param encoder
 */
void ttLibC_AvcodecEncoder_close(ttLibC_AvcodecEncoder **encoder) {
	ttLibC_AvcodecEncoder_ *target = (ttLibC_AvcodecEncoder_ *)*encoder;
	if(target == NULL) {
		return;
	}
	if(target->pcm_buffer != NULL) {
		ttLibC_free(target->pcm_buffer);
	}
	if(target->avframe != NULL) {
		av_free(target->avframe);
	}
	av_free_packet(&target->packet);
	if(target->enc != NULL) {
		avcodec_close(target->enc);
		av_free(target->enc);
	}
	ttLibC_Frame_close(&target->frame);
	ttLibC_free(target);
	*encoder = NULL;
}

#endif

