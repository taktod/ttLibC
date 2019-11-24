/*
 * @file   aac2.c
 * @brief  aac frame information
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2019/11/19
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

#include "aac2.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../_log.h"

#include "../../util/byteUtil.h"
#include "../../util/crc32Util.h"
#include "../../util/hexUtil.h"

typedef ttLibC_Frame_Audio_Aac2 ttLibC_Aac2_;

static uint32_t sample_rate_table[] = {
		96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000
};

ttLibC_Aac2 TT_ATTRIBUTE_API *ttLibC_Aac2_make(
    ttLibC_Aac2 *prev_frame,
    ttLibC_Aac2_Type type,
		uint32_t sample_rate,
		uint32_t sample_num,
		uint32_t channel_num,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		uint32_t object_type) {
  ttLibC_Aac2_ *aac = (ttLibC_Aac2_ *)ttLibC_Audio_make(
      (ttLibC_Audio *)prev_frame,
      sizeof(ttLibC_Aac2_),
      frameType_aac2,
			sample_rate,
			sample_num,
			channel_num,
			data,
			data_size,
			non_copy_mode,
			pts,
			timebase);
  if(aac != NULL) {
    aac->type = type;
    aac->object_type = object_type;
  }  
  return (ttLibC_Aac2 *)aac;
}

ttLibC_Aac2 TT_ATTRIBUTE_API *ttLibC_Aac2_clone(
    ttLibC_Aac2 *prev_frame,
    ttLibC_Aac2 *src_frame) {
  if(src_frame == NULL) {
    return NULL;
  }
  if(src_frame->inherit_super.inherit_super.type != frameType_aac2) {
    ERR_PRINT("try to clone non aac2 frame.");
    return NULL;
  }
  if(prev_frame != NULL && prev_frame->inherit_super.inherit_super.type != frameType_aac2) {
    ERR_PRINT("try to use non aac2 frame for reuse.");
    return NULL;
  }
  ttLibC_Aac2 *aac = ttLibC_Aac2_make(
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
      src_frame->object_type);
  if(aac != NULL) {
    aac->inherit_super.inherit_super.id = src_frame->inherit_super.inherit_super.id;
  }
  return aac;
}

ttLibC_Aac2 TT_ATTRIBUTE_API *ttLibC_Aac2_getFrame(
    ttLibC_Aac2 *prev_frame,
    void *data,
    size_t data_size,
    bool non_copy_mode,
    uint64_t pts,
    uint32_t timebase) {
  if(data_size < 2) {
    // data size is too short.
    return NULL;
  }
  ttLibC_ByteReader *reader = ttLibC_ByteReader_make(data, data_size, ByteUtilType_default);
  if(reader == NULL) {
    ERR_PRINT("failed to make byteReader.");
    return NULL;
  }
  if(ttLibC_ByteReader_bit(reader, 12) != 0xFFF) {
    ttLibC_ByteReader_close(&reader);
    // raw frame
    // prev_frameがなければ、asi情報であるとして、処理するか？
    if(prev_frame == NULL) {
      reader = ttLibC_ByteReader_make(data, data_size, ByteUtilType_default);
      uint32_t object_type = ttLibC_ByteReader_bit(reader, 5);
      if(object_type == 31) {
        object_type = ttLibC_ByteReader_bit(reader, 6);
      }
      uint32_t sample_rate_index = ttLibC_ByteReader_bit(reader, 4);
      uint32_t sample_rate = 44100;
      if(sample_rate_index == 15) {
        LOG_PRINT("sample_rate is not in index_table.");
        sample_rate = ttLibC_ByteReader_bit(reader, 24);
      }
      else {
        sample_rate = sample_rate_table[sample_rate_index];
      }
      uint32_t channel_num = ttLibC_ByteReader_bit(reader, 4);
      ttLibC_ByteReader_close(&reader);
      ttLibC_Aac2 *aac = ttLibC_Aac2_make(
        NULL,
        Aac2Type_asi,
        sample_rate,
        0,
        channel_num,
        data,
        data_size,
        non_copy_mode,
        pts,
        timebase,
        object_type);
      return aac;
    }
    else {
      return ttLibC_Aac2_make(
        prev_frame,
        Aac2Type_raw,
        prev_frame->inherit_super.sample_rate,
        1024,
        prev_frame->inherit_super.channel_num,
        data,
        data_size,
        non_copy_mode,
        pts,
        timebase,
        prev_frame->object_type);
    }
  }
  else {
    // adts frame
    ttLibC_ByteReader_bit(reader, 1);
    ttLibC_ByteReader_bit(reader, 2);
    ttLibC_ByteReader_bit(reader, 1);
    uint32_t object_type = ttLibC_ByteReader_bit(reader, 2) + 1;
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
    if(reader->error != Error_noError) {
      LOG_ERROR(reader->error);
      ttLibC_ByteReader_close(&reader);
      return NULL;
    }
    ttLibC_ByteReader_close(&reader);
    uint8_t *buffer = (uint8_t *)data;
    return ttLibC_Aac2_make(
      prev_frame,
      Aac2Type_raw,
      sample_rate,
      1024,
      channel_num,
      buffer + 7,
      frame_size - 7,
      non_copy_mode,
      pts,
      timebase,
      object_type);
  }
}

void TT_ATTRIBUTE_API ttLibC_Aac2_close(ttLibC_Aac2 **frame) {
  ttLibC_Audio_close_((ttLibC_Audio **)frame);
}


size_t TT_ATTRIBUTE_API ttLibC_Aac2_makeAdtsHeader(
    ttLibC_Aac2 *target,
    void *data,
    size_t data_size) {
  if(target == NULL) {
    return 0;
  }
  if(data_size < 7) {
    ERR_PRINT("adtsHeader buffer size is too small.");
    return 0;
  }
  ttLibC_Aac2_ *aac_ = (ttLibC_Aac2_ *)target;
  if(aac_->object_type >= 5) {
    ERR_PRINT("adts support only profile: main, low, ssr, and ltp.");
    return 0;
  }
  bool found = false;
  uint32_t frequency_index = 0;
  for(int i = 0, max = sizeof(sample_rate_table) / sizeof(uint32_t);i < max;++ i) {
    if(sample_rate_table[i] == aac_->inherit_super.sample_rate) {
      found = true;
      frequency_index = i;
      break;
    }
  }
  if(!found) {
    ERR_PRINT("failed to get frequency index.");
    return 0;
  }
  uint32_t channel_conf = aac_->inherit_super.channel_num;
  size_t aac_size = aac_->inherit_super.inherit_super.buffer_size + 7;
  LOG_PRINT("obj_type:%d, frequency_index:%d, channel_conf:%d", aac_->object_type, frequency_index, channel_conf);
  uint8_t *buf = (uint8_t *)data;
	buf[0] = 0xFF;
	buf[1] = 0xF1;
	buf[2] = (((aac_->object_type - 1) & 0x03) << 6) | ((frequency_index & 0x0F) << 2) | 0x00 | ((channel_conf & 0x07) >> 2);
	buf[3] = ((channel_conf & 0x03) << 6) | ((aac_size >> 11) & 0x03);
	buf[4] = (aac_size >> 3) & 0xFF;
	buf[5] = ((aac_size & 0x07) << 5) | 0x1F;
	buf[6] = 0xFC;
  return 7;
}

size_t TT_ATTRIBUTE_API ttLibC_Aac2_makeAsiHeader(
    ttLibC_Aac2 *target,
    void *data,
    size_t data_size) {
  ttLibC_Aac2_ *aac = (ttLibC_Aac2_ *)target;
  return ttLibC_Aac2_makeAsiHeaderWithParams(
    aac->object_type,
    aac->inherit_super.sample_rate,
    aac->inherit_super.channel_num,
    data,
    data_size);
}

size_t TT_ATTRIBUTE_API ttLibC_Aac2_makeAsiHeaderWithParams(
    uint32_t object_type,
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

uint32_t TT_ATTRIBUTE_API ttLibC_Aac2_getConfigCrc32(ttLibC_Aac2 *aac) {
  // こっちは、面倒なので、channel objectType sampleRateをcrc32かけるか・・・///
  ttLibC_Aac2_ *aac_ = (ttLibC_Aac2_ *)aac;
  ttLibC_Crc32 *crc32 = ttLibC_Crc32_make(0);
  if(crc32 == NULL) {
    return 0;
  }
  uint8_t *buf;
  buf = (uint8_t *)&aac_->object_type;
  for(int i = 0;i < 4;++ i) {
    ttLibC_Crc32_update(crc32, buf[i]);
  }
  buf = (uint8_t *)&aac_->inherit_super.sample_rate;
  for(int i = 0;i < 4;++ i) {
    ttLibC_Crc32_update(crc32, buf[i]);
  }
  buf = (uint8_t *)&aac_->inherit_super.channel_num;
  for(int i = 0;i < 4;++ i) {
    ttLibC_Crc32_update(crc32, buf[i]);
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

