/*
 * @file   audioTag.c
 * @brief  
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/12
 */

#include "audioTag.h"
#include "../flvTag.h"
#include "../../../log.h"
#include "../../../util/hexUtil.h"

#include "../../../frame/audio/aac.h"
#include "../../../frame/audio/mp3.h"

ttLibC_FlvAudioTag *ttLibC_FlvAudioTag_make(
		ttLibC_FlvTag *prev_tag,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		uint32_t track_id,
		uint8_t codec_id,
		uint8_t sample_rate_flag,
		uint8_t bit_count_flag,
		uint8_t channel_flag) {
	ttLibC_FlvAudioTag *audio_tag = (ttLibC_FlvAudioTag *)ttLibC_FlvTag_make(
			prev_tag,
			data,
			data_size,
			non_copy_mode,
			pts,
			timebase,
			FlvType_audio,
			track_id);
	if(audio_tag != NULL) {
		if(bit_count_flag == 1) {
			audio_tag->bit_count = 16;
		}
		else {
			audio_tag->bit_count = 8;
		}
		if(channel_flag == 1) {
			audio_tag->channel_num = 2;
		}
		else {
			audio_tag->channel_num = 1;
		}
		switch(sample_rate_flag) {
		case 0:
			audio_tag->sample_rate = 5512;
			break;
		case 1:
			audio_tag->sample_rate = 11025;
			break;
		case 2:
			audio_tag->sample_rate = 22050;
			break;
		case 3:
			audio_tag->sample_rate = 44100;
			break;
		}
		audio_tag->codec_id = codec_id;
		switch(codec_id) {
		case 0: // pcm(be)
			if(bit_count_flag == 0) {
				// pcmS8 // bigendian
				audio_tag->frame_type = frameType_unknown;
			}
			else {
				// pcmS16 // bigendian
				audio_tag->frame_type = frameType_pcmS16;
			}
			break;
		case 1: // adpcm (for flv)
			audio_tag->frame_type = frameType_unknown;
			break;
		case 2: // mp3
			audio_tag->frame_type = frameType_mp3;
			break;
		case 3: // little endian pcm
			if(bit_count_flag == 0) {
				// pcmS8 // bigendian
				audio_tag->frame_type = frameType_unknown;
			}
			else {
				// pcmS16 // bigendian
				audio_tag->frame_type = frameType_pcmS16;
			}
			break;
		case 4: // nelly 16k
			audio_tag->frame_type = frameType_nellymoser;
			audio_tag->sample_rate = 16000;
			break;
		case 5: // nelly 8k
			audio_tag->frame_type = frameType_nellymoser;
			audio_tag->sample_rate = 8000;
			break;
		case 6: // nelly 44.1 22.05 11.025 5k
			audio_tag->frame_type = frameType_nellymoser;
			break;
		case 7: // g711_a
			audio_tag->frame_type = frameType_pcm_alaw;
			break;
		case 8: // g711_u
			audio_tag->frame_type = frameType_pcm_mulaw;
			break;
		case 9: // reserved
			audio_tag->frame_type = frameType_unknown;
			break;
		case 10: // aac
			audio_tag->frame_type = frameType_aac;
			break;
		case 11: // speex 16kHz monoral
			audio_tag->frame_type = frameType_speex;
			break;
		case 12: // unknown
		case 13: // undefined
		default:
			audio_tag->frame_type = frameType_unknown;
			break;
		case 14: // mp3 8kHz
			audio_tag->frame_type = frameType_mp3;
			audio_tag->sample_rate = 8000;
			break;
		case 15: // device specific
			audio_tag->frame_type = frameType_unknown;
			break;
		}
	}
	return audio_tag;
}

ttLibC_FlvAudioTag *ttLibC_FlvAudioTag_getTag(
		ttLibC_FlvTag *prev_tag,
		uint8_t *data,
		size_t data_size) {
	/**
	 * 内部メモ
	 * 1byteフラグ
	 * 3byte size
	 * 3byte timestamp
	 * 1byte timestamp-ext
	 * 3byte track_id
	 * 4bit codec_type
	 * 2bit sample_rate flag
	 * 1bit bit count flag
	 * 1bit channel flag
	 */
	size_t   size      = ((data[1] << 16) & 0xFF0000) | ((data[2] << 8) & 0xFF00) | (data[3] & 0xFF);
	uint32_t timestamp = ((data[4] << 16) & 0xFF0000) | ((data[5] << 8) & 0xFF00) | (data[6] & 0xFF) | ((data[7] << 24) & 0xFF000000);
	uint32_t track_id  = ((data[8] << 16) & 0xFF0000) | ((data[9] << 8) & 0xFF00) | (data[10] & 0xFF);
	uint8_t codec_id         = (data[11] >> 4) & 0x0F;
	uint8_t sample_rate_flag = (data[11] >> 2) & 0x03;
	uint8_t bit_count_flag   = (data[11] >> 1) & 0x01;
	uint8_t channel_flag     = (data[11]) & 0x01;
	data += 12;
	data_size -= 12;

	size_t post_size = ((*(data + data_size - 4)) << 24) | ((*(data + data_size - 3)) << 16) | ((*(data + data_size - 2)) << 8) | (*(data + data_size - 1));
	if(size + 11 != post_size) {
		ERR_PRINT("crazy size data, out of flv format?");
		return NULL;
	}
	return ttLibC_FlvAudioTag_make(
			prev_tag,
			data,
			data_size - 4,
			true,
			timestamp,
			1000,
			track_id,
			codec_id,
			sample_rate_flag,
			bit_count_flag,
			channel_flag);
}

