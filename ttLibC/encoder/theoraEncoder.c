/**
 * @file   theoraEncoder.c
 * @brief  encode theora with libtheora.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/04/04
 */
#ifdef __ENABLE_THEORA__

#include "theoraEncoder.h"
#include "../allocator.h"
#include "../_log.h"
#include <theora/theoraenc.h>
#include <string.h>

/**
 * structure defail encoder definition.
 */
typedef struct ttLibC_Encoder_TheoraEncoder_ {
	ttLibC_TheoraEncoder inherit_super;
	th_enc_ctx *ctx;
	th_info ti;
	th_comment tc;
	th_ycbcr_buffer image_buffer;
	ttLibC_Theora *theora;
	bool is_first_access;
} ttLibC_Encoder_TheoraEncoder_;

typedef ttLibC_Encoder_TheoraEncoder_ ttLibC_TheoraEncoder_;

/**
 * make theora encoder.
 */
ttLibC_TheoraEncoder TT_ATTRIBUTE_API *ttLibC_TheoraEncoder_make(
		uint32_t width,
		uint32_t height) {
	return ttLibC_TheoraEncoder_make_ex(width, height, 0, 320000, 15);
}

/**
 * make theora encoder with special values.
 * @param width
 * @param height
 * @param quality 0-63
 * @param bitrate in bit/sec
 * @param key_frame_interval 1 - 31
 * @return theoraEncoder object.
 */
ttLibC_TheoraEncoder TT_ATTRIBUTE_API *ttLibC_TheoraEncoder_make_ex(
		uint32_t width,
		uint32_t height,
		uint32_t quality,
		uint32_t bitrate,
		uint32_t key_frame_interval) {
	th_info ti;
//	th_comment tc;
	th_info_init(&ti);
	ti.frame_width  = (((((width)  - 1) >> 4) + 1) << 4);
	ti.frame_height = (((((height) - 1) >> 4) + 1) << 4);
//	ti.frame_width = (width % 16 == 0) ? width : ((int)(width / 16) + 1) * 16;
//	ti.frame_height = (height % 16 == 0) ? height : ((int)(height / 16) + 1) * 16;
	ti.pic_width = width;
	ti.pic_height = height;
	ti.pic_x = 0;
	ti.pic_y = 0;
	ti.fps_numerator = 15;
	ti.fps_denominator = 1;
	ti.aspect_numerator = 1;
	ti.aspect_denominator = 1;
	ti.pixel_fmt = TH_PF_420; // only support yuv420.
	ti.target_bitrate = bitrate;
	ti.quality = quality;
	ti.keyframe_granule_shift = key_frame_interval;
	ttLibC_TheoraEncoder *encoder = ttLibC_TheoraEncoder_makeWithInfo(&ti);
	th_info_clear(&ti);
	return encoder;
}

ttLibC_TheoraEncoder TT_ATTRIBUTE_API *ttLibC_TheoraEncoder_makeWithInfo(void *ti) {
	ttLibC_TheoraEncoder_ *encoder = ttLibC_malloc(sizeof(ttLibC_TheoraEncoder_));
	if(encoder == NULL) {
		return NULL;
	}
	th_info_init(&encoder->ti);
	th_comment_init(&encoder->tc);
	memcpy(&encoder->ti, ti, sizeof(th_info));
	// make theora comment information.
	th_comment_add_tag(&encoder->tc, "ENCODER", "ttLibC_TheoraEncoder");
	// try to make context.
	encoder->ctx = th_encode_alloc(&encoder->ti);
	if(encoder->ctx == NULL) {
		ERR_PRINT("failed to make theora context.");
		th_info_clear(&encoder->ti);
		th_comment_clear(&encoder->tc);
		ttLibC_free(encoder);
		return NULL;
	}
	encoder->theora               = NULL;
	encoder->is_first_access      = true;
	encoder->inherit_super.width  = encoder->ti.pic_width;
	encoder->inherit_super.height = encoder->ti.pic_height;
	return (ttLibC_TheoraEncoder *)encoder;
}

void TT_ATTRIBUTE_API *ttLibC_TheoraEncoder_refNativeEncodeContext(ttLibC_TheoraEncoder *encoder) {
	ttLibC_TheoraEncoder_ *encoder_ = (ttLibC_TheoraEncoder_ *)encoder;
	if(encoder_ == NULL) {
		return NULL;
	}
	return encoder_->ctx;
}

