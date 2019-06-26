/**
 * @file   theora.h
 * @brief  theora image frame information
 *
 * this code is under 3-Cause BSD license
 *
 * @author taktod
 * @date   2015/08/06
 */

#ifndef TTLIBC_FRAME_VIDEO_THEORA_H_
#define TTLIBC_FRAME_VIDEO_THEORA_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../../ttLibC_predef.h"
#include "video.h"

/**
 * theora frame type definition.
 */
typedef enum ttLibC_Theora_Type {
	TheoraType_identificationHeaderDecodeFrame,
	TheoraType_commentHeaderFrame,
	TheoraType_setupHeaderFrame,
	TheoraType_intraFrame,
	TheoraType_innerFrame,
} ttLibC_Theora_Type;

/**
 * theora frame definition.
 */
typedef struct ttLibC_Frame_Video_Theora {
	/** inherit data from ttLibC_Video */
	ttLibC_Video inherit_super;
	/** information of granule position. */
	uint64_t granule_pos;
	ttLibC_Theora_Type type;
} ttLibC_Frame_Video_Theora;

typedef ttLibC_Frame_Video_Theora ttLibC_Theora;

/**
 * make theora frame
 * @param prev_frame    reuse frame
 * @param type          theora frame type.
 * @param width         width
 * @param height        height
 * @param data          theora data
 * @param data_size     theora data size
 * @param non_copy_mode true:hold the data pointer. false:data will copy.
 * @param pts           pts for theora data.
 * @param timebase      timebase number for pts.
 */
ttLibC_Theora TT_ATTRIBUTE_API *ttLibC_Theora_make(
		ttLibC_Theora *prev_frame,
		ttLibC_Theora_Type type,
		uint32_t width,
		uint32_t height,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase);

/**
 * make clone frame.
 * always make copy buffer on it.
 * @param prev_frame reuse frame object.
 * @param src_frame  source of clone.
 */
ttLibC_Theora TT_ATTRIBUTE_API *ttLibC_Theora_clone(
		ttLibC_Theora *prev_frame,
		ttLibC_Theora *src_frame);

/**
 * check if the theora binary is key frame.
 * @param data      theora data
 * @param data_size theora data size
 * @return true: key frame false:inter frame
 */
bool TT_ATTRIBUTE_API ttLibC_Theora_isKey(void *data, size_t data_size);

/**
 * analyze the width information from theora binary.
 * @param prev_frame ref for prev analyzed theora frame.
 * @param data       theora data
 * @param data_size  theora data size
 * @return 0:error or width size.
 */
uint32_t TT_ATTRIBUTE_API ttLibC_Theora_getWidth(ttLibC_Theora *prev_frame, uint8_t *data, size_t data_size);

/**
 * analyze the height information from theora binary.
 * @param prev_frame ref for prev analyzed theora frame.
 * @param data       theora data
 * @param data_size  theora data size
 * @return 0:error or height size.
 */
uint32_t TT_ATTRIBUTE_API ttLibC_Theora_getHeight(ttLibC_Theora *prev_frame, uint8_t *data, size_t data_size);

/**
 * make frame object from theora binary data.
 * @param prev_frame    ref for prev analyzed theora frame.
 * @param data          theora data
 * @param data_size     theora data size
 * @param non_copy_mode true:hold pointer. false:copy data.
 * @param pts           pts for theora frame.
 * @param timebase      timebase for pts.
 * @return theora frame
 */
ttLibC_Theora TT_ATTRIBUTE_API *ttLibC_Theora_getFrame(
		ttLibC_Theora *prev_frame,
		uint8_t *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase);

/**
 * close frame
 * @param frame
 */
void TT_ATTRIBUTE_API ttLibC_Theora_close(ttLibC_Theora **frame);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_FRAME_VIDEO_THEORA_H_ */
