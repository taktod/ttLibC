/**
 * @file   flv.h
 * @brief  flv container support
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/12
 */

#ifndef TTLIBC_CONTAINER_FLV_H_
#define TTLIBC_CONTAINER_FLV_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "container.h"

/**
 * enum for flv tag type.
 */
typedef enum ttLibC_Flv_Type {
	FlvType_audio  = 0x08,
	FlvType_header = 0x00,
	FlvType_meta   = 0x12,
	FlvType_video  = 0x09,
} ttLibC_Flv_Type;

/**
 * definition of flv tag.
 */
typedef struct ttLibC_Container_Flv {
	ttLibC_Container inherit_super;
	ttLibC_Flv_Type type;
} ttLibC_Container_Flv;

typedef ttLibC_Container_Flv ttLibC_Flv;

/**
 * get frame from flv object.
 * @param flv      flv object
 * @param callback callback for get frame
 * @param ptr      user def pointer object.
 * @param true:success false:error.
 */
bool ttLibC_Flv_getFrame(
		ttLibC_Flv *flv,
		ttLibC_getFrameFunc callback,
		void *ptr);

/**
 * close flv object
 * @param flv
 */
void ttLibC_Flv_close(ttLibC_Flv **flv);

// -------------------------------------------------------------- //

/**
 * definition of flv reader.
 */
typedef struct ttLibC_ContainerReader_FlvReader {
	ttLibC_ContainerReader inherit_super;
	bool has_audio;
	bool has_video;
} ttLibC_ContainerReader_FlvReader;

typedef ttLibC_ContainerReader_FlvReader ttLibC_FlvReader;

/**
 * callback of flv reader function.
 * @param ptr user def pointer object.
 * @param flv flv object.
 * @return bool true:continue false:stop
 */
typedef bool (* ttLibC_FlvReadFunc)(void *ptr, ttLibC_Flv *flv);

/**
 * make flv reader.
 * @return flv reader object.
 */
ttLibC_FlvReader *ttLibC_FlvReader_make();

/**
 * read flv object from binary data.
 * @param reader    flv reader object
 * @param data      binary data
 * @param data_size data size
 * @param callback  callback function
 * @param ptr       user def pointer for callback.
 * @return true:success false:error
 */
bool ttLibC_FlvReader_read(
		ttLibC_FlvReader *reader,
		void *data,
		size_t data_size,
		ttLibC_FlvReadFunc callback,
		void *ptr);

/**
 * close flv reader
 * @param reader
 */
void ttLibC_FlvReader_close(ttLibC_FlvReader **reader);

// -------------------------------------------------------------- //

/**
 * definition of flv writer object.
 */
typedef struct ttLibC_ContainerWriter_FlvWriter {
	ttLibC_ContainerWriter inherit_super;
} ttLibC_ContainerWriter_FlvWriter;

typedef ttLibC_ContainerWriter_FlvWriter ttLibC_FlvWriter;

/**
 * make flv writer.
 * @param video_type target video frame type.
 * @param audio_type target audio frame type.
 * @return flv writer object.
 */
ttLibC_FlvWriter *ttLibC_FlvWriter_make(
		ttLibC_Frame_Type video_type,
		ttLibC_Frame_Type audio_type);

/**
 * write frame on flv container.
 * @param writer
 * @param frame
 * @param callback
 * @param ptr
 * @return
 */
bool ttLibC_FlvWriter_write(
		ttLibC_FlvWriter *writer,
		ttLibC_Frame *frame,
		ttLibC_ContainerWriteFunc callback,
		void *ptr);

/**
 * close writer
 * @param writer
 */
void ttLibC_FlvWriter_close(ttLibC_FlvWriter **writer);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_CONTAINER_FLV_H_ */
