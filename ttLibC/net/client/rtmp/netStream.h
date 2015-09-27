/**
 * @file   netStream.h
 * @brief  netStream functions.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/09/05
 */

#ifndef TTLIBC_NET_RTMP_NETSTREAM_H_
#define TTLIBC_NET_RTMP_NETSTREAM_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint32_t id; // 外部公開ID
	uint32_t stream_id; // 動作stream_id
	bool is_publish; // true:publish false:play
	char name[256]; // とりあえず固定でもたせておく。
} ttLibC_RtmpStreamDetail;

typedef struct {
	ttLibC_RtmpStream inherit_super;
	// id -> stream_idのmapが必要
	// id -> typeのmapも必要(publishであるか、playであるか)
} ttLibC_Net_RtmpStream_;

typedef ttLibC_Net_RtmpStream_ ttLibC_RtmpStream_;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TTLIBC_NET_RTMP_NETSTREAM_H_ */
