/*
 * @file   theora.c
 * @brief  theora image frame information
 *
 * this code is under 3-Cause BSD license
 *
 * @author taktod
 * @date   2015/08/06
 */

#include "theora.h"
#include "../../log.h"
#include "../../util/byteUtil.h"
#include "../../util/ioUtil.h"

typedef ttLibC_Frame_Video_Theora ttLibC_Theora_;

/*
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
ttLibC_Theora *ttLibC_Theora_make(
		ttLibC_Theora *prev_frame,
		ttLibC_Theora_Type type,
		uint32_t width,
		uint32_t height,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	ttLibC_Video_Type video_type = videoType_inner;
	switch(type) {
	case TheoraType_commentHeaderFrame:
	case TheoraType_identificationHeaderDecodeFrame:
	case TheoraType_setupHeaderFrame:
	default:
		video_type = videoType_info;
		break;
	case TheoraType_innerFrame:
		video_type = videoType_inner;
		break;
	case TheoraType_intraFrame:
		video_type = videoType_key;
		break;
	}
	ttLibC_Theora_ *theora = (ttLibC_Theora_ *)ttLibC_Video_make(
			(ttLibC_Video *)prev_frame,
			sizeof(ttLibC_Theora_),
			frameType_theora,
			video_type,
			width,
			height,
			data,
			data_size,
			non_copy_mode,
			pts,
			timebase);
	if(theora != NULL) {
		theora->type = type;
	}
	return (ttLibC_Theora *)theora;
}

/*
 * make clone frame.
 * always make copy buffer on it.
 * @param prev_frame reuse frame object.
 * @param src_frame  source of clone.
 */
ttLibC_Theora *ttLibC_Theora_clone(
		ttLibC_Theora *prev_frame,
		ttLibC_Theora *src_frame) {
	if(src_frame == NULL) {
		return NULL;
	}
	if(src_frame->inherit_super.inherit_super.type != frameType_theora) {
		ERR_PRINT("try to clone non theora frame.");
		return NULL;
	}
	if(prev_frame != NULL && prev_frame->inherit_super.inherit_super.type != frameType_theora) {
		ERR_PRINT("try to use non theora frame for reuse.");
		return NULL;
	}
	ttLibC_Theora *theora = ttLibC_Theora_make(
			prev_frame,
			src_frame->type,
			src_frame->inherit_super.width,
			src_frame->inherit_super.height,
			src_frame->inherit_super.inherit_super.data,
			src_frame->inherit_super.inherit_super.buffer_size,
			false,
			src_frame->inherit_super.inherit_super.pts,
			src_frame->inherit_super.inherit_super.timebase);
	if(theora != NULL) {
		theora->inherit_super.inherit_super.id = src_frame->inherit_super.inherit_super.id;
	}
	return theora;
}

/*
 * check if the theora binary is key frame.
 * @param data      theora data
 * @param data_size theora data size
 * @return true: key frame false:inter frame
 */
bool ttLibC_Theora_isKey(void *data, size_t data_size) {
	uint8_t first_byte = *((uint8_t *)data);
	return (first_byte & 0xC0) != 0x40;
}

/*
 * analyze the width information from theora binary.
 * @param prev_frame ref for prev analyzed theora frame.
 * @param data       theora data
 * @param data_size  theora data size
 * @return 0:error or width size.
 */
