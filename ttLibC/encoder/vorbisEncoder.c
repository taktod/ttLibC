/**
 * @file   vorbisEncoder.c
 * @brief  encode vorbis with libvorbis
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/04/08
 */
#ifdef __ENABLE_VORBIS_ENCODE__

#include "vorbisEncoder.h"
#include "../ttLibC_predef.h"
#include "../allocator.h"
#include "../_log.h"
#include <vorbis/vorbisenc.h>
#include <string.h>
#include "../frame/audio/pcmf32.h"
#include "../frame/audio/pcms16.h"
#include "../util/hexUtil.h"

typedef struct ttLibC_Encoder_VorbisEncoder_{
	ttLibC_VorbisEncoder inherit_super;
	vorbis_info      vi;
	vorbis_comment   vc;
	vorbis_dsp_state vd;
	vorbis_block     vb;
	ttLibC_Vorbis *vorbis;
	bool is_first_access;
} ttLibC_Encoder_VorbisEncoder_;

typedef ttLibC_Encoder_VorbisEncoder_ ttLibC_VorbisEncoder_;

ttLibC_VorbisEncoder TT_VISIBILITY_DEFAULT *ttLibC_VorbisEncoder_make(
		uint32_t sample_rate,
		uint32_t channel_num) {
	ttLibC_VorbisEncoder_ *encoder = ttLibC_malloc(sizeof(ttLibC_VorbisEncoder_));
	if(encoder == NULL) {
		return NULL;
	}
	vorbis_info_init(&encoder->vi);
	int ret = vorbis_encode_init_vbr(&encoder->vi, channel_num, sample_rate, 0.4); // quality 0.4 for instance.
	if(ret != 0) {
		ERR_PRINT("failed to init vorbis");
		vorbis_info_clear(&encoder->vi);
		ttLibC_free(encoder);
		return NULL;
	}
	vorbis_comment_init(&encoder->vc);
	vorbis_comment_add_tag(&encoder->vc, "ENCODER", "ttLibC_VorbisEncoder");

	vorbis_analysis_init(&encoder->vd, &encoder->vi);
	vorbis_block_init(&encoder->vd, &encoder->vb);
	encoder->is_first_access = true;
	encoder->vorbis = NULL;
	encoder->inherit_super.sample_rate = sample_rate;
	encoder->inherit_super.channel_num = channel_num;
	return (ttLibC_VorbisEncoder *)encoder;
}

ttLibC_VorbisEncoder TT_VISIBILITY_HIDDEN *ttLibC_VorbisEncoder_makeWithInfo(void *vi) {
	(void)vi;
	return NULL;
}

void TT_VISIBILITY_HIDDEN *ttLibC_VorbisEncoder_refNativeEncodeContext(ttLibC_VorbisEncoder *encoder) {
	(void)encoder;
	return NULL;
}

