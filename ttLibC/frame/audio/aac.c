/*
 * @file   aac.c
 * @brief  aac frame information
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/07/23
 */

/*
 * memo for data format.
 * adts header.
 * 12bit syncbit fill with 1
 * 1bit id
 * 2bit layer
 * 1bit protection absent
 * 2bit profile(object type - 1)
 * 4bit sampling frequence index
 * 1bit private bit
 * 3bit channel configuration
 * 1bit original flag
 * 1bit home
 * 1bit copyright identification bit
 * 1bit copyright identification start
 * 13bit frame size
 * 11bit adts buffer full ness
 * 2bit no raw data blocks in frame.
 *
 * for dsi info.
 * 5bit object1 type.(profile + 1 same as ttLibC_Aac_Object)
 * [6bit object2 type.(in the case of object1 = 31 only.)]
 * 4bit frequencyIndex.(see sample_rate_table array.)
 * [24bit frequency.(in the case of frequencyIndex = 15 only.)]
 * 4bit channelConfiguration.(channel num 1:monoral 2:stereo.)
 * bit (padding for 0.)
 */

#include "aac.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../log.h"
#include "../../allocator.h"

#include "../../util/byteUtil.h"
#include "../../util/crc32Util.h"
#include "../../util/hexUtil.h"

typedef struct {
	ttLibC_Aac inherit_super;
	uint64_t dsi_info; // for raw, need to have dsi_information.
} ttLibC_Frame_Audio_Aac_;

typedef ttLibC_Frame_Audio_Aac_ ttLibC_Aac_;

static uint32_t sample_rate_table[] = {
		96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000
};

/*
 * make aac frame.
 * @param prev_frame    reuse frame.
 * @param type          type of aac
 * @param sample_rate   sample rate of data
 * @param sample_num    sample num of data(1024 fixed?)
 * @param channel_num   channel number of data
 * @param data          aac data
 * @param data_size     aac data size
 * @param non_copy_mode true:hold the data pointer. false:data will copy.
 * @param pts           pts for aac data.
 * @param timebase      timebase number for pts.
 * @return aac object.
 */
ttLibC_Aac *ttLibC_Aac_make(
		ttLibC_Aac *prev_frame,
		ttLibC_Aac_Type type,
		uint32_t sample_rate,
		uint32_t sample_num,
		uint32_t channel_num,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		uint64_t dsi_info) {
	ttLibC_Aac_ *aac = (ttLibC_Aac_ *)prev_frame;
	size_t data_size_ = data_size;
	size_t buffer_size_ = data_size;
	switch(type) {
	case AacType_adts:
	case AacType_raw:
		break;
	default:
		ERR_PRINT("unknown aac type.%d", type);
		return NULL;
	}
	if(aac == NULL) {
		aac = ttLibC_malloc(sizeof(ttLibC_Aac_));
		if(aac == NULL) {
			ERR_PRINT("failed to allocate memory for aac frame.");
			return NULL;
		}
		aac->inherit_super.inherit_super.inherit_super.data = NULL;
	}
	else {
		if(!aac->inherit_super.inherit_super.inherit_super.is_non_copy) {
			if(non_copy_mode || aac->inherit_super.inherit_super.inherit_super.data_size < data_size) {
				ttLibC_free(aac->inherit_super.inherit_super.inherit_super.data);
				aac->inherit_super.inherit_super.inherit_super.data = NULL;
			}
			else {
				data_size_ = aac->inherit_super.inherit_super.inherit_super.data_size;
			}
		}
	}
	aac->dsi_info                                              = dsi_info;
	aac->inherit_super.type                                    = type;
	aac->inherit_super.inherit_super.channel_num               = channel_num;
	aac->inherit_super.inherit_super.sample_rate               = sample_rate;
	aac->inherit_super.inherit_super.sample_num                = sample_num;
	aac->inherit_super.inherit_super.inherit_super.buffer_size = buffer_size_;
	aac->inherit_super.inherit_super.inherit_super.data_size   = data_size_;
	aac->inherit_super.inherit_super.inherit_super.is_non_copy = non_copy_mode;
	aac->inherit_super.inherit_super.inherit_super.pts         = pts;
	aac->inherit_super.inherit_super.inherit_super.timebase    = timebase;
	aac->inherit_super.inherit_super.inherit_super.type        = frameType_aac;
	if(non_copy_mode) {
		aac->inherit_super.inherit_super.inherit_super.data = data;
	}
	else {
		if(aac->inherit_super.inherit_super.inherit_super.data == NULL) {
			aac->inherit_super.inherit_super.inherit_super.data = ttLibC_malloc(data_size);
			if(aac->inherit_super.inherit_super.inherit_super.data == NULL) {
				ERR_PRINT("failed to allocate memory for data.");
				if(prev_frame == NULL) {
					ttLibC_free(aac);
				}
				return NULL;
			}
		}
		memcpy(aac->inherit_super.inherit_super.inherit_super.data, data, data_size);
	}
	return (ttLibC_Aac *)aac;
}

