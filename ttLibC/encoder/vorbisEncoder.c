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
#include "../allocator.h"
#include "../log.h"
#include <vorbis/vorbisenc.h>
#include <string.h>
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

ttLibC_VorbisEncoder *ttLibC_VorbisEncoder_make(
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

ttLibC_VorbisEncoder *ttLibC_VorbisEncoder_makeWithInfo(void *vi) {
	return NULL;
}

void *ttLibC_VorbisEncoder_refNativeEncodeContext(ttLibC_VorbisEncoder *encoder) {
	return NULL;
}

bool ttLibC_VorbisEncoder_encode(
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
		v = ttLibC_Vorbis_make(
				encoder_->vorbis,
				VorbisType_identification,
				encoder_->inherit_super.sample_rate,
				0,
				encoder_->inherit_super.channel_num,
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

		v = ttLibC_Vorbis_make(
				encoder_->vorbis,
				VorbisType_comment,
				encoder_->inherit_super.sample_rate,
				0,
				encoder_->inherit_super.channel_num,
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

		v = ttLibC_Vorbis_make(
				encoder_->vorbis,
				VorbisType_setup,
				encoder_->inherit_super.sample_rate,
				0,
				encoder_->inherit_super.channel_num,
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
	int i = 0;
	switch(pcm->inherit_super.type) {
	case frameType_pcmF32:
		{
			float *data = (float *)pcm->inherit_super.data;
			switch(pcm->channel_num) {
			case 1:
				{
					for(i = 0;i < pcm->sample_num; ++ i) {
						buffer[0][i] = *data;
						++ data;
					}
				}
				break;
			case 2:
				{
					for(i = 0;i < pcm->sample_num; ++ i) {
						buffer[0][i] = *data;
						++ data;
						buffer[1][i] = *data;
						++ data;
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
			int16_t *data = (int16_t *)pcm->inherit_super.data;
			switch(pcm->channel_num) {
			case 1:
				{
					for(i = 0;i < pcm->sample_num; ++ i) {
						buffer[0][i] = (*data) / 32768.f;
						++ data;
					}
				}
				break;
			case 2:
				{
					for(i = 0;i < pcm->sample_num; ++ i) {
						buffer[0][i] = (*data) / 32768.f;
						++ data;
						buffer[1][i] = (*data) / 32768.f;
						++ data;
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
			v = ttLibC_Vorbis_make(
					encoder_->vorbis,
					VorbisType_frame,
					encoder_->inherit_super.sample_rate,
					encoder_->vd.granulepos - op.granulepos,
					encoder_->inherit_super.channel_num,
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

void ttLibC_VorbisEncoder_close(ttLibC_VorbisEncoder **encoder) {
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
