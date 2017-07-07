/*
 * @file   mpegtsReader.c
 * @brief  mpegts container reader.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2016/11/18
 */

#include "mpegtsReader.h"
#include "../mpegts.h"
#include "../../ttLibC_predef.h"
#include "../../_log.h"
#include "../../allocator.h"
#include "../../util/hexUtil.h"
#include "../../util/ioUtil.h"

#include <stdlib.h>
#include <string.h>

/*
 * make mpegtsReader.
 */
ttLibC_MpegtsReader TT_VISIBILITY_DEFAULT *ttLibC_MpegtsReader_make() {
	ttLibC_MpegtsReader_ *reader = (ttLibC_MpegtsReader_ *)ttLibC_ContainerReader_make(containerType_mpegts, sizeof(ttLibC_MpegtsReader_));

	reader->pat = NULL;
	reader->pmt = NULL;
	reader->sdt = NULL;
	reader->pes_list = NULL;
	reader->target_size = 188;

	reader->tmp_buffer = ttLibC_DynamicBuffer_make();
	reader->is_reading = false;
	return (ttLibC_MpegtsReader *)reader;
}

static bool MpegtsReader_read(
		ttLibC_MpegtsReader_ *reader,
		uint8_t *buffer,
		size_t left_size,
		ttLibC_MpegtsReadFunc callback,
		void *ptr) {
	(void)left_size;
	bool result = true;
	if(buffer[0] != 0x47) {
		ERR_PRINT("malformed mpegts packet, not start with 0x47");
		return false;
	}
	uint32_t pid = ((buffer[1] & 0x1F) << 8) | buffer[2];
	if(pid == MpegtsType_sdt) {
		ttLibC_Sdt *sdt = ttLibC_Sdt_getPacket(reader->sdt, buffer, reader->target_size);
		if(sdt == NULL) {
			LOG_PRINT("failed to get pat.");
			return false;
		}
		reader->sdt = sdt;
		result = callback(ptr, (ttLibC_Mpegts *)reader->sdt);
	}
	else if(pid == MpegtsType_pat) {
		ttLibC_Pat *pat = ttLibC_Pat_getPacket(reader->pat, buffer, reader->target_size);
		if(pat == NULL) {
			LOG_PRINT("failed to get pat.");
		}
		reader->pat = pat;
		reader->pmt_pid = pat->pmt_pid;
		result = callback(ptr, (ttLibC_Mpegts *)reader->pat);
	}
	else if(pid == reader->pmt_pid) {
		ttLibC_Pmt *pmt =ttLibC_Pmt_getPacket(reader->pmt, buffer, reader->target_size, reader->pmt_pid);
		if(pmt == NULL) {
			LOG_PRINT("failed to get pmt.");
			return false;
		}
		reader->pmt = pmt;
		if(reader->pes_list == NULL) {
			reader->pes_list = ttLibC_StlMap_make();
			if(reader->pes_list == NULL) {
				ERR_PRINT("failed to allocate for pes_list.");
				return false;
			}
		}
		result = callback(ptr, (ttLibC_Mpegts *)reader->pmt);
	}
	else {
		bool find = false;
		// maybe pes...
		for(uint32_t i = 0;i < reader->pmt->pes_track_num;++ i) {
			if(pid == reader->pmt->pmtElementaryField_list[i].pid) {
				find = true;
				ttLibC_Pes *prev_pes = NULL;
				if(reader->pes_list != NULL) {
					prev_pes = (ttLibC_Pes *)ttLibC_StlMap_get(reader->pes_list, (void *)(long)pid);
				}
				// check unit start.
				if((buffer[1] & 0x40) != 0) {
					// prev data is finished.
					if(prev_pes != NULL) {
						if(!prev_pes->is_used) {
							result = callback(ptr, (ttLibC_Mpegts *)prev_pes);
							prev_pes->is_used = true;
						}
					}
				}
				else {
					if(prev_pes == NULL) {
						// pes without unit start... skip this data.
						return true;
					}
				}
				ttLibC_Pes *pes = ttLibC_Pes_getPacket(
						prev_pes,
						buffer,
						reader->target_size,
						reader->pmt->pmtElementaryField_list[i].stream_type,
						reader->pmt->pmtElementaryField_list[i].pid);
				if(pes == NULL) {
					return false;
				}
				ttLibC_StlMap_put(reader->pes_list, (void *)(long)pid, (void *)pes);
				if(pes->frame_size != 0 && pes->frame_size == pes->inherit_super.inherit_super.inherit_super.buffer_size) {
					result = callback(ptr, (ttLibC_Mpegts *)pes);
					pes->is_used = true;
				}
				break;
			}
		}
		// ignore incomplete data.
		if(!find) {
			return true;
		}
	}
	return result;
}

bool TT_VISIBILITY_DEFAULT ttLibC_MpegtsReader_read(
		ttLibC_MpegtsReader *reader,
		void *data,
		size_t data_size,
		ttLibC_MpegtsReadFunc callback,
		void *ptr) {
	ttLibC_MpegtsReader_ *reader_ = (ttLibC_MpegtsReader_ *)reader;
	if(reader_ == NULL) {
		ERR_PRINT("reader is null");
		return false;
	}
	ttLibC_DynamicBuffer_append(reader_->tmp_buffer, data, data_size);
	if(reader_->is_reading) {
		return true;
	}
	reader_->is_reading = true;
	// do each 188 bytes.
	do {
		uint8_t *buffer = ttLibC_DynamicBuffer_refData(reader_->tmp_buffer);
		size_t left_size = ttLibC_DynamicBuffer_refSize(reader_->tmp_buffer);
		if(left_size < reader_->target_size) {
			// no more enough size. do next time.
			break;
		}
		if(!MpegtsReader_read(reader_, buffer, left_size, callback, ptr)) {
			reader_->is_reading = false;
			return false;
		}
		ttLibC_DynamicBuffer_markAsRead(reader_->tmp_buffer, reader_->target_size);
	} while(true);
	ttLibC_DynamicBuffer_clear(reader_->tmp_buffer);
	reader_->is_reading = false;
	return true;
}

static bool MpegtsReader_closePes(void *ptr, void *key, void *item) {
	(void)ptr;
	(void)key;
	if(item != NULL) {
		ttLibC_MpegtsPacket_close((ttLibC_MpegtsPacket **)&item);
	}
	return true;
}

void TT_VISIBILITY_DEFAULT ttLibC_MpegtsReader_close(ttLibC_MpegtsReader **reader) {
	ttLibC_MpegtsReader_ *target = (ttLibC_MpegtsReader_ *)*reader;
	if(target == NULL) {
		return;
	}
	if(target->inherit_super.inherit_super.type != containerType_mpegts) {
		ERR_PRINT("this reader is not mpegts reader.");
		return;
	}
	ttLibC_MpegtsPacket_close((ttLibC_MpegtsPacket **)&target->pat);
	ttLibC_MpegtsPacket_close((ttLibC_MpegtsPacket **)&target->pmt);
	ttLibC_MpegtsPacket_close((ttLibC_MpegtsPacket **)&target->sdt);
	ttLibC_StlMap_forEach(target->pes_list, MpegtsReader_closePes, NULL);
	ttLibC_StlMap_close(&target->pes_list);
	ttLibC_DynamicBuffer_close(&target->tmp_buffer);
	ttLibC_free(target);
	*reader = NULL;
}