uint32_t ttLibC_Theora_getWidth(ttLibC_Theora *prev_frame, uint8_t *data, size_t data_size) {
	/*
	 * 1bit header flag 1:header 0:frame
	 * 1bit intra flag 0:keyframe 1:inner frame.
	 * from here for identification header decode frame information.
	 * 6bit padding.
	 * 6byte(48bit) theora
	 * 8bit vmaj
	 * 8bit vmin
	 * 8bit vrev
	 * 16bit fmbW : x 16 for width
	 * 16bit fmvH : x 16 for height
	 *
	 *  32bit Nsbs ?
	 *  36bit nbs ?
	 *  32bit nmbs ?
	 *
	 * 24bit picW
	 * 24bit picH
	 * 8bit picX
	 * 8bit picY
	 */
	// check frame is identification header decode frame or not.
	if(prev_frame != NULL) {
		return prev_frame->inherit_super.width;
	}

	uint8_t *dat = (uint8_t *)data;
	if(data_size < 14) {
		ERR_PRINT("data is too small.");
		return 0;
	}
	if((*dat & 0x80) == 0) {
		return 0;
	}
	dat ++;
	// theora
	if((*dat != 't')
	|| (*(dat + 1) != 'h')
	|| (*(dat + 2) != 'e')
	|| (*(dat + 3) != 'o')
	|| (*(dat + 4) != 'r')
	|| (*(dat + 5) != 'a')) {
		ERR_PRINT("header string is invalid.");
		return 0;
	}
	dat += 6;
	dat += 3;
//	uint32_t fmbW = *((uint16_t *)dat);
	dat += 2;
//	uint32_t fmbH = *((uint16_t *)dat);
	dat += 2;
	uint32_t picW = *((uint32_t *)dat);
	return ((be_uint32_t(picW)) >> 8);
}

/*
 * analyze the height information from theora binary.
 * @param prev_frame ref for prev analyzed theora frame.
 * @param data       theora data
 * @param data_size  theora data size
 * @return 0:error or height size.
 */
uint32_t ttLibC_Theora_getHeight(ttLibC_Theora *prev_frame, uint8_t *data, size_t data_size) {
	if(prev_frame != NULL) {
		return prev_frame->inherit_super.height;
	}

	uint8_t *dat = (uint8_t *)data;
	if(data_size < 14) {
		ERR_PRINT("data is too small.");
		return 0;
	}
	if((*dat & 0x80) == 0) {
		return 0;
	}
	dat ++;
	// theora
	if((*dat       != 't')
	|| (*(dat + 1) != 'h')
	|| (*(dat + 2) != 'e')
	|| (*(dat + 3) != 'o')
	|| (*(dat + 4) != 'r')
	|| (*(dat + 5) != 'a')) {
		ERR_PRINT("header string is invalid.");
		return 0;
	}
	dat += 6;
	dat += 3;
//	uint32_t fmbW = *((uint16_t *)dat);
	dat += 2;
//	uint32_t fmbH = *((uint16_t *)dat);
	dat += 5;
	uint32_t picH = *((uint32_t *)dat);
	return ((be_uint32_t(picH)) >> 8);
}

/*
 * make frame object from theora binary data.
 * @param prev_frame    ref for prev analyzed theora frame.
 * @param data          theora data
 * @param data_size     theora data size
 * @param non_copy_mode true:hold pointer. false:copy data.
 * @param pts           pts for theora frame.
 * @param timebase      timebase for pts.
 * @return theora frame
 */
ttLibC_Theora *ttLibC_Theora_getFrame(
		ttLibC_Theora *prev_frame,
		uint8_t *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	uint8_t first_byte = *data;
	uint32_t width  = ttLibC_Theora_getWidth(prev_frame, data, data_size);
	uint32_t height = ttLibC_Theora_getHeight(prev_frame, data, data_size);
	ttLibC_Theora_Type type = TheoraType_innerFrame;
	if((first_byte & 0x80) != 0) {
		switch(first_byte) {
		case 0x80: // info
			type = TheoraType_identificationHeaderDecodeFrame;
			break;
		case 0x81: // comment
			type = TheoraType_commentHeaderFrame;
			break;
		case 0x82: // setup
			type = TheoraType_setupHeaderFrame;
			break;
		default:
			ERR_PRINT("unknown theora header frame.");
			return NULL;
		}
	}
	else {
		if((first_byte & 0x40) == 0) {
			type = TheoraType_intraFrame;
		}
		if(width == 0 || height == 0) {
			return NULL;
		}
	}
	return ttLibC_Theora_make(
			prev_frame,
			type,
			width,
			height,
			data,
			data_size,
			non_copy_mode,
			pts,
			timebase);
}

/*
 * close frame
 * @param frame
 */
void ttLibC_Theora_close(ttLibC_Theora **frame) {
	ttLibC_Video_close_((ttLibC_Video **)frame);
}
