/*
 * @file   mp3lameEncoder.c
 * @brief  encode pcms16 with mp3lame.
 *
 * this code is under LGPLv3 license.
 *
 * @author taktod
 * @date   2015/07/21
 */

#ifdef __ENABLE_MP3LAME_ENCODE__

#include "mp3lameEncoder.h"
#include "../log.h"
#include "../allocator.h"
#include <stdint.h>
#include <stdlib.h>
#include <lame/lame.h>

/*
 * mp3lame encoder detail definition.
 */
typedef struct {
	/** inherit data from ttLibC_Mp3lameEncoder */
	ttLibC_Mp3lameEncoder inherit_super;
	/** global flags of lame. */
	lame_global_flags *gflags;
	/** output mp3 object for reuse. */
	ttLibC_Mp3 *mp3;
	/** data buffer for encoded data. */
	uint8_t *data;
	/** size of data */
	size_t data_size;
	/** position of append. */
	size_t data_start_pos;
	/** pts data for next mp3 object. */
	uint64_t pts;
	bool is_pts_initialized;
} ttLibC_Encoder_Mp3lameEncoder_;

typedef ttLibC_Encoder_Mp3lameEncoder_ ttLibC_Mp3lameEncoder_;

/*
 * make mp3lame encoder (cbr)
 * @param sample_rate desire sample_rate
 * @param channel_num desire channel_num
 * @param quality     target quality from 0 to 10. 2:near best 5:good fast 7:ok,very fast.
 * @return mp3lame encoder object.
 */
ttLibC_Mp3lameEncoder *ttLibC_Mp3lameEncoder_make(
		uint32_t sample_rate,
		uint32_t channel_num,
		uint32_t quality) {
	ttLibC_Mp3lameEncoder_ *encoder = ttLibC_malloc(sizeof(ttLibC_Mp3lameEncoder_));
	if(encoder == NULL) {
		ERR_PRINT("failed to allocate memory for encoder.");
		return NULL;
	}
	encoder->gflags = lame_init();
	if(encoder->gflags == NULL) {
		ERR_PRINT("failed to initialize lame.");
		ttLibC_free(encoder);
		return NULL;
	}
	lame_set_num_channels(encoder->gflags, channel_num);
	switch(channel_num) {
	default:
		ERR_PRINT("channel is 1:mono or 2:stereo");
		lame_close(encoder->gflags);
		ttLibC_free(encoder);
		return NULL;
	case 1:
		lame_set_mode(encoder->gflags, MONO);
		break;
	case 2:
		lame_set_mode(encoder->gflags, STEREO);
		break;
	}
	lame_set_in_samplerate(encoder->gflags, sample_rate);
	lame_set_out_samplerate(encoder->gflags, sample_rate);
	lame_set_quality(encoder->gflags, quality);
	lame_set_VBR(encoder->gflags, vbr_off);
	if(lame_init_params(encoder->gflags) < 0) {
		ERR_PRINT("failed to setup lame.");
		lame_close(encoder->gflags);
		ttLibC_free(encoder);
		return NULL;
	}
	encoder->mp3                       = NULL;
	encoder->data_size                 = 65536;
	encoder->data                      = ttLibC_malloc(encoder->data_size); // mp3encode buffer.
	encoder->pts                       = 0;
	encoder->data_start_pos            = 0;
	encoder->inherit_super.channel_num = channel_num;
	encoder->inherit_super.quality     = quality;
	encoder->inherit_super.sample_rate = sample_rate;
	encoder->inherit_super.type        = Mp3lameEncoderType_CBR;
	encoder->is_pts_initialized = false;
	if(encoder->data == NULL) {
		ERR_PRINT("failed to allocate encode data buffer.");
		lame_close(encoder->gflags);
		ttLibC_free(encoder);
		return NULL;
	}
	return (ttLibC_Mp3lameEncoder *)encoder;
}

/*
 * handle encoded data after encode.
 * @param encoder     encoder object.
 * @param encode_size size of encode data.
 * @param callback    callback func.
 * @param ptr         user def data pointer.
 * @return true / false
 */
