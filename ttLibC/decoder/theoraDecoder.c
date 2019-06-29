/**
 * @file   theoraDecoder.c
 * @brief  decode theora with libtheora.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/04/08
 */

#ifdef __ENABLE_THEORA__

#include "theoraDecoder.h"
#include "../ttLibC_predef.h"
#include "../allocator.h"
#include "../_log.h"
#include <theora/theoradec.h>
#include <string.h>

typedef struct ttLibC_Decoder_TheoraDecoder_ {
	ttLibC_TheoraDecoder inherit_super;
	th_dec_ctx *ctx;
	th_setup_info *ts;
	th_info ti;
	th_comment tc;
	th_ycbcr_buffer image_buffer;
	uint64_t counter;
	ttLibC_Yuv420 *yuv420;
	bool is_identification_done;
	bool is_comment_done;
	bool is_setup_done;
} ttLibC_Decoder_TheoraDecoder_;

typedef ttLibC_Decoder_TheoraDecoder_ ttLibC_TheoraDecoder_;

ttLibC_TheoraDecoder TT_VISIBILITY_DEFAULT *ttLibC_TheoraDecoder_make() {
	return ttLibC_TheoraDecoder_makeWithInfo(NULL);
}

ttLibC_TheoraDecoder TT_VISIBILITY_DEFAULT *ttLibC_TheoraDecoder_makeWithInfo(void *ti) {
	ttLibC_TheoraDecoder_ *decoder = ttLibC_malloc(sizeof(ttLibC_TheoraDecoder_));
	if(decoder == NULL) {
		return NULL;
	}
	decoder->ctx = NULL;
	decoder->ts = NULL;
	decoder->yuv420 = NULL;
	decoder->is_identification_done = false;
	decoder->is_comment_done = false;
	decoder->is_setup_done = false;
	decoder->counter = 0;
	th_info_init(&decoder->ti);
	if(ti != NULL) {
		memcpy(&decoder->ti, ti, sizeof(th_info));
	}
	th_comment_init(&decoder->tc);
	return (ttLibC_TheoraDecoder *)decoder;
}

void TT_VISIBILITY_DEFAULT *ttLibC_TheoraDecoder_refNativeDecodeContext(ttLibC_TheoraDecoder *decoder) {
	ttLibC_TheoraDecoder_ *decoder_ = (ttLibC_TheoraDecoder_ *)decoder;
	if(decoder_ == NULL) {
		return NULL;
	}
	return decoder_->ctx;
}

bool TT_VISIBILITY_DEFAULT ttLibC_TheoraDecoder_decode(
		ttLibC_TheoraDecoder *decoder,
		ttLibC_Theora *theora,
		ttLibC_TheoraDecodeFunc callback,
		void *ptr) {
	ttLibC_TheoraDecoder_ *decoder_ = (ttLibC_TheoraDecoder_ *)decoder;
	if(decoder_ == NULL) {
		return false;
	}
	if(theora == NULL) {
		return true;
	}
	ogg_packet op;
	op.b_o_s = 0;
	op.e_o_s = 0;
	op.packetno = decoder_->counter ++;
	op.granulepos = theora->granule_pos;
	op.packet = theora->inherit_super.inherit_super.data;
	op.bytes = theora->inherit_super.inherit_super.buffer_size;
	if(!decoder_->is_identification_done) {
		if(theora->type != TheoraType_identificationHeaderDecodeFrame) {
			ERR_PRINT("need identification frame.");
			return false;
		}
		op.b_o_s = 1;
		int res = th_decode_headerin(&decoder_->ti, &decoder_->tc, &decoder_->ts, &op);
		if(res == TH_EBADHEADER) {
			LOG_PRINT("need more data or broken.");
		}
		else if(res > 0) {
			LOG_PRINT("header read complete.");
		}
		decoder_->is_identification_done = true;
		return true;
	}
	if(!decoder_->is_comment_done) {
		if(theora->type != TheoraType_commentHeaderFrame) {
			ERR_PRINT("need comment frame.");
			return false;
		}
		int res = th_decode_headerin(&decoder_->ti, &decoder_->tc, &decoder_->ts, &op);
		if(res == TH_EBADHEADER) {
			LOG_PRINT("need more data or broken.");
		}
		else if(res > 0) {
			LOG_PRINT("header read complete.");
		}
		decoder_->is_comment_done = true;
		return true;
	}
	if(!decoder_->is_setup_done) {
		if(theora->type != TheoraType_setupHeaderFrame) {
			ERR_PRINT("need setup frame.");
			return false;
		}
		int res = th_decode_headerin(&decoder_->ti, &decoder_->tc, &decoder_->ts, &op);
		if(res == TH_EBADHEADER) {
			LOG_PRINT("need more data or broken.");
		}
		else if(res > 0) {
			LOG_PRINT("header read complete.");
		}
		decoder_->is_setup_done = true;
		decoder_->ctx = th_decode_alloc(&decoder_->ti, decoder_->ts);
		return true;
	}
	switch(theora->type) {
	case TheoraType_innerFrame:
	case TheoraType_intraFrame:
		break;
	default:
		ERR_PRINT("find unexpected frame.");
		return false;
	}
	ogg_int64_t videoBuf_granulePos = 0;
	int result = th_decode_packetin(decoder_->ctx, &op, &videoBuf_granulePos);
	if(result == TH_DUPFRAME) { // skip dup frame.
		return true;
	}
	if(result != 0) {
		ERR_PRINT("error on decode. :%d %x", result, result);
		return false;
	}
	result = th_decode_ycbcr_out(decoder_->ctx, decoder_->image_buffer);
	if(result != 0) {
		ERR_PRINT("failed to get ycbcr image.");
		return false;
	}
	ttLibC_Yuv420 *y = ttLibC_Yuv420_make(
			decoder_->yuv420,
			Yuv420Type_planar,
			theora->inherit_super.width,
			theora->inherit_super.height,
			NULL,
			0,
			decoder_->image_buffer[0].data,
			decoder_->image_buffer[0].stride,
			decoder_->image_buffer[1].data,
			decoder_->image_buffer[1].stride,
			decoder_->image_buffer[2].data,
			decoder_->image_buffer[2].stride,
			true,
			theora->inherit_super.inherit_super.pts,
			theora->inherit_super.inherit_super.timebase);
	if(y == NULL) {
		ERR_PRINT("failed to make yuv buffer.");
		return false;
	}
	decoder_->yuv420 = y;
	decoder_->yuv420->inherit_super.inherit_super.id = theora->inherit_super.inherit_super.id;
	if(callback != NULL) {
		if(!callback(ptr, decoder_->yuv420)) {
			return false;
		}
	}
	return true;
}

void TT_VISIBILITY_DEFAULT ttLibC_TheoraDecoder_close(ttLibC_TheoraDecoder **decoder) {
	ttLibC_TheoraDecoder_ *target = (ttLibC_TheoraDecoder_ *)*decoder;
	if(target == NULL) {
		return;
	}
	if(target->ts != NULL) {
		th_setup_free(target->ts);
	}
	th_info_clear(&target->ti);
	th_comment_clear(&target->tc);
	if(target->ctx != NULL) {
		th_decode_free(target->ctx);
	}
	ttLibC_Yuv420_close(&target->yuv420);
	ttLibC_free(target);
	*decoder = NULL;
}

#endif
