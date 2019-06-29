/**
 * @file   swresampleResampler.c
 * @brief  
 * @author taktod
 * @date   2017/04/28
 */

#ifdef __ENABLE_SWRESAMPLE__

#include "swresampleResampler.h"
#include <libswresample/swresample.h>
#include <libavutil/channel_layout.h>
#include "../allocator.h"
#include "../_log.h"

typedef struct {
	ttLibC_SwresampleResampler inherit_super;
	struct SwrContext *convertCtx;
	ttLibC_Frame_Type  input_type;
	uint32_t           input_sub_type;
	uint32_t           input_sample_rate;
	uint32_t           input_channel_num;
	ttLibC_Frame_Type  output_type;
	uint32_t           output_sub_type;
	uint32_t           output_sample_rate;
	uint32_t           output_channel_num;
	ttLibC_Frame      *frame;
} ttLibC_Resampler_SwresampleResampler_;

typedef ttLibC_Resampler_SwresampleResampler_ ttLibC_SwresampleResampler_;

static enum AVSampleFormat SwresampleResampler_getSampleFormat(
		ttLibC_Frame_Type type,
		uint32_t sub_type) {
	switch(type) {
	case frameType_pcmF32:
		{
			ttLibC_PcmF32_Type f32Type = (ttLibC_PcmF32_Type)sub_type;
			switch(f32Type) {
			case PcmF32Type_interleave:
				return AV_SAMPLE_FMT_FLT;
			case PcmF32Type_planar:
				return AV_SAMPLE_FMT_FLTP;
			default:
				return AV_SAMPLE_FMT_NONE;
			}
		}
		break;
	case frameType_pcmS16:
		{
			ttLibC_PcmS16_Type s16Type = (ttLibC_PcmS16_Type)sub_type;
			switch(s16Type) {
			case PcmS16Type_bigEndian:
				return AV_SAMPLE_FMT_NONE;
			case PcmS16Type_bigEndian_planar:
				return AV_SAMPLE_FMT_NONE;
			case PcmS16Type_littleEndian:
				return AV_SAMPLE_FMT_S16;
			case PcmS16Type_littleEndian_planar:
				return AV_SAMPLE_FMT_S16P;
			default:
				return AV_SAMPLE_FMT_NONE;
			}
		}
		break;
	default:
		return AV_SAMPLE_FMT_NONE;
	}
}

static bool SwresampleResampler_setupData(
		uint8_t     **data,
		ttLibC_Frame *frame) {
	switch(frame->type) {
	case frameType_pcmF32:
		{
			ttLibC_PcmF32 *pcm = (ttLibC_PcmF32 *)frame;
			switch(pcm->type) {
			case PcmF32Type_interleave:
				{
					data[0] = pcm->l_data;
					data[1] = NULL;
				}
				break;
			case PcmF32Type_planar:
				{
					data[0] = pcm->l_data;
					if(pcm->inherit_super.channel_num == 1) {
						data[1] = NULL;
					}
					else {
						data[1] = pcm->r_data;
					}
				}
				break;
			default:
				return false;
			}
		}
		break;
	case frameType_pcmS16:
		{
			ttLibC_PcmS16 *pcm = (ttLibC_PcmS16 *)frame;
			switch(pcm->type) {
			case PcmS16Type_littleEndian:
				{
					data[0] = pcm->l_data;
					data[1] = NULL;
				}
				break;
			case PcmS16Type_littleEndian_planar:
				{
					data[0] = pcm->l_data;
					if(pcm->inherit_super.channel_num == 1) {
						data[1] = NULL;
					}
					else {
						data[1] = pcm->r_data;
					}
				}
				break;
			default:
				return false;
			}
		}
		break;
	default:
		return false;
	}
	return true;
}

