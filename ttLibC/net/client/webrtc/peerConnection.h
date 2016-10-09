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
#include <webrtc/api/mediaconstraintsinterface.h>

using namespace webrtc;

namespace ttLibC_webrtc {

class FactoryWrapper;

/**
 * constraint object
 * ttLibC_WebrtcConstraint -> MediaConstraintsInterfaceの仲だちさせる
 */
class WebrtcConstraint : public MediaConstraintsInterface {
public:
	WebrtcConstraint() {};
	virtual ~WebrtcConstraint() {};

	virtual const Constraints& GetMandatory() const {
		return _mandatory;
	}
	virtual const Constraints& GetOptional() const {
		return _optional;
	}
	template<class T>
	void AddMandatory(const std::string &key, const T &value) {
		_mandatory.push_back(Constraint(key, rtc::ToString<T>(value)));
	}

	template<class T>
	void SetMandatory(const std::string &key, const T &value) {
		std::string value_str;
		if(_mandatory.FindFirst(key, &value_str)) {
			for(Constraints::iterator iter = _mandatory.begin();iter != _mandatory.end(); ++iter) {
				if(iter->key == key) {
					_mandatory.erase(iter);
					break;
				}
			}
		}
		_mandatory.push_back(Constraint(key, rtc::ToString<T>(value)));
	}

	template<class T>
	void AddOptional(const std::string &key, const T &value) {
		_optional.push_back(Constraint(key, rtc::ToString<T>(value)));
	}
	// ttLibC_WebrtcConstraintの内容で更新する。
	void update(ttLibC_WebrtcConstraint *constraint);
private:
	void analyze(Constraints& target, ttLibC_net_client_WebrtcConstraintData *data);
	template<class T>
	void add(Constraints& target, const std::string& key, const T &value) {
		target.push_back(Constraint(key, rtc::ToString<T>(value)));
	}
	Constraints _mandatory;
	Constraints _optional;
};

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
	rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> _nativePeerConnectionFactory;
	// イベントからcallback関数を呼ぶために構造体のデータを持っとく
	ttLibC_WebrtcPeerConnection *_cPeerConnection;
	// sdpの設定動作はobserberが必要らしい。とりあえず共通で使い回すものを準備しとく。
	rtc::scoped_refptr<SetSdpObserver> _setSdpObserver;
	// answerの場合sdpデータの設定をnativePeerConnection作成前にくることがある。
	// その場合にsdpのrefを保持しておく。本当はrefじゃなくて、copyにしたい。
	ttLibC_WebrtcSdp *_sdpCopy;
};

}

typedef struct ttLibC_net_client_WebrtcPeerConnection_ {
	ttLibC_WebrtcPeerConnection inherit_super;
	ttLibC_webrtc::PeerConnectionWrapper *peerConnectionWrapper;
} ttLibC_net_client_WebrtcPeerConnection_;
typedef ttLibC_net_client_WebrtcPeerConnection_ ttLibC_WebrtcPeerConnection_;

#endif /* TTLIBC_NET_CLIENT_WEBRTC_PEERCONNECTION_H_ */
