/**
 * @file   mpegtsPacket.h
 * @brief  mpegts container support.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2016/11/18
 */

#ifndef TTLIBC_CONTAINER_MPEGTS_MPEGTSPACKET_H_
#define TTLIBC_CONTAINER_MPEGTS_MPEGTSPACKET_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../../ttLibC_predef.h"
#include "../mpegts.h"
#include "../containerCommon.h"
#include "../../util/byteUtil.h"

/**
 * adaptation field
 */
typedef struct ttLibC_AdaptationField {
	uint32_t size;
	ttLibC_uint1_t discontinuityIndicator;
	ttLibC_uint1_t randomAccessIndicator;
	ttLibC_uint1_t elementaryStreamPriorityIndicator;
	ttLibC_uint1_t pcrFlag;
	ttLibC_uint1_t opcrFlag;
	ttLibC_uint1_t splicingPointFlag;
	ttLibC_uint1_t transportPrivateDataFlag;
	ttLibC_uint1_t adaptationFieldExtensionFlag;
	uint64_t pcrBase;
	uint32_t pcrExtension;
} ttLibC_AdaptationField;

/**
 * header information (first 4 bytes)
 */
typedef struct ttLibC_MpegtsPacket_Header {
	ttLibC_uint8_t  syncByte;
	ttLibC_uint1_t  transportErrorIndicator;
	ttLibC_uint1_t  payloadUnitStartIndicator;
	ttLibC_uint1_t  transportPriority;
	ttLibC_uint13_t pid;
	ttLibC_uint2_t  scramblingControl;
	ttLibC_uint1_t  adaptationFieldExist;
	ttLibC_uint1_t  payloadFieldExist;
	ttLibC_uint4_t  continuityCounter;

	ttLibC_AdaptationField adaptationField;
} ttLibC_MpegtsPacket_Header;

/**
 * header information for program packet(pat pmt sdt...)
 */
typedef struct ttLibC_ProgramPacket_Header {
	ttLibC_MpegtsPacket_Header header;

	ttLibC_uint8_t  pointerField;
	ttLibC_uint8_t  tableId;
	ttLibC_uint1_t  sectionSyntaxIndicator;
	ttLibC_uint1_t  reservedFutureUse1;
	ttLibC_uint2_t  reserved1;
	ttLibC_uint12_t sectionLength;
	ttLibC_uint16_t programNumber;
	ttLibC_uint2_t  reserved;
	ttLibC_uint5_t  versionNumber;
	ttLibC_uint1_t  currentNextOrder;
	ttLibC_uint8_t  sectionNumber;
	ttLibC_uint8_t  lastSectionNumber;
} ttLibC_ProgramPacket_Header;

/**
 * definition of mpegtsPacket.
 */
typedef struct ttLibC_Container_MpegtsPacket {
	ttLibC_Mpegts inherit_super;
	uint8_t continuity_counter;
} ttLibC_Container_MpegtsPacket;

typedef ttLibC_Container_MpegtsPacket ttLibC_MpegtsPacket;

/**
 * make mpegtsPacket
 * @param prev_packet
 * @param data
 * @param data_size
 * @param non_copy_mode
 * @param pts
 * @param timebase
 * @param type
 * @param pid
 * @param continuity_counter
 */
ttLibC_MpegtsPacket TT_ATTRIBUTE_INNER *ttLibC_MpegtsPacket_make(
		ttLibC_MpegtsPacket *prev_packet,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase,
		ttLibC_Mpegts_Type type,
		uint16_t pid,
		uint8_t continuity_counter);

/**
 * loadMpegtsPacketHeader information(first 4byte and adaptation field.)
 * @param reader
 * @param header_info
 * @return true:ok false:error
 */
bool TT_ATTRIBUTE_INNER ttLibC_MpegtsPacket_loadMpegtsPacketHeader(
		ttLibC_ByteReader *reader,
		ttLibC_MpegtsPacket_Header *header_info);

/**
 * loadProgramPacketHeader information(first 4byte, adaptation field, and program packet information(til lastSectionNumber.))
 * @param reader
 * @param header_info
 * @return true:ok false:error
 */
bool TT_ATTRIBUTE_INNER ttLibC_MpegtsPacket_loadProgramPacketHeader(
		ttLibC_ByteReader *reader,
		ttLibC_ProgramPacket_Header *header_info);

/**
 * close mpegtsPacket
 * @param packet
 */
void TT_ATTRIBUTE_INNER ttLibC_MpegtsPacket_close(ttLibC_MpegtsPacket **packet);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MPEGTS_MPEGTSPACKET_H_ */