ttLibC_SwresampleResampler TT_ATTRIBUTE_API *ttLibC_SwresampleResampler_make(
		ttLibC_Frame_Type input_frame_type,
		uint32_t          input_sub_type,
		uint32_t          src_sample_rate,
		uint32_t          src_channel_num,
		ttLibC_Frame_Type output_frame_type,
		uint32_t          output_sub_type,
		uint32_t          target_sample_rate,
		uint32_t          target_channel_num) {
	ttLibC_SwresampleResampler_ *resampler = ttLibC_malloc(sizeof(ttLibC_SwresampleResampler_));
	if(resampler == NULL) {
		return NULL;
	}
	resampler->inherit_super.channel_num = target_channel_num;
	resampler->inherit_super.sample_rate = target_sample_rate;
	resampler->frame = NULL;
	resampler->input_type = input_frame_type;
	resampler->input_sub_type = input_sub_type;
	resampler->input_sample_rate = src_sample_rate;
	resampler->input_channel_num = src_channel_num;
	resampler->output_type = output_frame_type;
	resampler->output_sub_type = output_sub_type;
	resampler->output_sample_rate = target_sample_rate;
	resampler->output_channel_num = target_channel_num;

	enum AVSampleFormat input_format = SwresampleResampler_getSampleFormat(input_frame_type, input_sub_type);
	int64_t input_ch_layout = (src_channel_num == 1 ? AV_CH_LAYOUT_MONO : AV_CH_LAYOUT_STEREO);
	enum AVSampleFormat output_format = SwresampleResampler_getSampleFormat(output_frame_type, output_sub_type);
	int64_t output_ch_layout = (target_channel_num == 1 ? AV_CH_LAYOUT_MONO : AV_CH_LAYOUT_STEREO);

	resampler->convertCtx = swr_alloc_set_opts(
			NULL,
			output_ch_layout,
			output_format,
			target_sample_rate,
			input_ch_layout,
			input_format,
			src_sample_rate,
			0,
			NULL);
	swr_init(resampler->convertCtx);
	return (ttLibC_SwresampleResampler *)resampler;
}

