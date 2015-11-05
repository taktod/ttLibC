/*
 * @file   jpeg.c
 * @brief  jpeg image frame information
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/11/05
 */

#include "jpeg.h"
#include "../../log.h"

typedef ttLibC_Frame_Video_Jpeg ttLibC_Jpeg_;

/*
 * make jpeg frame
 * @param prev_frame    reuse frame
 * @param width         width
 * @param height        height
 * @param data          jpeg data
 * @param data_size     jpeg data size
 * @param non_copy_mode true:hold the data pointer. false:data will copy.
 * @param pts           pts for jpeg data.
 * @param timebase      timebase number for pts
 */
ttLibC_Jpeg *ttLibC_Jpeg_make(
		ttLibC_Jpeg *prev_frame,
		uint32_t width,
		uint32_t height,
		void *data,
		size_t data_size,
		bool non_copy_mode,
		uint64_t pts,
		uint32_t timebase) {
	return (ttLibC_Jpeg *)ttLibC_Video_make(
			(ttLibC_Video *)prev_frame,
			sizeof(ttLibC_Jpeg_),
			frameType_jpeg,
			videoType_key,
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
void ttLibC_Jpeg_close(ttLibC_Jpeg **frame) {
	ttLibC_Video_close_((ttLibC_Video **)frame);
}
