/*
 * @file   mp3lameDecoder.c
 * @brief  decode mp3 with mp3lame.
 *
 * this code is under GPLv3 license.
 *
 * @author taktod
 * @date   2015/07/28
 */

#ifdef __ENABLE_MP3LAME_DECODE__

#include "mp3lameDecoder.h"
#include "../_log.h"
#include "../allocator.h"
#include <stdlib.h>
#include <lame/lame.h>

/*
 * mp3lame decoder detail definition.
 */
typedef struct {
	/** inherit data from ttLibC_Mp3lameDecoder */
	ttLibC_Mp3lameDecoder inherit_super;
	/** global flags of lame */
	hip_t hip_gflags;
	/** output pcms16 object for reuse */
	ttLibC_PcmS16 *pcms16;
	/** buffer for left right buffer */
	uint8_t *buffer;
	/** buffer size */
	size_t buffer_size;
	/** buffer for pcm(pointer for buffer) */
	int16_t *pcm_buffer;
	/** buffer for left(pointer for buffer) */
	int16_t *left_buffer;
	/** buffer for right(pointer for buffer) */
	int16_t *right_buffer;
} ttLibC_Decoder_Mp3lameDecoder_;

typedef ttLibC_Decoder_Mp3lameDecoder_ ttLibC_Mp3lameDecoder_;

/*
 * make mp3lame decoder
 */
ttLibC_Mp3lameDecoder TT_ATTRIBUTE_API *ttLibC_Mp3lameDecoder_make() {
	ttLibC_Mp3lameDecoder_ *decoder = ttLibC_malloc(sizeof(ttLibC_Mp3lameDecoder_));
	if(decoder == NULL) {
		ERR_PRINT("failed to allocate memory for decoder.");
		return NULL;
	}
	decoder->pcms16 = NULL;
	decoder->inherit_super.channel_num = 0;
	decoder->inherit_super.sample_rate = 0;
	decoder->buffer_size = 65536;
	decoder->buffer = ttLibC_malloc(decoder->buffer_size);
	if(decoder->buffer == NULL) {
		ERR_PRINT("failed to allocate memory for buffer.");
		ttLibC_free(decoder);
		return NULL;
	}
	// max sample_num = buffer_size / 4;
	decoder->left_buffer  = (int16_t *) decoder->buffer;
	decoder->right_buffer = (int16_t *)(decoder->buffer + (decoder->buffer_size >> 2));
	decoder->pcm_buffer   = (int16_t *)(decoder->buffer + (decoder->buffer_size >> 1));
	decoder->hip_gflags = hip_decode_init();
	if(decoder->hip_gflags == NULL) {
		ERR_PRINT("failed to initialize hip.");
		ttLibC_free(decoder);
		return NULL;
	}
	return (ttLibC_Mp3lameDecoder *)decoder;
}

/*
 * decode frame.
 * @param decoder  mp3lame decoder object
 * @param mp3      source mp3 data.
 * @param callback callback func for mp3 decode.
 * @param ptr      pointer for user def value, which willl call in callback.
 * @return true / false
 */
