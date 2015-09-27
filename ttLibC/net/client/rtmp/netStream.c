/*
 * @file   netStream.c
 * @brief  netStream functions.
 *
 * this code is under 3-Cause BSD license.
 *
 * @author taktod
 * @date   2015/08/26
 */

#ifdef __ENABLE_FILE__
#include "../rtmp.h"
#include "netStream.h"
#include "../../../log.h"
#include "../../../allocator.h"

ttLibC_RtmpStream *ttLibC_RtmpStream_make(ttLibC_RtmpConnection *conn) {
	// rtmpStreamをつくって応答する。
	// rtmpStreamは内部でid -> streamIdのマップを保持している。
	// 該当idのデータがplayであるか、publishであるかの判定が必要になりそう。
	LOG_PRINT("rtmpStreamのmakeが呼ばれました。");
	ttLibC_RtmpStream_ *stream = ttLibC_malloc(sizeof(ttLibC_RtmpStream_));
	if(stream == NULL) {
		return NULL;
	}
	return (ttLibC_RtmpStream *)stream;
}

uint32_t ttLibC_RtmpStream_play(
		ttLibC_RtmpStream *stream,
		const char *name,
		ttLibC_RtmpFrameFunc callback,
		void *ptr) {
	LOG_PRINT("rtmpStreamのplayが呼ばれました。");
	return 0;
}

uint32_t ttLibC_RtmpStream_publish(
		ttLibC_RtmpStream *stream,
		const char *name) {
	if(stream == NULL) {
		return 0;
	}
	// publish動作を実施する。
	// createStreamを送る
	// resultを受け取って、該当用のstreamIdを知る必要がある。
	// publish命令を飛ばす。
	// 放送開始メッセージを受け取る。
	// あとはframeデータを送りつければOK
	// statusの応答先はnetConnectionととりあえず同じにしておく。
	LOG_PRINT("rtmpStreamのpublishが呼ばれました。");
	// RtmpStream_createStream(); // createstreamをつくって投げる。
	// NetConnectionのclientHandlerで応答を取得するので、そこから必要なデータをこっちに投げてもらう必要がある。
	// clientHandlerからstreamIdをもらうことができたら、続いてpublish命令を投げる。
	// この関数内で呼び出すわけではない
	// 最後にNetStream.Publish.Startを受け取っておわり。
	return 0;
}
bool ttLibC_RtmpStream_stop(ttLibC_RtmpStream *stream, uint32_t id) {
	LOG_PRINT("rtmpStreamのplayが呼ばれました。");
	return true;
}

void ttLibC_RtmpStream_feed(ttLibC_RtmpStream *stream, uint32_t id, ttLibC_Frame *frame) {

}

void ttLibC_RtmpStream_close(ttLibC_RtmpStream **stream) {
	LOG_PRINT("rtmpStreamのcloseが呼ばれました。");
	ttLibC_RtmpStream_ *target = (ttLibC_RtmpStream_ *)*stream;
	if(target == NULL) {
		return;
	}
	ttLibC_free(target);
	*stream = NULL;
}

#endif



