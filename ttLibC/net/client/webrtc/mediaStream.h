/**
 * @file   mediaStream.h
 * @brief  mediaStream object for webrtc.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/10/08
 */

#ifndef TTLIBC_NET_CLIENT_WEBRTC_MEDIASTREAM_H_
#define TTLIBC_NET_CLIENT_WEBRTC_MEDIASTREAM_H_

#include "../webrtc.h"
#include "../../../frame/video/yuv420.h"
#include "../../../frame/audio/pcms16.h"
#include <webrtc/api/mediastreaminterface.h>
/*
MediaStream
映像はトラックにpushすればよさげ。
pushしたデータはそのままsignalイベントで拾われてシステムに流れる模様。
AudioSourceInterfaceにくっつけたSinkで設定できる気がする
やってみないとわからない。

取得に関してはTrackにAddSink(音声)とAddRenderer(映像)をくっつければOKとなりそう。
つまり自分で生成して、PeerConnectionにくっついているstreamについては、書き込み可能で
相手から受け取ってOnAddStreamで取得したstreamについては、参照するためのlistenerが設定可能ということか・・・
stream = ttLibC_WebrtcFactory_createLocalStream();
track = ttLibC_WebrtcMediaStream_addAudioTrack(stream, "name");
track = ttLibC_WebrtcMediaStream_addVideoTrack(stream, "name");
track = ttLibC_WebrtcMediaStream_getVideoTrack(stream, "name");
ttLibC_WebrtcMediaTrack_addFrame(track, yuvFrame); // みたいな感じでframeを追加することが可能になるということにしておこうと思う。
という形でnameを追加することが可能になって
getUserMediaでつくったtrackってidがついているだけで、自分で好きな名前を設定できるわけじゃないのか・・・
勝手に内部で作る方がよさげではあるけど・・・
入力してもらうことにするか・・・
stringデータのポインタが一定ではないので、stringで管理したとしても・・・ダメか・・・
stdMapをつかって管理すればいいか・・std::stringをキーにして大丈夫っぽい。
これでいけるかな・・・
videoにせよaudioにせよ、getTrackKeysというiteratorの動作がほしくなるな・・・
c言語的にはcallbackで帰ってくる的な感じで

とりあえずこれでいってみるか・・・

一時的にwrapするのって必要なのかな？
まぁ、あっていいか。
あ、でもremoveStreamするときに同じstreamオブジェクトが取得できるとうれしいか・・・
そういう意味では、相手からきたstreamでもfactoryで管理している方がお得といえなくもない。
peerからfactoryって復元できないか・・・
でもfactoryの実態は保持してるか・・・
なるほどなるほど
FactoryWrapperがもっていればそれで十分かな。

trackってやっぱり数値にしておくか・・・
いくつ目のtrackという感じで

addのところだけ面倒だな。そこだけなんとかしなきゃ。
uuidを使うようにしておこう。
 */

namespace ttLibC_webrtc {

class FactoryWrapper;
class MediaStreamWrapper;

// ここにsink用のlistenerクラスを定義しなければならない。
class VideoFrameListener : public rtc::VideoSinkInterface<cricket::VideoFrame> {
public:
	VideoFrameListener(
			MediaStreamWrapper *streamWrapper,
			uint32_t id) : _streamWrapper(streamWrapper), _id(id), _startPts(0) {};
	void OnFrame(const cricket::VideoFrame& frame);
private:
	MediaStreamWrapper *_streamWrapper;
	uint32_t _id;
	uint64_t _startPts;
};

class AudioFrameListener : public webrtc::AudioTrackSinkInterface {
public:
	AudioFrameListener(
			MediaStreamWrapper *streamWrapper,
			uint32_t id) : _streamWrapper(streamWrapper), _id(id), _pts(0) {};
	void OnData(
			const void *data,
			int bits_per_sample,
			int sample_rate,
			size_t number_of_channels,
			size_t number_of_frames);
private:
	MediaStreamWrapper *_streamWrapper;
	uint32_t _id;
	uint64_t _pts;
};

class MediaStreamWrapper {
	// こいつにlocalMediaStreamかremoteからきたMediaStreamInterfaceを持たせておく。
public:
	// local用
	MediaStreamWrapper(
			FactoryWrapper *factoryWrapper,
			const char *name);
	// remote用
	MediaStreamWrapper(
			FactoryWrapper *factoryWrapper,
			rtc::scoped_refptr<webrtc::MediaStreamInterface> nativeStream);
	rtc::scoped_refptr<webrtc::MediaStreamInterface> refNativeStream() {
		return _nativeStream;
	};
	~MediaStreamWrapper();
	// なんかtrackを取得することができないのは心配になってくるね。
	// とりあえず要件的にはこれで問題ないはず。
	bool createNewVideoTrack(ttLibC_WebrtcConstraint *constraint);
	bool createNewAudioTrack(ttLibC_WebrtcConstraint *constraint);
	bool addFrameListener(
			ttLibC_WebrtcMediaStreamFrameFunc callback,
			void *ptr); // これで全体で1つになる。やったね。
	bool addFrame(ttLibC_Frame *frame);

	// イベントの受け取り処理
	void OnFrame(
			const cricket::VideoFrame& frame,
			uint32_t id,
			uint64_t pts);
	void OnData(
			const void *data,
			int bits_per_sample,
			int sample_rate,
			size_t number_of_channels,
			size_t number_of_frames,
			uint32_t id,
			uint64_t pts);
private:
	bool _isLocal;
	FactoryWrapper *_factoryWrapper;
	rtc::scoped_refptr<webrtc::MediaStreamInterface> _nativeStream;
	ttLibC_WebrtcMediaStreamFrameFunc _callback;
	ttLibC_PcmS16 *_pcm_frame;
	ttLibC_Yuv420 *_yuv_frame;
	void *_ptr;
};

}

typedef struct ttLibC_net_client_WebrtcMediaStream_ {
	// ref_scopedptrなので、classを別につくらないとだめっぽい。
	ttLibC_WebrtcMediaStream inherit_super;
	ttLibC_webrtc::MediaStreamWrapper *streamWrapper;
} ttLibC_net_client_WebrtcMediaStream_;
typedef ttLibC_net_client_WebrtcMediaStream_ ttLibC_WebrtcMediaStream_;

#endif /* TTLIBC_NET_CLIENT_WEBRTC_MEDIASTREAM_H_ */
