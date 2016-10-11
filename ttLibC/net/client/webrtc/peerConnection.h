/**
 * @file   peerConnection.h
 * @brief  support peerConnection.
 *
 * this code is under 3-Cause BSD License.
 *
 * @author taktod
 * @date   2016/10/08
 */

#ifndef TTLIBC_NET_CLIENT_WEBRTC_PEERCONNECTION_H_
#define TTLIBC_NET_CLIENT_WEBRTC_PEERCONNECTION_H_

#include "../webrtc.h"
#include <webrtc/api/peerconnectioninterface.h>

using namespace webrtc;

namespace ttLibC_webrtc {

class FactoryWrapper;

/**
 * sdpデータ生成時のobserver
 * createOfferやcreateAnswerで利用する。
 */
class CreateSdpObserver : public CreateSessionDescriptionObserver {
public:
	CreateSdpObserver(
			ttLibC_WebrtcEventFunc func,
			ttLibC_PeerConnection *cPeerConnection)
				 : _func(func),
				   _cPeerConnection(cPeerConnection){};
	void OnSuccess(SessionDescriptionInterface *sdp);
	void OnFailure(const std::string& msg);
private:
	ttLibC_WebrtcEventFunc _func;
	ttLibC_PeerConnection *_cPeerConnection;
};

/**
 * sdpデータ適応時のobserver
 * 別に必要ないが、これを設定しないとセット動作が死ぬので、とりあえず作った。
 * failureくらいなんかしてもよさそうだけど。
 */
class SetSdpObserver : public SetSessionDescriptionObserver {
public:
	void OnSuccess() {};
	void OnFailure(const std::string& msg) {};
};

/**
 * peerConnectionWrapperオブジェクト
 * このオブジェクトはrefptrで管理しなくて大丈夫っぽいので、構造体に直接くっつけることができる。
 */
class PeerConnectionWrapper : public PeerConnectionObserver {
public:
	PeerConnectionWrapper(
			FactoryWrapper *factoryWrapper,
			ttLibC_WebrtcConfig *config,
			ttLibC_WebrtcPeerConnection *cPeerConnection);
	virtual ~PeerConnectionWrapper();
	// メソッド
	bool createOffer(
			ttLibC_WebrtcEventFunc func,
			ttLibC_WebrtcConstraint *constraint);
	bool createAnswer(
			ttLibC_WebrtcEventFunc func,
			ttLibC_WebrtcConstraint *constraint);
	bool setLocalDescription(ttLibC_WebrtcSdp *sdp);
	bool setRemoteDescription(ttLibC_WebrtcSdp *sdp);
	bool addIceCandidate(ttLibC_WebrtcCandidate *candidate);
	bool addStream(ttLibC_WebrtcMediaStream *stream);
	// イベント
	void OnSignalingChange(PeerConnectionInterface::SignalingState new_state);
	void OnAddStream(rtc::scoped_refptr<MediaStreamInterface> stream);
	void OnRemoveStream(rtc::scoped_refptr<MediaStreamInterface> stream);
	void OnDataChannel(rtc::scoped_refptr<DataChannelInterface> data_channel);
	void OnRenegotiationNeeded();
	void OnIceConnectionChange(PeerConnectionInterface::IceConnectionState new_state);
	void OnIceGatheringChange(PeerConnectionInterface::IceGatheringState new_state);
	void OnIceCandidate(const IceCandidateInterface* candidate);
	void OnIceCandidatesRemoved(const std::vector<cricket::Candidate>& candidates);
	void OnIceConnectionReceivingChange(bool receiving);
protected:
private:
	// iceServerのconfigデータはあらかじめつくって保持しておく。
	PeerConnectionInterface::IceServers _iceServers;
	// nativePeerConnectionオブジェクト
	rtc::scoped_refptr<webrtc::PeerConnectionInterface> _nativePeerConnection;
	// nativePeerConnectionFactoryオブジェクト
	FactoryWrapper *_factoryWrapper;
	// イベントからcallback関数を呼ぶために構造体のデータを持っとく
	ttLibC_WebrtcPeerConnection *_cPeerConnection;
	// sdpの設定動作はobserberが必要らしい。とりあえず共通で使い回すものを準備しとく。
	rtc::scoped_refptr<SetSdpObserver> _setSdpObserver;
	// answerの場合sdpデータの設定をnativePeerConnection作成前にくることがある。
	std::string _sdp_type;
	std::string _sdp_value;
};

}

typedef struct ttLibC_net_client_WebrtcPeerConnection_ {
	ttLibC_WebrtcPeerConnection inherit_super;
	ttLibC_webrtc::PeerConnectionWrapper *peerConnectionWrapper;
} ttLibC_net_client_WebrtcPeerConnection_;
typedef ttLibC_net_client_WebrtcPeerConnection_ ttLibC_WebrtcPeerConnection_;

#endif /* TTLIBC_NET_CLIENT_WEBRTC_PEERCONNECTION_H_ */
