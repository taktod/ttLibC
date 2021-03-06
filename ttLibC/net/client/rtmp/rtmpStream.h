/**
 * @file   rtmpStream.h
 * @brief  detail definition of rtmpStream.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/03/12
 */

#ifndef TTLIBC_NET_CLIENT_RTMP_RTMPSTREAM_H_
#define TTLIBC_NET_CLIENT_RTMP_RTMPSTREAM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../rtmp.h"
#include "rtmpConnection.h"

#include "../../../util/flvFrameUtil.h"
#include "../../../container/misc.h"

typedef struct ttLibC_Net_Client_Rtmp_RtmpStream_ {
	ttLibC_RtmpStream inherit_super;

	ttLibC_RtmpConnection_ *conn;
	uint32_t stream_id;

	ttLibC_TettyPromise *promise;

	ttLibC_RtmpEventFunc event_callback;
	void *event_ptr;

	ttLibC_RtmpStream_getFrameFunc frame_callback;
	void *frame_ptr;

	uint64_t pts;
//	uint32_t timebase; // 1000

	ttLibC_FlvFrameManager *frame_manager;
	bool sent_dsi_info; // flag for sending dsi information.

	ttLibC_Frame_Type video_type;
	ttLibC_Frame_Type audio_type;
	ttLibC_FrameQueue *video_queue;
	ttLibC_FrameQueue *audio_queue;
} ttLibC_Net_Client_Rtmp_RtmpStream_;

typedef ttLibC_Net_Client_Rtmp_RtmpStream_ ttLibC_RtmpStream_;

#ifdef __cplusplus
}
#endif

#endif /* TTLIBC_NET_CLIENT_RTMP_RTMPSTREAM_H_ */
