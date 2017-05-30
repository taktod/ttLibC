/**
 * @file   vorbisDecoder.c
 * @brief  decode vorbis with libvorbis
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/04/08
 */

#ifdef __ENABLE_VORBIS_DECODE__

#include "vorbisDecoder.h"
#include "../allocator.h"
#include "../_log.h"
#include <vorbis/codec.h>
#include <string.h>

typedef struct ttLibC_Decoder_VorbisDecoder_ {
	ttLibC_VorbisDecoder inherit_super;
	vorbis_info      vi;
	vorbis_comment   vc;
	vorbis_dsp_state vd;
	vorbis_block     vb;
	ttLibC_PcmF32 *pcm;
	bool is_identification_done;
	bool is_comment_done;
	bool is_setup_done;
} ttLibC_Decoder_VorbisDecoder_;

typedef ttLibC_Decoder_VorbisDecoder_ ttLibC_VorbisDecoder_;

ttLibC_VorbisDecoder *ttLibC_VorbisDecoder_make() {
	ttLibC_VorbisDecoder_ *decoder = ttLibC_malloc(sizeof(ttLibC_VorbisDecoder_));
	if(decoder == NULL) {
		return NULL;
	}
	vorbis_info_init(&decoder->vi);
	vorbis_comment_init(&decoder->vc);
	decoder->pcm = NULL;
	decoder->is_identification_done = false;
	decoder->is_comment_done = false;
	decoder->is_setup_done = false;
	return (ttLibC_VorbisDecoder *)decoder;
}

ttLibC_VorbisDecoder *ttLibC_VorbisDecoder_makeWithInfo(void *vi) {
	(void)vi;
	return NULL;
}

bool ttLibC_VorbisDecoder_decode(
		ttLibC_VorbisDecoder *decoder,
		ttLibC_Vorbis *vorbis,
		ttLibC_VorbisDecodeFunc callback,
		void *ptr) {
	ttLibC_VorbisDecoder_ *decoder_ = (ttLibC_VorbisDecoder_ *)decoder;
	if(decoder_ == NULL) {
		return false;
	}
	if(vorbis == NULL) {
		return true;
	}
	ogg_packet op;
	op.b_o_s = 0;
	op.bytes = vorbis->inherit_super.inherit_super.buffer_size;
	op.e_o_s = 0;
	op.granulepos = 0;
	op.packet = vorbis->inherit_super.inherit_super.data;
	op.packetno = 0;
	if(!decoder_->is_identification_done) {
		op.b_o_s = 1;
		if(vorbis_synthesis_headerin(&decoder_->vi, &decoder_->vc, &op) < 0) {
			ERR_PRINT("failed to read vorbis header.");
			return false;
		}
		decoder_->is_identification_done = true;
		return true;
	}
	if(!decoder_->is_comment_done) {
		if(vorbis_synthesis_headerin(&decoder_->vi, &decoder_->vc, &op) < 0) {
			ERR_PRINT("failed to read vorbis header.");
			return false;
		}
		decoder_->is_comment_done = true;
		return true;
	}
	if(!decoder_->is_setup_done) {
		if(vorbis_synthesis_headerin(&decoder_->vi, &decoder_->vc, &op) < 0) {
			ERR_PRINT("failed to read vorbis header.");
			return false;
		}
		decoder_->is_setup_done = true;
		if(vorbis_synthesis_init(&decoder_->vd, &decoder_->vi) != 0) {
			ERR_PRINT("failed to setup decoder.");
			return false;
		}
		vorbis_block_init(&decoder_->vd, &decoder_->vb);
		return true;
	}
	op.granulepos = vorbis->inherit_super.inherit_super.pts;
	if(vorbis_synthesis(&decoder_->vb, &op) != 0) {
		ERR_PRINT("failed to analyze data.");
		return false;
	}
	vorbis_synthesis_blockin(&decoder_->vd, &decoder_->vb);
	float **pcm;
	int samples;
	while((samples = vorbis_synthesis_pcmout(&decoder_->vd, &pcm)) > 0) {
		// now make pcm and callback it.
		ttLibC_PcmF32 *p = ttLibC_PcmF32_make(
				decoder_->pcm,
				PcmF32Type_interleave,
				decoder_->vi.rate,
				samples,
				decoder_->vi.channels,
				NULL,
				0,
				pcm[0],
				samples * 4 * decoder_->vi.channels,
/*				pcm[1],
				samples * 4,*/
				NULL,
				0,
				true,
				vorbis->inherit_super.inherit_super.pts,
				vorbis->inherit_super.inherit_super.timebase);
		bool is_success = (p != NULL);
		if(is_success) {
			decoder_->pcm = p;
			if(callback != NULL) {
				if(!callback(ptr, decoder_->pcm)) {
					is_success = false;
				}
			}
		}
		vorbis_synthesis_read(&decoder_->vd, samples);
		if(!is_success) {
			return false;
		}
	}
	return true;
}

void ttLibC_VorbisDecoder_close(ttLibC_VorbisDecoder **decoder) {
	ttLibC_VorbisDecoder_ *target = (ttLibC_VorbisDecoder_ *)*decoder;
	if(target == NULL) {
		return;
	}
	if(target->is_identification_done
	&& target->is_comment_done
	&& target->is_setup_done) {
		vorbis_block_clear(&target->vb);
		vorbis_dsp_clear(&target->vd);
	}
	vorbis_comment_clear(&target->vc);
	vorbis_info_clear(&target->vi);
	ttLibC_PcmF32_close(&target->pcm);
	ttLibC_free(target);
	*decoder = NULL;
}

#endif
