/**
 * @file   mpegts.h
 * @brief  
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/13
 * TODO やっとくこと。
 * ・終端の書き込み動作がない
 * ・音声のみのmpegtsの書き出しが不可能
 * ・映像のみのmpegtsの書き出しも不可能
 * ・複数音声、複数映像の書き出しも不可能
 */

#ifndef TTLIBC_CONTAINER_MPEGTS_H_
#define TTLIBC_CONTAINER_MPEGTS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "container.h"
#include "../frame/frame.h"

#define MaxPesTracks 5

/**
 * enum for mpegts type.
 */
typedef enum {
	MpegtsType_pat = 0x0000,
	MpegtsType_pes = 0x0100,
	MpegtsType_pmt = 0x1000,
	MpegtsType_sdt = 0x0011,
} ttLibC_Mpegts_Type;

/**
 * definition of mpegts
 */
typedef struct {
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
typedef struct {
	ttLibC_ContainerReader inherit_super;
} ttLibC_ContainerReader_MpegtsReader;

typedef ttLibC_ContainerReader_MpegtsReader ttLibC_MpegtsReader;

/**
 * callback for mpegts object reading.
 * @param ptr    user def pointer.
 * @param mpegts read mpegts object.
 */
typedef bool (* ttLibC_MpegtsReaderFunc)(void *ptr, ttLibC_Mpegts *mpegts);

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
		ttLibC_MpegtsReaderFunc callback,
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
typedef struct {
	uint16_t pid;
	ttLibC_Frame_Type frame_type;
} ttLibC_ContainerWriter_MpegtsTrackInfo;

typedef ttLibC_ContainerWriter_MpegtsTrackInfo ttLibC_MpegtsTrackInfo;

/**
 * definition of mpegts writer object.
 */
typedef struct {
	ttLibC_ContainerWriter inherit_super;
	/** ref the track info to get pid and frame type. */
	ttLibC_MpegtsTrackInfo trackInfo[MaxPesTracks];
} ttLibC_ContainerWriter_MpegtsWriter;

typedef ttLibC_ContainerWriter_MpegtsWriter ttLibC_MpegtsWriter;

/**
 * make mpegts writer object
 * @param target_frame_types array of ttLibC_Frame_Type
 * @param types_num          size of target_frame_types array.
 * @return mpegts writer object.
 */
ttLibC_MpegtsWriter *ttLibC_MpegtsWriter_make(
		ttLibC_Frame_Type* target_frame_types,
		uint32_t types_num);

/**
 * make mpegts writer object (detail)
 * @param target_frame_types array of ttLibC_Frame_Type
 * @param types_num          size of target_frame_types array.
 * @param max_unit_duration  unit duration for audio data.
 * @return mpegts writer object.
 */
ttLibC_MpegtsWriter *ttLibC_MpegtsWriter_make_ex(
		ttLibC_Frame_Type *target_frame_types,
		uint32_t types_num,
		uint32_t max_unit_duration);

/**
 * write frame on mpegts writer.
 * @param writer           mpegts writer object
 * @param update_info_flag if true try to write sdt pat pmt infromation.
 * @param pid              target pid.
 * @param frame            add frame object.
 * @param callback         callback func
 * @param ptr              user def pointer.
 * @return true:success false:error
 */
bool ttLibC_MpegtsWriter_write(
		ttLibC_MpegtsWriter *writer,
		bool update_info_flag,
		uint16_t pid,
		ttLibC_Frame *frame,
		ttLibC_ContainerWriterFunc callback,
		void *ptr);

/**
 * close mpegts writer
 * @param writer
 */
void ttLibC_MpegtsWriter_close(ttLibC_MpegtsWriter **writer);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MPEGTS_H_ */