/*
 * make clone frame.
 * always make copy buffer on it.
 * @param prev_frame reuse frame object.
 * @param src_frame  source of clone.
 */
ttLibC_Aac *ttLibC_Aac_clone(
		ttLibC_Aac *prev_frame,
		ttLibC_Aac *src_frame) {
	if(src_frame == NULL) {
		return NULL;
	}
	if(src_frame->inherit_super.inherit_super.type != frameType_aac) {
		ERR_PRINT("try to clone non aac frame.");
		return NULL;
	}
	if(prev_frame != NULL && prev_frame->inherit_super.inherit_super.type != frameType_aac) {
		ERR_PRINT("try to use non aac frame for reuse.");
		return NULL;
	}
	ttLibC_Aac_ *src_frame_ = (ttLibC_Aac_ *)src_frame;
	ttLibC_Aac *aac = ttLibC_Aac_make(
			prev_frame,
			src_frame->type,
			src_frame->inherit_super.sample_rate,
			src_frame->inherit_super.sample_num,
			src_frame->inherit_super.channel_num,
			src_frame->inherit_super.inherit_super.data,
			src_frame->inherit_super.inherit_super.buffer_size,
			false,
			src_frame->inherit_super.inherit_super.pts,
			src_frame->inherit_super.inherit_super.timebase,
			src_frame_->dsi_info);
	if(aac != NULL) {
		aac->inherit_super.inherit_super.id = src_frame->inherit_super.inherit_super.id;
	}
	return aac;
}

/*
 * analyze aac frame and make data.
 * only support adts.
 * @param prev_frame    reuse frame
 * @param data          aac binary data.
 * @param data_size     data size
 * @param non_copy_mode true:hold data pointer false:copy data.
 * @param pts           pts for aac frame.
 * @param timebase      timebase for pts.
 * @return aac object
 */
ttLibC_Aac *ttLibC_Aac_getFrame(
		ttLibC_Aac *prev_frame,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	ttLibC_ByteReader *reader = ttLibC_ByteReader_make(data, data_size, ByteUtilType_default);
	if(ttLibC_ByteReader_bit(reader, 12) != 0xFFF) {
		ERR_PRINT("sync bit is invalid.");
		ttLibC_ByteReader_close(&reader);
		return NULL;
	}
	ttLibC_ByteReader_bit(reader, 1);
	ttLibC_ByteReader_bit(reader, 2);
	ttLibC_ByteReader_bit(reader, 1);
	ttLibC_ByteReader_bit(reader, 2);
	uint32_t sample_rate_index = ttLibC_ByteReader_bit(reader, 4);
	uint32_t sample_rate = sample_rate_table[sample_rate_index];
	ttLibC_ByteReader_bit(reader, 1);
	uint32_t channel_num = ttLibC_ByteReader_bit(reader, 3);
	ttLibC_ByteReader_bit(reader, 1);
	ttLibC_ByteReader_bit(reader, 1);
	ttLibC_ByteReader_bit(reader, 1);
	ttLibC_ByteReader_bit(reader, 1);
	uint32_t frame_size = ttLibC_ByteReader_bit(reader, 13);
	ttLibC_ByteReader_bit(reader, 11);
	ttLibC_ByteReader_bit(reader, 2);
	if(reader == NULL) {
		ERR_PRINT("failed to make byteReader.");
		return NULL;
	}
	if(reader->error != Error_noError) {
		LOG_ERROR(reader->error);
		ttLibC_ByteReader_close(&reader);
		return NULL;
	}
	ttLibC_ByteReader_close(&reader);
	// this frame_size includes the adts header.
	return ttLibC_Aac_make(
			prev_frame,
			AacType_adts,
			sample_rate,
			1024,
			channel_num,
			data,
			frame_size,
			non_copy_mode,
			pts,
			timebase,
			0);
}

/*
 * get adts header from aac information.
 * @param target_aac target aac object.
 * @param data       written target buffer.
 * @param data_size  buffer size
 * @return 0:error others:written size.
 */