// dsiは43bit以上必要みたいなので、uint64_tで保持しておこうかな・・・
bool ttLibC_FlvAudioTag_getFrame(
		ttLibC_FlvAudioTag *audio_tag,
		ttLibC_getFrameFunc callback,
		void *ptr) {
	uint8_t *buffer = audio_tag->inherit_super.inherit_super.inherit_super.data;
	size_t left_size = audio_tag->inherit_super.inherit_super.inherit_super.buffer_size;
	switch(audio_tag->frame_type) {
	case frameType_aac:
		// aacの場合は始めのbyteが0なら、mshとなる。
		// dsiを読み取って、他のaacに追加するべきheader情報を構築しないといけない。
		// adtsをそこからつくる。
		if(buffer[0] == 0) {
			uint8_t *dsi = (uint8_t *)&audio_tag->aac_dsi_info;
			for(int i = 1;i < left_size;++ i) {
				*dsi = buffer[i];
				++ dsi;
			}
			return true;
		}
		else if(buffer[0] == 1) {
			ttLibC_Aac *aac = ttLibC_Aac_make(
					(ttLibC_Aac *)audio_tag->inherit_super.frame,
					AacType_raw,
					audio_tag->sample_rate,
					1024,
					audio_tag->channel_num,
					buffer + 1,
					left_size - 1,
					true,
					audio_tag->inherit_super.inherit_super.inherit_super.pts,
					audio_tag->inherit_super.inherit_super.inherit_super.timebase,
					audio_tag->aac_dsi_info);
			if(aac == NULL) {
				ERR_PRINT("failed to make raw aac frame.");
				return false;
			}
			audio_tag->inherit_super.frame = (ttLibC_Frame *)aac;
			return callback(ptr, audio_tag->inherit_super.frame);
		}
		else {
			return false;
		}
	case frameType_mp3:
		{
			ttLibC_Mp3 *mp3 = ttLibC_Mp3_getFrame(
					(ttLibC_Mp3 *)audio_tag->inherit_super.frame,
					buffer,
					left_size,
					audio_tag->inherit_super.inherit_super.inherit_super.pts,
					audio_tag->inherit_super.inherit_super.inherit_super.timebase);
			if(mp3 == NULL) {
				ERR_PRINT("failed to make mp3 frame.");
				return false;
			}
			audio_tag->inherit_super.frame = (ttLibC_Frame *)mp3;
			return callback(ptr, audio_tag->inherit_super.frame);
		}
		break;
	case frameType_nellymoser:
		LOG_PRINT("nellymoser: no read process.");
		break;
	case frameType_pcmS16:
		LOG_PRINT("pcms16: no read process.");
		break;
	case frameType_pcm_alaw:
		LOG_PRINT("pcm alaw: no read process.");
		break;
	case frameType_pcm_mulaw:
		LOG_PRINT("pcm mulaw: no read process.");
		break;
	case frameType_speex:
		LOG_PRINT("speex: no read process.");
		break;
	default:
		break;
	}
	return false;
}

