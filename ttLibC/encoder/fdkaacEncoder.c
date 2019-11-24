/**
 * @file   fdkaacEncoder.c
 * @brief  encode aac with fdk-aac
 * 
 * this code is under LGPLv3 license
 * 
 * @author taktod
 * @date   2019/07/15
 */

#ifdef __ENABLE_FDKAAC__

#include "fdkaacEncoder.h"
#include "../_log.h"
#include "../allocator.h"
#include <fdk-aac/aacenc_lib.h>
#include <string.h>

typedef struct ttLibC_Encoder_FdkaacEncoder_ {
  ttLibC_FdkaacEncoder inherit_super;
  HANDLE_AACENCODER handle_;
  ttLibC_Aac2 *aac_;
	uint8_t    *pcm_buffer_;
	size_t      pcm_buffer_size_;
	size_t      pcm_buffer_next_pos_;
	uint8_t    *data_;
	size_t      data_size_;
	uint64_t    pts_;
	uint32_t    sample_rate_;
	uint32_t    sample_num_;
	bool        is_pts_initialized_;
} ttLibC_Encoder_FdkaacEncoder_ ;

typedef ttLibC_Encoder_FdkaacEncoder_ ttLibC_FdkaacEncoder_;

ttLibC_FdkaacEncoder TT_ATTRIBUTE_API *ttLibC_FdkaacEncoder_make(
    const char *aac_type,
    uint32_t sample_rate,
    uint32_t channel_num,
    uint32_t bitrate) {
  // 作っていく
  ttLibC_FdkaacEncoder_ *encoder = (ttLibC_FdkaacEncoder_ *)ttLibC_malloc(sizeof(ttLibC_FdkaacEncoder_));
  if(encoder == NULL) {
    ERR_PRINT("failed to alloc encoder object.");
    return NULL;
  }
  AACENC_ERROR err;
  if((err = aacEncOpen(&encoder->handle_, 0, channel_num)) != AACENC_OK) {
    ERR_PRINT("failed to open fdkaac.");
  }
#define ENUM(str)	else if(strcmp(aac_type, str) == 0)
  AUDIO_OBJECT_TYPE aactype;
	if(0) {}
	ENUM("AOT_NONE")
		aactype = AOT_NONE;
	ENUM("AOT_NULL_OBJECT")
		aactype = AOT_NULL_OBJECT;
	ENUM("AOT_AAC_MAIN")
		aactype = AOT_AAC_MAIN;
	ENUM("AOT_AAC_LC")
		aactype = AOT_AAC_LC;
	ENUM("AOT_AAC_SSR")
		aactype = AOT_AAC_SSR;
	ENUM("AOT_AAC_LTP")
		aactype = AOT_AAC_LTP;
	ENUM("AOT_SBR")
		aactype = AOT_SBR;
	ENUM("AOT_AAC_SCAL")
		aactype = AOT_AAC_SCAL;
	ENUM("AOT_TWIN_VQ")
		aactype = AOT_TWIN_VQ;
	ENUM("AOT_CELP")
		aactype = AOT_CELP;
	ENUM("AOT_HVXC")
		aactype = AOT_HVXC;
	ENUM("AOT_RSVD_10")
		aactype = AOT_RSVD_10;
	ENUM("AOT_RSVD_11")
		aactype = AOT_RSVD_11;
	ENUM("AOT_TTSI")
		aactype = AOT_TTSI;
	ENUM("AOT_MAIN_SYNTH")
		aactype = AOT_MAIN_SYNTH;
	ENUM("AOT_WAV_TAB_SYNTH")
		aactype = AOT_WAV_TAB_SYNTH;
	ENUM("AOT_GEN_MIDI")
		aactype = AOT_GEN_MIDI;
	ENUM("AOT_ALG_SYNTH_AUD_FX")
		aactype = AOT_ALG_SYNTH_AUD_FX;
	ENUM("AOT_ER_AAC_LC")
		aactype = AOT_ER_AAC_LC;
	ENUM("AOT_RSVD_18")
		aactype = AOT_RSVD_18;
	ENUM("AOT_ER_AAC_LTP")
		aactype = AOT_ER_AAC_LTP;
	ENUM("AOT_ER_AAC_SCAL")
		aactype = AOT_ER_AAC_SCAL;
	ENUM("AOT_ER_TWIN_VQ")
		aactype = AOT_ER_TWIN_VQ;
	ENUM("AOT_ER_BSAC")
		aactype = AOT_ER_BSAC;
	ENUM("AOT_ER_AAC_LD")
		aactype = AOT_ER_AAC_LD;
	ENUM("AOT_ER_CELP")
		aactype = AOT_ER_CELP;
	ENUM("AOT_ER_HVXC")
		aactype = AOT_ER_HVXC;
	ENUM("AOT_ER_HILN")
		aactype = AOT_ER_HILN;
	ENUM("AOT_ER_PARA")
		aactype = AOT_ER_PARA;
	ENUM("AOT_RSVD_28")
		aactype = AOT_RSVD_28;
	ENUM("AOT_PS")
		aactype = AOT_PS;
	ENUM("AOT_MPEGS")
		aactype = AOT_MPEGS;
	ENUM("AOT_ESCAPE")
		aactype = AOT_ESCAPE;
	ENUM("AOT_MP3ONMP4_L1")
		aactype = AOT_MP3ONMP4_L1;
	ENUM("AOT_MP3ONMP4_L2")
		aactype = AOT_MP3ONMP4_L2;
	ENUM("AOT_MP3ONMP4_L3")
		aactype = AOT_MP3ONMP4_L3;
	ENUM("AOT_RSVD_35")
		aactype = AOT_RSVD_35;
	ENUM("AOT_RSVD_36")
		aactype = AOT_RSVD_36;
	ENUM("AOT_AAC_SLS")
		aactype = AOT_AAC_SLS;
	ENUM("AOT_SLS")
		aactype = AOT_SLS;
	ENUM("AOT_ER_AAC_ELD")
		aactype = AOT_ER_AAC_ELD;
	ENUM("AOT_USAC")
		aactype = AOT_USAC;
	ENUM("AOT_SAOC")
		aactype = AOT_SAOC;
	ENUM("AOT_LD_MPEGS")
		aactype = AOT_LD_MPEGS;
	ENUM("AOT_DRM_AAC")
		aactype = AOT_DRM_AAC;
	ENUM("AOT_DRM_SBR")
		aactype = AOT_DRM_SBR;
	ENUM("AOT_DRM_MPEG_PS")
		aactype = AOT_DRM_MPEG_PS;
	else {
		err = AACENC_INIT_ERROR;
	}
#undef ENUM
  if(err == AACENC_OK && (err = aacEncoder_SetParam(encoder->handle_, AACENC_AOT, aactype)) != AACENC_OK) {
    ERR_PRINT("failed to set profile.");
  }
  if(err == AACENC_OK && (err = aacEncoder_SetParam(encoder->handle_, AACENC_SAMPLERATE, sample_rate)) != AACENC_OK) {
    ERR_PRINT("failed to set sampleRate.");
  }
	CHANNEL_MODE mode;
	switch(channel_num) {
	case 1:
		mode = MODE_1;
		break;
	case 2:
		mode = MODE_2;
		break;
	default:
		err = AACENC_INIT_ERROR;
		break;
	}
	if(err == AACENC_OK && (err = aacEncoder_SetParam(encoder->handle_, AACENC_CHANNELMODE, mode)) != AACENC_OK) {
    ERR_PRINT("failed to set channelNum.");
	}
	if(err == AACENC_OK && (err = aacEncoder_SetParam(encoder->handle_, AACENC_CHANNELORDER, 1)) != AACENC_OK) {
    ERR_PRINT("failed to set channelOrder");
	}
	if(err == AACENC_OK && (err = aacEncoder_SetParam(encoder->handle_, AACENC_BITRATE, bitrate)) != AACENC_OK) {
    ERR_PRINT("failed to set bitrate");
	}
	if(err == AACENC_OK && (err = aacEncoder_SetParam(encoder->handle_, AACENC_TRANSMUX, 2)) != AACENC_OK) {
    ERR_PRINT("failed to set transmux");
	}
	if(err == AACENC_OK && (err = aacEncoder_SetParam(encoder->handle_, AACENC_AFTERBURNER, 1)) != AACENC_OK) {
    ERR_PRINT("failed to set afterburner");
	}
  // try initialize
	if(err == AACENC_OK && (err = aacEncEncode(encoder->handle_, NULL, NULL, NULL, NULL)) != AACENC_OK) {
    ERR_PRINT("failed to initialize fdkaac.");
	}
	AACENC_InfoStruct info;
	if(err == AACENC_OK && (err = aacEncInfo(encoder->handle_, &info)) != AACENC_OK) {
    ERR_PRINT("failed to get encode information.");
	}
	if(err != AACENC_OK) {
		if(encoder->handle_ != NULL) {
			aacEncClose(&encoder->handle_);
			encoder->handle_ = NULL;
		}
    ttLibC_free(encoder);
    return NULL;
	}
  encoder->aac_ = NULL;
  encoder->data_size_ = 2048;
  encoder->data_ = (uint8_t *)ttLibC_malloc(encoder->data_size_);
  if(encoder->data_ == NULL) {
    aacEncClose(&encoder->handle_);
    encoder->handle_ = NULL;
    ttLibC_free(encoder);
    return NULL;
  }
  encoder->pcm_buffer_next_pos_ = 0;
  encoder->pcm_buffer_size_ = info.frameLength * channel_num * sizeof(int16_t);
  encoder->pcm_buffer_ = (uint8_t *)ttLibC_malloc(encoder->pcm_buffer_size_);
  if(encoder->pcm_buffer_ == NULL) {
    aacEncClose(&encoder->handle_);
    encoder->handle_ = NULL;
    ttLibC_free(encoder->data_);
    encoder->data_ = NULL;
    ttLibC_free(encoder);
  }
  encoder->pts_ = 0;
  encoder->sample_rate_ = sample_rate;
  encoder->sample_num_ = info.frameLength;
  encoder->is_pts_initialized_ = false;
  return (ttLibC_FdkaacEncoder *)encoder;
}