size_t ttLibC_Aac_readAdtsHeader(
		ttLibC_Aac *target_aac,
		void *data,
		size_t data_size) {
	if(target_aac == NULL) {
		return 0;
	}
	if(data_size < 7) {
		ERR_PRINT("adtsHeader buffer size is too small.");
		return 0;
	}
	ttLibC_Aac_ *aac_ = (ttLibC_Aac_ *)target_aac;
	ttLibC_ByteReader *reader = ttLibC_ByteReader_make(&aac_->dsi_info, sizeof(aac_->dsi_info), ByteUtilType_default);
	uint32_t object_type = ttLibC_ByteReader_bit(reader, 5);
	if(object_type == 31) {
		ERR_PRINT("adts support only profile:main, low, ssr, and ltp.");
		object_type = ttLibC_ByteReader_bit(reader, 6);
		ttLibC_ByteReader_close(&reader);
		return 0;
	}
	uint32_t frequency_index = ttLibC_ByteReader_bit(reader, 4);
	if(frequency_index == 15) {
		ERR_PRINT("not tested yet. now return error.");
		uint32_t frequency = ttLibC_ByteReader_bit(reader, 24);
		ttLibC_ByteReader_close(&reader);
		return 0;
	}
	uint32_t channel_conf = ttLibC_ByteReader_bit(reader, 4);
	if(reader == NULL) {
		ERR_PRINT("failed to allocate byteReader.");
		return NULL;
	}
	if(reader->error != Error_noError) {
		LOG_ERROR(reader->error);
		ttLibC_ByteReader_close(&reader);
		return NULL;
	}
	ttLibC_ByteReader_close(&reader);
	-- object_type; // to make adts, need to decrement.
	size_t aac_size = target_aac->inherit_super.inherit_super.buffer_size + 7;
	// ready to work. make adts header.
	uint8_t *buf = (uint8_t *)data;
	buf[0] = 0xFF;
	buf[1] = 0xF1;
	buf[2] = ((object_type & 0x03) << 6) | ((frequency_index & 0x0F) << 2) | 0x00 | ((channel_conf & 0x07) >> 2);
	buf[3] = ((channel_conf & 0x03) << 6) | ((aac_size >> 11) & 0x03);
	buf[4] = (aac_size >> 3) & 0xFF;
	buf[5] = ((aac_size & 0x07) << 5) | 0x1F;
	buf[6] = 0xFC;
	return 7;
}

/**
 * calcurate crc32 value for configdata.
 * if this value is changed, the configuration of aac is changed.
 * @param aac target aac object.
 * @return value of crc32. 0 for error.
 */
uint32_t ttLibC_Aac_getConfigCrc32(ttLibC_Aac *aac) {
	ttLibC_Aac_ *aac_ = (ttLibC_Aac_ *)aac;
	ttLibC_Crc32 *crc32 = ttLibC_Crc32_make(0);
	if(crc32 == NULL) {
		return 0;
	}
	uint8_t *buf;
	switch(aac->type) {
	case AacType_raw:
		// treat dsi_info as 8byte data.
		buf = (uint8_t *)(&aac_->dsi_info);
		for(int i = 0;i < 8;i ++) {
			ttLibC_Crc32_update(crc32, buf[i]);
		}
		break;
	default:
	case AacType_adts:
		// for adts check until 5bits on the forth byte.
		buf = aac->inherit_super.inherit_super.data;
		ttLibC_Crc32_update(crc32, buf[0]);
		ttLibC_Crc32_update(crc32, buf[1]);
		ttLibC_Crc32_update(crc32, buf[2]);
		ttLibC_Crc32_update(crc32, buf[3] & 0xF8);
		break;
	}
	uint32_t value = ttLibC_Crc32_getValue(crc32);
	if(crc32->error != Error_noError) {
		LOG_ERROR(crc32->error);
		ttLibC_Crc32_close(&crc32);
		return 0;
	}
	ttLibC_Crc32_close(&crc32);
	return value;
}

/**
 * get dsi buffer for aac data.
 * @param aac
 * @param data
 * @param data_size
 * @return write size. 0 for error.
 */