bool TT_ATTRIBUTE_API ttLibC_Mp3lameDecoder_decode(
		ttLibC_Mp3lameDecoder *decoder,
		ttLibC_Mp3 *mp3,
		ttLibC_Mp3lameDecodeFunc callback,
		void *ptr) {
	if(decoder == NULL) {
		return false;
	}
	if(mp3 == NULL) {
		return true;
	}
	ttLibC_Mp3lameDecoder_ *decoder_ = (ttLibC_Mp3lameDecoder_ *)decoder;
	if(decoder_->buffer == NULL) {
		ERR_PRINT("buffer is null. something wrong is happen!!!");
		return false;
	}
	switch(decoder_->inherit_super.channel_num) {
	case 0:
		decoder_->inherit_super.channel_num = mp3->inherit_super.channel_num;
		break;
	default:
		if(decoder_->inherit_super.channel_num != mp3->inherit_super.channel_num) {
			ERR_PRINT("mp3 channel number is changed.");
			return false;
		}
		break;
	}
	switch(decoder_->inherit_super.sample_rate) {
	case 0:
		decoder_->inherit_super.sample_rate = mp3->inherit_super.sample_rate;
		break;
	default:
		if(decoder_->inherit_super.sample_rate != mp3->inherit_super.sample_rate) {
			ERR_PRINT("mp3 samplerate is changed.");
			return false;
		}
		break;
	}
	// check sample_num
	if(decoder_->buffer_size < (mp3->inherit_super.sample_num << 4)) {
		// need more buffer.
		ttLibC_free(decoder_->buffer);
		decoder_->left_buffer  = NULL;
		decoder_->right_buffer = NULL;
		decoder_->buffer_size = (mp3->inherit_super.sample_num << 4);
		decoder_->buffer = ttLibC_malloc(decoder_->buffer_size);
		if(decoder_->buffer == NULL) {
			ERR_PRINT("failed to realloc buffer.");
			return false;
		}
		decoder_->left_buffer  = (int16_t *) decoder_->buffer;
		decoder_->right_buffer = (int16_t *)(decoder_->buffer + (decoder_->buffer_size >> 2));
		decoder_->pcm_buffer   = (int16_t *)(decoder_->buffer + (decoder_->buffer_size >> 1));
	}
	int num = hip_decode(decoder_->hip_gflags, mp3->inherit_super.inherit_super.data, mp3->inherit_super.inherit_super.buffer_size, decoder_->left_buffer, decoder_->right_buffer);
	// for begin of decode. num can be 0, and get the chunk of num.
	if(num == 0) {
		return true;
	}
/*	if(num != mp3->inherit_super.sample_num) {
		LOG_PRINT("decode output is invalid. must be the same as mp3 sample_num. %d = %d(mp3)", num, mp3->inherit_super.sample_num);
		return;
	}*/
	// make planar buf to interleave buf.
	int16_t *left_buf = decoder_->left_buffer;
	int16_t *right_buf = decoder_->right_buffer;
	int16_t *pcm_buf = decoder_->pcm_buffer;
	for(int i = 0;i < num;++ i) {
		(*pcm_buf) = (*left_buf);
		++ pcm_buf;
		++ left_buf;
		if(mp3->inherit_super.channel_num == 2) {
			(*pcm_buf) = (*right_buf);
			++ pcm_buf;
			++ right_buf;
		}
	}
	// make pcms16 object.
	ttLibC_PcmS16 *p = ttLibC_PcmS16_make(
			decoder_->pcms16,
			PcmS16Type_littleEndian,
			mp3->inherit_super.sample_rate,
			num,
			mp3->inherit_super.channel_num,
			decoder_->pcm_buffer,
			num * 2 * mp3->inherit_super.channel_num,
			decoder_->pcm_buffer,
			num * 2 * mp3->inherit_super.channel_num,
			NULL,
			0,
			true,
			mp3->inherit_super.inherit_super.pts,
			mp3->inherit_super.inherit_super.timebase);
	if(p == NULL) {
		return false;
	}
	decoder_->pcms16 = p;
	decoder_->pcms16->inherit_super.inherit_super.id = mp3->inherit_super.inherit_super.id;
	// call
	if(!callback(ptr, decoder_->pcms16)) {
		return false;
	}
	return true;
}

/*
 * close mp3lame decoder.
 * @param decoder.
 */
void TT_ATTRIBUTE_API ttLibC_Mp3lameDecoder_close(ttLibC_Mp3lameDecoder **decoder) {
	ttLibC_Mp3lameDecoder_ *target = (ttLibC_Mp3lameDecoder_ *)*decoder;
	if(target == NULL) {
		return;
	}
	if(target->hip_gflags) {
		hip_decode_exit(target->hip_gflags);
	}
	ttLibC_PcmS16_close(&target->pcms16);
	ttLibC_free(target->buffer);
	ttLibC_free(target);
	*decoder = NULL;
}

#endif