bool ttLibC_FlvAudioTag_writeTag(
		ttLibC_FlvWriter_ *writer,
		ttLibC_Frame *frame,
		ttLibC_ContainerWriterFunc callback,
		void *ptr) {
	uint8_t buf[256];
	switch(frame->type) {
	case frameType_aac:
		{
			ttLibC_Aac *aac = (ttLibC_Aac *)frame;
			uint32_t crc32_value = ttLibC_Aac_getConfigCrc32(aac);
			if(writer->audio_track.crc32 == 0 || writer->audio_track.crc32 != crc32_value) {
				// crc32が変わっている場合(もしくは初アクセス)
				// aacのヘッダを書き出す。
				uint8_t dsi[16];
				size_t size = ttLibC_Aac_readDsiInfo(aac, dsi, sizeof(dsi));
				if(size == 0) {
					ERR_PRINT("dsi data size is 0. something wrong.");
					return false;
				}
				// あとはaacのデータを書き出す。
				buf[0]  = 0x08;
				uint32_t pre_size = size + 2;
				buf[1]  = (pre_size >> 16) & 0xFF;
				buf[2]  = (pre_size >> 8) & 0xFF;
				buf[3]  = pre_size & 0xFF;
				// pts値
				buf[4]  = (aac->inherit_super.inherit_super.pts >> 16) & 0xFF;
				buf[5]  = (aac->inherit_super.inherit_super.pts >> 8) & 0xFF;
				buf[6]  = aac->inherit_super.inherit_super.pts & 0xFF;
				buf[7]  = (aac->inherit_super.inherit_super.pts >> 24) & 0xFF;
				// track
				buf[8]  = 0x00;
				buf[9]  = 0x00;
				buf[10] = 0x00;
				// tag
				// ここがどうなるかは、aacの周波数、チャンネル次第。
				buf[11] = 0xA0;
				buf[11] |= 0x02; // 16bit force.
				switch(aac->inherit_super.channel_num) {
				case 1:
					buf[11] |= 0x00;
					break;
				case 2:
					buf[11] |= 0x01;
					break;
				default:
					return false;
				}
				switch(aac->inherit_super.sample_rate) {
				case 44100:
					buf[11] |= 0x0C;
					break;
				case 22050:
					buf[11] |= 0x08;
					break;
				case 11025:
					buf[11] |= 0x04;
					break;
				case 5512:
					buf[11] |= 0x00;
					break;
				default:
					return false;
				}
				// type
				buf[12] = 0x00;
				if(!callback(ptr, buf, 13)) {
					return false;
				}
				if(!callback(ptr, dsi, size)) {
					return false;;
				}
				// post size
				uint32_t post_size = pre_size + 11;
				buf[0] = (post_size >> 24) & 0xFF;
				buf[1] = (post_size >> 16) & 0xFF;
				buf[2] = (post_size >> 8) & 0xFF;
				buf[3] = post_size & 0xFF;
				if(!callback(ptr, buf, 4)) {
					return false;
				}
				writer->audio_track.crc32 = crc32_value;
//				return true;
			}
			// あとは通常のフレームを書き込む
			uint8_t *audio_data = aac->inherit_super.inherit_super.data;
			uint32_t audio_data_size = aac->inherit_super.inherit_super.buffer_size;
			if(aac->type == AacType_adts) {
				// adtsの場合は、7byte目から保持する必要がある。
				audio_data += 7;
				audio_data_size -= 7;
			}
			buf[0]  = 0x08;
			uint32_t pre_size = audio_data_size + 2;
			buf[1]  = (pre_size >> 16) & 0xFF;
			buf[2]  = (pre_size >> 8) & 0xFF;
			buf[3]  = pre_size & 0xFF;
			// pts値
			buf[4]  = (aac->inherit_super.inherit_super.pts >> 16) & 0xFF;
			buf[5]  = (aac->inherit_super.inherit_super.pts >> 8) & 0xFF;
			buf[6]  = aac->inherit_super.inherit_super.pts & 0xFF;
			buf[7]  = (aac->inherit_super.inherit_super.pts >> 24) & 0xFF;
			// track
			buf[8]  = 0x00;
			buf[9]  = 0x00;
			buf[10] = 0x00;
			buf[11] = 0xA0;
			buf[11] |= 0x02; // 16bit force.
			switch(aac->inherit_super.channel_num) {
			case 1:
				buf[11] |= 0x00;
				break;
			case 2:
				buf[11] |= 0x01;
				break;
			default:
				return false;
			}
			switch(aac->inherit_super.sample_rate) {
			case 44100:
				buf[11] |= 0x0C;
				break;
			case 22050:
				buf[11] |= 0x08;
				break;
			case 11025:
				buf[11] |= 0x04;
				break;
			case 5512:
				buf[11] |= 0x00;
				break;
			default:
				return false;
			}
			// type
			buf[12] = 0x01;
			if(!callback(ptr, buf, 13)) {
				return false;
			}
			// data_body
			if(!callback(ptr, audio_data, audio_data_size)) {
				return false;
			}
			uint32_t post_size = pre_size + 11;
			buf[0] = (post_size >> 24) & 0xFF;
			buf[1] = (post_size >> 16) & 0xFF;
			buf[2] = (post_size >> 8) & 0xFF;
			buf[3] = post_size & 0xFF;
			if(!callback(ptr, buf, 4)) {
				return false;
			}
			return true;
		}
		break;
	case frameType_mp3:
	case frameType_nellymoser:
	case frameType_pcmS16:
	case frameType_pcm_alaw:
	case frameType_pcm_mulaw:
	case frameType_speex:
		LOG_PRINT("no write process for the frame.:%d", frame->type);
		break;
	default:
		ERR_PRINT("unexpected data.");
		return false;
	}
	return false;
}