size_t ttLibC_Aac_readDsiInfo(
		ttLibC_Aac *aac,
		void *data,
		size_t data_size) {
	ttLibC_Aac_ *aac_ = (ttLibC_Aac_ *)aac;
	uint8_t *buf;
	switch(aac->type) {
	case AacType_raw:
		{
			// dsi_info is just copy of dsi.
			// however, need to check the length.
			buf = (uint8_t *)(&aac_->dsi_info);
			ttLibC_ByteReader *reader = ttLibC_ByteReader_make(buf, 8, ByteUtilType_default);
			if(reader == NULL) {
				ERR_PRINT("failed to allocate byteReader");
				return 0;
			}
			uint32_t need_bit = 5;
			if(ttLibC_ByteReader_bit(reader, 5) == 31) {
				ttLibC_ByteReader_bit(reader, 6);
				need_bit += 6;
			}
			need_bit += 4;
			if(ttLibC_ByteReader_bit(reader, 4) == 15) {
				ttLibC_ByteReader_bit(reader, 24);
				need_bit += 24;
			}
			need_bit += 4;
			// got the need bits length.
			bool round_up = ((need_bit % 8) != 0);
			uint32_t size = need_bit / 8;
			if(round_up) {
				++ size;
			}
			if(reader->error != Error_noError) {
				LOG_ERROR(reader->error);
				ttLibC_ByteReader_close(&reader);
				return 0;
			}
			ttLibC_ByteReader_close(&reader);
			// copy the data.
			uint8_t *dat = data;
			for(uint32_t i = 0;i < size;++ i) {
				dat[i] = buf[i];
			}
			return size;
		}
		break;
	case AacType_adts:
		{
			// only check 3 or 4 byte.
			buf = aac->inherit_super.inherit_super.data;
			buf += 2;
			ttLibC_ByteReader *reader = ttLibC_ByteReader_make(buf, 2, ByteUtilType_default);
			if(reader == NULL) {
				ERR_PRINT("failed to make byteReader");
				return 0;
			}
			uint32_t object_type = ttLibC_ByteReader_bit(reader, 2) + 1;
			uint32_t frequency_index = ttLibC_ByteReader_bit(reader, 4);
			ttLibC_ByteReader_bit(reader, 1);
			uint32_t channel_conf = ttLibC_ByteReader_bit(reader, 3);
			if(reader->error != Error_noError) {
				LOG_ERROR(reader->error);
				ttLibC_ByteReader_close(&reader);
				return 0;
			}
			// ready to go.
			ttLibC_ByteReader_close(&reader);
			uint8_t *dat = data;
			if(frequency_index == 15) {
				ERR_PRINT("I don't now how to deal with frequency index is 15.");
				return 0;
			}
			if(object_type == 31) {
				LOG_PRINT("not check yet.");
				dat[0] = (31 << 3) | ((object_type >> 3) & 0x07);
				dat[1] = ((object_type << 5) & 0xE0) | ((frequency_index << 1) & 0x1E) | ((channel_conf >> 3) & 0x01);
				dat[2] = (channel_conf << 5) & 0xE0;
				return 3;
			}
			else {
				dat[0] = ((object_type << 3) & 0xF8) | ((frequency_index >> 1) & 0x07);
				dat[1] = ((frequency_index << 7) & 0x80) | ((channel_conf << 3) & 0x78);
				return 2;
			}
		}
	default:
		return 0;
	}
}

/**
 * make dsi buffer from information.
 * @param object_type
 * @param sample_rate
 * @param channel_num
 * @param data
 * @param data_size
 * @return size of generate dsi information.
 */
size_t ttLibC_Aac_getDsiInfo(
		ttLibC_Aac_Object object_type,
		uint32_t sample_rate,
		uint32_t channel_num,
		void *data,
		size_t data_size) {
	memset(data, 0, data_size);
	ttLibC_ByteConnector *connector = ttLibC_ByteConnector_make(data, data_size, ByteUtilType_default);
	if(connector == NULL) {
		ERR_PRINT("failed to make byteConnector.");
		return 0;
	}
	// object_type
	if((int)(object_type) > 31) {
		ttLibC_ByteConnector_bit(connector, 31, 5);
		ttLibC_ByteConnector_bit(connector, object_type, 6);
	}
	else {
		ttLibC_ByteConnector_bit(connector, object_type, 5);
	}
	// frequency
	bool found = false;
	uint32_t index = 15;
	for(int i = 0, max = sizeof(sample_rate_table) / sizeof(uint32_t);i < max;++ i) {
		if(sample_rate_table[i] == sample_rate) {
			found = true;
			index = i;
			break;
		}
	}
	if(found) {
		ttLibC_ByteConnector_bit(connector, index, 4);
	}
	else {
		// not in index.
		ttLibC_ByteConnector_bit(connector, 15, 4);
		ttLibC_ByteConnector_bit(connector, sample_rate, 24);
	}
	// channel_configuration
	ttLibC_ByteConnector_bit(connector, channel_num, 4);
	size_t write_size = connector->write_size;
	if(connector->error != Error_noError) {
		LOG_ERROR(connector->error);
		ttLibC_ByteConnector_close(&connector);
		return 0;
	}
	ttLibC_ByteConnector_close(&connector);
	return write_size;
}

/*
 * close frame
 * @param frame
 */
void ttLibC_Aac_close(ttLibC_Aac **frame) {
	ttLibC_Aac_ *target = (ttLibC_Aac_ *)*frame;
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.inherit_super.inherit_super.type != frameType_aac) {
		ERR_PRINT("found non aac frame in aac_close.");
		return;
	}
	if(!target->inherit_super.inherit_super.inherit_super.is_non_copy) {
		ttLibC_free(target->inherit_super.inherit_super.inherit_super.data);
	}
	ttLibC_free(target);
	*frame = NULL;
}
