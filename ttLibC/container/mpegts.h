/**
 * @file   mpegts.h
 * @brief  mpegts container support
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/13
 */

#ifndef TTLIBC_CONTAINER_MPEGTS_H_
#define TTLIBC_CONTAINER_MPEGTS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "container.h"
#include "../frame/frame.h"

/**
 * enum for mpegts type.
 */
typedef enum ttLibC_Mpegts_Type {
	MpegtsType_pat = 0x0000,
	MpegtsType_pes = 0x0100,
	MpegtsType_pmt = 0x1000,
	MpegtsType_sdt = 0x0011,
} ttLibC_Mpegts_Type;

/**
 * definition of mpegts
 */
typedef struct ttLibC_Container_Mpegts {
	ttLibC_Container inherit_super;
	ttLibC_Mpegts_Type type;
	uint16_t pid;
} ttLibC_Container_Mpegts;

typedef ttLibC_Container_Mpegts ttLibC_Mpegts;

/**
 * get frame object from mpegts object.
 * @param mpegts
 * @param callback
 * @param ptr
 */
bool ttLibC_Mpegts_getFrame(
		ttLibC_Mpegts *mpegts,
		ttLibC_getFrameFunc callback,
		void *ptr);

/**
 * close mpegts object.
 * @param mpegts
 */
void ttLibC_Mpegts_close(ttLibC_Mpegts **mpegts);

// -------------------------------------------------------------- //

/**
 * mpegts reader definition
 */
typedef struct ttLibC_ContainerReader_MpegtsReader {
	ttLibC_ContainerReader inherit_super;
} ttLibC_ContainerReader_MpegtsReader;

typedef ttLibC_ContainerReader_MpegtsReader ttLibC_MpegtsReader;

/**
 * callback for mpegts object reading.
 * @param ptr    user def pointer.
 * @param mpegts read mpegts object.
 */
typedef bool (* ttLibC_MpegtsReadFunc)(void *ptr, ttLibC_Mpegts *mpegts);

/**
 * make mpegts reader object
 * @return mpegts reader object.
 */
ttLibC_MpegtsReader *ttLibC_MpegtsReader_make();

/**
 * read mpegts object.
 * @param reader
 * @param data
 * @param data_size
 * @param callback
 * @param ptr
 * @return true:success false:error
 */
bool ttLibC_MpegtsReader_read(
		ttLibC_MpegtsReader *reader,
		void *data,
		size_t data_size,
		ttLibC_MpegtsReadFunc callback,
		void *ptr);

/**
 * close mpegts reader
 * @param reader
 */
void ttLibC_MpegtsReader_close(ttLibC_MpegtsReader **reader);

// -------------------------------------------------------------- //

/**
 * definition of mpegts track information.
 */
typedef struct ttLibC_ContainerWriter_MpegtsTrackInfo {
	uint16_t pid;
	ttLibC_Frame_Type frame_type;
} ttLibC_ContainerWriter_MpegtsTrackInfo;

typedef ttLibC_ContainerWriter_MpegtsTrackInfo ttLibC_MpegtsTrackInfo;

/**
 * definition of mpegts writer object.
 */
typedef ttLibC_ContainerWriter ttLibC_MpegtsWriter;

/**
 * make mpegts writer object
 * @param target_frame_types array of ttLibC_Frame_Type
 * @param types_num          size of target_frame_types array.
 * @return mpegts writer object.
 * TODO remove types_num instead of using types_num, add frameType_none for end marker of array.
 */
ttLibC_MpegtsWriter *ttLibC_MpegtsWriter_make(
		ttLibC_Frame_Type* target_frame_types,
		uint32_t types_num);

/**
 * make mpegts writer object (detail)
 * @param target_frame_types array of ttLibC_Frame_Type
 * @param types_num          size of target_frame_types array.
 * @param unit_duration  unit duration for audio data.
 * @return mpegts writer object.
 * TODO remove types_num instead of using types_num, add frameType_none for end marker of array.
 */
ttLibC_MpegtsWriter *ttLibC_MpegtsWriter_make_ex(
		ttLibC_Frame_Type *target_frame_types,
		uint32_t types_num,
		uint32_t unit_duration);

/**
 * update margin of dts calcuration.
 * @param writer
 * @param margin
 * @node default is 20000
 */
bool ttLibC_MpegtsWriter_updateDtsMargin(
		ttLibC_MpegtsWriter *writer,
		uint64_t margin);

/**
 * write frame on mpegts writer.
 * @param writer      mpegts writer object
 * @param update_info if true try to write sdt pat pmt infromation.
 * @param pid         target pid.
 * @param frame       add frame object.
 * @param callback    callback func
 * @param ptr         user def pointer.
 * @return true:success false:error
 */
/*bool ttLibC_MpegtsWriter_write__(
		ttLibC_MpegtsWriter *writer,
		bool update_info,
		uint16_t pid,
		ttLibC_Frame *frame,
		ttLibC_ContainerWriteFunc callback,
		void *ptr);*/

/**
 * write binaries for sdt pat pmt.
 */
bool ttLibC_MpegtsWriter_writeInfo(
		ttLibC_MpegtsWriter *writer,
		ttLibC_ContainerWriteFunc callback,
		void *ptr);

/**
 * write frame on mpegts writer.
 */
bool ttLibC_MpegtsWriter_write(
		ttLibC_MpegtsWriter *writer,
		ttLibC_Frame *frame,
		ttLibC_ContainerWriteFunc callback,
		void *ptr);


/**
 * try to reduce the mpegts data size.
 * @param writer
 * @param reduce_mode_flag
 * @return true:success false:error
 */
bool ttLibC_MpegtsWriter_setReduceMode(
		ttLibC_MpegtsWriter *writer,
		bool reduce_mode_flag);

/**
 * close mpegts writer
 * @param writer
 */
void ttLibC_MpegtsWriter_close(ttLibC_MpegtsWriter **writer);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MPEGTS_H_ */
