/*
 * @file   mpegtsReader.c
 * @brief  mpegts container reader.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/13
 */

#include "../mpegts.h"

#include "mpegtsReader.h"

#include "../../log.h"
#include "../../allocator.h"
#include "../../util/hexUtil.h"
#include "../../util/ioUtil.h"

#include <stdlib.h>
#include <string.h>

/*
 * make mpegtsReader.
 */
ttLibC_MpegtsReader *ttLibC_MpegtsReader_make() {
	ttLibC_MpegtsReader_ *reader = (ttLibC_MpegtsReader_ *)ttLibC_ContainerReader_make(containerType_mpegts, sizeof(ttLibC_MpegtsReader_));

	reader->pat = NULL;
	reader->pmt = NULL;
	reader->sdt = NULL;
	reader->pes_list = NULL;
	reader->pes_track_num = 0;
	reader->target_size = 188;

	reader->tmp_buffer = ttLibC_DynamicBuffer_make();
	return (ttLibC_MpegtsReader *)reader;
}

/**
 * read mpegts binary data.
 */
static bool MpegtsReader_read(
		ttLibC_MpegtsReader_ *reader,
		uint8_t *buffer,
		size_t left_size,
		ttLibC_MpegtsReadFunc callback,
		void *ptr) {
	bool result = true;
	if(buffer[0] != 0x47) {
		ERR_PRINT("malformed mpegts packet, not start with 0x47");
		return false;
	}
	uint32_t pid = ((buffer[1] & 0x1F) << 8) | buffer[2];
	if(pid == MpegtsType_sdt) {
		ttLibC_Sdt *sdt = ttLibC_Sdt_getPacket(reader->sdt, buffer, reader->target_size);
		if(sdt == NULL) {
			return false;
		}
		reader->sdt = sdt;
		result = callback(ptr, (ttLibC_Mpegts *)reader->sdt);
	}
	else if(pid == MpegtsType_pat){
		// get pmt pid from pat.
		ttLibC_Pat *pat = ttLibC_Pat_getPacket(reader->pat, buffer, reader->target_size);
		if(pat == NULL) {
			LOG_PRINT("failed to get pat.");
			return false;
		}
		reader->pat = pat;
		reader->pmt_pid = pat->pmt_pid;
		result = callback(ptr, (ttLibC_Mpegts *)reader->pat);
	}
	else if(pid == reader->pmt_pid) {
		// get pes list from pmt.
		ttLibC_Pmt *pmt = ttLibC_Pmt_getPacket(reader->pmt, buffer, reader->target_size, reader->pmt_pid);
		if(pmt == NULL) {
			LOG_PRINT("failed to get pmt.");
			return false;
		}
		reader->pmt = pmt;
		if(reader->pes_track_num != pmt->pes_track_num) {
			// in the case of track num is changed(usually only in first time.)
			if(reader->pes_list != NULL) {
				ERR_PRINT("unexpected pmt update.");
				// in this case, we need to free prev track list.
			}
			reader->pes_list = ttLibC_malloc(sizeof(ttLibC_Pes) * pmt->pes_track_num);
			if(reader->pes_list == NULL) {
				ERR_PRINT("failed to allocate for pes_list");
				return false;
			}
			// initialize pes_list.
			for(int i = 0;i < pmt->pes_track_num;++ i) {
				reader->pes_list[i] = NULL;
			}
		}
		reader->pes_track_num = pmt->pes_track_num; // pes_track_numを保持しておく。
		result = callback(ptr, (ttLibC_Mpegts *)reader->pmt);
	}
	else {
		bool find = false;
		for(int i = 0;i < reader->pes_track_num;++ i) {
			if(pid == reader->pmt->pmtElementaryField_list[i].pid) {
				find = true;
				// if unit start. we realize prev data is finished.
				if((buffer[1] & 0x40) != 0) {
					if(reader->pes_list[i] != NULL && !reader->pes_list[i]->is_used) {
						// if there is data, callback it.
						result = callback(ptr, (ttLibC_Mpegts *)reader->pes_list[i]);
						reader->pes_list[i]->is_used = true;
					}
				}
				else {
					if(reader->pes_list[i] == NULL) {
						// got pes without unit start. skip.
						return true;
					}
				}
				ttLibC_Pes *pes = ttLibC_Pes_getPacket(
						reader->pes_list[i],
						buffer,
						reader->target_size,
						reader->pmt->pmtElementaryField_list[i].stream_type,
						reader->pmt->pmtElementaryField_list[i].pid);
				if(pes == NULL) {
					return false;
				}
				reader->pes_list[i] = pes;
				if(pes->frame_size != 0 && pes->frame_size == pes->inherit_super.inherit_super.inherit_super.buffer_size) {
					result = callback(ptr, (ttLibC_Mpegts *)reader->pes_list[i]);
					reader->pes_list[i]->is_used = true;
				}
				break;
			}
		}
		if(!find) {
			// in the case of not found.
			// find pes before pmt?
			// just skip.
			return true;
		}
	}
	return result;
}

/*
 * read mpegts binary data.
 */
bool ttLibC_MpegtsReader_read(
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
	// do each 188 bytes.
	do {
		uint8_t *buffer = ttLibC_DynamicBuffer_refData(reader_->tmp_buffer);
		size_t left_size = ttLibC_DynamicBuffer_refSize(reader_->tmp_buffer);
		if(left_size < reader_->target_size) {
			// no more enough size. do next time.
			break;
		}
		if(!MpegtsReader_read(reader_, buffer, left_size, callback, ptr)) {
			return false;
		}
		ttLibC_DynamicBuffer_markAsRead(reader_->tmp_buffer, reader_->target_size);
	} while(true);
	ttLibC_DynamicBuffer_clear(reader_->tmp_buffer);
	return true;
}

/*
 * close mpegtsReader
 */
void ttLibC_MpegtsReader_close(ttLibC_MpegtsReader **reader) {
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
	if(target->pes_list != NULL) {
		for(int i = 0;i < target->pes_track_num;++ i) {
			ttLibC_MpegtsPacket_close((ttLibC_MpegtsPacket **)&target->pes_list[i]);
		}
		ttLibC_free(target->pes_list);
	}
	ttLibC_DynamicBuffer_close(&target->tmp_buffer);
	ttLibC_free(target);
	*reader = NULL;
}