bool TT_ATTRIBUTE_API ttLibC_FdkaacEncoder_encode(
    ttLibC_FdkaacEncoder *encoder,
    ttLibC_PcmS16 *frame,
    ttLibC_FdkaacEncodeFunc callback,
    void *ptr) {
  if(encoder == NULL) {
    return false;
  }
  if(frame == NULL) {
    return true;
  }
  ttLibC_FdkaacEncoder_ *encoder_ = (ttLibC_FdkaacEncoder_ *)encoder;
  if(encoder_->is_pts_initialized_) {
    encoder_->is_pts_initialized_ = true;
    encoder_->pts_ = frame->inherit_super.inherit_super.pts * encoder_->sample_rate_ / frame->inherit_super.inherit_super.timebase;
  }
  uint8_t *data = (uint8_t *)frame->inherit_super.inherit_super.data;
  size_t left_size = frame->inherit_super.inherit_super.buffer_size;
	AACENC_ERROR err;
	AACENC_BufDesc in_buf   = {}, out_buf = {};
	AACENC_InArgs  in_args  = {};
	AACENC_OutArgs out_args = {};
	int in_identifier = IN_AUDIO_DATA;
	int in_size = encoder_->pcm_buffer_size_;
	int in_elem_size = sizeof(int16_t);
	void *in_ptr = NULL;
	void *out_ptr = encoder_->data_;
	int out_identifier = OUT_BITSTREAM_DATA;
	int out_size = encoder_->data_size_;
	int out_elem_size = 1;
	in_args.numInSamples = in_size >> 1;

	in_buf.numBufs = 1;
	in_buf.bufferIdentifiers = &in_identifier;
//	in_buf.bufs = &in_ptr;
	in_buf.bufSizes = &in_size;
	in_buf.bufElSizes = &in_elem_size;

	out_buf.numBufs = 1;
	out_buf.bufferIdentifiers = &out_identifier;
//	out_buf.bufs = &out_ptr;
	out_buf.bufSizes = &out_size;
	out_buf.bufElSizes = &out_elem_size;
	if(encoder_->pcm_buffer_next_pos_ != 0) {
		if(left_size < in_size - encoder_->pcm_buffer_next_pos_) {
			// append data is not enough for encode. just add.
			memcpy(encoder_->pcm_buffer_ + encoder_->pcm_buffer_next_pos_, data, left_size);
			encoder_->pcm_buffer_next_pos_ += left_size;
			return true;
		}
		memcpy(encoder_->pcm_buffer_ + encoder_->pcm_buffer_next_pos_, data, in_size - encoder_->pcm_buffer_next_pos_);
		data += in_size - encoder_->pcm_buffer_next_pos_;
		left_size -= in_size - encoder_->pcm_buffer_next_pos_;
		in_ptr = encoder_->pcm_buffer_;
		in_buf.bufs = &in_ptr;
		out_ptr = encoder_->data_;
		out_buf.bufs = &out_ptr;
		// ここでエンコードを実施すればよい。
		if((err = aacEncEncode(encoder_->handle_, &in_buf, &out_buf, &in_args, &out_args)) != AACENC_OK) {
			puts("failed to encode data.");
			return false;
		}
		if(out_args.numOutBytes > 0) {
			ttLibC_Aac2 *aac = ttLibC_Aac2_getFrame(
				encoder_->aac_,
				encoder_->data_,
				out_args.numOutBytes,
				true,
				encoder_->pts_,
				encoder_->sample_rate_);
			if(aac == NULL) {
				return false;
			}
			encoder_->aac_ = aac;
      encoder_->aac_->inherit_super.inherit_super.id = frame->inherit_super.inherit_super.id;
			encoder_->pts_ += 1024;
      if(callback != NULL) {
        if(!callback(ptr, encoder_->aac_)) {
          return false;
        }
      }
/*      if(!ttLibGoFrameCallback(ptr, (ttLibC_Frame *)aac_)) {
  reset(cFrame, goFrame);
        return false;
      }*/
		}
		encoder_->pcm_buffer_next_pos_ = 0;
	}
	do {
		// get the data from pcm buffer.
		if((int)left_size < in_size) {
			// need more data.
			if(left_size != 0) {
				memcpy(encoder_->pcm_buffer_, data, left_size);
				encoder_->pcm_buffer_next_pos_ = left_size;
			}
//  reset(cFrame, goFrame);
			return true;
		}
		// こっちもここでencodeを実施すればよい。
		// data have enough size.
		in_ptr = data;
		in_buf.bufs = &in_ptr;
		out_ptr = encoder_->data_;
		out_buf.bufs = &out_ptr;
		// ここでエンコードを実施すればよい。
		if((err = aacEncEncode(encoder_->handle_, &in_buf, &out_buf, &in_args, &out_args)) != AACENC_OK) {
			puts("failed to encode data.");
			return false;
		}
		if(out_args.numOutBytes > 0) {
			ttLibC_Aac2 *aac = ttLibC_Aac2_getFrame(
				encoder_->aac_,
				encoder_->data_,
				out_args.numOutBytes,
				true,
				encoder_->pts_,
				encoder_->sample_rate_);
			if(aac == NULL) {
				return false;
			}
			encoder_->aac_ = aac;
      encoder_->aac_->inherit_super.inherit_super.id = frame->inherit_super.inherit_super.id;
			encoder_->pts_ += 1024;
      if(callback != NULL) {
        if(!callback(ptr, encoder_->aac_)) {
          return false;
        }
      }
/*      if(!ttLibGoFrameCallback(ptr, (ttLibC_Frame *)aac_)) {
        return false;
      }*/
		}
		data += in_size;
		left_size -= in_size;
	}while(true);
  return true;
}

void TT_ATTRIBUTE_API ttLibC_FdkaacEncoder_close(ttLibC_FdkaacEncoder **encoder) {
  ttLibC_FdkaacEncoder_ *target = (ttLibC_FdkaacEncoder_ *)*encoder;
  if(target == NULL) {
    return;
  }
  if(target->handle_) {
    aacEncClose(&target->handle_);
    target->handle_ = NULL;
  }
  if(target->data_) {
    ttLibC_free(target->data_);
    target->data_ = NULL;
  }
  if(target->pcm_buffer_) {
    ttLibC_free(target->pcm_buffer_);
    target->pcm_buffer_ = NULL;
  }
  ttLibC_Aac2_close(&target->aac_);
  ttLibC_free(target);
  *encoder = NULL;
}

bool TT_ATTRIBUTE_API ttLibC_FdkaacEncoder_setBitrate(ttLibC_FdkaacEncoder *encoder, uint32_t value) {
  if(encoder == NULL) {
    return false;
  }
  ttLibC_FdkaacEncoder_ *encoder_ = (ttLibC_FdkaacEncoder_ *)encoder;
	return aacEncoder_SetParam(encoder_->handle_, AACENC_BITRATE, value) == AACENC_OK;
}

#endif