bool TT_ATTRIBUTE_API ttLibC_TheoraEncoder_encode(
		ttLibC_TheoraEncoder *encoder,
		ttLibC_Yuv420 *yuv420,
		ttLibC_TheoraEncodeFunc callback,
		void *ptr) {
	ttLibC_TheoraEncoder_ *encoder_ = (ttLibC_TheoraEncoder_ *)encoder;
	if(encoder_ == NULL) {
		return false;
	}
	ogg_packet op;
	// do for identification, comment, and setup code.
	if(encoder_->is_first_access) {
		while(th_encode_flushheader(encoder_->ctx, &encoder_->tc, &op)) {
			// try to make theora frame.
			ttLibC_Theora *t = ttLibC_Theora_getFrame(encoder_->theora, op.packet, op.bytes, true, 0, yuv420->inherit_super.inherit_super.timebase);
			if(t == NULL) {
				return false;
			}
			encoder_->theora = t;
			if(callback != NULL) {
				if(!callback(ptr, encoder_->theora)) {
					return false;
				}
			}
		}
		encoder_->is_first_access = false;
	}
	if(yuv420 == NULL) {
		return true;
	}
	switch(yuv420->type) {
	case Yuv420Type_planar:
	case Yvu420Type_planar:
		break;
	case Yuv420Type_semiPlanar:
	case Yvu420Type_semiPlanar:
		ERR_PRINT("only support planar.");
		return false;
	}
	uint32_t width = yuv420->inherit_super.width;
	uint32_t height = yuv420->inherit_super.height;
	width  = (((((width)  - 1) >> 4) + 1) << 4);
	height = (((((height) - 1) >> 4) + 1) << 4);
//	width = (width % 16 == 0) ? width : ((int)(width / 16) + 1) * 16;
//	height = (height % 16 == 0) ? height : ((int)(height / 16) + 1) * 16;
	// y
	encoder_->image_buffer[0].width  = width;
	encoder_->image_buffer[0].height = height;
	encoder_->image_buffer[0].stride = yuv420->y_stride;
	encoder_->image_buffer[0].data   = yuv420->y_data;
	// cb
	encoder_->image_buffer[1].width  = (width  + 1) >> 1;
	encoder_->image_buffer[1].height = (height + 1) >> 1;
	encoder_->image_buffer[1].stride = yuv420->u_stride;
	encoder_->image_buffer[1].data   = yuv420->u_data;
	// cr
	encoder_->image_buffer[2].width  = (width  + 1) >> 1;
	encoder_->image_buffer[2].height = (height + 1) >> 1;
	encoder_->image_buffer[2].stride = yuv420->v_stride;
	encoder_->image_buffer[2].data   = yuv420->v_data;

	int result = th_encode_ycbcr_in(encoder_->ctx, encoder_->image_buffer);
	if(result != 0) {
		ERR_PRINT("failed to register yuv image.");
		return false;
	}
	result = th_encode_packetout(encoder_->ctx, 0, &op);
	if(result != 1) {
		ERR_PRINT("failed to get encode data.");
		return false;
	}
	// use original yuv time information.
	ttLibC_Theora *theora = ttLibC_Theora_getFrame(
			encoder_->theora,
			op.packet,
			op.bytes,
			true,
			yuv420->inherit_super.inherit_super.pts,
			yuv420->inherit_super.inherit_super.timebase);
	if(theora == NULL) {
		return false;
	}
	// put granule position.
	theora->granule_pos = op.granulepos;
	encoder_->theora = theora;
	if(!callback(ptr, encoder_->theora)) {
		return false;
	}
	return true;
}

void TT_ATTRIBUTE_API ttLibC_TheoraEncoder_close(ttLibC_TheoraEncoder **encoder) {
	ttLibC_TheoraEncoder_ *target = (ttLibC_TheoraEncoder_ *)*encoder;
	if(target == NULL) {
		return;
	}
	th_info_clear(&target->ti);
	th_comment_clear(&target->tc);
	th_encode_free(target->ctx);
	target->ctx = NULL;
	ttLibC_Theora_close(&target->theora);
	ttLibC_free(target);
	*encoder = NULL;
}

#endif