bool TT_VISIBILITY_DEFAULT ttLibC_VorbisEncoder_encode(
		ttLibC_VorbisEncoder *encoder,
		ttLibC_Audio *pcm,
		ttLibC_VorbisEncodeFunc callback,
		void *ptr) {
	ttLibC_VorbisEncoder_ *encoder_ = (ttLibC_VorbisEncoder_ *)encoder;
	if(encoder_ == NULL) {
		return false;
	}
	ogg_packet op;
	if(encoder_->is_first_access) {
		// need to make identification, comment, setup_code frame.
		ogg_packet header, header_comment, header_code;
		vorbis_analysis_headerout(
				&encoder_->vd,
				&encoder_->vc,
				&header,
				&header_comment,
				&header_code);
		ttLibC_Vorbis *v;
		v = ttLibC_Vorbis_getFrame(
				encoder_->vorbis,
				header.packet,
				header.bytes,
				true,
				0,
				encoder_->inherit_super.sample_rate);
		if(v == NULL) {
			return false;
		}
		encoder_->vorbis = v;
		if(callback != NULL) {
			if(!callback(ptr, encoder_->vorbis)) {
				return false;
			}
		}
		v = ttLibC_Vorbis_getFrame(
				encoder_->vorbis,
				header_comment.packet,
				header_comment.bytes,
				true,
				0,
				encoder_->inherit_super.sample_rate);
		if(v == NULL) {
			return false;
		}
		encoder_->vorbis = v;
		if(callback != NULL) {
			if(!callback(ptr, encoder_->vorbis)) {
				return false;
			}
		}
		v = ttLibC_Vorbis_getFrame(
				encoder_->vorbis,
				header_code.packet,
				header_code.bytes,
				true,
				0,
				encoder_->inherit_super.sample_rate);
		if(v == NULL) {
			return false;
		}
		encoder_->vorbis = v;
		if(callback != NULL) {
			if(!callback(ptr, encoder_->vorbis)) {
				return false;
			}
		}
		encoder_->is_first_access = false;
	}
	// encoder for input frame.
	float **buffer = vorbis_analysis_buffer(&encoder_->vd, pcm->sample_num);
	uint32_t i = 0;
	// only for interleave
	switch(pcm->inherit_super.type) {
	case frameType_pcmF32:
		{
			ttLibC_PcmF32 *pcmf = (ttLibC_PcmF32 *)pcm;
			switch(pcm->channel_num) {
			case 1:
				{
					float *data = (float *)pcmf->l_data;
					for(i = 0;i < pcm->sample_num; ++ i) {
						buffer[0][i] = *data;
						++ data;
					}
				}
				break;
			case 2:
				{
					switch(pcmf->type) {
					default:
					case PcmF32Type_interleave:
						{
							float *data = (float *)pcmf->l_data;
							for(i = 0;i < pcm->sample_num; ++ i) {
								buffer[0][i] = *data;
								++ data;
								buffer[1][i] = *data;
								++ data;
							}
						}
						break;
					case PcmF32Type_planar:
						{
							float *l_data = (float *)pcmf->l_data;
							float *r_data = (float *)pcmf->r_data;
							for(i = 0;i < pcm->sample_num; ++ i) {
								buffer[0][i] = *l_data;
								++ l_data;
								buffer[1][i] = *r_data;
								++ r_data;
							}
						}
						break;
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
			ttLibC_PcmS16 *pcms = (ttLibC_PcmS16 *)pcm;
			switch(pcm->channel_num) {
			case 1:
				{
					int16_t *data = (int16_t *)pcms->l_data;
					for(i = 0;i < pcm->sample_num; ++ i) {
						buffer[0][i] = (*data) / 32768.f;
						++ data;
					}
				}
				break;
			case 2:
				{
					switch(pcms->type) {
					case PcmS16Type_bigEndian:
					case PcmS16Type_bigEndian_planar:
					default:
						ERR_PRINT("not support bigendign.");
						return false;
					case PcmS16Type_littleEndian:
						{
							int16_t *data = (int16_t *)pcms->l_data;
							for(i = 0;i < pcm->sample_num; ++ i) {
								buffer[0][i] = (*data) / 32768.f;
								++ data;
								buffer[1][i] = (*data) / 32768.f;
								++ data;
							}
						}
						break;
					case PcmS16Type_littleEndian_planar:
						{
							int16_t *l_data = (int16_t *)pcms->l_data;
							int16_t *r_data = (int16_t *)pcms->r_data;
							for(i = 0;i < pcm->sample_num; ++ i) {
								buffer[0][i] = (*l_data) / 32768.f;
								++ l_data;
								buffer[1][i] = (*r_data) / 32768.f;
								++ r_data;
							}
						}
						break;
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
	vorbis_analysis_wrote(&encoder_->vd, i);
	while(vorbis_analysis_blockout(&encoder_->vd, &encoder_->vb) == 1) {
		vorbis_analysis(&encoder_->vb, NULL);
		vorbis_bitrate_addblock(&encoder_->vb);
		while(vorbis_bitrate_flushpacket(&encoder_->vd, &op)) {
			ttLibC_Vorbis *v;
			v = ttLibC_Vorbis_getFrame(
					encoder_->vorbis,
					op.packet,
					op.bytes,
					true,
					op.granulepos,
					encoder_->inherit_super.sample_rate);
			if(v == NULL) {
				return false;
			}
			encoder_->vorbis = v;
			if(callback != NULL) {
				if(!callback(ptr, encoder_->vorbis)) {
					return false;
				}
			}
		}
	}
	return true;
}

void TT_VISIBILITY_DEFAULT ttLibC_VorbisEncoder_close(ttLibC_VorbisEncoder **encoder) {
	ttLibC_VorbisEncoder_ *target = (ttLibC_VorbisEncoder_ *)*encoder;
	if(target == NULL) {
		return;
	}
	vorbis_block_clear(&target->vb);
	vorbis_dsp_clear(&target->vd);
	vorbis_comment_clear(&target->vc);
	vorbis_info_clear(&target->vi);
	ttLibC_Vorbis_close(&target->vorbis);
	ttLibC_free(target);
	*encoder = NULL;
}

#endif
