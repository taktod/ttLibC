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

ttLibC_Mp4Reader *ttLibC_Mp4Reader_make();

bool ttLibC_Mp4Reader_read(
		ttLibC_Mp4Reader *reader,
		void *data,
		size_t data_size,
		ttLibC_Mp4ReadFunc callback,
		void *ptr);

void ttLibC_Mp4Reader_close(ttLibC_Mp4Reader **reader);

// -------------------------------------------------------------- //
// writer
typedef struct ttLibC_ContainerWriter_Mp4Writer {
	ttLibC_ContainerWriter inherit_super;
} ttLibC_ContainerWriter_Mp4Writer;

typedef ttLibC_ContainerWriter_Mp4Writer ttLibC_Mp4Writer;

// we will have choice, mp4 or fmp4.
ttLibC_Mp4Writer *ttLibC_Mp4Writer_make();

bool ttLibC_Mp4Writer_write(
		ttLibC_Mp4Writer *writer,
		ttLibC_Frame *frame,
		ttLibC_ContainerWriteFunc callback,
		void *ptr);

void ttLibC_Mp4Writer_close(ttLibC_Mp4Writer **writer);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_MP4_H_ */