bool TT_ATTRIBUTE_API ttLibC_SwresampleResampler_resample(
		ttLibC_SwresampleResampler *resampler,
		ttLibC_Frame *frame,
		ttLibC_getSwresampleFrameFunc callback,
		void *ptr) {
	ttLibC_SwresampleResampler_ *resampler_ = (ttLibC_SwresampleResampler_ *)resampler;
	if(resampler_ == NULL) {
		return false;
	}
	if(frame == NULL) {
		return true;
	}
	if(frame->type != resampler_->input_type) {
		ERR_PRINT("frame type is different from setting.");
		return false;
	}
	uint8_t *src_data[2];
	uint8_t *dst_data[2];

	ttLibC_Audio *audio = (ttLibC_Audio *)frame;
	int out_sample_num = swr_get_out_samples(resampler_->convertCtx, audio->sample_num);
	uint32_t memory_size = 0;
	switch(resampler_->output_type) {
	case frameType_pcmF32:
		memory_size = out_sample_num * resampler_->output_channel_num * 4;
		break;
	case frameType_pcmS16:
		memory_size = out_sample_num * resampler_->output_channel_num * 2;
		break;
	default:
		return false;
	}
	if(resampler_->frame == NULL || resampler_->frame->data_size < memory_size) {
		// in the case of reuse frame is missing or memory size is too small for resample.
		// realloc frame.
		if(resampler_->frame != NULL) {
			resampler_->frame->is_non_copy = true;
			ttLibC_free(resampler_->frame->data);
		}
		uint8_t *data = ttLibC_malloc(memory_size);
		if(data == NULL) {
			return false;
		}
		memset(data, 0, memory_size);
		switch(resampler_->output_type) {
		case frameType_pcmF32:
			{
				ttLibC_PcmF32 *pcm = NULL;
				switch(resampler_->output_sub_type) {
				case PcmF32Type_interleave:
					{
						pcm = ttLibC_PcmF32_make(
								(ttLibC_PcmF32 *)resampler_->frame,
								resampler_->output_sub_type,
								resampler_->output_sample_rate,
								out_sample_num,
								resampler_->output_channel_num,
								data,
								memory_size,
								data,
								memory_size,
								NULL,
								0,
								true,
								0,
								resampler_->output_sample_rate);
					}
					break;
				case PcmF32Type_planar:
					{
						if(resampler_->output_channel_num == 1) {
							pcm = ttLibC_PcmF32_make(
									(ttLibC_PcmF32 *)resampler_->frame,
									resampler_->output_sub_type,
									resampler_->output_sample_rate,
									out_sample_num,
									resampler_->output_channel_num,
									data,
									memory_size,
									data,
									memory_size,
									NULL,
									0,
									true,
									0,
									resampler_->output_sample_rate);
						}
						else {
							pcm = ttLibC_PcmF32_make(
									(ttLibC_PcmF32 *)resampler_->frame,
									resampler_->output_sub_type,
									resampler_->output_sample_rate,
									out_sample_num,
									resampler_->output_channel_num,
									data,
									memory_size,
									data,
									memory_size >> 1,
									data + (memory_size >> 1),
									memory_size >> 1,
									true,
									0,
									resampler_->output_sample_rate);
						}
					}
					break;
				default:
					{
						ttLibC_free(data);
					}
					return false;
				}
				if(pcm == NULL) {
					ttLibC_free(data);
					return false;
				}
				pcm->inherit_super.inherit_super.is_non_copy = false;
				resampler_->frame = (ttLibC_Frame *)pcm;
			}
			break;
		case frameType_pcmS16:
			{
				ttLibC_PcmS16 *pcm = NULL;
				switch(resampler_->output_sub_type) {
				case PcmS16Type_bigEndian:
				case PcmS16Type_bigEndian_planar:
					{
						ttLibC_free(data);
					}
					return false;
				case PcmS16Type_littleEndian:
					{
						pcm = ttLibC_PcmS16_make(
								(ttLibC_PcmS16 *)resampler_->frame,
								resampler_->output_sub_type,
								resampler_->output_sample_rate,
								out_sample_num,
								resampler_->output_channel_num,
								data,
								memory_size,
								data,
								memory_size,
								NULL,
								0,
								true,
								0,
								resampler_->output_sample_rate);
					}
					break;
				case PcmS16Type_littleEndian_planar:
					{
						if(resampler_->output_channel_num == 1) {
							pcm = ttLibC_PcmS16_make(
									(ttLibC_PcmS16 *)resampler_->frame,
									resampler_->output_sub_type,
									resampler_->output_sample_rate,
									out_sample_num,
									resampler_->output_channel_num,
									data,
									memory_size,
									data,
									memory_size,
									NULL,
									0,
									true,
									0,
									resampler_->output_sample_rate);
						}
						else {
							pcm = ttLibC_PcmS16_make(
									(ttLibC_PcmS16 *)resampler_->frame,
									resampler_->output_sub_type,
									resampler_->output_sample_rate,
									out_sample_num,
									resampler_->output_channel_num,
									data,
									memory_size,
									data,
									memory_size >> 1,
									data + (memory_size >> 1),
									memory_size >> 1,
									true,
									0,
									resampler_->output_sample_rate);
						}
					}
					break;
				default:
					{
						ttLibC_free(data);
					}
					return false;
				}
				if(pcm == NULL) {
					ttLibC_free(data);
					return false;
				}
				pcm->inherit_super.inherit_super.is_non_copy = false;
				resampler_->frame = (ttLibC_Frame *)pcm;
			}
			break;
		default:
			{
				ttLibC_free(data);
			}
			return false;
		}
	}

	// update pointer
	if(!SwresampleResampler_setupData(src_data, frame)) {
		return false;
	}
	if(!SwresampleResampler_setupData(dst_data, resampler_->frame)) {
		return false;
	}
	// do resample.
	int sample_num = swr_convert(
			resampler_->convertCtx,
			dst_data,
			out_sample_num,
			(const uint8_t **)src_data,
			audio->sample_num);
	if(sample_num == 0) {
		return false;
	}
	// update resampled pcm.
	// TODO need to update pts.
	switch(resampler_->frame->type) {
	case frameType_pcmF32:
		{
			ttLibC_PcmF32 *pcm = (ttLibC_PcmF32 *)resampler_->frame;
			pcm->inherit_super.sample_num = sample_num;
			pcm->inherit_super.inherit_super.buffer_size = sample_num * 4 * pcm->inherit_super.channel_num;
			switch(pcm->type) {
			case PcmF32Type_interleave:
				pcm->l_stride = sample_num * 4 * pcm->inherit_super.channel_num;
				break;
			case PcmF32Type_planar:
				pcm->l_stride = sample_num * 4;
				if(pcm->inherit_super.channel_num == 2) {
					pcm->r_stride = sample_num * 4;
				}
				break;
			default:
				return false;
			}
		}
		break;
	case frameType_pcmS16:
		{
			ttLibC_PcmS16 *pcm = (ttLibC_PcmS16 *)resampler_->frame;
			pcm->inherit_super.sample_num = sample_num;
			pcm->inherit_super.inherit_super.buffer_size = sample_num * 2 * pcm->inherit_super.channel_num;
			switch(pcm->type) {
			case PcmS16Type_bigEndian:
			case PcmS16Type_bigEndian_planar:
				return false;
			case PcmS16Type_littleEndian:
				pcm->l_stride = sample_num * 2 * pcm->inherit_super.channel_num;
				break;
			case PcmS16Type_littleEndian_planar:
				pcm->l_stride = sample_num * 2;
				if(pcm->inherit_super.channel_num == 2) {
					pcm->r_stride = sample_num * 2;
				}
				break;
			default:
				return false;
			}
		}
		break;
	default:
		return false;
	}
	resampler_->frame->id = frame->id;
	if(callback != NULL) {
		if(!callback(ptr, resampler_->frame)) {
			return false;
		}
	}
	return true;
}

void TT_ATTRIBUTE_API ttLibC_SwresampleResampler_close(ttLibC_SwresampleResampler **resampler) {
	ttLibC_SwresampleResampler_ *target = (ttLibC_SwresampleResampler_ *)*resampler;
	if(target == NULL) {
		return;
	}
	swr_close(target->convertCtx);
	swr_free(&target->convertCtx);
	ttLibC_Frame_close(&target->frame);
	ttLibC_free(target);
	*resampler = NULL;
}

#endif
