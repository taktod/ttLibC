/**
 * @file   mp4.h
 * @brief  mp4 container support.
 * @author taktod
 * @date   2016/07/03
 */

#ifndef TTLIBC_CONTAINER_MP4_H_
#define TTLIBC_CONTAINER_MP4_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "container.h"

/**
 * enum for mp4 atom type.
 */
typedef enum ttLibC_Mp4_Type {
	Mp4Type_Ftyp = 'ftyp',
	Mp4Type_Free = 'free',
	Mp4Type_Mdat = 'mdat',
	Mp4Type_Moov = 'moov',
		Mp4Type_Mvhd = 'mvhd',
		Mp4Type_Iods = 'iods',
		Mp4Type_Trak = 'trak',
			Mp4Type_Tkhd = 'tkhd',
			Mp4Type_Edts = 'edts',
			Mp4Type_Mdia = 'mdia',
				Mp4Type_Mdhd = 'mdhd',
				Mp4Type_Hdlr = 'hdlr',
				Mp4Type_Minf = 'minf',
					Mp4Type_Vmhd = 'vmhd',
					Mp4Type_Smhd = 'smhd',
					Mp4Type_Dinf = 'dinf',
					Mp4Type_Stbl = 'stbl',
						Mp4Type_Stsd = 'stsd',
						Mp4Type_Stts = 'stts',
						Mp4Type_Stss = 'stss',
						Mp4Type_Stsc = 'stsc',
						Mp4Type_Stsz = 'stsz',
						Mp4Type_Stco = 'stco',
		Mp4Type_Udta = 'udta',
		Mp4Type_Mvex = 'mvex',
			Mp4Type_Mehd = 'mehd',
			Mp4Type_Trex = 'trex',
			Mp4Type_Trep = 'trep',
	Mp4Type_Styp = 'styp',
	Mp4Type_Sidx = 'sidx',
	Mp4Type_Moof = 'moof',
		Mp4Type_Mfhd = 'mfhd',
		Mp4Type_Traf = 'traf',
			Mp4Type_Tfhd = 'tfhd',
			Mp4Type_Tfdt = 'tfdt',
			Mp4Type_Trun = 'trun',
} ttLibC_Mp4_Type;

/**
 * definition of mp4 object.
 */
typedef struct ttLibC_Container_Mp4 {
	ttLibC_Container inherit_super;
	ttLibC_Mp4_Type type;
	bool is_complete; // for mdat, we could have incomplete atom data.
} ttLibC_Container_Mp4;

typedef ttLibC_Container_Mp4 ttLibC_Mp4;

/**
 * get frame from mp4 container object.
 * @param mp4      target mp4 object.
 * @param callback callback to get frame.
 * @param ptr      user def data pointer.
 */
bool ttLibC_Mp4_getFrame(
		ttLibC_Mp4 *mp4,
		ttLibC_getFrameFunc callback,
		void *ptr);

// -------------------------------------------------------------- //
// reader
typedef struct ttLibC_ContainerReader_Mp4Reader {
	ttLibC_ContainerReader inherit_super;
} ttLibC_ContainerReader_Mp4Reader;

typedef ttLibC_ContainerReader_Mp4Reader ttLibC_Mp4Reader;

typedef bool (* ttLibC_Mp4ReadFunc)(void *ptr, ttLibC_Mp4 *mp4);

/**
 * make mp4 reader object.
 */
ttLibC_Mp4Reader *ttLibC_Mp4Reader_make();

/**
 * read data from binary data.
 * @param reader
 * @param data
 * @param data_size
 * @param callback
 * @param ptr
 * @return true:success / false:error
 */
bool ttLibC_Mp4Reader_read(
		ttLibC_Mp4Reader *reader,
		void *data,
		size_t data_size,
		ttLibC_Mp4ReadFunc callback,
		void *ptr);

/**
 * close reader object.
 * @param reader
 */
void ttLibC_Mp4Reader_close(ttLibC_Mp4Reader **reader);

// -------------------------------------------------------------- //
// writer
/**
 * track info data.(not implemented.)
 */
typedef struct ttLibC_ContainerWriter_Mp4TrackInfo {
	uint16_t track_id;
	ttLibC_Frame_Type frame_type;
} ttLibC_ContainerWriter_Mp4TrackInfo;

/**
 * target writer.
 */
typedef struct ttLibC_ContainerWriter_Mp4Writer {
	ttLibC_ContainerWriter inherit_super;
} ttLibC_ContainerWriter_Mp4Writer;

typedef ttLibC_ContainerWriter_Mp4Writer ttLibC_Mp4Writer;

/**
 * make mp4Writer
 * @param target_frame_types array of use frame type list.
 * @param types_num          number of array.
 */
ttLibC_Mp4Writer *ttLibC_Mp4Writer_make(
		ttLibC_Frame_Type* target_frame_types,
		uint32_t types_num);

/**
 * make mp4Writer with detail information.
 * @param target_frame_types array of use frame type list.
 * @param types_num          number of array.
 * @param max_unit_duration  divide mp4 chunk unit with this duration at least(in milisec.)
 */
ttLibC_Mp4Writer *ttLibC_Mp4Writer_make_ex(
		ttLibC_Frame_Type* target_frame_types,
		uint32_t types_num,
		uint32_t max_unit_duration);

/**
 * write frame data to writer.
 * @param writer
 * @param frame
 * @param callback
 * @param ptr
 */
bool ttLibC_Mp4Writer_write(
		ttLibC_Mp4Writer *writer,
		ttLibC_Frame *frame,
		ttLibC_ContainerWriteFunc callback,
		void *ptr);

/**
 * close writer.
 * @param writer
 */
void ttLibC_Mp4Writer_close(ttLibC_Mp4Writer **writer);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MP4_H_ */