static bool checkEncodedData(ttLibC_Mp3lameEncoder_ *encoder, uint32_t encode_size, ttLibC_Mp3lameEncodeFunc callback, void *ptr) {
	uint8_t *data = encoder->data;
	ttLibC_Mp3 *mp3 = NULL;
	do {
		mp3 = ttLibC_Mp3_getFrame(encoder->mp3, data, encode_size, true, 0, 1000);
		if(mp3 != NULL) {
			// set the pts and timebase.
			mp3->inherit_super.inherit_super.pts = encoder->pts;
			mp3->inherit_super.inherit_super.timebase = mp3->inherit_super.sample_rate;
			encoder->pts += mp3->inherit_super.sample_num;
			encoder->mp3 = mp3;
			// call callback with created object.
			if(!callback(ptr, encoder->mp3)) {
				return false;
			}
			data += encoder->mp3->inherit_super.inherit_super.buffer_size;
			encode_size -= encoder->mp3->inherit_super.inherit_super.buffer_size;
		}
	} while(mp3 != NULL);

	if(encoder->data != data) {
		// if we have remain data. shift it to the begin of data.
		uint8_t *dst_data = encoder->data;
		for(uint32_t i = 0;i < encode_size;i ++) {
			(*dst_data) = (*data);
			++ dst_data;
			++ data;
		}
	}
	// next append position of data = encode_remain_size.
	encoder->data_start_pos = encode_size;
	return true;
}

/*
 * encode frame.
 * @param encoder  mp3lame encoder object.
 * @param pcm      source pcm data.
 * @param callback callback func for mp3 creation.
 * @param ptr      pointer for user def value, which will call in callback.
 * @return true / false
 */
bool ttLibC_Mp3lameEncoder_encode(ttLibC_Mp3lameEncoder *encoder, ttLibC_PcmS16 *pcm, ttLibC_Mp3lameEncodeFunc callback, void *ptr) {
	if(encoder == NULL) {
		return false;
	}
	if(pcm == NULL) {
		return true;
	}
	ttLibC_Mp3lameEncoder_ *encoder_ = (ttLibC_Mp3lameEncoder_ *)encoder;
	if(!encoder_->is_pts_initialized) {
		encoder_->is_pts_initialized = true;
		encoder_->pts = pcm->inherit_super.inherit_super.pts * encoder_->inherit_super.sample_rate / pcm->inherit_super.inherit_super.timebase;
	}
	switch(pcm->type) {
	case PcmS16Type_bigEndian:
	case PcmS16Type_bigEndian_planar:
		ERR_PRINT("bigendian is not support to convert.");
		return false;
	case PcmS16Type_littleEndian:
		{
			if(encoder_->inherit_super.channel_num == 1) {
				uint32_t size = lame_encode_buffer(encoder_->gflags, (const short*)pcm->l_data, NULL, pcm->inherit_super.sample_num, encoder_->data + encoder_->data_start_pos, encoder_->data_size - encoder_->data_start_pos);
				if(!checkEncodedData(encoder_, size + encoder_->data_start_pos, callback, ptr)) {
					return false;
				}
			}
			else {
				uint32_t size = lame_encode_buffer_interleaved(encoder_->gflags, (short*)pcm->l_data, pcm->inherit_super.sample_num, encoder_->data + encoder_->data_start_pos, encoder_->data_size - encoder_->data_start_pos);
				if(!checkEncodedData(encoder_, size + encoder_->data_start_pos, callback, ptr)) {
					return false;
				}
			}
		}
		break;
	case PcmS16Type_littleEndian_planar:
		{
			uint32_t size = lame_encode_buffer(encoder_->gflags, (const short*)pcm->l_data, (const short*)pcm->r_data, pcm->inherit_super.sample_num, encoder_->data + encoder_->data_start_pos, encoder_->data_size - encoder_->data_start_pos);
			if(!checkEncodedData(encoder_, size + encoder_->data_start_pos, callback, ptr)) {
				return false;
			}
		}
		break;
	default:
		ERR_PRINT("unknown pcms16_type:%d", pcm->type);
		return false;
	}
	return true;
}

/*
 * close mp3lame encoder.
 * @param encoder
 */
void ttLibC_Mp3lameEncoder_close(ttLibC_Mp3lameEncoder **encoder) {
	if(*encoder == NULL) {
		return;
	}
	ttLibC_Mp3lameEncoder_ *target = (ttLibC_Mp3lameEncoder_ *)*encoder;
	// release encoded data buffer.
	ttLibC_free(target->data);
	target->data = NULL;
	// release reuse mp3 object.
	ttLibC_Mp3_close(&target->mp3);
	// close lame.
	lame_close(target->gflags);
	// free mp3lameEncoder
	ttLibC_free(target);
	*encoder = NULL;
}

#endif
